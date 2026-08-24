#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <chrono>
#include <memory>

using namespace epfd;

// ==========================================
// 1. ML Resilience & Fault Tolerance Suite
// ==========================================
EPFD_TEST(MLIntegrationSuite, FailOpenPolicyOnUnavailableModel) {
    MLResilienceConfig cfg;
    cfg.failure_policy = MLFailurePolicy::FAIL_OPEN;

    // Resilient proxy without underlying model
    ResilientModelPredictor resilient_predictor(nullptr, cfg);
    ASSERT_FALSE(resilient_predictor.isReady());

    std::vector<double> features(18, 1.0);
    auto res = resilient_predictor.predict(features);

    ASSERT_FALSE(res.is_success);
    ASSERT_NEAR(res.fraud_probability, 0.0, 0.001);
    ASSERT_TRUE(res.error_message.find("FAIL_OPEN") != std::string::npos);
    ASSERT_EQ(resilient_predictor.getFailureCount(), 1);
}

EPFD_TEST(MLIntegrationSuite, FailClosedPolicyOnUnavailableModel) {
    MLResilienceConfig cfg;
    cfg.failure_policy = MLFailurePolicy::FAIL_CLOSED;

    ResilientModelPredictor resilient_predictor(nullptr, cfg);
    std::vector<double> features(18, 1.0);
    auto res = resilient_predictor.predict(features);

    ASSERT_FALSE(res.is_success);
    ASSERT_NEAR(res.fraud_probability, 1.0, 0.001);
    ASSERT_TRUE(res.error_message.find("FAIL_CLOSED") != std::string::npos);
}

EPFD_TEST(MLIntegrationSuite, FeatureDimensionMismatchSafety) {
    auto native_ml = std::make_shared<NativeMLModelPredictor>("models/fraud_ml_model.json");
    MLResilienceConfig cfg;
    cfg.failure_policy = MLFailurePolicy::FALLBACK_SCORE;
    cfg.fallback_score = 45.0;

    ResilientModelPredictor resilient_predictor(native_ml, cfg);

    // Pass invalid feature vector with only 5 elements instead of 18
    std::vector<double> corrupted_features = {100.0, 12.0, 0.0, 1.0, 50.0};
    auto res = resilient_predictor.predict(corrupted_features);

    ASSERT_FALSE(res.is_success);
    ASSERT_NEAR(res.fraud_probability, 0.45, 0.01);
    ASSERT_TRUE(res.error_message.find("Feature dimension mismatch") != std::string::npos);
    ASSERT_EQ(resilient_predictor.getFailureCount(), 1);
}

EPFD_TEST(MLIntegrationSuite, TransparentSuccessUnderNormalOperation) {
    auto native_ml = std::make_shared<NativeMLModelPredictor>("models/fraud_ml_model.json");
    ResilientModelPredictor resilient_predictor(native_ml);

    TransactionFeatures f_clean;
    f_clean.transaction_amount = 50.0;
    f_clean.hour_of_day = 14.0;
    f_clean.speed_from_last_tx_kmh = 10.0;

    auto res = resilient_predictor.predict(f_clean.toVector());
    ASSERT_TRUE(res.is_success);
    ASSERT_TRUE(res.fraud_probability < 0.20);
    ASSERT_EQ(resilient_predictor.getSuccessCount(), 1);
    ASSERT_EQ(resilient_predictor.getFailureCount(), 0);
}

// ==========================================
// 2. ML Model Registry & Hot-Swapping Suite
// ==========================================
EPFD_TEST(MLIntegrationSuite, ModelRegistryVersionTrackingAndHotSwapping) {
    MLModelRegistry registry;

    auto model_v1 = std::make_shared<MockModelPredictor>("MockBaseline", "v1.0.0", 18, true, 0.15);
    auto model_v2 = std::make_shared<NativeMLModelPredictor>("models/fraud_ml_model.json");

    ASSERT_TRUE(registry.registerModel("v1.0.0", model_v1));
    ASSERT_TRUE(registry.registerModel("v2.0.0", model_v2));
    ASSERT_EQ(registry.count(), 2);

    // Initial active version is v1.0.0
    ASSERT_EQ(registry.getActiveVersion(), "v1.0.0");
    auto active_1 = registry.getActiveModel();
    ASSERT_TRUE(active_1 != nullptr);
    ASSERT_EQ(active_1->getModelName(), "MockBaseline");

    // Hot-swap active model to v2.0.0 during runtime
    ASSERT_TRUE(registry.setActiveVersion("v2.0.0"));
    ASSERT_EQ(registry.getActiveVersion(), "v2.0.0");
    auto active_2 = registry.getActiveModel();
    ASSERT_TRUE(active_2 != nullptr);
    ASSERT_EQ(active_2->getModelName(), "LogisticRegression");
}
