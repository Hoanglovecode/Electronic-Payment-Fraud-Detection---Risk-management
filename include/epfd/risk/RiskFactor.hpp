#ifndef EPFD_RISK_RISK_FACTOR_HPP
#define EPFD_RISK_RISK_FACTOR_HPP

#include <string>

namespace epfd {

/**
 * @brief Discrete explainable risk factor contribution for transparency and auditability.
 */
struct RiskFactor {
    std::string name;               // e.g. "High Amount", "New Device", "High Velocity", "ML Risk"
    double score_contribution{0.0}; // Points contributed to total risk score
    double weight{1.0};             // Relative weight
    std::string description;        // Business explanation for compliance & ops

    std::string toString() const {
        return name + " (" + (score_contribution >= 0 ? "+" : "") + 
               std::to_string(static_cast<int>(score_contribution)) + " pts): " + description;
    }
};

} // namespace epfd

#endif // EPFD_RISK_RISK_FACTOR_HPP
