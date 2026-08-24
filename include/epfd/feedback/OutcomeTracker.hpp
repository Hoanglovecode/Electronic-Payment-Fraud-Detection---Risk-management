#ifndef EPFD_FEEDBACK_OUTCOME_TRACKER_HPP
#define EPFD_FEEDBACK_OUTCOME_TRACKER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include "epfd/common/Types.hpp"

namespace epfd {

enum class OutcomeType {
    CHARGEBACK_RECEIVED,
    CUSTOMER_CONFIRMED_LEGIT,
    AUTH_3DS_PASSED,
    AUTH_3DS_FAILED,
    MERCHANT_REFUND,
    FRAUD_REPORTED_BY_ISSUER
};

struct OutcomeEvent {
    std::string event_id;
    std::string transaction_id;
    OutcomeType type{OutcomeType::CHARGEBACK_RECEIVED};
    double loss_amount{0.0};
    std::string details;
    Timestamp timestamp{std::chrono::system_clock::now()};

    OutcomeEvent() = default;
    OutcomeEvent(std::string id, std::string tx_id, OutcomeType t, double amount = 0.0, std::string det = "")
        : event_id(std::move(id)),
          transaction_id(std::move(tx_id)),
          type(t),
          loss_amount(amount),
          details(std::move(det)),
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @brief Tracks real-world transaction outcomes post-decision (chargebacks, disputes, customer verifications).
 */
class OutcomeTracker {
public:
    OutcomeTracker() = default;

    void recordOutcome(const OutcomeEvent& event);
    void recordOutcome(const std::string& event_id,
                       const std::string& tx_id,
                       OutcomeType type,
                       double amount = 0.0,
                       const std::string& details = "");

    std::vector<OutcomeEvent> getOutcomesForTransaction(const std::string& tx_id) const;
    std::vector<OutcomeEvent> getAllOutcomes() const;

    size_t getChargebackCount() const noexcept;
    size_t getLegitConfirmationCount() const noexcept;
    double getTotalChargebackLoss() const noexcept;
    size_t totalCount() const noexcept;
    void clear();

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::vector<OutcomeEvent>> tx_outcomes_;
    std::vector<OutcomeEvent> all_events_;
    size_t chargeback_count_{0};
    size_t legit_count_{0};
    double total_chargeback_loss_{0.0};
};

} // namespace epfd

#endif // EPFD_FEEDBACK_OUTCOME_TRACKER_HPP
