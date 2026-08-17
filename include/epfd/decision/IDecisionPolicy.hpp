#ifndef EPFD_DECISION_I_DECISION_POLICY_HPP
#define EPFD_DECISION_I_DECISION_POLICY_HPP

#include <string>
#include <vector>
#include "epfd/common/Types.hpp"
#include "epfd/models/FraudAlert.hpp"

namespace epfd {

struct DecisionResult {
    DecisionAction action{DecisionAction::APPROVE};
    std::string policy_applied;
    std::string rationale;
};

/**
 * @brief Strategy interface for Decision Engine policies (OCP & SRP).
 * Separates Risk Scoring from Business Action determination.
 */
class IDecisionPolicy {
public:
    virtual ~IDecisionPolicy() = default;

    virtual const std::string& getName() const noexcept = 0;
    virtual DecisionResult decide(double risk_score, RiskLevel risk_level, const std::vector<FraudAlert>& alerts) const = 0;
};

} // namespace epfd

#endif // EPFD_DECISION_I_DECISION_POLICY_HPP
