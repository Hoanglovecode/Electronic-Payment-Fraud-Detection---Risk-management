#ifndef EPFD_RISK_POLICY_CHANGE_AUDIT_MANAGER_HPP
#define EPFD_RISK_POLICY_CHANGE_AUDIT_MANAGER_HPP

#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include "epfd/common/Types.hpp"

namespace epfd {

enum class PolicyProposalStatus {
    PENDING,
    APPROVED,
    REJECTED,
    ROLLED_BACK
};

inline std::string_view toString(PolicyProposalStatus status) {
    switch (status) {
        case PolicyProposalStatus::PENDING: return "PENDING";
        case PolicyProposalStatus::APPROVED: return "APPROVED";
        case PolicyProposalStatus::REJECTED: return "REJECTED";
        case PolicyProposalStatus::ROLLED_BACK: return "ROLLED_BACK";
    }
    return "UNKNOWN";
}

inline std::ostream& operator<<(std::ostream& os, PolicyProposalStatus status) {
    return os << toString(status);
}

struct PolicyChangeProposal {
    std::string proposal_id;
    std::string policy_name;
    std::string parameter_name;
    std::string old_value;
    std::string new_value;
    std::string proposer_id;
    std::string approver_id;
    std::string reason;
    PolicyProposalStatus status{PolicyProposalStatus::PENDING};
    Timestamp proposed_at{std::chrono::system_clock::now()};
    Timestamp resolved_at{};
    std::string rejection_reason;
};

/**
 * @brief Policy Change Governance & Audit Manager with Four-Eyes Principle enforcement.
 * Records who changed policy, when, old -> new value, reason, and independent approval.
 */
class PolicyChangeAuditManager {
public:
    PolicyChangeAuditManager() = default;

    /**
     * @brief Proposes a policy change.
     */
    std::string proposeChange(const std::string& policy_name,
                              const std::string& parameter_name,
                              const std::string& old_value,
                              const std::string& new_value,
                              const std::string& reason,
                              const std::string& proposer_id);

    /**
     * @brief Approves a pending change proposal.
     * Enforces Four-Eyes Principle (Approver must be distinct from Proposer).
     */
    bool approveChange(const std::string& proposal_id, const std::string& approver_id);

    /**
     * @brief Rejects a pending change proposal.
     */
    bool rejectChange(const std::string& proposal_id, const std::string& reviewer_id, const std::string& reason);

    /**
     * @brief Rolls back an approved change.
     */
    bool rollbackChange(const std::string& proposal_id, const std::string& rollback_actor, const std::string& reason);

    std::vector<PolicyChangeProposal> getAuditLog() const;
    std::vector<PolicyChangeProposal> getPendingProposals() const;
    size_t count() const;
    void clear();

private:
    mutable std::mutex mutex_;
    std::vector<PolicyChangeProposal> proposals_;
    size_t proposal_counter_{0};
};

} // namespace epfd

#endif // EPFD_RISK_POLICY_CHANGE_AUDIT_MANAGER_HPP
