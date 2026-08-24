#include "epfd/feedback/OutcomeTracker.hpp"

namespace epfd {

void OutcomeTracker::recordOutcome(const OutcomeEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    tx_outcomes_[event.transaction_id].push_back(event);
    all_events_.push_back(event);

    if (event.type == OutcomeType::CHARGEBACK_RECEIVED ||
        event.type == OutcomeType::FRAUD_REPORTED_BY_ISSUER) {
        chargeback_count_++;
        total_chargeback_loss_ += event.loss_amount;
    } else if (event.type == OutcomeType::CUSTOMER_CONFIRMED_LEGIT ||
               event.type == OutcomeType::AUTH_3DS_PASSED) {
        legit_count_++;
    }
}

void OutcomeTracker::recordOutcome(const std::string& event_id,
                                   const std::string& tx_id,
                                   OutcomeType type,
                                   double amount,
                                   const std::string& details) {
    recordOutcome(OutcomeEvent(event_id, tx_id, type, amount, details));
}

std::vector<OutcomeEvent> OutcomeTracker::getOutcomesForTransaction(const std::string& tx_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tx_outcomes_.find(tx_id);
    if (it != tx_outcomes_.end()) {
        return it->second;
    }
    return {};
}

std::vector<OutcomeEvent> OutcomeTracker::getAllOutcomes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return all_events_;
}

size_t OutcomeTracker::getChargebackCount() const noexcept {
    return chargeback_count_;
}

size_t OutcomeTracker::getLegitConfirmationCount() const noexcept {
    return legit_count_;
}

double OutcomeTracker::getTotalChargebackLoss() const noexcept {
    return total_chargeback_loss_;
}

size_t OutcomeTracker::totalCount() const noexcept {
    return all_events_.size();
}

void OutcomeTracker::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    tx_outcomes_.clear();
    all_events_.clear();
    chargeback_count_ = 0;
    legit_count_ = 0;
    total_chargeback_loss_ = 0.0;
}

} // namespace epfd
