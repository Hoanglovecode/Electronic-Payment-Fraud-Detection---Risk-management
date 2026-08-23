#ifndef EPFD_RISK_CUSTOMER_RISK_TIER_MANAGER_HPP
#define EPFD_RISK_CUSTOMER_RISK_TIER_MANAGER_HPP

#include <string>
#include <memory>
#include <mutex>
#include "epfd/common/Types.hpp"
#include "epfd/models/Customer.hpp"
#include "epfd/risk/RiskProfile.hpp"

namespace epfd {

struct TierEvaluationResult {
    CustomerRiskTier tier{CustomerRiskTier::NEW};
    double trust_multiplier{1.0};
    size_t velocity_5m_limit{5};
    double max_frictionless_amount{500.0};
    std::string rationale;
};

/**
 * @brief Strategy interface for Customer Risk Tier evaluation (OCP).
 */
class ITieringPolicy {
public:
    virtual ~ITieringPolicy() = default;

    virtual const std::string& getName() const noexcept = 0;
    virtual TierEvaluationResult evaluateTier(const Customer& customer, const RiskProfile& profile) const = 0;
};

/**
 * @brief Standard industry tiering policy.
 * NEW (< 30 days or < 5 txs) -> ESTABLISHED (>= 30 days & >= 5 txs) -> TRUSTED (KYC + >= 90 days + 0 disputes).
 */
class StandardTieringPolicy : public ITieringPolicy {
public:
    const std::string& getName() const noexcept override {
        static const std::string name = "StandardTieringPolicy";
        return name;
    }

    TierEvaluationResult evaluateTier(const Customer& customer, const RiskProfile& profile) const override;
};

/**
 * @brief Customer Risk Tier Manager for dynamic tier assessment and risk baseline calibration.
 */
class CustomerRiskTierManager {
public:
    explicit CustomerRiskTierManager(std::shared_ptr<ITieringPolicy> policy = nullptr);

    TierEvaluationResult evaluateCustomer(const Customer& customer, const RiskProfile& profile) const;
    void setPolicy(std::shared_ptr<ITieringPolicy> policy);
    std::shared_ptr<ITieringPolicy> getPolicy() const;

private:
    std::shared_ptr<ITieringPolicy> policy_;
    mutable std::mutex mutex_;
};

} // namespace epfd

#endif // EPFD_RISK_CUSTOMER_RISK_TIER_MANAGER_HPP
