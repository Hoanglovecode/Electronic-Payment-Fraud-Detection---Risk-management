#include "epfd/risk/CustomerRiskTierManager.hpp"
#include <chrono>

namespace epfd {

TierEvaluationResult StandardTieringPolicy::evaluateTier(const Customer& customer, const RiskProfile& profile) const {
    TierEvaluationResult res;

    // 1. Check for Restricted status (Disputes >= 2 or Fraud Incidents >= 1 or Blacklisted)
    if (customer.isBlacklisted() || profile.getFraudIncidents() >= 1 || profile.getDisputeCount() >= 2) {
        res.tier = CustomerRiskTier::RESTRICTED;
        res.trust_multiplier = 2.0;
        res.velocity_5m_limit = 2;
        res.max_frictionless_amount = 50.0;
        res.rationale = "Customer placed in RESTRICTED tier due to previous fraud incident, dispute history, or blacklist match.";
        return res;
    }

    // Calculate account age in days
    auto now = std::chrono::system_clock::now();
    auto age_days = std::chrono::duration_cast<std::chrono::hours>(now - customer.getCreatedAt()).count() / 24;

    // 2. Check for Trusted Tier (KYC verified + >= 90 days tenure + >= 20 clean transactions)
    if (customer.isKycVerified() && (age_days >= 90 || customer.isVip()) && profile.getTotalTransactions() >= 20 && profile.getDisputeCount() == 0) {
        res.tier = CustomerRiskTier::TRUSTED;
        res.trust_multiplier = 0.75;
        res.velocity_5m_limit = 10;
        res.max_frictionless_amount = 2000.0;
        res.rationale = "Customer promoted to TRUSTED tier (KYC verified, " + std::to_string(age_days) + " days tenure, " +
                        std::to_string(profile.getTotalTransactions()) + " clean txs).";
        return res;
    }

    // 3. Check for Established Tier (>= 30 days tenure or >= 5 transactions)
    if (age_days >= 30 || profile.getTotalTransactions() >= 5) {
        res.tier = CustomerRiskTier::ESTABLISHED;
        res.trust_multiplier = 1.00;
        res.velocity_5m_limit = 5;
        res.max_frictionless_amount = 500.0;
        res.rationale = "Customer in ESTABLISHED tier with regular transaction history.";
        return res;
    }

    // 4. Default: New Customer Tier
    res.tier = CustomerRiskTier::NEW;
    res.trust_multiplier = 1.20;
    res.velocity_5m_limit = 3;
    res.max_frictionless_amount = 200.0;
    res.rationale = "Customer in NEW tier (Tenure " + std::to_string(age_days) + " days, " +
                    std::to_string(profile.getTotalTransactions()) + " txs). Stricter velocity limits applied.";
    return res;
}

CustomerRiskTierManager::CustomerRiskTierManager(std::shared_ptr<ITieringPolicy> policy)
    : policy_(std::move(policy)) {
    if (!policy_) {
        policy_ = std::make_shared<StandardTieringPolicy>();
    }
}

TierEvaluationResult CustomerRiskTierManager::evaluateCustomer(const Customer& customer, const RiskProfile& profile) const {
    std::shared_ptr<ITieringPolicy> current_policy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        current_policy = policy_;
    }
    if (!current_policy) {
        current_policy = std::make_shared<StandardTieringPolicy>();
    }
    return current_policy->evaluateTier(customer, profile);
}

void CustomerRiskTierManager::setPolicy(std::shared_ptr<ITieringPolicy> policy) {
    std::lock_guard<std::mutex> lock(mutex_);
    policy_ = std::move(policy);
}

std::shared_ptr<ITieringPolicy> CustomerRiskTierManager::getPolicy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return policy_;
}

} // namespace epfd
