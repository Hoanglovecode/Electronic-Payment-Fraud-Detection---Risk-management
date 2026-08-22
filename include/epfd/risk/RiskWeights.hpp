#ifndef EPFD_RISK_RISK_WEIGHTS_HPP
#define EPFD_RISK_RISK_WEIGHTS_HPP

namespace epfd {

/**
 * @brief Configurable weights for Multi-Source Risk Scoring (No magic numbers).
 */
struct RiskWeights {
    double rule_weight{0.50};           // Contribution of Rule Engine [0.0, 1.0]
    double ml_weight{0.50};             // Contribution of ML Model [0.0, 1.0]
    double velocity_weight{0.05};       // Direct velocity factor weight
    double amount_weight{0.05};         // Direct amount anomaly factor weight
    double device_weight{0.05};         // Direct device risk factor weight
    double location_weight{0.05};       // Direct location risk factor weight

    double critical_override_score{85.0}; // Baseline score enforced on Critical threat triggers
};

} // namespace epfd

#endif // EPFD_RISK_RISK_WEIGHTS_HPP
