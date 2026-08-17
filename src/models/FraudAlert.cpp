#include "epfd/models/FraudAlert.hpp"
#include <sstream>

namespace epfd {

FraudAlert::FraudAlert(std::string alert_id,
                       std::string transaction_id,
                       std::string rule_id,
                       std::string rule_name,
                       FraudRuleCategory category,
                       double score_contribution,
                       std::string reason,
                       RiskLevel severity,
                       Timestamp triggered_at)
    : alert_id_(std::move(alert_id)),
      transaction_id_(std::move(transaction_id)),
      rule_id_(std::move(rule_id)),
      rule_name_(std::move(rule_name)),
      category_(category),
      score_contribution_(score_contribution),
      reason_(std::move(reason)),
      severity_(severity),
      triggered_at_(triggered_at) {}

std::string FraudAlert::toString() const {
    std::ostringstream oss;
    oss << "FraudAlert[id=" << alert_id_
        << ", tx=" << transaction_id_
        << ", rule=" << rule_name_ << " (" << rule_id_ << ")"
        << ", score=" << score_contribution_
        << ", severity=" << epfd::toString(severity_)
        << ", reason=\"" << reason_ << "\""
        << "]";
    return oss.str();
}

} // namespace epfd
