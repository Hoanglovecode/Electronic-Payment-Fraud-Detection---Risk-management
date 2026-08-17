#include "epfd/models/RiskAssessment.hpp"
#include <sstream>

namespace epfd {

RiskAssessment::RiskAssessment(std::string assessment_id,
                               std::string transaction_id,
                               double rule_score,
                               double ml_score,
                               double combined_score,
                               RiskLevel risk_level,
                               DecisionAction decision,
                               std::vector<FraudAlert> alerts,
                               std::vector<std::string> reasons,
                               Timestamp evaluated_at)
    : assessment_id_(std::move(assessment_id)),
      transaction_id_(std::move(transaction_id)),
      rule_score_(rule_score),
      ml_score_(ml_score),
      combined_score_(combined_score),
      risk_level_(risk_level),
      decision_(decision),
      alerts_(std::move(alerts)),
      reasons_(std::move(reasons)),
      evaluated_at_(evaluated_at) {}

void RiskAssessment::addAlert(FraudAlert alert) {
    alerts_.push_back(std::move(alert));
}

void RiskAssessment::addReason(std::string reason) {
    reasons_.push_back(std::move(reason));
}

std::string RiskAssessment::toString() const {
    std::ostringstream oss;
    oss << "RiskAssessment[id=" << assessment_id_
        << ", tx=" << transaction_id_
        << ", ruleScore=" << rule_score_
        << ", mlScore=" << ml_score_
        << ", combined=" << combined_score_
        << ", level=" << epfd::toString(risk_level_)
        << ", decision=" << epfd::toString(decision_)
        << ", alertsCount=" << alerts_.size()
        << "]";
    return oss.str();
}

} // namespace epfd
