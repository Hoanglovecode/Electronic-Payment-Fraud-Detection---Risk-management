#ifndef EPFD_MODELS_DISPUTE_HPP
#define EPFD_MODELS_DISPUTE_HPP

#include <string>
#include <chrono>
#include "epfd/common/Types.hpp"

namespace epfd {

class Dispute {
public:
    Dispute() = default;
    Dispute(std::string dispute_id,
            std::string transaction_id,
            std::string customer_id,
            double amount,
            std::string reason_code,
            CaseStatus status = CaseStatus::OPEN,
            Timestamp filed_at = std::chrono::system_clock::now());

    // Getters
    const std::string& getDisputeId() const noexcept { return dispute_id_; }
    const std::string& getTransactionId() const noexcept { return transaction_id_; }
    const std::string& getCustomerId() const noexcept { return customer_id_; }
    double getAmount() const noexcept { return amount_; }
    const std::string& getReasonCode() const noexcept { return reason_code_; }
    CaseStatus getStatus() const noexcept { return status_; }
    Timestamp getFiledAt() const noexcept { return filed_at_; }
    Timestamp getResolvedAt() const noexcept { return resolved_at_; }
    const std::string& getResolutionNotes() const noexcept { return resolution_notes_; }

    // Dispute Lifecycle
    void resolve(bool is_confirmed_fraud, std::string notes = "");
    bool isResolved() const noexcept;

    std::string toString() const;

private:
    std::string dispute_id_;
    std::string transaction_id_;
    std::string customer_id_;
    double amount_{0.0};
    std::string reason_code_;
    CaseStatus status_{CaseStatus::OPEN};
    Timestamp filed_at_{std::chrono::system_clock::now()};
    Timestamp resolved_at_{};
    std::string resolution_notes_;
};

} // namespace epfd

#endif // EPFD_MODELS_DISPUTE_HPP
