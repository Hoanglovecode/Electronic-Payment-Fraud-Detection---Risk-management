#ifndef EPFD_MODELS_RISK_ASSESSMENT_HPP
#define EPFD_MODELS_RISK_ASSESSMENT_HPP

#include <string>
#include <vector>
#include <chrono>
#include "epfd/common/Types.hpp"
#include "epfd/models/FraudAlert.hpp"

namespace epfd {

class RiskAssessment {
public:
    RiskAssessment() = default;
    RiskAssessment(std::string assessment_id,
                   std::string transaction_id,
                   double rule_score,
                   double ml_score,
                   double combined_score,
                   RiskLevel risk_level,
                   DecisionAction decision,
                   std::vector<FraudAlert> alerts = {},
                   std::vector<std::string> reasons = {},
                   Timestamp evaluated_at = std::chrono::system_clock::now());

    // Getters
    const std::string& getAssessmentId() const noexcept { return assessment_id_; }
    const std::string& getTransactionId() const noexcept { return transaction_id_; }
    double getRuleScore() const noexcept { return rule_score_; }
    double getMlScore() const noexcept { return ml_score_; }
    double getCombinedScore() const noexcept { return combined_score_; }
    RiskLevel getRiskLevel() const noexcept { return risk_level_; }
    DecisionAction getDecision() const noexcept { return decision_; }
    const std::vector<FraudAlert>& getAlerts() const noexcept { return alerts_; }
    const std::vector<std::string>& getReasons() const noexcept { return reasons_; }
    Timestamp getEvaluatedAt() const noexcept { return evaluated_at_; }

    // Domain methods
    void addAlert(FraudAlert alert);
    void addReason(std::string reason);
    bool isApproved() const noexcept { return decision_ == DecisionAction::APPROVE; }
    bool isReviewed() const noexcept { return decision_ == DecisionAction::REVIEW; }
    bool isChallenged() const noexcept { return decision_ == DecisionAction::CHALLENGE_3DS; }
    bool isBlocked() const noexcept { return decision_ == DecisionAction::BLOCK; }

    std::string toString() const;

private:
    std::string assessment_id_;
    std::string transaction_id_;
    double rule_score_{0.0};
    double ml_score_{0.0};
    double combined_score_{0.0};
    RiskLevel risk_level_{RiskLevel::VERY_LOW};
    DecisionAction decision_{DecisionAction::APPROVE};
    std::vector<FraudAlert> alerts_;
    std::vector<std::string> reasons_;
    Timestamp evaluated_at_{std::chrono::system_clock::now()};
};

} // namespace epfd

#endif // EPFD_MODELS_RISK_ASSESSMENT_HPP
