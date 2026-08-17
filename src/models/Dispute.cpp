#include "epfd/models/Dispute.hpp"
#include <sstream>

namespace epfd {

Dispute::Dispute(std::string dispute_id,
                 std::string transaction_id,
                 std::string customer_id,
                 double amount,
                 std::string reason_code,
                 CaseStatus status,
                 Timestamp filed_at)
    : dispute_id_(std::move(dispute_id)),
      transaction_id_(std::move(transaction_id)),
      customer_id_(std::move(customer_id)),
      amount_(amount),
      reason_code_(std::move(reason_code)),
      status_(status),
      filed_at_(filed_at) {}

void Dispute::resolve(bool is_confirmed_fraud, std::string notes) {
    if (is_confirmed_fraud) {
        status_ = CaseStatus::RESOLVED_CONFIRMED_FRAUD;
    } else {
        status_ = CaseStatus::RESOLVED_FALSE_POSITIVE;
    }
    resolution_notes_ = std::move(notes);
    resolved_at_ = std::chrono::system_clock::now();
}

bool Dispute::isResolved() const noexcept {
    return status_ == CaseStatus::RESOLVED_CONFIRMED_FRAUD ||
           status_ == CaseStatus::RESOLVED_FALSE_POSITIVE ||
           status_ == CaseStatus::CLOSED;
}

std::string Dispute::toString() const {
    std::ostringstream oss;
    oss << "Dispute[id=" << dispute_id_
        << ", tx=" << transaction_id_
        << ", amount=" << amount_
        << ", reason=" << reason_code_
        << ", status=" << epfd::toString(status_)
        << "]";
    return oss.str();
}

} // namespace epfd
