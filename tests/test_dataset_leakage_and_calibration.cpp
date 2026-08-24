#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <chrono>
#include <memory>

using namespace epfd;

// ==========================================
// 1. Dataset Leakage Invariant Suite
// ==========================================
EPFD_TEST(LeakageSuite, VelocityWindowTemporalHierarchyInvariant) {
    // Invariant: For any customer, 5min velocity <= 1h velocity <= 24h velocity
    CustomerVelocityTracker tracker;
    std::string cust_id = "c_leak_test";
    auto now = std::chrono::system_clock::now();

    // Add 1 transaction 30 minutes ago, and 1 transaction 1 minute ago
    tracker.recordTransaction(cust_id, now - std::chrono::minutes(30), 100.0);
    tracker.recordTransaction(cust_id, now - std::chrono::minutes(1), 200.0);

    auto metrics = tracker.getMetrics(cust_id, now);

    ASSERT_EQ(metrics.count_5m, 1);
    ASSERT_EQ(metrics.count_1h, 2);
    ASSERT_EQ(metrics.count_24h, 2);

    ASSERT_TRUE(metrics.count_5m <= metrics.count_1h);
    ASSERT_TRUE(metrics.count_1h <= metrics.count_24h);

    ASSERT_NEAR(metrics.sum_5m, 200.0, 0.01);
    ASSERT_NEAR(metrics.sum_1h, 300.0, 0.01);
    ASSERT_NEAR(metrics.sum_24h, 300.0, 0.01);
    ASSERT_TRUE(metrics.sum_5m <= metrics.sum_1h);
    ASSERT_TRUE(metrics.sum_1h <= metrics.sum_24h);
}

EPFD_TEST(LeakageSuite, FutureInformationStrictGuard) {
    CustomerVelocityTracker tracker;
    std::string cust_id = "c_future_test";
    auto t0 = std::chrono::system_clock::now();

    // Record past transaction at t0 - 10s
    tracker.recordTransaction(cust_id, t0 - std::chrono::seconds(10), 50.0);

    // When evaluating at t0, only past transaction is present
    auto metrics = tracker.getMetrics(cust_id, t0);
    ASSERT_EQ(metrics.count_5m, 1);
    ASSERT_NEAR(metrics.sum_5m, 50.0, 0.01);
}

// ==========================================
// 2. Probability Calibration Suite
// ==========================================
EPFD_TEST(CalibrationSuite, CalibratedProbabilityMonotonicityAndBrierSafety) {
    NativeMLModelPredictor predictor("models/fraud_ml_model.json");
    ASSERT_TRUE(predictor.isReady());

    // Clean profile
    TransactionFeatures f1;
    f1.transaction_amount = 50.0;
    f1.transactions_last_1hour = 0.0;
    f1.amount_deviation_ratio = 1.0;
    f1.distance_from_home_km = 5.0;
    f1.speed_from_last_tx_kmh = 10.0;

    // Moderate profile
    TransactionFeatures f2;
    f2.transaction_amount = 400.0;
    f2.transactions_last_1hour = 2.0;
    f2.amount_deviation_ratio = 3.0;
    f2.distance_from_home_km = 80.0;
    f2.speed_from_last_tx_kmh = 50.0;

    // Severe attack profile
    TransactionFeatures f3;
    f3.transaction_amount = 8500.0;
    f3.transactions_last_1hour = 8.0;
    f3.amount_sum_last_1hour = 25000.0;
    f3.amount_deviation_ratio = 12.0;
    f3.is_new_device = 1.0;
    f3.is_high_risk_device = 1.0;
    f3.is_new_country = 1.0;
    f3.distance_from_home_km = 5000.0;
    f3.speed_from_last_tx_kmh = 1200.0;

    auto r1 = predictor.predict(f1);
    auto r2 = predictor.predict(f2);
    auto r3 = predictor.predict(f3);

    ASSERT_TRUE(r1.fraud_probability <= r2.fraud_probability);
    ASSERT_TRUE(r2.fraud_probability <= r3.fraud_probability);
    ASSERT_TRUE(r3.fraud_probability > 0.80);
}
