/*
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System
 * Module: Decision Engine & Policy Abstraction (Strategy Pattern)
 * Team Members: Hoang, Khiem, Triet (OOP Project)
 *
 * OOP Design Notes:
 * - Strategy Pattern: IDecisionPolicy định nghĩa chiến lược ra quyết định (APPROVE, CHALLENGE_3DS, REVIEW, BLOCK).
 *   Hệ thống có thể linh hoạt chuyển đổi giữa StandardPolicy, StrictPolicy hoặc VipPolicy tại runtime.
 * - DecisionAction Enum: Đại diện cho máy trạng thái quyết định của nghiệp vụ ngân hàng.
 */

#ifndef EPFD_DECISION_I_DECISION_POLICY_HPP
#define EPFD_DECISION_I_DECISION_POLICY_HPP

#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include "epfd/common/Types.hpp"
#include "epfd/models/FraudAlert.hpp"
#include "epfd/models/RiskAssessment.hpp"

namespace epfd {

struct DecisionResult {
    std::string decision_id;
    std::string transaction_id;
    DecisionAction action{DecisionAction::APPROVE};
    double risk_score{0.0};
    RiskLevel risk_level{RiskLevel::VERY_LOW};
    std::string policy_applied;
    std::string applied_thresholds;
    std::vector<std::string> top_contributors;
    std::string rationale;
    bool requires_step_up_auth{false};
    bool requires_manual_review{false};
    std::string blocked_reason;
    Timestamp evaluated_at{std::chrono::system_clock::now()};

    DecisionResult() = default;
    DecisionResult(DecisionAction act, std::string policy, std::string rat)
        : action(act), policy_applied(std::move(policy)), rationale(std::move(rat)) {
        requires_step_up_auth = (action == DecisionAction::CHALLENGE_3DS);
        requires_manual_review = (action == DecisionAction::REVIEW);
    }

    bool isApproved() const noexcept { return action == DecisionAction::APPROVE; }
    bool isChallenged() const noexcept { return action == DecisionAction::CHALLENGE_3DS; }
    bool isReviewed() const noexcept { return action == DecisionAction::REVIEW; }
    bool isBlocked() const noexcept { return action == DecisionAction::BLOCK; }

    std::string toString() const {
        std::ostringstream oss;
        oss << "[Decision: " << (action == DecisionAction::APPROVE ? "APPROVE" :
                                 action == DecisionAction::CHALLENGE_3DS ? "CHALLENGE_3DS" :
                                 action == DecisionAction::REVIEW ? "REVIEW" : "BLOCK")
            << "] Score: " << risk_score << " (" << epfd::toString(risk_level) << ") | Policy: "
            << policy_applied << " | Rationale: " << rationale;
        return oss.str();
    }
};

/**
 * @brief Strategy interface for Decision Engine policies (OCP & SRP).
 * Converts RiskAssessment into an operational business action.
 */
class IDecisionPolicy {
public:
    virtual ~IDecisionPolicy() = default;

    virtual const std::string& getName() const noexcept = 0;
    
    virtual DecisionResult decide(double risk_score, 
                                  RiskLevel risk_level, 
                                  const std::vector<FraudAlert>& alerts) const = 0;

    virtual DecisionResult decide(const RiskAssessment& assessment) const {
        auto res = decide(assessment.getCombinedScore(), assessment.getRiskLevel(), assessment.getAlerts());
        res.transaction_id = assessment.getTransactionId();
        res.decision_id = "DEC_" + assessment.getTransactionId();
        res.risk_score = assessment.getCombinedScore();
        res.risk_level = assessment.getRiskLevel();
        res.top_contributors = assessment.getReasons();
        return res;
    }
};

} // namespace epfd

#endif // EPFD_DECISION_I_DECISION_POLICY_HPP
