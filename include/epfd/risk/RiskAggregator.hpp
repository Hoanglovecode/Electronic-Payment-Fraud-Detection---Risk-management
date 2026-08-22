#ifndef EPFD_RISK_RISK_AGGREGATOR_HPP
#define EPFD_RISK_RISK_AGGREGATOR_HPP

#include <vector>
#include <string>
#include <memory>
#include "epfd/risk/RiskFactor.hpp"
#include "epfd/risk/RiskProfile.hpp"
#include "epfd/risk/RiskWeights.hpp"
#include "epfd/models/FraudAlert.hpp"
#include "epfd/features/TransactionFeatures.hpp"

namespace epfd {

struct RiskAggregationResult {
    double rule_score{0.0};
    double ml_score{0.0};
    double combined_score{0.0};
    std::vector<RiskFactor> factors;
    std::string explanation;
    bool has_critical_override{false};
};

/**
 * @brief Multi-Source Risk Aggregator combining Rule signals, ML inference, and contextual features.
 */
class RiskAggregator {
public:
    explicit RiskAggregator(RiskWeights weights = RiskWeights{});

    RiskAggregationResult aggregate(const std::vector<FraudAlert>& rule_alerts,
                                    double ml_probability,
                                    const TransactionFeatures& features,
                                    const RiskProfile& profile = RiskProfile{}) const;

    const RiskWeights& getWeights() const noexcept { return weights_; }
    void setWeights(RiskWeights weights) noexcept { weights_ = weights; }

private:
    RiskWeights weights_;
};

} // namespace epfd

#endif // EPFD_RISK_RISK_AGGREGATOR_HPP
