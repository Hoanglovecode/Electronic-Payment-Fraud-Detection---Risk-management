#ifndef EPFD_FEATURES_FEATURE_EXTRACTOR_HPP
#define EPFD_FEATURES_FEATURE_EXTRACTOR_HPP

#include <memory>
#include <mutex>
#include "epfd/models/Transaction.hpp"
#include "epfd/models/Customer.hpp"
#include "epfd/models/Merchant.hpp"
#include "epfd/features/TransactionFeatures.hpp"
#include "epfd/dsa/CustomerVelocityTracker.hpp"
#include "epfd/dsa/FastLookupIndex.hpp"
#include "epfd/dsa/HashMap.hpp"

namespace epfd {

class ICustomerRepository;
class ITransactionRepository;

/**
 * @brief Real-time Feature Extraction Engine (SRP & Leakage Prevention).
 * Guarantees zero future/target data leakage by strictly calculating signals
 * from state prior to transaction ingestion.
 */
class FeatureExtractor {
public:
    FeatureExtractor(std::shared_ptr<CustomerVelocityTracker> velocity_tracker = nullptr,
                     std::shared_ptr<FastLookupIndex> lookup_index = nullptr,
                     std::shared_ptr<ICustomerRepository> customer_repo = nullptr,
                     std::shared_ptr<ITransactionRepository> tx_repo = nullptr);

    /**
     * @brief Extracts real-time features for the given transaction without state mutation.
     * Guaranteed NO LEAKAGE: State is NOT modified during extract().
     */
    TransactionFeatures extract(const Transaction& tx) const;

    /**
     * @brief Updates internal velocity and association state AFTER transaction evaluation.
     */
    void updateState(const Transaction& tx);

    // Merchant metadata registry
    void registerMerchant(Merchant merchant);

    // Last known transaction cache for speed/distance calculations
    void setLastTransaction(const std::string& customer_id, const Transaction& tx);

    void clear();

private:
    std::shared_ptr<CustomerVelocityTracker> velocity_tracker_;
    std::shared_ptr<FastLookupIndex> lookup_index_;
    std::shared_ptr<ICustomerRepository> customer_repo_;
    std::shared_ptr<ITransactionRepository> tx_repo_;

    mutable dsa::HashMap<std::string, Merchant> merchant_registry_;
    mutable dsa::HashMap<std::string, Transaction> last_transaction_by_customer_;
    mutable std::mutex mutex_;
};

} // namespace epfd

#endif // EPFD_FEATURES_FEATURE_EXTRACTOR_HPP
