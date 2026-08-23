#include "epfd/risk/PolicyChangeAuditManager.hpp"
#include <algorithm>

namespace epfd {

std::string PolicyChangeAuditManager::proposeChange(const std::string& policy_name,
                                                    const std::string& parameter_name,
                                                    const std::string& old_value,
                                                    const std::string& new_value,
                                                    const std::string& reason,
                                                    const std::string& proposer_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    proposal_counter_++;
    std::string prop_id = "PROP_" + std::to_string(proposal_counter_);

    PolicyChangeProposal prop;
    prop.proposal_id = prop_id;
    prop.policy_name = policy_name;
    prop.parameter_name = parameter_name;
    prop.old_value = old_value;
    prop.new_value = new_value;
    prop.reason = reason;
    prop.proposer_id = proposer_id;
    prop.status = PolicyProposalStatus::PENDING;
    prop.proposed_at = std::chrono::system_clock::now();

    proposals_.push_back(prop);
    return prop_id;
}

bool PolicyChangeAuditManager::approveChange(const std::string& proposal_id, const std::string& approver_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& prop : proposals_) {
        if (prop.proposal_id == proposal_id) {
            if (prop.status != PolicyProposalStatus::PENDING) {
                return false; // Already resolved
            }

            // Enforce Four-Eyes Approval Principle (Proposer cannot approve own proposal)
            if (prop.proposer_id == approver_id) {
                return false; // Dual control violation
            }

            prop.approver_id = approver_id;
            prop.status = PolicyProposalStatus::APPROVED;
            prop.resolved_at = std::chrono::system_clock::now();
            return true;
        }
    }
    return false;
}

bool PolicyChangeAuditManager::rejectChange(const std::string& proposal_id, const std::string& reviewer_id, const std::string& reason) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& prop : proposals_) {
        if (prop.proposal_id == proposal_id) {
            if (prop.status != PolicyProposalStatus::PENDING) {
                return false;
            }
            prop.approver_id = reviewer_id;
            prop.rejection_reason = reason;
            prop.status = PolicyProposalStatus::REJECTED;
            prop.resolved_at = std::chrono::system_clock::now();
            return true;
        }
    }
    return false;
}

bool PolicyChangeAuditManager::rollbackChange(const std::string& proposal_id, const std::string& rollback_actor, const std::string& reason) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& prop : proposals_) {
        if (prop.proposal_id == proposal_id) {
            if (prop.status != PolicyProposalStatus::APPROVED) {
                return false; // Can only rollback approved changes
            }
            prop.status = PolicyProposalStatus::ROLLED_BACK;
            prop.rejection_reason = "Rolled back by " + rollback_actor + ": " + reason;
            prop.resolved_at = std::chrono::system_clock::now();
            return true;
        }
    }
    return false;
}

std::vector<PolicyChangeProposal> PolicyChangeAuditManager::getAuditLog() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return proposals_;
}

std::vector<PolicyChangeProposal> PolicyChangeAuditManager::getPendingProposals() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<PolicyChangeProposal> pending;
    for (const auto& prop : proposals_) {
        if (prop.status == PolicyProposalStatus::PENDING) {
            pending.push_back(prop);
        }
    }
    return pending;
}

size_t PolicyChangeAuditManager::count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return proposals_.size();
}

void PolicyChangeAuditManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    proposals_.clear();
    proposal_counter_ = 0;
}

} // namespace epfd
