#include "epfd/fraud/AdvancedFraudRules.hpp"

namespace epfd {

// ==========================================
// 13. MuleAccountSmurfingRule
// ==========================================
MuleAccountSmurfingRule::MuleAccountSmurfingRule(double lower_threshold, double upper_threshold, double weight)
    : BaseFraudRule("RULE_MULE_SMURFING", "AML Structuring / Smurfing Pattern", FraudRuleCategory::AMOUNT_DEVIATION, weight),
      lower_threshold_(lower_threshold),
      upper_threshold_(upper_threshold) {}

std::optional<FraudAlert> MuleAccountSmurfingRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return std::nullopt;

    // Detect amounts just below regulatory CTR reporting limit ($10,000)
    bool is_smurfing_amount = (tx.getAmount() >= lower_threshold_ && tx.getAmount() <= upper_threshold_);
    bool is_high_frequency = (features.transactions_last_24hours >= 2.0 || features.transactions_last_1hour >= 1.0);

    if (is_smurfing_amount && is_high_frequency) {
        std::string alert_id = "ALT_SMURF_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "AML Structuring alert: Amount $" + std::to_string(static_cast<int>(tx.getAmount())) +
                          " falls within structuring band [$" + std::to_string(static_cast<int>(lower_threshold_)) +
                          ", $" + std::to_string(static_cast<int>(upper_threshold_)) + "]",
                          RiskLevel::HIGH);
    }
    return std::nullopt;
}

// ==========================================
// 14. DormantAccountReactivationRule
// ==========================================
DormantAccountReactivationRule::DormantAccountReactivationRule(double min_reactivation_amount, double weight)
    : BaseFraudRule("RULE_DORMANT_REACTIVATION", "Sudden High-Value Dormant Account Activity", FraudRuleCategory::BEHAVIORAL, weight),
      min_reactivation_amount_(min_reactivation_amount) {}

std::optional<FraudAlert> DormantAccountReactivationRule::evaluate(const Transaction& tx, const TransactionFeatures& features) const {
    if (!enabled_) return std::nullopt;

    // If 0 transactions in last 24h and amount is large on first activity
    bool is_dormant = (features.transactions_last_24hours == 0.0 && features.amount_deviation_ratio >= 3.0);
    bool is_large = (tx.getAmount() >= min_reactivation_amount_);

    if (is_dormant && is_large) {
        std::string alert_id = "ALT_DORMANT_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Dormant account reactivation: $" + std::to_string(static_cast<int>(tx.getAmount())) +
                          " transaction after prolonged inactivity",
                          RiskLevel::MEDIUM);
    }
    return std::nullopt;
}

// ==========================================
// 15. DynamicListMatchingRule
// ==========================================
DynamicListMatchingRule::DynamicListMatchingRule(std::shared_ptr<FastLookupIndex> lookup_index, double weight)
    : BaseFraudRule("RULE_DYNAMIC_LIST", "Dynamic Entity Intelligence Matching", FraudRuleCategory::LIST_MATCHING, weight),
      lookup_index_(std::move(lookup_index)) {}

std::optional<FraudAlert> DynamicListMatchingRule::evaluate(const Transaction& tx, const TransactionFeatures&) const {
    if (!enabled_ || !lookup_index_) return std::nullopt;

    if (lookup_index_->isBlacklisted(tx.getIpAddress()) ||
        lookup_index_->isBlacklisted(tx.getDevice().getDeviceFingerprint()) ||
        lookup_index_->isBlacklisted(tx.getPaymentMethod().getCardBin()) ||
        lookup_index_->isBlacklisted(tx.getMerchantId()) ||
        lookup_index_->isBlacklisted(tx.getCustomerId())) {
        
        std::string alert_id = "ALT_DYN_BLACK_" + tx.getTransactionId();
        return FraudAlert(alert_id, tx.getTransactionId(), id_, name_, category_,
                          weight_,
                          "Dynamic blacklist match across IP, Device, BIN, or Merchant",
                          RiskLevel::CRITICAL);
    }
    return std::nullopt;
}

} // namespace epfd
