#include "epfd/feedback/ReviewCase.hpp"
#include <sstream>

namespace epfd {

ReviewCase::ReviewCase(std::string case_id,
                       std::string transaction_id,
                       std::string customer_id,
                       double initial_risk_score,
                       DecisionAction original_decision,
                       std::vector<std::string> initial_evidence)
    : case_id_(std::move(case_id)),
      transaction_id_(std::move(transaction_id)),
      customer_id_(std::move(customer_id)),
      initial_risk_score_(initial_risk_score),
      original_decision_(original_decision),
      status_(CaseStatus::OPEN),
      evidence_list_(std::move(initial_evidence)),
      created_at_(std::chrono::system_clock::now()),
      updated_at_(created_at_) {}

bool ReviewCase::assignTo(const std::string& analyst_id) {
    if (analyst_id.empty() || status_ == CaseStatus::CLOSED) {
        return false;
    }
    assigned_analyst_id_ = analyst_id;
    status_ = CaseStatus::IN_INVESTIGATION;
    updated_at_ = std::chrono::system_clock::now();
    return true;
}

void ReviewCase::addEvidence(const std::string& evidence) {
    if (!evidence.empty()) {
        evidence_list_.push_back(evidence);
        updated_at_ = std::chrono::system_clock::now();
    }
}

void ReviewCase::addNote(const std::string& note) {
    if (!note.empty()) {
        notes_.push_back(note);
        updated_at_ = std::chrono::system_clock::now();
    }
}

bool ReviewCase::resolve(CaseStatus resolution, const std::string& summary) {
    if (resolution != CaseStatus::RESOLVED_CONFIRMED_FRAUD &&
        resolution != CaseStatus::RESOLVED_FALSE_POSITIVE) {
        return false;
    }
    status_ = resolution;
    resolution_summary_ = summary;
    resolved_at_ = std::chrono::system_clock::now();
    updated_at_ = resolved_at_;
    return true;
}

bool ReviewCase::close() {
    if (isResolved()) {
        status_ = CaseStatus::CLOSED;
        updated_at_ = std::chrono::system_clock::now();
        return true;
    }
    return false;
}

bool ReviewCase::isResolved() const noexcept {
    return status_ == CaseStatus::RESOLVED_CONFIRMED_FRAUD ||
           status_ == CaseStatus::RESOLVED_FALSE_POSITIVE ||
           status_ == CaseStatus::CLOSED;
}

std::string ReviewCase::toString() const {
    std::ostringstream oss;
    oss << "ReviewCase[ID=" << case_id_
        << ", TX=" << transaction_id_
        << ", Cust=" << customer_id_
        << ", Score=" << initial_risk_score_
        << ", Status=" << static_cast<int>(status_)
        << ", Analyst=" << (assigned_analyst_id_.empty() ? "Unassigned" : assigned_analyst_id_)
        << "]";
    return oss.str();
}

} // namespace epfd
