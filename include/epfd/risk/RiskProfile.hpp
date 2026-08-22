#ifndef EPFD_RISK_RISK_PROFILE_HPP
#define EPFD_RISK_RISK_PROFILE_HPP

#include <string>
#include <chrono>
#include "epfd/common/Types.hpp"

namespace epfd {

/**
 * @brief Customer or Account historical risk profile for baseline calibration.
 */
class RiskProfile {
public:
    explicit RiskProfile(std::string entity_id = "",
                         double base_risk_score = 10.0,
                         double trust_multiplier = 1.0)
        : entity_id_(std::move(entity_id)),
          base_risk_score_(base_risk_score),
          trust_multiplier_(trust_multiplier),
          last_updated_(std::chrono::system_clock::now()) {}

    const std::string& getEntityId() const noexcept { return entity_id_; }
    double getBaseRiskScore() const noexcept { return base_risk_score_; }
    double getTrustMultiplier() const noexcept { return trust_multiplier_; }
    size_t getTotalTransactions() const noexcept { return total_transactions_; }
    size_t getFraudIncidents() const noexcept { return fraud_incidents_; }
    size_t getDisputeCount() const noexcept { return dispute_count_; }
    Timestamp getLastUpdated() const noexcept { return last_updated_; }

    void recordTransaction(bool is_fraudulent = false) {
        total_transactions_++;
        if (is_fraudulent) {
            fraud_incidents_++;
            trust_multiplier_ = std::min(2.0, trust_multiplier_ + 0.25);
            base_risk_score_ = std::min(100.0, base_risk_score_ + 20.0);
        } else {
            // Gradually reward clean history
            if (total_transactions_ > 20 && fraud_incidents_ == 0) {
                trust_multiplier_ = std::max(0.70, trust_multiplier_ - 0.01);
                base_risk_score_ = std::max(5.0, base_risk_score_ - 0.5);
            }
        }
        last_updated_ = std::chrono::system_clock::now();
    }

    void recordDispute() {
        dispute_count_++;
        trust_multiplier_ = std::min(2.5, trust_multiplier_ + 0.35);
        base_risk_score_ = std::min(100.0, base_risk_score_ + 15.0);
        last_updated_ = std::chrono::system_clock::now();
    }

private:
    std::string entity_id_;
    double base_risk_score_{10.0};
    double trust_multiplier_{1.0};
    size_t total_transactions_{0};
    size_t fraud_incidents_{0};
    size_t dispute_count_{0};
    Timestamp last_updated_{std::chrono::system_clock::now()};
};

} // namespace epfd

#endif // EPFD_RISK_RISK_PROFILE_HPP
