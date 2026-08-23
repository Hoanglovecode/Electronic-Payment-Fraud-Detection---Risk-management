#ifndef EPFD_FRAUD_ADVANCED_FRAUD_RULES_HPP
#define EPFD_FRAUD_ADVANCED_FRAUD_RULES_HPP

#include "epfd/fraud/IFraudRule.hpp"
#include "epfd/dsa/FastLookupIndex.hpp"
#include <memory>

namespace epfd {

/**
 * @brief 13. MuleAccountSmurfingRule
 * Detects anti-money laundering (AML) structuring / smurfing patterns just below reporting limits ($8,000 - $9,999).
 */
class MuleAccountSmurfingRule : public BaseFraudRule {
public:
    explicit MuleAccountSmurfingRule(double lower_threshold = 8000.0,
                                     double upper_threshold = 9999.0,
                                     double weight = 65.0);

    using BaseFraudRule::evaluate;
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;

private:
    double lower_threshold_;
    double upper_threshold_;
};

/**
 * @brief 14. DormantAccountReactivationRule
 * Detects sudden high-value transaction on an account with zero activity for > 90 days.
 */
class DormantAccountReactivationRule : public BaseFraudRule {
public:
    explicit DormantAccountReactivationRule(double min_reactivation_amount = 1000.0,
                                            double weight = 55.0);

    using BaseFraudRule::evaluate;
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;

private:
    double min_reactivation_amount_;
};

/**
 * @brief 15. DynamicListMatchingRule
 * Advanced list matching with entity scoping, regex-like matching, and dynamic weight.
 */
class DynamicListMatchingRule : public BaseFraudRule {
public:
    explicit DynamicListMatchingRule(std::shared_ptr<FastLookupIndex> lookup_index,
                                     double weight = 100.0);

    using BaseFraudRule::evaluate;
    std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const override;

private:
    std::shared_ptr<FastLookupIndex> lookup_index_;
};

} // namespace epfd

#endif // EPFD_FRAUD_ADVANCED_FRAUD_RULES_HPP
