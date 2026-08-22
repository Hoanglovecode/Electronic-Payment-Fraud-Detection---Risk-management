#include "epfd/features/FeatureExtractor.hpp"
#include "epfd/database/ICustomerRepository.hpp"
#include "epfd/database/ITransactionRepository.hpp"
#include <ctime>
#include <cmath>

namespace epfd {

FeatureExtractor::FeatureExtractor(std::shared_ptr<CustomerVelocityTracker> velocity_tracker,
                                   std::shared_ptr<FastLookupIndex> lookup_index,
                                   std::shared_ptr<ICustomerRepository> customer_repo,
                                   std::shared_ptr<ITransactionRepository> tx_repo)
    : velocity_tracker_(std::move(velocity_tracker)),
      lookup_index_(std::move(lookup_index)),
      customer_repo_(std::move(customer_repo)),
      tx_repo_(std::move(tx_repo)) {
    if (!velocity_tracker_) {
        velocity_tracker_ = std::make_shared<CustomerVelocityTracker>();
    }
    if (!lookup_index_) {
        lookup_index_ = std::make_shared<FastLookupIndex>();
    }
}

TransactionFeatures FeatureExtractor::extract(const Transaction& tx) const {
    std::lock_guard<std::mutex> lock(mutex_);
    TransactionFeatures f;

    // 1. Transaction Core
    f.transaction_amount = tx.getAmount();

    std::time_t t = std::chrono::system_clock::to_time_t(tx.getTimestamp());
    std::tm tm_buf{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    f.hour_of_day = static_cast<double>(tm_buf.tm_hour);
    f.is_weekend = (tm_buf.tm_wday == 0 || tm_buf.tm_wday == 6) ? 1.0 : 0.0;

    // 2. Velocity Features (Strictly BEFORE current transaction)
    if (velocity_tracker_) {
        auto v = velocity_tracker_->getMetrics(tx.getCustomerId(), tx.getTimestamp());
        f.transactions_last_5min = static_cast<double>(v.count_5m);
        f.amount_sum_last_5min = v.sum_5m;
        f.transactions_last_1hour = static_cast<double>(v.count_1h);
        f.amount_sum_last_1hour = v.sum_1h;
        f.transactions_last_24hours = static_cast<double>(v.count_24h);
        f.amount_sum_last_24hours = v.sum_24h;
        f.average_amount_24h = v.average_amount_24h;

        if (v.average_amount_24h > 0.0) {
            f.amount_deviation_ratio = tx.getAmount() / v.average_amount_24h;
        } else {
            f.amount_deviation_ratio = 1.0;
        }
    }

    // 3. Device & Network Features
    f.is_high_risk_device = tx.getDevice().isHighRiskEnvironment() ? 1.0 : 0.0;
    if (lookup_index_) {
        size_t count = lookup_index_->getAccountCountOnDevice(tx.getDevice().getDeviceFingerprint());
        f.accounts_on_device_count = static_cast<double>(count > 0 ? count : 1);
    }

    // 4. Customer & Geographic Features
    if (customer_repo_) {
        auto cust_opt = customer_repo_->findById(tx.getCustomerId());
        if (cust_opt.has_value()) {
            const auto& cust = cust_opt.value();
            f.customer_historical_risk_score = cust.getRiskScore();
            f.is_new_device = cust.isKnownDevice(tx.getDevice().getDeviceId()) ? 0.0 : 1.0;
            f.is_new_country = tx.isCrossBorder(cust.getHomeLocation().getCountry()) ? 1.0 : 0.0;
            f.distance_from_home_km = cust.getHomeLocation().distanceKmTo(tx.getLocation());
        }
    }

    // 5. Speed Calculation from Last Known Transaction
    auto it_last = last_transaction_by_customer_.find(tx.getCustomerId());
    if (it_last != last_transaction_by_customer_.end()) {
        const auto& prev_tx = it_last->second;
        double dist_km = prev_tx.getLocation().distanceKmTo(tx.getLocation());
        auto duration_sec = std::chrono::duration_cast<std::chrono::seconds>(tx.getTimestamp() - prev_tx.getTimestamp()).count();
        if (duration_sec > 0) {
            double duration_hours = static_cast<double>(duration_sec) / 3600.0;
            f.speed_from_last_tx_kmh = dist_km / duration_hours;
        }
    }

    // 6. Merchant Features
    auto it_m = merchant_registry_.find(tx.getMerchantId());
    if (it_m != merchant_registry_.end()) {
        f.is_high_risk_mcc = it_m->second.isHighRiskMCC() ? 1.0 : 0.0;
        f.merchant_risk_rating = it_m->second.getRiskRating();
    }

    return f;
}

void FeatureExtractor::updateState(const Transaction& tx) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (velocity_tracker_) {
        velocity_tracker_->recordTransaction(tx.getCustomerId(), tx.getTimestamp(), tx.getAmount());
    }
    if (lookup_index_) {
        lookup_index_->recordDeviceUsage(tx.getDevice().getDeviceFingerprint(), tx.getCustomerId());
        lookup_index_->recordIpUsage(tx.getIpAddress(), tx.getCustomerId());
        lookup_index_->recordCardUsage(tx.getPaymentMethod().getCardBin(), tx.getCustomerId());
    }
    last_transaction_by_customer_[tx.getCustomerId()] = tx;
}

void FeatureExtractor::registerMerchant(Merchant merchant) {
    std::lock_guard<std::mutex> lock(mutex_);
    merchant_registry_.insert(merchant.getMerchantId(), std::move(merchant));
}

void FeatureExtractor::setLastTransaction(const std::string& customer_id, const Transaction& tx) {
    std::lock_guard<std::mutex> lock(mutex_);
    last_transaction_by_customer_[customer_id] = tx;
}

void FeatureExtractor::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (velocity_tracker_) velocity_tracker_->clear();
    if (lookup_index_) lookup_index_->clear();
    merchant_registry_.clear();
    last_transaction_by_customer_.clear();
}

} // namespace epfd
