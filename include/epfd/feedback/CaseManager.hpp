#ifndef EPFD_FEEDBACK_CASE_MANAGER_HPP
#define EPFD_FEEDBACK_CASE_MANAGER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <optional>
#include "epfd/feedback/ReviewCase.hpp"
#include "epfd/feedback/LabelStore.hpp"
#include "epfd/feedback/OutcomeTracker.hpp"

namespace epfd {

/**
 * @brief Master Case Management & Feedback Orchestrator.
 * Connects Analyst Investigation workflows with Ground Truth LabelStore and OutcomeTracker.
 */
class CaseManager {
public:
    explicit CaseManager(std::shared_ptr<LabelStore> label_store = nullptr,
                         std::shared_ptr<OutcomeTracker> outcome_tracker = nullptr);

    std::string createCase(const std::string& transaction_id,
                           const std::string& customer_id,
                           double initial_risk_score,
                           DecisionAction original_decision,
                           const std::vector<std::string>& initial_evidence = {});

    bool assignCase(const std::string& case_id, const std::string& analyst_id);
    bool addEvidence(const std::string& case_id, const std::string& evidence);
    bool addNote(const std::string& case_id, const std::string& note);

    bool resolveCase(const std::string& case_id,
                     CaseStatus resolution,
                     const std::string& summary,
                     const TransactionFeatures& features = TransactionFeatures{});

    std::optional<ReviewCase> getCase(const std::string& case_id) const;
    std::optional<ReviewCase> findByTransactionId(const std::string& tx_id) const;
    std::vector<ReviewCase> findByStatus(CaseStatus status) const;
    std::vector<ReviewCase> findByAnalyst(const std::string& analyst_id) const;
    std::vector<ReviewCase> getAllCases() const;

    size_t getOpenCaseCount() const;
    size_t getResolvedCaseCount() const;
    size_t totalCases() const;

    std::shared_ptr<LabelStore> getLabelStore() const { return label_store_; }
    std::shared_ptr<OutcomeTracker> getOutcomeTracker() const { return outcome_tracker_; }

    void clear();

private:
    mutable std::mutex mutex_;
    std::shared_ptr<LabelStore> label_store_;
    std::shared_ptr<OutcomeTracker> outcome_tracker_;
    std::unordered_map<std::string, ReviewCase> cases_by_id_;
    std::unordered_map<std::string, std::string> case_id_by_tx_id_;
    size_t case_counter_{1};
};

} // namespace epfd

#endif // EPFD_FEEDBACK_CASE_MANAGER_HPP
