#ifndef EPFD_RISK_I_RISK_RULE_HPP
#define EPFD_RISK_I_RISK_RULE_HPP

#include <string>
#include "epfd/models/Transaction.hpp"

namespace epfd {

struct RiskFactorContribution {
    std::string factor_name;
    double raw_score{0.0};      // [0, 100]
    double weight{1.0};         // Weight factor
    double weighted_score{0.0}; // raw_score * weight
    std::string description;
};

/**
 * @brief Strategy interface for individual risk scoring factors.
 */
class IRiskRule {
public:
    virtual ~IRiskRule() = default;

    virtual const std::string& getName() const noexcept = 0;
    virtual double getWeight() const noexcept = 0;
    virtual void setWeight(double weight) = 0;
    virtual RiskFactorContribution evaluate(const Transaction& tx) const = 0;
};

} // namespace epfd

#endif // EPFD_RISK_I_RISK_RULE_HPP
