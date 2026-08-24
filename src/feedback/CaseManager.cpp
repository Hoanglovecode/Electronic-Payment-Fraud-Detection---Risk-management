#include "epfd/feedback/CaseManager.hpp"
#include <iomanip>
#include <sstream>

namespace epfd {

CaseManager::CaseManager(std::shared_ptr<LabelStore> label_store,
                         std::shared_ptr<OutcomeTracker> outcome_tracker)
    : label_store_(std::move(label_store)),
      outcome_tracker_(std::move(outcome_tracker)) {
    if (!label_store_) {
        label_store_ = std::make_shared<LabelStore>();
    }
    if (!outcome_tracker_) {
        outcome_tracker_ = std::make_shared<OutcomeTracker>();
    }
}

std::string CaseManager::createCase(const std::string& transaction_id,
                                    const std::string& customer_id,
                                    double initial_risk_score,
                                    DecisionAction original_decision,
                                    const std::vector<std::string>& initial_evidence) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Check if case already exists for transaction
    auto it = case_id_by_tx_id_.find(transaction_id);
    if (it != case_id_by_tx_id_.end()) {
        return it->second;
    }

    std::ostringstream oss;
    oss << "CASE_" << std::setw(6) << std::setfill('0') << case_counter_++;
    std::string case_id = oss.str();

    ReviewCase rc(case_id, transaction_id, customer_id, initial_risk_score,
                  original_decision, initial_evidence);

    cases_by_id_[case_id] = rc;
    case_id_by_tx_id_[transaction_id] = case_id;
    return case_id;
}

bool CaseManager::assignCase(const std::string& case_id, const std::string& analyst_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cases_by_id_.find(case_id);
    if (it != cases_by_id_.end()) {
        return it->second.assignTo(analyst_id);
    }
    return false;
}

bool CaseManager::addEvidence(const std::string& case_id, const std::string& evidence) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cases_by_id_.find(case_id);
    if (it != cases_by_id_.end()) {
        it->second.addEvidence(evidence);
        return true;
    }
    return false;
}

bool CaseManager::addNote(const std::string& case_id, const std::string& note) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cases_by_id_.find(case_id);
    if (it != cases_by_id_.end()) {
        it->second.addNote(note);
        return true;
    }
    return false;
}

bool CaseManager::resolveCase(const std::string& case_id,
                              CaseStatus resolution,
                              const std::string& summary,
                              const TransactionFeatures& features) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cases_by_id_.find(case_id);
    if (it == cases_by_id_.end()) {
        return false;
    }

    if (!it->second.resolve(resolution, summary)) {
        return false;
    }

    // Automatically generate ground-truth record in LabelStore
    if (label_store_) {
        GroundTruthLabel lbl = (resolution == CaseStatus::RESOLVED_CONFIRMED_FRAUD)
                               ? GroundTruthLabel::FRAUD
                               : GroundTruthLabel::LEGITIMATE;
        GroundTruthRecord rec(it->second.getTransactionId(),
                              lbl,
                              LabelSource::ANALYST_INVESTIGATION,
                              1.0,
                              features,
                              summary);
        label_store_->storeLabel(rec);
    }

    return true;
}

std::optional<ReviewCase> CaseManager::getCase(const std::string& case_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cases_by_id_.find(case_id);
    if (it != cases_by_id_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<ReviewCase> CaseManager::findByTransactionId(const std::string& tx_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it_id = case_id_by_tx_id_.find(tx_id);
    if (it_id != case_id_by_tx_id_.end()) {
        auto it = cases_by_id_.find(it_id->second);
        if (it != cases_by_id_.end()) {
            return it->second;
        }
    }
    return std::nullopt;
}

std::vector<ReviewCase> CaseManager::findByStatus(CaseStatus status) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ReviewCase> result;
    for (const auto& [_, rc] : cases_by_id_) {
        if (rc.getStatus() == status) {
            result.push_back(rc);
        }
    }
    return result;
}

std::vector<ReviewCase> CaseManager::findByAnalyst(const std::string& analyst_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ReviewCase> result;
    for (const auto& [_, rc] : cases_by_id_) {
        if (rc.getAssignedAnalystId() == analyst_id) {
            result.push_back(rc);
        }
    }
    return result;
}

std::vector<ReviewCase> CaseManager::getAllCases() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ReviewCase> result;
    result.reserve(cases_by_id_.size());
    for (const auto& [_, rc] : cases_by_id_) {
        result.push_back(rc);
    }
    return result;
}

size_t CaseManager::getOpenCaseCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t count = 0;
    for (const auto& [_, rc] : cases_by_id_) {
        if (rc.getStatus() == CaseStatus::OPEN || rc.getStatus() == CaseStatus::IN_INVESTIGATION) {
            count++;
        }
    }
    return count;
}

size_t CaseManager::getResolvedCaseCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t count = 0;
    for (const auto& [_, rc] : cases_by_id_) {
        if (rc.isResolved()) {
            count++;
        }
    }
    return count;
}

size_t CaseManager::totalCases() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cases_by_id_.size();
}

void CaseManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    cases_by_id_.clear();
    case_id_by_tx_id_.clear();
    case_counter_ = 1;
}

} // namespace epfd
