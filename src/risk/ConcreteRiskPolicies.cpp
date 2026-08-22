#include "epfd/risk/ConcreteRiskPolicies.hpp"
#include <algorithm>

namespace epfd {

// ==========================================
// 1. StandardWeightedRiskPolicy
// ==========================================

double StandardWeightedRiskPolicy::calculateCombinedScore(const std::vector<FraudAlert>& rule_alerts, double ml_probability) const {
    double rule_sum = 0.0;
    bool has_critical = false;
    for (const auto& alert : rule_alerts) {
        rule_sum += alert.getScoreContribution();
        if (alert.getSeverity() == RiskLevel::CRITICAL) {
            has_critical = true;
        }
    }
    double rule_score = std::min(100.0, rule_sum);
    double ml_score = std::clamp(ml_probability * 100.0, 0.0, 100.0);

    double combined = (rule_score * rule_weight_) + (ml_score * ml_weight_);
    if (has_critical) {
        combined = std::max(combined, 85.0);
    }
    return std::clamp(combined, 0.0, 100.0);
}

RiskLevel StandardWeightedRiskPolicy::mapToRiskLevel(double risk_score) const {
    if (risk_score < 20.0) return RiskLevel::VERY_LOW;
    if (risk_score < 40.0) return RiskLevel::LOW;
    if (risk_score < 60.0) return RiskLevel::MEDIUM;
    if (risk_score < 80.0) return RiskLevel::HIGH;
    return RiskLevel::CRITICAL;
}

// ==========================================
// 2. ConservativeRiskPolicy
// ==========================================

double ConservativeRiskPolicy::calculateCombinedScore(const std::vector<FraudAlert>& rule_alerts, double ml_probability) const {
    double rule_sum = 0.0;
    for (const auto& alert : rule_alerts) {
        rule_sum += alert.getScoreContribution();
    }
    double rule_score = std::min(100.0, rule_sum);
    double ml_score = std::clamp(ml_probability * 100.0, 0.0, 100.0);

    double combined = (rule_score * rule_weight_) + (ml_score * ml_weight_);
    return std::clamp(combined, 0.0, 100.0);
}

RiskLevel ConservativeRiskPolicy::mapToRiskLevel(double risk_score) const {
    if (risk_score < 15.0) return RiskLevel::VERY_LOW;
    if (risk_score < 30.0) return RiskLevel::LOW;
    if (risk_score < 50.0) return RiskLevel::MEDIUM;
    if (risk_score < 70.0) return RiskLevel::HIGH;
    return RiskLevel::CRITICAL;
}

// ==========================================
// 3. CriticalOverrideRiskPolicy
// ==========================================

double CriticalOverrideRiskPolicy::calculateCombinedScore(const std::vector<FraudAlert>& rule_alerts, double ml_probability) const {
    bool has_critical = false;
    double rule_sum = 0.0;
    for (const auto& alert : rule_alerts) {
        rule_sum += alert.getScoreContribution();
        if (alert.getSeverity() == RiskLevel::CRITICAL) {
            has_critical = true;
        }
    }
    if (has_critical || ml_probability >= 0.90) {
        return 100.0;
    }
    double rule_score = std::min(100.0, rule_sum);
    double ml_score = std::clamp(ml_probability * 100.0, 0.0, 100.0);
    return std::clamp((rule_score * 0.5) + (ml_score * 0.5), 0.0, 100.0);
}

RiskLevel CriticalOverrideRiskPolicy::mapToRiskLevel(double risk_score) const {
    if (risk_score < 20.0) return RiskLevel::VERY_LOW;
    if (risk_score < 40.0) return RiskLevel::LOW;
    if (risk_score < 60.0) return RiskLevel::MEDIUM;
    if (risk_score < 80.0) return RiskLevel::HIGH;
    return RiskLevel::CRITICAL;
}

} // namespace epfd
