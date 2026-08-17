#ifndef EPFD_MODELS_FRAUD_ALERT_HPP
#define EPFD_MODELS_FRAUD_ALERT_HPP

#include <string>
#include <chrono>
#include "epfd/common/Types.hpp"

namespace epfd {

class FraudAlert {
public:
    FraudAlert() = default;
    FraudAlert(std::string alert_id,
               std::string transaction_id,
               std::string rule_id,
               std::string rule_name,
               FraudRuleCategory category,
               double score_contribution,
               std::string reason,
               RiskLevel severity = RiskLevel::MEDIUM,
               Timestamp triggered_at = std::chrono::system_clock::now());

    // Getters
    const std::string& getAlertId() const noexcept { return alert_id_; }
    const std::string& getTransactionId() const noexcept { return transaction_id_; }
    const std::string& getRuleId() const noexcept { return rule_id_; }
    const std::string& getRuleName() const noexcept { return rule_name_; }
    FraudRuleCategory getCategory() const noexcept { return category_; }
    double getScoreContribution() const noexcept { return score_contribution_; }
    const std::string& getReason() const noexcept { return reason_; }
    RiskLevel getSeverity() const noexcept { return severity_; }
    Timestamp getTriggeredAt() const noexcept { return triggered_at_; }

    std::string toString() const;

private:
    std::string alert_id_;
    std::string transaction_id_;
    std::string rule_id_;
    std::string rule_name_;
    FraudRuleCategory category_{FraudRuleCategory::VELOCITY};
    double score_contribution_{0.0};
    std::string reason_;
    RiskLevel severity_{RiskLevel::MEDIUM};
    Timestamp triggered_at_{std::chrono::system_clock::now()};
};

} // namespace epfd

#endif // EPFD_MODELS_FRAUD_ALERT_HPP
