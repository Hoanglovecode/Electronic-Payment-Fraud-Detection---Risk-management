#ifndef EPFD_RISK_CONCRETE_RISK_POLICIES_HPP
#define EPFD_RISK_CONCRETE_RISK_POLICIES_HPP

#include "epfd/risk/IRiskPolicy.hpp"

namespace epfd {

/**
 * @brief Standard industry-grade weighted risk policy.
 * Baseline Levels: 0-20 VERY_LOW, 21-40 LOW, 41-60 MEDIUM, 61-80 HIGH, 81-100 CRITICAL.
 */
class StandardWeightedRiskPolicy : public IRiskPolicy {
public:
    explicit StandardWeightedRiskPolicy(double rule_weight = 0.5, double ml_weight = 0.5)
        : rule_weight_(rule_weight), ml_weight_(ml_weight) {}

    const std::string& getName() const noexcept override {
        static const std::string name = "StandardWeightedRiskPolicy";
        return name;
    }

    double calculateCombinedScore(const std::vector<FraudAlert>& rule_alerts, double ml_probability) const override;
    RiskLevel mapToRiskLevel(double risk_score) const override;

private:
    double rule_weight_;
    double ml_weight_;
};

/**
 * @brief Conservative risk policy with lower tolerance thresholds.
 */
class ConservativeRiskPolicy : public IRiskPolicy {
public:
    explicit ConservativeRiskPolicy(double rule_weight = 0.45, double ml_weight = 0.55)
        : rule_weight_(rule_weight), ml_weight_(ml_weight) {}

    const std::string& getName() const noexcept override {
        static const std::string name = "ConservativeRiskPolicy";
        return name;
    }

    double calculateCombinedScore(const std::vector<FraudAlert>& rule_alerts, double ml_probability) const override;
    RiskLevel mapToRiskLevel(double risk_score) const override;

private:
    double rule_weight_;
    double ml_weight_;
};

/**
 * @brief Critical override risk policy: overrides combined score immediately when critical alerts trigger.
 */
class CriticalOverrideRiskPolicy : public IRiskPolicy {
public:
    explicit CriticalOverrideRiskPolicy(double override_floor = 85.0)
        : override_floor_(override_floor) {}

    const std::string& getName() const noexcept override {
        static const std::string name = "CriticalOverrideRiskPolicy";
        return name;
    }

    double calculateCombinedScore(const std::vector<FraudAlert>& rule_alerts, double ml_probability) const override;
    RiskLevel mapToRiskLevel(double risk_score) const override;

private:
    double override_floor_;
};

} // namespace epfd

#endif // EPFD_RISK_CONCRETE_RISK_POLICIES_HPP
