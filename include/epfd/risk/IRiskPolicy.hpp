#ifndef EPFD_RISK_I_RISK_POLICY_HPP
#define EPFD_RISK_I_RISK_POLICY_HPP

#include <vector>
#include <memory>
#include "epfd/models/Transaction.hpp"
#include "epfd/models/RiskAssessment.hpp"
#include "epfd/risk/IRiskRule.hpp"

namespace epfd {

/**
 * @brief Strategy interface for risk aggregation and explainable scoring.
 */
class IRiskPolicy {
public:
    virtual ~IRiskPolicy() = default;

    virtual const std::string& getName() const noexcept = 0;
    
    /**
     * @brief Computes combined risk score from rule alerts and ML probability.
     * @param rule_alerts List of triggered fraud alerts from rule engine.
     * @param ml_probability Predicted fraud probability from ML model [0.0, 1.0].
     * @return Final combined risk score [0.0, 100.0].
     */
    virtual double calculateCombinedScore(const std::vector<FraudAlert>& rule_alerts, double ml_probability) const = 0;

    /**
     * @brief Maps numerical risk score [0, 100] to categorical RiskLevel.
     */
    virtual RiskLevel mapToRiskLevel(double risk_score) const = 0;
};

} // namespace epfd

#endif // EPFD_RISK_I_RISK_POLICY_HPP
