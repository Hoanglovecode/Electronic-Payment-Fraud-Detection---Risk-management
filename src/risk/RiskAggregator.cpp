#include "epfd/risk/RiskAggregator.hpp"
#include <algorithm>
#include <sstream>

namespace epfd {

RiskAggregator::RiskAggregator(RiskWeights weights)
    : weights_(weights) {}

RiskAggregationResult RiskAggregator::aggregate(const std::vector<FraudAlert>& rule_alerts,
                                                double ml_probability,
                                                const TransactionFeatures& features,
                                                const RiskProfile& profile) const {
    RiskAggregationResult res;

    // 1. Calculate Rule Score & detect Critical Alert
    double raw_rule_score = 0.0;
    bool has_critical = false;
    for (const auto& alert : rule_alerts) {
        raw_rule_score += alert.getScoreContribution();
        if (alert.getSeverity() == RiskLevel::CRITICAL) {
            has_critical = true;
        }
    }
    res.rule_score = std::min(100.0, raw_rule_score);
    res.ml_score = std::clamp(ml_probability * 100.0, 0.0, 100.0);

    // 2. Build Granular Explainable Risk Factors
    // Factor 1: ML Model Risk
    if (res.ml_score > 10.0) {
        double ml_pts = res.ml_score * weights_.ml_weight;
        res.factors.push_back(RiskFactor{
            "ML Risk",
            ml_pts,
            weights_.ml_weight,
            "Machine learning model predicted " + std::to_string(static_cast<int>(res.ml_score)) + "% fraud probability"
        });
    }

    // Factor 2: High Amount Anomaly
    if (features.transaction_amount >= 5000.0 || features.amount_deviation_ratio >= 3.0) {
        double pts = (features.transaction_amount >= 10000.0 ? 25.0 : 15.0);
        res.factors.push_back(RiskFactor{
            "High Amount",
            pts,
            weights_.amount_weight,
            "Transaction amount $" + std::to_string(static_cast<int>(features.transaction_amount)) + 
            " (Deviation: " + std::to_string(static_cast<int>(features.amount_deviation_ratio)) + "x avg)"
        });
    }

    // Factor 3: New Device
    if (features.is_new_device > 0.5) {
        res.factors.push_back(RiskFactor{
            "New Device",
            10.0,
            weights_.device_weight,
            "Unrecognized device fingerprint"
        });
    }

    // Factor 4: High Velocity Burst
    if (features.transactions_last_5min >= 3.0 || features.transactions_last_1hour >= 8.0) {
        double pts = features.transactions_last_5min >= 5.0 ? 30.0 : 20.0;
        res.factors.push_back(RiskFactor{
            "High Velocity",
            pts,
            weights_.velocity_weight,
            std::to_string(static_cast<int>(features.transactions_last_5min)) + " txs in last 5m"
        });
    }

    // Factor 5: Behavior Risk (Unusual Night Time)
    if (features.hour_of_day >= 1.0 && features.hour_of_day <= 5.0) {
        res.factors.push_back(RiskFactor{
            "Behavior Risk",
            12.0,
            0.05,
            "Activity during nocturnal off-hours (" + std::to_string(static_cast<int>(features.hour_of_day)) + ":00)"
        });
    }

    // Factor 6: Impossible Travel / Geographic
    if (features.speed_from_last_tx_kmh >= 800.0) {
        res.factors.push_back(RiskFactor{
            "Impossible Travel",
            40.0,
            weights_.location_weight,
            "Travel speed " + std::to_string(static_cast<int>(features.speed_from_last_tx_kmh)) + " km/h exceeds physical limits"
        });
    } else if (features.is_new_country > 0.5) {
        res.factors.push_back(RiskFactor{
            "Foreign Country",
            15.0,
            weights_.location_weight,
            "Cross-border transaction outside home country"
        });
    }

    // Factor 7: Suspicious Merchant
    if (features.is_high_risk_mcc > 0.5 || features.merchant_risk_rating >= 0.70) {
        res.factors.push_back(RiskFactor{
            "Suspicious Merchant",
            20.0,
            0.05,
            "High-risk Merchant Category Code (MCC) or rating"
        });
    }

    // 3. Compute Combined Weighted Score
    double base_score = (res.rule_score * weights_.rule_weight) + (res.ml_score * weights_.ml_weight);
    
    // Add additional contextual feature boosts
    double context_boost = 0.0;
    for (const auto& factor : res.factors) {
        if (factor.name != "ML Risk") {
            context_boost += factor.score_contribution * 0.20; // 20% fractional boost
        }
    }
    double combined = (base_score + context_boost) * profile.getTrustMultiplier();

    // 4. Critical Override Check
    if (has_critical || res.rule_score >= 90.0 || res.ml_score >= 95.0) {
        res.has_critical_override = true;
        combined = std::max(combined, weights_.critical_override_score);
    }

    res.combined_score = std::clamp(combined, 0.0, 100.0);

    // 5. Generate Textual Explanation
    std::ostringstream oss;
    if (res.factors.empty()) {
        oss << "Standard low-risk transaction baseline";
    } else {
        for (size_t i = 0; i < res.factors.size(); ++i) {
            if (i > 0) oss << " + ";
            oss << res.factors[i].name << " (+" << static_cast<int>(res.factors[i].score_contribution) << ")";
        }
        oss << " = Total " << static_cast<int>(res.combined_score);
        if (res.has_critical_override) {
            oss << " [CRITICAL OVERRIDE ENFORCED]";
        }
    }
    res.explanation = oss.str();

    return res;
}

} // namespace epfd
