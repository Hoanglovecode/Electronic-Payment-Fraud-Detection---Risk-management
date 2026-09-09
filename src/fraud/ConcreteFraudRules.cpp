/*
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System
 * Module: Concrete Fraud Detection Rules Implementation
 * Team Members: Hoang, Khiem, Triet (OOP Project)
 */

#include "epfd/fraud/ConcreteFraudRules.hpp"
#include <string>

using namespace std;

namespace epfd {

// 1. LargeAmountRule - Phát hiện giao dịch đột biến số tiền
LargeAmountRule::LargeAmountRule(double threshold, double weight)
    : BaseFraudRule("RULE_LARGE_AMOUNT", "Large Transaction Amount Anomaly", FraudRuleCategory::AMOUNT_DEVIATION, weight),
      threshold_(threshold) {}

optional<FraudAlert> LargeAmountRule::evaluate(const Transaction& tx, const TransactionFeatures&) const {
    if (!enabled_) return nullopt;

    if (tx.getAmount() >= threshold_) {
        RiskLevel severity = tx.getAmount() >= (threshold_ * 2.0) ? RiskLevel::HIGH : RiskLevel::MEDIUM;
        string alert_id = "ALT_LARGE_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction amount $" + to_string(tx.getAmount()) + " exceeds threshold $" + to_string(threshold_),
                          severity);
    }
    return nullopt;
}

// 2. HighVelocityRule - Phát hiện tần suất giao dịch dày đặc trong thời gian ngắn
HighVelocityRule::HighVelocityRule(size_t threshold_5m, size_t threshold_1h, double weight)
    : BaseFraudRule("RULE_HIGH_VELOCITY", "High Transaction Frequency (Velocity Burst)", FraudRuleCategory::VELOCITY, weight),
      threshold_5m_(threshold_5m), threshold_1h_(threshold_1h) {}

optional<FraudAlert> HighVelocityRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return nullopt;

    if (features.transactions_last_5min >= threshold_5m_ || features.transactions_last_1hour >= threshold_1h_) {
        string alert_id = "ALT_VELOCITY_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Velocity burst: " + to_string(static_cast<int>(features.transactions_last_5min)) + " txs in last 5m (threshold: " +
                          to_string(threshold_5m_) + "), " + to_string(static_cast<int>(features.transactions_last_1hour)) + " in last 1h",
                          RiskLevel::HIGH);
    }
    return nullopt;
}

// 3. NewDeviceRule - Cảnh báo thiết bị lạ lần đầu phát sinh giao dịch
NewDeviceRule::NewDeviceRule(double weight)
    : BaseFraudRule("RULE_NEW_DEVICE", "Unrecognized Device Fingerprint", FraudRuleCategory::DEVICE_INTEGRITY, weight) {}

optional<FraudAlert> NewDeviceRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return nullopt;

    if (features.is_new_device > 0.5) {
        string alert_id = "ALT_NEW_DEV_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction performed from an unrecognized device: " + tx.getDevice().getDeviceFingerprint(),
                          RiskLevel::LOW);
    }
    return nullopt;
}

// 4. ImpossibleTravelRule - Di chuyển với vận tốc phi thực tế giữa hai địa điểm
ImpossibleTravelRule::ImpossibleTravelRule(double max_speed_kmh, double weight)
    : BaseFraudRule("RULE_IMPOSSIBLE_TRAVEL", "Physically Impossible Travel Velocity", FraudRuleCategory::GEO_LOCATION, weight),
      max_speed_kmh_(max_speed_kmh) {}

optional<FraudAlert> ImpossibleTravelRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return nullopt;

    // Triet: Ngưỡng 800 km/h tương đương tốc độ máy bay thương mại; vượt quá là dấu hiệu chia sẻ tài khoản hoặc botnet
    if (features.speed_from_last_tx_kmh >= max_speed_kmh_) {
        string alert_id = "ALT_IMP_TRAVEL_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Calculated travel speed between consecutive transactions is " +
                          to_string(static_cast<int>(features.speed_from_last_tx_kmh)) + " km/h (threshold: " +
                          to_string(static_cast<int>(max_speed_kmh_)) + " km/h)",
                          RiskLevel::CRITICAL);
    }
    return nullopt;
}

// 5. ForeignCountryRule - Giao dịch xuyên biên giới ngoài quốc gia cư trú
ForeignCountryRule::ForeignCountryRule(double weight)
    : BaseFraudRule("RULE_FOREIGN_COUNTRY", "Cross-Border Foreign Transaction", FraudRuleCategory::GEO_LOCATION, weight) {}

optional<FraudAlert> ForeignCountryRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return nullopt;

    if (features.is_new_country > 0.5) {
        string alert_id = "ALT_FOREIGN_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction executed in a foreign country outside home jurisdiction: " + tx.getLocation().getCountry(),
                          RiskLevel::MEDIUM);
    }
    return nullopt;
}

// 6. UnusualTimeRule - Giao dịch vào khung giờ đêm bất thường
UnusualTimeRule::UnusualTimeRule(double start_hour, double end_hour, double weight)
    : BaseFraudRule("RULE_UNUSUAL_TIME", "Off-Hours Transaction (Night Activity)", FraudRuleCategory::BEHAVIORAL, weight),
      start_hour_(start_hour), end_hour_(end_hour) {}

optional<FraudAlert> UnusualTimeRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return nullopt;

    if (features.hour_of_day >= start_hour_ && features.hour_of_day <= end_hour_) {
        string alert_id = "ALT_TIME_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction occurred during unusual nocturnal hours (" + to_string(static_cast<int>(features.hour_of_day)) + ":00)",
                          RiskLevel::LOW);
    }
    return nullopt;
}

// 7. SuspiciousMerchantRule - Giao dịch ở danh mục đối tác rủi ro cao (Cờ bạc, Crypto...)
SuspiciousMerchantRule::SuspiciousMerchantRule(double risk_rating_threshold, double weight)
    : BaseFraudRule("RULE_SUSPICIOUS_MERCHANT", "High-Risk Merchant / MCC Activity", FraudRuleCategory::BEHAVIORAL, weight),
      risk_rating_threshold_(risk_rating_threshold) {}

optional<FraudAlert> SuspiciousMerchantRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return nullopt;

    if (features.is_high_risk_mcc > 0.5 || features.merchant_risk_rating >= risk_rating_threshold_) {
        string alert_id = "ALT_MERCHANT_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction placed at high risk merchant category (MCC) or rating: " + to_string(features.merchant_risk_rating),
                          RiskLevel::HIGH);
    }
    return nullopt;
}

// 8. BehaviorDeviationRule - Lệch quá lớn so với mức chi tiêu trung bình lịch sử
BehaviorDeviationRule::BehaviorDeviationRule(double deviation_threshold, double weight)
    : BaseFraudRule("RULE_BEHAVIOR_DEVIATION", "Abnormal Amount Deviation vs Historical Profile", FraudRuleCategory::AMOUNT_DEVIATION, weight),
      deviation_threshold_(deviation_threshold) {}

optional<FraudAlert> BehaviorDeviationRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return nullopt;

    // Khiem: Chỉ đánh giá độ lệch khi khách hàng đã có ít nhất 1 giao dịch lịch sử trong 24h
    if (features.transactions_last_24hours > 0 && features.amount_deviation_ratio >= deviation_threshold_) {
        string alert_id = "ALT_DEV_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction amount is " + to_string(features.amount_deviation_ratio) + "x customer's 24h historical average",
                          RiskLevel::MEDIUM);
    }
    return nullopt;
}

// 9. CardTestingRule - Kỹ thuật dò thẻ micro-auth ($1.00)
CardTestingRule::CardTestingRule(double micro_amount_limit, double weight)
    : BaseFraudRule("RULE_CARD_TESTING", "Card Testing Micro-Probing Pattern", FraudRuleCategory::CARD_TESTING, weight),
      micro_amount_limit_(micro_amount_limit) {}

optional<FraudAlert> CardTestingRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return nullopt;

    bool is_micro = tx.getAmount() <= micro_amount_limit_;
    bool is_rapid = features.transactions_last_5min >= 2.0;
    bool is_multi_account_device = features.accounts_on_device_count >= 3.0;

    if ((is_micro && is_rapid) || is_multi_account_device) {
        string alert_id = "ALT_CARD_TEST_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Card testing probe detected (Micro amount or device shared by " +
                          to_string(static_cast<int>(features.accounts_on_device_count)) + " distinct accounts)",
                          RiskLevel::HIGH);
    }
    return nullopt;
}

// 10. AccountTakeoverSignalRule - Dấu hiệu chiếm đoạt tài khoản (Thiết bị mới + Đổi số tiền cực lớn)
AccountTakeoverSignalRule::AccountTakeoverSignalRule(double weight)
    : BaseFraudRule("RULE_ATO_SIGNAL", "Account Takeover (ATO) Pattern", FraudRuleCategory::ACCOUNT_TAKEOVER, weight) {}

optional<FraudAlert> AccountTakeoverSignalRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return nullopt;

    bool is_new_dev = features.is_new_device > 0.5;
    bool is_extreme_amount = features.amount_deviation_ratio >= 2.5 || tx.getAmount() >= 2000.0;
    bool is_high_risk_env = features.is_high_risk_device > 0.5;

    if (is_new_dev && (is_extreme_amount || is_high_risk_env)) {
        string alert_id = "ALT_ATO_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "High confidence Account Takeover signal (New device combined with amount anomaly or emulator)",
                          RiskLevel::CRITICAL);
    }
    return nullopt;
}

// 11. BlacklistRule - Đối chiếu danh sách đen toàn cầu (O(1) qua Custom HashMap)
BlacklistRule::BlacklistRule(shared_ptr<FastLookupIndex> lookup_index, double weight)
    : BaseFraudRule("RULE_BLACKLIST", "Global Threat Intelligence Blacklist Match", FraudRuleCategory::LIST_MATCHING, weight),
      lookup_index_(move(lookup_index)) {}

optional<FraudAlert> BlacklistRule::evaluate(const Transaction& tx, const TransactionFeatures&) const {
    if (!enabled_ || !lookup_index_) return nullopt;

    if (lookup_index_->isBlacklisted(tx.getIpAddress()) ||
        lookup_index_->isBlacklisted("IP:" + tx.getIpAddress()) ||
        lookup_index_->isBlacklisted(tx.getDevice().getDeviceFingerprint()) ||
        lookup_index_->isBlacklisted("DEV:" + tx.getDevice().getDeviceFingerprint()) ||
        lookup_index_->isBlacklisted(tx.getPaymentMethod().getCardBin()) ||
        lookup_index_->isBlacklisted("CARD:" + tx.getPaymentMethod().getCardBin()) ||
        lookup_index_->isBlacklisted(tx.getMerchantId()) ||
        lookup_index_->isBlacklisted("MERCHANT:" + tx.getMerchantId()) ||
        lookup_index_->isBlacklisted(tx.getCustomerId()) ||
        lookup_index_->isBlacklisted("CUST:" + tx.getCustomerId())) {
        
        string alert_id = "ALT_BLACKLIST_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Entity found in global threat blacklist",
                          RiskLevel::CRITICAL);
    }
    return nullopt;
}

// 12. WhitelistRule - Bỏ qua cảnh báo cho thực thể uy tín
WhitelistRule::WhitelistRule(shared_ptr<FastLookupIndex> lookup_index, double weight)
    : BaseFraudRule("RULE_WHITELIST", "Trusted Whitelist Entity Match", FraudRuleCategory::LIST_MATCHING, weight),
      lookup_index_(move(lookup_index)) {}

optional<FraudAlert> WhitelistRule::evaluate(const Transaction& tx, const TransactionFeatures&) const {
    if (!enabled_ || !lookup_index_) return nullopt;

    if (lookup_index_->isWhitelisted(tx.getMerchantId()) ||
        lookup_index_->isWhitelisted("MERCHANT:" + tx.getMerchantId()) ||
        lookup_index_->isWhitelisted(tx.getCustomerId()) ||
        lookup_index_->isWhitelisted("CUST:" + tx.getCustomerId())) {
        
        string alert_id = "ALT_WHITELIST_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction matched trusted whitelist policy",
                          RiskLevel::LOW);
    }
    return nullopt;
}

} // namespace epfd
