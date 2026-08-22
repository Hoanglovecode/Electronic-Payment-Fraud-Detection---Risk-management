#ifndef EPFD_FRAUD_I_FRAUD_RULE_HPP
#define EPFD_FRAUD_I_FRAUD_RULE_HPP

#include <string>
#include <optional>
#include "epfd/models/Transaction.hpp"
#include "epfd/models/FraudAlert.hpp"
#include "epfd/features/TransactionFeatures.hpp"

namespace epfd {

/**
 * @brief Strategy interface for individual fraud detection rules (OCP & SRP).
 * New fraud rules can be added without modifying existing rule evaluation logic.
 */
class IFraudRule {
public:
    virtual ~IFraudRule() = default;

    virtual const std::string& getId() const noexcept = 0;
    virtual const std::string& getName() const noexcept = 0;
    virtual FraudRuleCategory getCategory() const noexcept = 0;
    virtual double getWeight() const noexcept = 0;
    virtual bool isEnabled() const noexcept = 0;
    virtual void setEnabled(bool enabled) noexcept = 0;

    /**
     * @brief Evaluates a transaction against this rule with pre-extracted features.
     */
    virtual std::optional<FraudAlert> evaluate(const Transaction& tx, const TransactionFeatures& features) const = 0;

    /**
     * @brief Evaluates a transaction against this rule (standalone overload).
     */
    virtual std::optional<FraudAlert> evaluate(const Transaction& tx) const = 0;
};

/**
 * @brief Base class providing standard implementation for rule identity and toggles.
 */
class BaseFraudRule : public IFraudRule {
public:
    BaseFraudRule(std::string id, std::string name, FraudRuleCategory category, double weight, bool enabled = true)
        : id_(std::move(id)), name_(std::move(name)), category_(category), weight_(weight), enabled_(enabled) {}

    const std::string& getId() const noexcept override { return id_; }
    const std::string& getName() const noexcept override { return name_; }
    FraudRuleCategory getCategory() const noexcept override { return category_; }
    double getWeight() const noexcept override { return weight_; }
    bool isEnabled() const noexcept override { return enabled_; }
    void setEnabled(bool enabled) noexcept override { enabled_ = enabled; }

    std::optional<FraudAlert> evaluate(const Transaction& tx) const override {
        TransactionFeatures dummy;
        return evaluate(tx, dummy);
    }

    std::optional<FraudAlert> evaluate(const Transaction& /*tx*/, const TransactionFeatures& /*features*/) const override {
        return std::nullopt;
    }

protected:
    std::string id_;
    std::string name_;
    FraudRuleCategory category_;
    double weight_{0.0};
    bool enabled_{true};
};

} // namespace epfd

#endif // EPFD_FRAUD_I_FRAUD_RULE_HPP
