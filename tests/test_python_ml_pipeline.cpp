#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <chrono>
#include <memory>

using namespace epfd;

// ==========================================
// 1. Native ML Predictor Suite
// ==========================================
EPFD_TEST(MLPipelineSuite, NativeMLPredictorCleanTransaction) {
    NativeMLModelPredictor predictor("models/fraud_ml_model.json");
    ASSERT_TRUE(predictor.isReady());

    TransactionFeatures f_clean;
    f_clean.transaction_amount = 45.0;
    f_clean.hour_of_day = 14.0;
    f_clean.is_weekend = 0.0;
    f_clean.transactions_last_5min = 0.0;
    f_clean.amount_sum_last_5min = 0.0;
    f_clean.transactions_last_1hour = 0.0;
    f_clean.amount_sum_last_1hour = 0.0;
    f_clean.transactions_last_24hours = 1.0;
    f_clean.amount_sum_last_24hours = 45.0;
    f_clean.average_amount_24h = 45.0;
    f_clean.amount_deviation_ratio = 1.0;
    f_clean.is_new_device = 0.0;
    f_clean.is_high_risk_device = 0.0;
    f_clean.accounts_on_device_count = 1.0;
    f_clean.is_new_country = 0.0;
    f_clean.distance_from_home_km = 2.5;
    f_clean.speed_from_last_tx_kmh = 15.0;
    f_clean.is_high_risk_mcc = 0.0;

    auto res = predictor.predict(f_clean);
    ASSERT_TRUE(res.is_success);
    ASSERT_TRUE(res.fraud_probability < 0.20);
    ASSERT_TRUE(res.latency_ms < 5.0); // Sub-millisecond performance
}

EPFD_TEST(MLPipelineSuite, NativeMLPredictorHighRiskTransaction) {
    NativeMLModelPredictor predictor("models/fraud_ml_model.json");
    ASSERT_TRUE(predictor.isReady());

    TransactionFeatures f_fraud;
    f_fraud.transaction_amount = 9500.0;
    f_fraud.hour_of_day = 3.0; // Midnight attack
    f_fraud.is_weekend = 1.0;
    f_fraud.transactions_last_5min = 5.0;
    f_fraud.amount_sum_last_5min = 25000.0;
    f_fraud.transactions_last_1hour = 8.0;
    f_fraud.amount_sum_last_1hour = 35000.0;
    f_fraud.transactions_last_24hours = 14.0;
    f_fraud.amount_sum_last_24hours = 55000.0;
    f_fraud.average_amount_24h = 500.0;
    f_fraud.amount_deviation_ratio = 8.5;
    f_fraud.is_new_device = 1.0;
    f_fraud.is_high_risk_device = 1.0;
    f_fraud.accounts_on_device_count = 5.0;
    f_fraud.is_new_country = 1.0;
    f_fraud.distance_from_home_km = 5500.0;
    f_fraud.speed_from_last_tx_kmh = 1800.0; // Impossible speed
    f_fraud.is_high_risk_mcc = 1.0;

    auto res = predictor.predict(f_fraud);
    ASSERT_TRUE(res.is_success);
    ASSERT_TRUE(res.fraud_probability > 0.80);
}

EPFD_TEST(MLPipelineSuite, NativeMLPredictorIntegratedWithRiskEngine) {
    auto native_ml = std::make_shared<NativeMLModelPredictor>("models/fraud_ml_model.json");
    auto fraud_detector = std::make_shared<FraudDetectorEngine>();
    auto feature_extractor = std::make_shared<FeatureExtractor>();
    auto aggregator = std::make_shared<RiskAggregator>();
    auto policy = std::make_shared<StandardWeightedRiskPolicy>();

    RiskEngine risk_engine(policy, aggregator, native_ml, fraud_detector, feature_extractor);

    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("dev_1", "fp_1", "192.168.1.1");
    PaymentMethod pm("pm_1", PaymentType::CREDIT_CARD, "4111111111111111", "Nguyen A", 12, 2028);

    Transaction tx("tx_ml_int_1", TransactionType::PURCHASE, "c_1", "m_1", "acc_1", 80.0, "USD",
                   std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_1", pm);

    RiskProfile profile("c_1", 10.0, 1.0);
    auto assessment = risk_engine.assess(tx, profile);

    ASSERT_EQ(assessment.getTransactionId(), "tx_ml_int_1");
    ASSERT_TRUE(assessment.getMlScore() < 25.0);
    ASSERT_TRUE(assessment.getCombinedScore() < 30.0);
    ASSERT_EQ(assessment.getDecision(), DecisionAction::APPROVE);
}
