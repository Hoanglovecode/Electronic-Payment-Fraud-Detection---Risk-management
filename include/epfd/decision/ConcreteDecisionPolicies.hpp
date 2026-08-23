#ifndef EPFD_DECISION_CONCRETE_DECISION_POLICIES_HPP
#define EPFD_DECISION_CONCRETE_DECISION_POLICIES_HPP

#include "epfd/decision/IDecisionPolicy.hpp"

namespace epfd {

/**
 * @brief Standard industry-grade decision policy.
 * VERY_LOW / LOW  -> APPROVE
 * MEDIUM          -> CHALLENGE_3DS
 * HIGH            -> REVIEW
 * CRITICAL        -> BLOCK
 */
class StandardDecisionPolicy : public IDecisionPolicy {
public:
    const std::string& getName() const noexcept override {
        static const std::string name = "StandardDecisionPolicy";
        return name;
    }

    DecisionResult decide(double risk_score, 
                          RiskLevel risk_level, 
                          const std::vector<FraudAlert>& alerts) const override;
};

/**
 * @brief Strict compliance policy for high-risk channels, cross-border, or VIP segments.
 * VERY_LOW -> APPROVE
 * LOW      -> CHALLENGE_3DS
 * MEDIUM   -> REVIEW
 * HIGH     -> BLOCK
 * CRITICAL -> BLOCK
 */
class StrictComplianceDecisionPolicy : public IDecisionPolicy {
public:
    const std::string& getName() const noexcept override {
        static const std::string name = "StrictComplianceDecisionPolicy";
        return name;
    }

    DecisionResult decide(double risk_score, 
                          RiskLevel risk_level, 
                          const std::vector<FraudAlert>& alerts) const override;
};

/**
 * @brief Frictionless decision policy prioritizing checkout conversion for low-risk merchants.
 * VERY_LOW / LOW / MEDIUM -> APPROVE
 * HIGH                    -> CHALLENGE_3DS
 * CRITICAL                -> BLOCK
 */
class FrictionlessDecisionPolicy : public IDecisionPolicy {
public:
    const std::string& getName() const noexcept override {
        static const std::string name = "FrictionlessDecisionPolicy";
        return name;
    }

    DecisionResult decide(double risk_score, 
                          RiskLevel risk_level, 
                          const std::vector<FraudAlert>& alerts) const override;
};

/**
 * @brief Fully configurable numerical threshold decision policy.
 */
class CustomThresholdDecisionPolicy : public IDecisionPolicy {
public:
    explicit CustomThresholdDecisionPolicy(double challenge_threshold = 40.0,
                                           double review_threshold = 60.0,
                                           double block_threshold = 80.0)
        : challenge_threshold_(challenge_threshold),
          review_threshold_(review_threshold),
          block_threshold_(block_threshold) {}

    const std::string& getName() const noexcept override {
        static const std::string name = "CustomThresholdDecisionPolicy";
        return name;
    }

    DecisionResult decide(double risk_score, 
                          RiskLevel risk_level, 
                          const std::vector<FraudAlert>& alerts) const override;

    double getChallengeThreshold() const noexcept { return challenge_threshold_; }
    double getReviewThreshold() const noexcept { return review_threshold_; }
    double getBlockThreshold() const noexcept { return block_threshold_; }

private:
    double challenge_threshold_{40.0};
    double review_threshold_{60.0};
    double block_threshold_{80.0};
};

} // namespace epfd

#endif // EPFD_DECISION_CONCRETE_DECISION_POLICIES_HPP
