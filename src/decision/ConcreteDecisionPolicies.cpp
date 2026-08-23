#include "epfd/decision/ConcreteDecisionPolicies.hpp"

namespace epfd {

// ==========================================
// 1. StandardDecisionPolicy
// ==========================================
DecisionResult StandardDecisionPolicy::decide(double risk_score, 
                                              RiskLevel risk_level, 
                                              const std::vector<FraudAlert>& alerts) const {
    DecisionResult res;
    res.risk_score = risk_score;
    res.risk_level = risk_level;
    res.policy_applied = getName();
    res.applied_thresholds = "[0-40) APPROVE, [40-60) CHALLENGE_3DS, [60-80) REVIEW, [80-100] BLOCK";

    // Immediate Hard Override for Critical Alert triggers
    for (const auto& alert : alerts) {
        if (alert.getSeverity() == RiskLevel::CRITICAL) {
            res.action = DecisionAction::BLOCK;
            res.blocked_reason = "Triggered critical alert: " + alert.getRuleName() + " (" + alert.getReason() + ")";
            res.rationale = "Immediate BLOCK enforced due to critical security threat rule: " + alert.getRuleName();
            return res;
        }
    }

    switch (risk_level) {
        case RiskLevel::VERY_LOW:
        case RiskLevel::LOW:
            res.action = DecisionAction::APPROVE;
            res.rationale = "Risk score " + std::to_string(static_cast<int>(risk_score)) + " within low-risk threshold. Transaction approved.";
            break;

        case RiskLevel::MEDIUM:
            res.action = DecisionAction::CHALLENGE_3DS;
            res.requires_step_up_auth = true;
            res.rationale = "Risk score " + std::to_string(static_cast<int>(risk_score)) + " requires step-up identity verification (3D-Secure/OTP).";
            break;

        case RiskLevel::HIGH:
            res.action = DecisionAction::REVIEW;
            res.requires_manual_review = true;
            res.rationale = "Risk score " + std::to_string(static_cast<int>(risk_score)) + " queued for manual fraud investigation.";
            break;

        case RiskLevel::CRITICAL:
        default:
            res.action = DecisionAction::BLOCK;
            res.blocked_reason = "Risk score " + std::to_string(static_cast<int>(risk_score)) + " exceeds safety threshold.";
            res.rationale = "Transaction blocked due to critical risk score of " + std::to_string(static_cast<int>(risk_score));
            break;
    }

    return res;
}

// ==========================================
// 2. StrictComplianceDecisionPolicy
// ==========================================
DecisionResult StrictComplianceDecisionPolicy::decide(double risk_score, 
                                                      RiskLevel risk_level, 
                                                      const std::vector<FraudAlert>& alerts) const {
    DecisionResult res;
    res.risk_score = risk_score;
    res.risk_level = risk_level;
    res.policy_applied = getName();
    res.applied_thresholds = "[0-20) APPROVE, [20-40) CHALLENGE_3DS, [40-60) REVIEW, [60-100] BLOCK";

    for (const auto& alert : alerts) {
        if (alert.getSeverity() == RiskLevel::CRITICAL || alert.getSeverity() == RiskLevel::HIGH) {
            res.action = DecisionAction::BLOCK;
            res.blocked_reason = "Strict policy blocked on high/critical alert: " + alert.getRuleName();
            res.rationale = "Strict compliance block: alert " + alert.getRuleName();
            return res;
        }
    }

    switch (risk_level) {
        case RiskLevel::VERY_LOW:
            res.action = DecisionAction::APPROVE;
            res.rationale = "Strict policy approved very low risk transaction (Score " + std::to_string(static_cast<int>(risk_score)) + ").";
            break;

        case RiskLevel::LOW:
            res.action = DecisionAction::CHALLENGE_3DS;
            res.requires_step_up_auth = true;
            res.rationale = "Strict policy requires step-up verification for low-risk tier.";
            break;

        case RiskLevel::MEDIUM:
            res.action = DecisionAction::REVIEW;
            res.requires_manual_review = true;
            res.rationale = "Strict policy mandates human review for medium-risk tier.";
            break;

        case RiskLevel::HIGH:
        case RiskLevel::CRITICAL:
        default:
            res.action = DecisionAction::BLOCK;
            res.blocked_reason = "Strict policy blocked high/critical risk score: " + std::to_string(static_cast<int>(risk_score));
            res.rationale = "Strict compliance block enforced.";
            break;
    }

    return res;
}

// ==========================================
// 3. FrictionlessDecisionPolicy
// ==========================================
DecisionResult FrictionlessDecisionPolicy::decide(double risk_score, 
                                                  RiskLevel risk_level, 
                                                  const std::vector<FraudAlert>& alerts) const {
    DecisionResult res;
    res.risk_score = risk_score;
    res.risk_level = risk_level;
    res.policy_applied = getName();
    res.applied_thresholds = "[0-60) APPROVE, [60-80) CHALLENGE_3DS, [80-100] BLOCK";

    for (const auto& alert : alerts) {
        if (alert.getSeverity() == RiskLevel::CRITICAL) {
            res.action = DecisionAction::BLOCK;
            res.blocked_reason = "Frictionless policy critical override: " + alert.getRuleName();
            res.rationale = "Critical threat override: " + alert.getRuleName();
            return res;
        }
    }

    switch (risk_level) {
        case RiskLevel::VERY_LOW:
        case RiskLevel::LOW:
        case RiskLevel::MEDIUM:
            res.action = DecisionAction::APPROVE;
            res.rationale = "Frictionless checkout approved (Score " + std::to_string(static_cast<int>(risk_score)) + ").";
            break;

        case RiskLevel::HIGH:
            res.action = DecisionAction::CHALLENGE_3DS;
            res.requires_step_up_auth = true;
            res.rationale = "Frictionless policy step-up auth triggered for high risk score.";
            break;

        case RiskLevel::CRITICAL:
        default:
            res.action = DecisionAction::BLOCK;
            res.blocked_reason = "Frictionless policy blocked critical score: " + std::to_string(static_cast<int>(risk_score));
            res.rationale = "Blocked due to critical score.";
            break;
    }

    return res;
}

// ==========================================
// 4. CustomThresholdDecisionPolicy
// ==========================================
DecisionResult CustomThresholdDecisionPolicy::decide(double risk_score, 
                                                      RiskLevel risk_level, 
                                                      const std::vector<FraudAlert>& alerts) const {
    DecisionResult res;
    res.risk_score = risk_score;
    res.risk_level = risk_level;
    res.policy_applied = getName();
    res.applied_thresholds = "[0-" + std::to_string(static_cast<int>(challenge_threshold_)) + ") APPROVE, [" +
                             std::to_string(static_cast<int>(challenge_threshold_)) + "-" +
                             std::to_string(static_cast<int>(review_threshold_)) + ") CHALLENGE_3DS, [" +
                             std::to_string(static_cast<int>(review_threshold_)) + "-" +
                             std::to_string(static_cast<int>(block_threshold_)) + ") REVIEW, [" +
                             std::to_string(static_cast<int>(block_threshold_)) + "-100] BLOCK";

    for (const auto& alert : alerts) {
        if (alert.getSeverity() == RiskLevel::CRITICAL) {
            res.action = DecisionAction::BLOCK;
            res.blocked_reason = "Custom threshold policy critical alert: " + alert.getRuleName();
            res.rationale = "Critical threat: " + alert.getRuleName();
            return res;
        }
    }

    if (risk_score < challenge_threshold_) {
        res.action = DecisionAction::APPROVE;
        res.rationale = "Score " + std::to_string(static_cast<int>(risk_score)) + " below challenge threshold (" + std::to_string(static_cast<int>(challenge_threshold_)) + "). Approved.";
    } else if (risk_score < review_threshold_) {
        res.action = DecisionAction::CHALLENGE_3DS;
        res.requires_step_up_auth = true;
        res.rationale = "Score " + std::to_string(static_cast<int>(risk_score)) + " in challenge range [" + std::to_string(static_cast<int>(challenge_threshold_)) + ", " + std::to_string(static_cast<int>(review_threshold_)) + ").";
    } else if (risk_score < block_threshold_) {
        res.action = DecisionAction::REVIEW;
        res.requires_manual_review = true;
        res.rationale = "Score " + std::to_string(static_cast<int>(risk_score)) + " in manual review range [" + std::to_string(static_cast<int>(review_threshold_)) + ", " + std::to_string(static_cast<int>(block_threshold_)) + ").";
    } else {
        res.action = DecisionAction::BLOCK;
        res.blocked_reason = "Score " + std::to_string(static_cast<int>(risk_score)) + " exceeds block threshold (" + std::to_string(static_cast<int>(block_threshold_)) + ").";
        res.rationale = "Score " + std::to_string(static_cast<int>(risk_score)) + " >= " + std::to_string(static_cast<int>(block_threshold_)) + ". Blocked.";
    }

    return res;
}

} // namespace epfd
