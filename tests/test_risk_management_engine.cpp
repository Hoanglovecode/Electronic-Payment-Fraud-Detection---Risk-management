#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <chrono>
#include <memory>

using namespace epfd;
using namespace std::chrono_literals;

// ==========================================
// 1. RiskFactor & RiskProfile Suite
// ==========================================
EPFD_TEST(RiskSuite, RiskFactorAndProfileAdaptation) {
    RiskFactor factor{"High Amount", 15.0, 0.05, "Amount exceeds $5,000 threshold"};
    std::string factor_str = factor.toString();
    ASSERT_TRUE(factor_str.find("High Amount") != std::string::npos);
    ASSERT_TRUE(factor_str.find("+15 pts") != std::string::npos);

    RiskProfile profile("cust_profile_1", 10.0, 1.0);
    ASSERT_EQ(profile.getBaseRiskScore(), 10.0);
    ASSERT_EQ(profile.getTrustMultiplier(), 1.0);

    // Record fraud incident -> trust multiplier increases, base score worsens
    profile.recordTransaction(true);
    ASSERT_EQ(profile.getFraudIncidents(), 1);
    ASSERT_TRUE(profile.getTrustMultiplier() > 1.0);
    ASSERT_TRUE(profile.getBaseRiskScore() > 10.0);

    // Record dispute
    profile.recordDispute();
    ASSERT_EQ(profile.getDisputeCount(), 1);
}

// ==========================================
// 2. RiskAggregator & Explainability Suite
// ==========================================
EPFD_TEST(RiskSuite, RiskAggregatorExplainableScoring) {
    RiskAggregator aggregator;

    TransactionFeatures f;
    f.transaction_amount = 6000.0;
    f.is_new_device = 1.0;
    f.transactions_last_5min = 4.0;
    f.hour_of_day = 2.0; // 2:00 AM nocturnal

    std::vector<FraudAlert> alerts;
    alerts.emplace_back("a1", "tx1", "R1", "Large Amount", FraudRuleCategory::AMOUNT_DEVIATION, 35.0, "Large amount", RiskLevel::MEDIUM);
    alerts.emplace_back("a2", "tx1", "R2", "High Velocity", FraudRuleCategory::VELOCITY, 50.0, "Velocity burst", RiskLevel::HIGH);

    double ml_probability = 0.75; // 75% ML probability

    auto res = aggregator.aggregate(alerts, ml_probability, f);
    ASSERT_TRUE(res.rule_score >= 85.0);
    ASSERT_NEAR(res.ml_score, 75.0, 0.001);
    ASSERT_TRUE(res.combined_score >= 60.0);
    ASSERT_TRUE(!res.factors.empty());
    
    // Check that explanation string contains factor contributions
    ASSERT_TRUE(res.explanation.find("High Amount") != std::string::npos);
    ASSERT_TRUE(res.explanation.find("New Device") != std::string::npos);
    ASSERT_TRUE(res.explanation.find("High Velocity") != std::string::npos);
    ASSERT_TRUE(res.explanation.find("Behavior Risk") != std::string::npos);
    ASSERT_TRUE(res.explanation.find("ML Risk") != std::string::npos);
    ASSERT_TRUE(res.explanation.find("Total") != std::string::npos);
}

// ==========================================
// 3. Risk Policies Suite
// ==========================================
EPFD_TEST(RiskSuite, ConcreteRiskPoliciesMapping) {
    StandardWeightedRiskPolicy std_policy;
    ASSERT_EQ(std_policy.mapToRiskLevel(10.0), RiskLevel::VERY_LOW);
    ASSERT_EQ(std_policy.mapToRiskLevel(30.0), RiskLevel::LOW);
    ASSERT_EQ(std_policy.mapToRiskLevel(50.0), RiskLevel::MEDIUM);
    ASSERT_EQ(std_policy.mapToRiskLevel(70.0), RiskLevel::HIGH);
    ASSERT_EQ(std_policy.mapToRiskLevel(90.0), RiskLevel::CRITICAL);

    ConservativeRiskPolicy cons_policy;
    ASSERT_EQ(cons_policy.mapToRiskLevel(10.0), RiskLevel::VERY_LOW);
    ASSERT_EQ(cons_policy.mapToRiskLevel(25.0), RiskLevel::LOW);
    ASSERT_EQ(cons_policy.mapToRiskLevel(45.0), RiskLevel::MEDIUM);
    ASSERT_EQ(cons_policy.mapToRiskLevel(65.0), RiskLevel::HIGH);
    ASSERT_EQ(cons_policy.mapToRiskLevel(75.0), RiskLevel::CRITICAL);

    // Critical Override Policy
    CriticalOverrideRiskPolicy crit_policy(85.0);
    std::vector<FraudAlert> crit_alerts;
    crit_alerts.emplace_back("a_crit", "tx1", "R_BL", "Blacklist", FraudRuleCategory::LIST_MATCHING, 100.0, "Threat intelligence hit", RiskLevel::CRITICAL);

    double crit_score = crit_policy.calculateCombinedScore(crit_alerts, 0.10);
    ASSERT_EQ(crit_score, 100.0);
    ASSERT_EQ(crit_policy.mapToRiskLevel(crit_score), RiskLevel::CRITICAL);
}

// ==========================================
// 4. Master RiskEngine End-to-End Suite
// ==========================================
EPFD_TEST(RiskSuite, MasterRiskEngineEndToEndAssessment) {
    auto policy = std::make_shared<StandardWeightedRiskPolicy>();
    auto aggregator = std::make_shared<RiskAggregator>();
    auto ml_predictor = std::make_shared<MockModelPredictor>("MockRiskMLModel", "v1.0", TransactionFeatures::FEATURE_DIMENSION, true, 0.05);
    auto detector = FraudDetectorEngine::createDefaultEngine();
    auto extractor = std::make_shared<FeatureExtractor>();

    RiskEngine risk_engine(policy, aggregator, ml_predictor, detector, extractor);

    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("dev_1", "fp_1", "192.168.1.1");
    PaymentMethod pm("pm_1", PaymentType::CREDIT_CARD, "4111111111111111", "Nguyen A", 12, 2028);

    // Case 1: Clean Low Risk Transaction ($50)
    ml_predictor->setFixedProbability(0.05); // 5% ML risk
    Transaction tx_clean("tx_clean", TransactionType::PURCHASE, "c_clean", "m_01", "acc_01", 50.0, "USD",
                         std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_01", pm);

    RiskAssessment clean_assessment = risk_engine.assess(tx_clean);
    ASSERT_EQ(clean_assessment.getTransactionId(), "tx_clean");
    ASSERT_TRUE(clean_assessment.getCombinedScore() < 25.0);
    ASSERT_EQ(clean_assessment.getRiskLevel(), RiskLevel::VERY_LOW);
    ASSERT_EQ(clean_assessment.getDecision(), DecisionAction::APPROVE);

    // Case 2: High Risk Composite Scenario ($8,000 + 85% ML risk)
    ml_predictor->setFixedProbability(0.85); // 85% ML risk
    Transaction tx_high("tx_high", TransactionType::PURCHASE, "c_high", "m_01", "acc_01", 8000.0, "USD",
                        std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_01", pm);

    RiskAssessment high_assessment = risk_engine.assess(tx_high);
    ASSERT_EQ(high_assessment.getTransactionId(), "tx_high");
    ASSERT_TRUE(high_assessment.getCombinedScore() >= 60.0);
    ASSERT_TRUE(high_assessment.getRiskLevel() == RiskLevel::HIGH || high_assessment.getRiskLevel() == RiskLevel::CRITICAL);
    ASSERT_TRUE(!high_assessment.getReasons().empty());
}
