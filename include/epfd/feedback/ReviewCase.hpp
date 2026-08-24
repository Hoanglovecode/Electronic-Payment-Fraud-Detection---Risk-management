#ifndef EPFD_FEEDBACK_REVIEW_CASE_HPP
#define EPFD_FEEDBACK_REVIEW_CASE_HPP

#include <string>
#include <vector>
#include <chrono>
#include "epfd/common/Types.hpp"
#include "epfd/decision/IDecisionPolicy.hpp"

namespace epfd {

/**
 * @brief Represents a fraud investigation case assigned to risk analysts.
 */
class ReviewCase {
public:
    ReviewCase() = default;
    ReviewCase(std::string case_id,
               std::string transaction_id,
               std::string customer_id,
               double initial_risk_score,
               DecisionAction original_decision,
               std::vector<std::string> initial_evidence = {});

    // Getters
    const std::string& getCaseId() const noexcept { return case_id_; }
    const std::string& getTransactionId() const noexcept { return transaction_id_; }
    const std::string& getCustomerId() const noexcept { return customer_id_; }
    double getInitialRiskScore() const noexcept { return initial_risk_score_; }
    DecisionAction getOriginalDecision() const noexcept { return original_decision_; }
    CaseStatus getStatus() const noexcept { return status_; }
    const std::string& getAssignedAnalystId() const noexcept { return assigned_analyst_id_; }
    const std::vector<std::string>& getEvidence() const noexcept { return evidence_list_; }
    const std::vector<std::string>& getNotes() const noexcept { return notes_; }
    Timestamp getCreatedAt() const noexcept { return created_at_; }
    Timestamp getUpdatedAt() const noexcept { return updated_at_; }
    Timestamp getResolvedAt() const noexcept { return resolved_at_; }
    const std::string& getResolutionSummary() const noexcept { return resolution_summary_; }

    // Workflow Actions
    bool assignTo(const std::string& analyst_id);
    void addEvidence(const std::string& evidence);
    void addNote(const std::string& note);
    bool resolve(CaseStatus resolution, const std::string& summary);
    bool close();

    bool isResolved() const noexcept;
    std::string toString() const;

private:
    std::string case_id_;
    std::string transaction_id_;
    std::string customer_id_;
    double initial_risk_score_{0.0};
    DecisionAction original_decision_{DecisionAction::REVIEW};
    CaseStatus status_{CaseStatus::OPEN};
    std::string assigned_analyst_id_;
    std::vector<std::string> evidence_list_;
    std::vector<std::string> notes_;
    Timestamp created_at_{std::chrono::system_clock::now()};
    Timestamp updated_at_{std::chrono::system_clock::now()};
    Timestamp resolved_at_{};
    std::string resolution_summary_;
};

} // namespace epfd

#endif // EPFD_FEEDBACK_REVIEW_CASE_HPP
