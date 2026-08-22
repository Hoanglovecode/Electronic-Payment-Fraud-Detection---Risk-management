#ifndef EPFD_FRAUD_CONCRETE_FRAUD_RULES_HPP
#define EPFD_FRAUD_CONCRETE_FRAUD_RULES_HPP

#include <memory>
#include "epfd/fraud/IFraudRule.hpp"
#include "epfd/dsa/FastLookupIndex.hpp"

namespace epfd {

// ==========================================
// 1. LargeAmountRule
// ==========================================
class LargeAmountRule : public BaseFraudRule {
public:
    using BaseFraudRule::evaluate;
    explicit LargeAmountRule(double threshold = 5000.0, double weight = 35.0);
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;

private:
    double threshold_;
};

// ==========================================
// 2. HighVelocityRule
// ==========================================
class HighVelocityRule : public BaseFraudRule {
public:
    using BaseFraudRule::evaluate;
    explicit HighVelocityRule(size_t threshold_5m = 3, size_t threshold_1h = 8, double weight = 50.0);
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;

private:
    size_t threshold_5m_;
    size_t threshold_1h_;
};

// ==========================================
// 3. NewDeviceRule
// ==========================================
class NewDeviceRule : public BaseFraudRule {
public:
    using BaseFraudRule::evaluate;
    explicit NewDeviceRule(double weight = 25.0);
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;
};

// ==========================================
// 4. ImpossibleTravelRule (New Location & Speed)
// ==========================================
class ImpossibleTravelRule : public BaseFraudRule {
public:
    using BaseFraudRule::evaluate;
    explicit ImpossibleTravelRule(double max_speed_kmh = 800.0, double weight = 70.0);
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;

private:
    double max_speed_kmh_;
};

// ==========================================
// 5. ForeignCountryRule
// ==========================================
class ForeignCountryRule : public BaseFraudRule {
public:
    using BaseFraudRule::evaluate;
    explicit ForeignCountryRule(double weight = 30.0);
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;
};

// ==========================================
// 6. UnusualTimeRule
// ==========================================
class UnusualTimeRule : public BaseFraudRule {
public:
    using BaseFraudRule::evaluate;
    explicit UnusualTimeRule(double start_hour = 1.0, double end_hour = 5.0, double weight = 15.0);
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;

private:
    double start_hour_;
    double end_hour_;
};

// ==========================================
// 7. SuspiciousMerchantRule
// ==========================================
class SuspiciousMerchantRule : public BaseFraudRule {
public:
    using BaseFraudRule::evaluate;
    explicit SuspiciousMerchantRule(double risk_rating_threshold = 0.70, double weight = 45.0);
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;

private:
    double risk_rating_threshold_;
};

// ==========================================
// 8. BehaviorDeviationRule
// ==========================================
class BehaviorDeviationRule : public BaseFraudRule {
public:
    using BaseFraudRule::evaluate;
    explicit BehaviorDeviationRule(double deviation_threshold = 3.0, double weight = 40.0);
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;

private:
    double deviation_threshold_;
};

// ==========================================
// 9. CardTestingRule
// ==========================================
class CardTestingRule : public BaseFraudRule {
public:
    using BaseFraudRule::evaluate;
    explicit CardTestingRule(double micro_amount_limit = 5.0, double weight = 60.0);
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;

private:
    double micro_amount_limit_;
};

// ==========================================
// 10. AccountTakeoverSignalRule
// ==========================================
class AccountTakeoverSignalRule : public BaseFraudRule {
public:
    using BaseFraudRule::evaluate;
    explicit AccountTakeoverSignalRule(double weight = 75.0);
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;
};

// ==========================================
// 11. BlacklistRule
// ==========================================
class BlacklistRule : public BaseFraudRule {
public:
    using BaseFraudRule::evaluate;
    explicit BlacklistRule(std::shared_ptr<FastLookupIndex> lookup_index, double weight = 100.0);
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;

private:
    std::shared_ptr<FastLookupIndex> lookup_index_;
};

// ==========================================
// 12. WhitelistRule
// ==========================================
class WhitelistRule : public BaseFraudRule {
public:
    using BaseFraudRule::evaluate;
    explicit WhitelistRule(std::shared_ptr<FastLookupIndex> lookup_index, double weight = 0.0);
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;

private:
    std::shared_ptr<FastLookupIndex> lookup_index_;
};

} // namespace epfd

#endif // EPFD_FRAUD_CONCRETE_FRAUD_RULES_HPP
