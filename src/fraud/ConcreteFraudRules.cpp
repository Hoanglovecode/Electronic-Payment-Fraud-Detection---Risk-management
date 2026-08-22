#include "epfd/fraud/ConcreteFraudRules.hpp"

namespace epfd {

// 1. LargeAmountRule
LargeAmountRule::LargeAmountRule(double threshold, double weight)
    : BaseFraudRule("RULE_LARGE_AMOUNT", "Large Transaction Amount Anomaly", FraudRuleCategory::AMOUNT_DEVIATION, weight),
      threshold_(threshold) {}

std::optional<FraudAlert> LargeAmountRule::evaluate(const Transaction& tx, const TransactionFeatures&) const {
    if (!enabled_) return std::nullopt;

    if (tx.getAmount() >= threshold_) {
        RiskLevel severity = tx.getAmount() >= (threshold_ * 2.0) ? RiskLevel::HIGH : RiskLevel::MEDIUM;
        std::string alert_id = "ALT_LARGE_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction amount $" + std::to_string(tx.getAmount()) + " exceeds threshold $" + std::to_string(threshold_),
                          severity);
    }
    return std::nullopt;
}

// 2. HighVelocityRule
HighVelocityRule::HighVelocityRule(size_t threshold_5m, size_t threshold_1h, double weight)
    : BaseFraudRule("RULE_HIGH_VELOCITY", "High Transaction Frequency (Velocity Burst)", FraudRuleCategory::VELOCITY, weight),
      threshold_5m_(threshold_5m), threshold_1h_(threshold_1h) {}

std::optional<FraudAlert> HighVelocityRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return std::nullopt;

    if (features.transactions_last_5min >= threshold_5m_ || features.transactions_last_1hour >= threshold_1h_) {
        std::string alert_id = "ALT_VELOCITY_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Velocity burst: " + std::to_string(static_cast<int>(features.transactions_last_5min)) + " txs in last 5m (threshold: " +
                          std::to_string(threshold_5m_) + "), " + std::to_string(static_cast<int>(features.transactions_last_1hour)) + " in last 1h",
                          RiskLevel::HIGH);
    }
    return std::nullopt;
}

// 3. NewDeviceRule
NewDeviceRule::NewDeviceRule(double weight)
    : BaseFraudRule("RULE_NEW_DEVICE", "Unrecognized Device Fingerprint", FraudRuleCategory::DEVICE_INTEGRITY, weight) {}

std::optional<FraudAlert> NewDeviceRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return std::nullopt;

    if (features.is_new_device > 0.5) {
        std::string alert_id = "ALT_NEW_DEV_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction performed from an unrecognized device: " + tx.getDevice().getDeviceFingerprint(),
                          RiskLevel::LOW);
    }
    return std::nullopt;
}

// 4. ImpossibleTravelRule
ImpossibleTravelRule::ImpossibleTravelRule(double max_speed_kmh, double weight)
    : BaseFraudRule("RULE_IMPOSSIBLE_TRAVEL", "Physically Impossible Travel Velocity", FraudRuleCategory::GEO_LOCATION, weight),
      max_speed_kmh_(max_speed_kmh) {}

std::optional<FraudAlert> ImpossibleTravelRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return std::nullopt;

    if (features.speed_from_last_tx_kmh >= max_speed_kmh_) {
        std::string alert_id = "ALT_IMP_TRAVEL_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Calculated travel speed between consecutive transactions is " +
                          std::to_string(static_cast<int>(features.speed_from_last_tx_kmh)) + " km/h (threshold: " +
                          std::to_string(static_cast<int>(max_speed_kmh_)) + " km/h)",
                          RiskLevel::CRITICAL);
    }
    return std::nullopt;
}

// 5. ForeignCountryRule
ForeignCountryRule::ForeignCountryRule(double weight)
    : BaseFraudRule("RULE_FOREIGN_COUNTRY", "Cross-Border Foreign Transaction", FraudRuleCategory::GEO_LOCATION, weight) {}

std::optional<FraudAlert> ForeignCountryRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return std::nullopt;

    if (features.is_new_country > 0.5) {
        std::string alert_id = "ALT_FOREIGN_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction executed in a foreign country outside home jurisdiction: " + tx.getLocation().getCountry(),
                          RiskLevel::MEDIUM);
    }
    return std::nullopt;
}

// 6. UnusualTimeRule
UnusualTimeRule::UnusualTimeRule(double start_hour, double end_hour, double weight)
    : BaseFraudRule("RULE_UNUSUAL_TIME", "Off-Hours Transaction (Night Activity)", FraudRuleCategory::BEHAVIORAL, weight),
      start_hour_(start_hour), end_hour_(end_hour) {}

std::optional<FraudAlert> UnusualTimeRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return std::nullopt;

    if (features.hour_of_day >= start_hour_ && features.hour_of_day <= end_hour_) {
        std::string alert_id = "ALT_TIME_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction occurred during unusual nocturnal hours (" + std::to_string(static_cast<int>(features.hour_of_day)) + ":00)",
                          RiskLevel::LOW);
    }
    return std::nullopt;
}

// 7. SuspiciousMerchantRule
SuspiciousMerchantRule::SuspiciousMerchantRule(double risk_rating_threshold, double weight)
    : BaseFraudRule("RULE_SUSPICIOUS_MERCHANT", "High-Risk Merchant / MCC Activity", FraudRuleCategory::BEHAVIORAL, weight),
      risk_rating_threshold_(risk_rating_threshold) {}

std::optional<FraudAlert> SuspiciousMerchantRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return std::nullopt;

    if (features.is_high_risk_mcc > 0.5 || features.merchant_risk_rating >= risk_rating_threshold_) {
        std::string alert_id = "ALT_MERCHANT_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction placed at high risk merchant category (MCC) or rating: " + std::to_string(features.merchant_risk_rating),
                          RiskLevel::HIGH);
    }
    return std::nullopt;
}

// 8. BehaviorDeviationRule
BehaviorDeviationRule::BehaviorDeviationRule(double deviation_threshold, double weight)
    : BaseFraudRule("RULE_BEHAVIOR_DEVIATION", "Abnormal Amount Deviation vs Historical Profile", FraudRuleCategory::AMOUNT_DEVIATION, weight),
      deviation_threshold_(deviation_threshold) {}

std::optional<FraudAlert> BehaviorDeviationRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return std::nullopt;

    if (features.transactions_last_24hours > 0 && features.amount_deviation_ratio >= deviation_threshold_) {
        std::string alert_id = "ALT_DEV_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction amount is " + std::to_string(features.amount_deviation_ratio) + "x customer's 24h historical average",
                          RiskLevel::MEDIUM);
    }
    return std::nullopt;
}

// 9. CardTestingRule
CardTestingRule::CardTestingRule(double micro_amount_limit, double weight)
    : BaseFraudRule("RULE_CARD_TESTING", "Card Testing Micro-Probing Pattern", FraudRuleCategory::CARD_TESTING, weight),
      micro_amount_limit_(micro_amount_limit) {}

std::optional<FraudAlert> CardTestingRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return std::nullopt;

    bool is_micro = tx.getAmount() <= micro_amount_limit_;
    bool is_rapid = features.transactions_last_5min >= 2.0;
    bool is_multi_account_device = features.accounts_on_device_count >= 3.0;

    if ((is_micro && is_rapid) || is_multi_account_device) {
        std::string alert_id = "ALT_CARD_TEST_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Card testing probe detected (Micro amount or device shared by " +
                          std::to_string(static_cast<int>(features.accounts_on_device_count)) + " distinct accounts)",
                          RiskLevel::HIGH);
    }
    return std::nullopt;
}

// 10. AccountTakeoverSignalRule
AccountTakeoverSignalRule::AccountTakeoverSignalRule(double weight)
    : BaseFraudRule("RULE_ATO_SIGNAL", "Account Takeover (ATO) Pattern", FraudRuleCategory::ACCOUNT_TAKEOVER, weight) {}

std::optional<FraudAlert> AccountTakeoverSignalRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return std::nullopt;

    bool is_new_dev = features.is_new_device > 0.5;
    bool is_extreme_amount = features.amount_deviation_ratio >= 2.5 || tx.getAmount() >= 2000.0;
    bool is_high_risk_env = features.is_high_risk_device > 0.5;

    if (is_new_dev && (is_extreme_amount || is_high_risk_env)) {
        std::string alert_id = "ALT_ATO_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "High confidence Account Takeover signal (New device combined with amount anomaly or emulator)",
                          RiskLevel::CRITICAL);
    }
    return std::nullopt;
}

// 11. BlacklistRule
BlacklistRule::BlacklistRule(std::shared_ptr<FastLookupIndex> lookup_index, double weight)
    : BaseFraudRule("RULE_BLACKLIST", "Global Threat Intelligence Blacklist Match", FraudRuleCategory::LIST_MATCHING, weight),
      lookup_index_(std::move(lookup_index)) {}

std::optional<FraudAlert> BlacklistRule::evaluate(const Transaction& tx, const TransactionFeatures&) const {
    if (!enabled_ || !lookup_index_) return std::nullopt;

    if (lookup_index_->isBlacklisted("IP:" + tx.getIpAddress()) ||
        lookup_index_->isBlacklisted("DEV:" + tx.getDevice().getDeviceFingerprint()) ||
        lookup_index_->isBlacklisted("CARD:" + tx.getPaymentMethod().getCardBin()) ||
        lookup_index_->isBlacklisted("MERCHANT:" + tx.getMerchantId())) {
        
        std::string alert_id = "ALT_BLACKLIST_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Entity found in global threat blacklist",
                          RiskLevel::CRITICAL);
    }
    return std::nullopt;
}

// 12. WhitelistRule
WhitelistRule::WhitelistRule(std::shared_ptr<FastLookupIndex> lookup_index, double weight)
    : BaseFraudRule("RULE_WHITELIST", "Trusted Whitelist Entity Match", FraudRuleCategory::LIST_MATCHING, weight),
      lookup_index_(std::move(lookup_index)) {}

std::optional<FraudAlert> WhitelistRule::evaluate(const Transaction& tx, const TransactionFeatures&) const {
    if (!enabled_ || !lookup_index_) return std::nullopt;

    if (lookup_index_->isWhitelisted("MERCHANT:" + tx.getMerchantId()) ||
        lookup_index_->isWhitelisted("CUST:" + tx.getCustomerId())) {
        
        std::string alert_id = "ALT_WHITELIST_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Transaction matched trusted whitelist policy",
                          RiskLevel::LOW);
    }
    return std::nullopt;
}

} // namespace epfd
