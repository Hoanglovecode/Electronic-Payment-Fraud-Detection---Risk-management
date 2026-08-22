#ifndef EPFD_FEATURES_TRANSACTION_FEATURES_HPP
#define EPFD_FEATURES_TRANSACTION_FEATURES_HPP

#include <string>
#include <vector>
#include "epfd/dsa/Vector.hpp"

namespace epfd {

/**
 * @brief Structured container of engineered features for Rule Engine and ML Model Inference.
 * Each feature has documented business meaning, source, and target usage.
 */
struct TransactionFeatures {
    // 1. Transaction Core Features
    double transaction_amount{0.0};             // Business: Raw transaction amount ($). Source: Transaction. Usage: Rules + ML.
    double hour_of_day{0.0};                    // Business: Hour [0, 23]. Source: Timestamp. Usage: ML.
    double is_weekend{0.0};                     // Business: Weekend indicator (1.0 or 0.0). Source: Timestamp. Usage: ML.

    // 2. Velocity & Frequency Features (No Leakage: strictly < t_current)
    double transactions_last_5min{0.0};         // Business: Velocity count in 5m window. Source: CustomerVelocityTracker. Usage: Rules + ML.
    double amount_sum_last_5min{0.0};           // Business: Total amount in 5m window. Source: CustomerVelocityTracker. Usage: Rules + ML.
    double transactions_last_1hour{0.0};        // Business: Velocity count in 1h window. Source: CustomerVelocityTracker. Usage: Rules + ML.
    double amount_sum_last_1hour{0.0};          // Business: Total amount in 1h window. Source: CustomerVelocityTracker. Usage: Rules + ML.
    double transactions_last_24hours{0.0};      // Business: Velocity count in 24h window. Source: CustomerVelocityTracker. Usage: Rules + ML.
    double amount_sum_last_24hours{0.0};        // Business: Total amount in 24h window. Source: CustomerVelocityTracker. Usage: Rules + ML.

    // 3. Amount Deviation & Behavioral Features
    double average_amount_24h{0.0};             // Business: Historical average amount in last 24h. Source: VelocityTracker. Usage: Rules + ML.
    double amount_deviation_ratio{1.0};         // Business: amount / (avg_24h + 1.0). Source: Derived. Usage: Rules + ML.

    // 4. Device & Network Features
    double is_new_device{0.0};                  // Business: 1.0 if device is unseen for this customer. Source: Customer Profile. Usage: Rules + ML.
    double is_high_risk_device{0.0};            // Business: 1.0 if emulator or rooted/jailbroken. Source: Device. Usage: Rules + ML.
    double accounts_on_device_count{1.0};       // Business: Number of accounts sharing this device. Source: FastLookupIndex. Usage: Rules + ML.

    // 5. Geographic & Impossible Travel Features
    double is_new_country{0.0};                 // Business: 1.0 if country differs from home country. Source: Location. Usage: Rules + ML.
    double distance_from_home_km{0.0};          // Business: Haversine distance from registered home. Source: Location. Usage: Rules + ML.
    double speed_from_last_tx_kmh{0.0};         // Business: Travel speed since previous transaction (km/h). Source: Location + History. Usage: Rules + ML.

    // 6. Merchant & Customer Risk Features
    double is_high_risk_mcc{0.0};               // Business: 1.0 if MCC is high-risk (Gambling, Crypto). Source: Merchant. Usage: Rules + ML.
    double merchant_risk_rating{0.0};           // Business: Merchant risk level [0.0, 1.0]. Source: Merchant. Usage: Rules + ML.
    double customer_historical_risk_score{0.0}; // Business: Customer risk score [0, 100]. Source: Customer. Usage: Rules + ML.

    /**
     * @brief Exports ordered feature values as a custom Vector for ML Model Predictor.
     * Total Feature Dimension: 18
     */
    dsa::Vector<double> toVector() const {
        dsa::Vector<double> vec;
        vec.reserve(18);
        vec.push_back(transaction_amount);
        vec.push_back(hour_of_day);
        vec.push_back(is_weekend);
        vec.push_back(transactions_last_5min);
        vec.push_back(amount_sum_last_5min);
        vec.push_back(transactions_last_1hour);
        vec.push_back(amount_sum_last_1hour);
        vec.push_back(transactions_last_24hours);
        vec.push_back(amount_sum_last_24hours);
        vec.push_back(average_amount_24h);
        vec.push_back(amount_deviation_ratio);
        vec.push_back(is_new_device);
        vec.push_back(is_high_risk_device);
        vec.push_back(accounts_on_device_count);
        vec.push_back(is_new_country);
        vec.push_back(distance_from_home_km);
        vec.push_back(speed_from_last_tx_kmh);
        vec.push_back(is_high_risk_mcc);
        return vec;
    }

    static std::vector<std::string> getFeatureNames() {
        return {
            "transaction_amount",
            "hour_of_day",
            "is_weekend",
            "transactions_last_5min",
            "amount_sum_last_5min",
            "transactions_last_1hour",
            "amount_sum_last_1hour",
            "transactions_last_24hours",
            "amount_sum_last_24hours",
            "average_amount_24h",
            "amount_deviation_ratio",
            "is_new_device",
            "is_high_risk_device",
            "accounts_on_device_count",
            "is_new_country",
            "distance_from_home_km",
            "speed_from_last_tx_kmh",
            "is_high_risk_mcc"
        };
    }

    static constexpr size_t FEATURE_DIMENSION = 18;
};

} // namespace epfd

#endif // EPFD_FEATURES_TRANSACTION_FEATURES_HPP
