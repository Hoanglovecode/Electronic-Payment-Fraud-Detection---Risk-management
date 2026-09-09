/*
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Management System
 * Module: Concrete Decision Policies Implementation (Strategy Pattern)
 * Team Members: Hoang, Khiem, Triet (OOP Project)
 */

#include "epfd/decision/ConcreteDecisionPolicies.hpp"

using namespace std;

namespace epfd {

// ==========================================
// 1. StandardDecisionPolicy (Chính sách cân bằng mặc định)
// ==========================================
DecisionResult StandardDecisionPolicy::decide(double risk_score, 
                                              RiskLevel risk_level, 
                                              const vector<FraudAlert>& alerts) const {
    DecisionResult res;
    res.risk_score = risk_score;
    res.risk_level = risk_level;
    res.policy_applied = getName();
    res.applied_thresholds = "[0-40) APPROVE, [40-60) CHALLENGE_3DS, [60-80) REVIEW, [80-100] BLOCK";

    // Immediate Hard Override: Chặn ngay nếu có cảnh báo CRITICAL (ví dụ Blacklist)
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
            res.rationale = "Risk score " + to_string(static_cast<int>(risk_score)) + " within low-risk threshold. Transaction approved.";
            break;

        case RiskLevel::MEDIUM:
            res.action = DecisionAction::CHALLENGE_3DS;
            res.requires_step_up_auth = true;
            res.rationale = "Risk score " + to_string(static_cast<int>(risk_score)) + " requires step-up identity verification (3D-Secure/OTP).";
            break;

        case RiskLevel::HIGH:
            res.action = DecisionAction::REVIEW;
            res.requires_manual_review = true;
            res.rationale = "Risk score " + to_string(static_cast<int>(risk_score)) + " queued for manual fraud investigation.";
            break;

        case RiskLevel::CRITICAL:
        default:
            res.action = DecisionAction::BLOCK;
            res.blocked_reason = "Risk score " + to_string(static_cast<int>(risk_score)) + " exceeds safety threshold.";
            res.rationale = "Transaction blocked due to critical risk score of " + to_string(static_cast<int>(risk_score));
            break;
    }

    return res;
}

// ==========================================
// 2. StrictComplianceDecisionPolicy (Chính sách tuân thủ nghiêm ngặt)
// ==========================================
DecisionResult StrictComplianceDecisionPolicy::decide(double risk_score, 
                                                      RiskLevel risk_level, 
                                                      const vector<FraudAlert>& alerts) const {
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
            res.rationale = "Strict policy approved very low risk transaction (Score " + to_string(static_cast<int>(risk_score)) + ").";
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
            res.blocked_reason = "Strict policy blocked high/critical risk score: " + to_string(static_cast<int>(risk_score));
            res.rationale = "Strict compliance block enforced.";
            break;
    }

    return res;
}

// ==========================================
// 3. FrictionlessDecisionPolicy (Chính sách ưu tiên trải nghiệm người dùng)
// ==========================================
DecisionResult FrictionlessDecisionPolicy::decide(double risk_score, 
                                                  RiskLevel risk_level, 
                                                  const vector<FraudAlert>& alerts) const {
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
            res.rationale = "Frictionless checkout approved (Score " + to_string(static_cast<int>(risk_score)) + ").";
            break;

        case RiskLevel::HIGH:
            res.action = DecisionAction::CHALLENGE_3DS;
            res.requires_step_up_auth = true;
            res.rationale = "Frictionless policy step-up auth triggered for high risk score.";
            break;

        case RiskLevel::CRITICAL:
        default:
            res.action = DecisionAction::BLOCK;
            res.blocked_reason = "Frictionless policy blocked critical score: " + to_string(static_cast<int>(risk_score));
            res.rationale = "Blocked due to critical score.";
            break;
    }

    return res;
}

// ==========================================
// 4. CustomThresholdDecisionPolicy (Chính sách tùy biến ngưỡng ranh giới)
// ==========================================
DecisionResult CustomThresholdDecisionPolicy::decide(double risk_score, 
                                                     RiskLevel risk_level, 
                                                     const vector<FraudAlert>& alerts) const {
    DecisionResult res;
    res.risk_score = risk_score;
    res.risk_level = risk_level;
    res.policy_applied = getName();
    res.applied_thresholds = "[0-" + to_string(static_cast<int>(challenge_threshold_)) + ") APPROVE, [" +
                             to_string(static_cast<int>(challenge_threshold_)) + "-" +
                             to_string(static_cast<int>(review_threshold_)) + ") CHALLENGE_3DS, [" +
                             to_string(static_cast<int>(review_threshold_)) + "-" +
                             to_string(static_cast<int>(block_threshold_)) + ") REVIEW, [" +
                             to_string(static_cast<int>(block_threshold_)) + "-100] BLOCK";

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
        res.rationale = "Score " + to_string(static_cast<int>(risk_score)) + " below challenge threshold (" + to_string(static_cast<int>(challenge_threshold_)) + "). Approved.";
    } else if (risk_score < review_threshold_) {
        res.action = DecisionAction::CHALLENGE_3DS;
        res.requires_step_up_auth = true;
        res.rationale = "Score " + to_string(static_cast<int>(risk_score)) + " in challenge range [" + to_string(static_cast<int>(challenge_threshold_)) + ", " + to_string(static_cast<int>(review_threshold_)) + ").";
    } else if (risk_score < block_threshold_) {
        res.action = DecisionAction::REVIEW;
        res.requires_manual_review = true;
        res.rationale = "Score " + to_string(static_cast<int>(risk_score)) + " in manual review range [" + to_string(static_cast<int>(review_threshold_)) + ", " + to_string(static_cast<int>(block_threshold_)) + ").";
    } else {
        res.action = DecisionAction::BLOCK;
        res.blocked_reason = "Score " + to_string(static_cast<int>(risk_score)) + " exceeds block threshold (" + to_string(static_cast<int>(block_threshold_)) + ").";
        res.rationale = "Score " + to_string(static_cast<int>(risk_score)) + " >= " + to_string(static_cast<int>(block_threshold_)) + ". Blocked.";
    }

    return res;
}

} // namespace epfd
