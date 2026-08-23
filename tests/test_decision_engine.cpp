#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <chrono>
#include <memory>
#include <vector>

using namespace epfd;

// Mock Observer to test event broadcasting
class TestDecisionObserver : public IObserver<DecisionResult> {
public:
    void onNotify(const DecisionResult& result) override {
        notifications.push_back(result);
    }
    std::vector<DecisionResult> notifications;
};

// ==========================================
// 1. Concrete Decision Policies Suite
// ==========================================
EPFD_TEST(DecisionSuite, StandardDecisionPolicyEvaluation) {
    StandardDecisionPolicy policy;
    std::vector<FraudAlert> empty_alerts;

    // VERY_LOW -> APPROVE
    auto r1 = policy.decide(10.0, RiskLevel::VERY_LOW, empty_alerts);
    ASSERT_EQ(r1.action, DecisionAction::APPROVE);
    ASSERT_FALSE(r1.requires_step_up_auth);
    ASSERT_FALSE(r1.requires_manual_review);

    // MEDIUM -> CHALLENGE_3DS
    auto r2 = policy.decide(50.0, RiskLevel::MEDIUM, empty_alerts);
    ASSERT_EQ(r2.action, DecisionAction::CHALLENGE_3DS);
    ASSERT_TRUE(r2.requires_step_up_auth);
    ASSERT_FALSE(r2.requires_manual_review);

    // HIGH -> REVIEW
    auto r3 = policy.decide(70.0, RiskLevel::HIGH, empty_alerts);
    ASSERT_EQ(r3.action, DecisionAction::REVIEW);
    ASSERT_FALSE(r3.requires_step_up_auth);
    ASSERT_TRUE(r3.requires_manual_review);

    // CRITICAL -> BLOCK
    auto r4 = policy.decide(90.0, RiskLevel::CRITICAL, empty_alerts);
    ASSERT_EQ(r4.action, DecisionAction::BLOCK);
    ASSERT_TRUE(!r4.blocked_reason.empty());

    // Critical Alert override on low numerical score
    std::vector<FraudAlert> crit_alerts;
    crit_alerts.emplace_back("a1", "tx1", "R_BL", "Blacklist Match", FraudRuleCategory::LIST_MATCHING, 100.0, "Blacklisted IP", RiskLevel::CRITICAL);
    auto r5 = policy.decide(15.0, RiskLevel::VERY_LOW, crit_alerts);
    ASSERT_EQ(r5.action, DecisionAction::BLOCK);
    ASSERT_TRUE(r5.rationale.find("critical security threat") != std::string::npos);
}

EPFD_TEST(DecisionSuite, StrictAndFrictionlessPolicies) {
    StrictComplianceDecisionPolicy strict_policy;
    std::vector<FraudAlert> empty_alerts;

    // Strict: LOW -> CHALLENGE_3DS, MEDIUM -> REVIEW, HIGH -> BLOCK
    auto rs1 = strict_policy.decide(25.0, RiskLevel::LOW, empty_alerts);
    ASSERT_EQ(rs1.action, DecisionAction::CHALLENGE_3DS);
    auto rs2 = strict_policy.decide(45.0, RiskLevel::MEDIUM, empty_alerts);
    ASSERT_EQ(rs2.action, DecisionAction::REVIEW);
    auto rs3 = strict_policy.decide(65.0, RiskLevel::HIGH, empty_alerts);
    ASSERT_EQ(rs3.action, DecisionAction::BLOCK);

    FrictionlessDecisionPolicy frictionless_policy;
    // Frictionless: MEDIUM -> APPROVE, HIGH -> CHALLENGE_3DS
    auto rf1 = frictionless_policy.decide(50.0, RiskLevel::MEDIUM, empty_alerts);
    ASSERT_EQ(rf1.action, DecisionAction::APPROVE);
    auto rf2 = frictionless_policy.decide(70.0, RiskLevel::HIGH, empty_alerts);
    ASSERT_EQ(rf2.action, DecisionAction::CHALLENGE_3DS);
}

EPFD_TEST(DecisionSuite, CustomThresholdPolicy) {
    // Thresholds: [0, 30) APPROVE, [30, 55) CHALLENGE, [55, 75) REVIEW, [75, 100] BLOCK
    CustomThresholdDecisionPolicy custom_policy(30.0, 55.0, 75.0);
    std::vector<FraudAlert> empty_alerts;

    ASSERT_EQ(custom_policy.decide(20.0, RiskLevel::VERY_LOW, empty_alerts).action, DecisionAction::APPROVE);
    ASSERT_EQ(custom_policy.decide(35.0, RiskLevel::LOW, empty_alerts).action, DecisionAction::CHALLENGE_3DS);
    ASSERT_EQ(custom_policy.decide(60.0, RiskLevel::MEDIUM, empty_alerts).action, DecisionAction::REVIEW);
    ASSERT_EQ(custom_policy.decide(80.0, RiskLevel::HIGH, empty_alerts).action, DecisionAction::BLOCK);
}

// ==========================================
// 2. Master DecisionEngine & Integration Suite
// ==========================================
EPFD_TEST(DecisionSuite, DecisionEngineObserverAndQueueIntegration) {
    auto decision_engine = std::make_shared<DecisionEngine>();
    auto review_queue = std::make_shared<InvestigationPriorityQueue>();
    decision_engine->setInvestigationQueue(review_queue);

    auto observer = std::make_shared<TestDecisionObserver>();
    decision_engine->attachObserver(observer);

    // Assessment with HIGH risk -> should yield REVIEW, enqueue into InvestigationPriorityQueue, and notify observer
    std::vector<std::string> reasons = {"High velocity burst detected (+20 pts)", "Device mismatch (+10 pts)"};
    RiskAssessment high_asm("ASM_101", "tx_101", 65.0, 70.0, 68.0, RiskLevel::HIGH, DecisionAction::REVIEW, {}, reasons);

    DecisionResult res = decision_engine->evaluate(high_asm);
    ASSERT_EQ(res.transaction_id, "tx_101");
    ASSERT_EQ(res.action, DecisionAction::REVIEW);
    ASSERT_TRUE(res.requires_manual_review);

    // Verify Review Queue Integration
    ASSERT_EQ(review_queue->size(), 1);
    InvestigationCase ic;
    ASSERT_TRUE(review_queue->pop(ic));
    ASSERT_EQ(ic.transaction_id, "tx_101");
    ASSERT_EQ(ic.case_id, "CASE_tx_101");

    // Verify Observer Notification
    ASSERT_EQ(observer->notifications.size(), 1);
    ASSERT_EQ(observer->notifications[0].transaction_id, "tx_101");
    ASSERT_EQ(observer->notifications[0].action, DecisionAction::REVIEW);

    // Test Detach Observer
    decision_engine->detachObserver(observer);
    decision_engine->evaluate(high_asm);
    ASSERT_EQ(observer->notifications.size(), 1); // Not incremented
}

EPFD_TEST(DecisionSuite, DecisionEngineEndToEndWithRiskEngine) {
    auto policy = std::make_shared<StandardWeightedRiskPolicy>();
    auto aggregator = std::make_shared<RiskAggregator>();
    auto ml_predictor = std::make_shared<MockModelPredictor>("MockRiskMLModel", "v1.0", TransactionFeatures::FEATURE_DIMENSION, true, 0.05);
    auto lookup_index = std::make_shared<FastLookupIndex>();
    auto detector = FraudDetectorEngine::createDefaultEngine(lookup_index);
    auto extractor = std::make_shared<FeatureExtractor>();

    auto risk_engine = std::make_shared<RiskEngine>(policy, aggregator, ml_predictor, detector, extractor);
    auto decision_engine = std::make_shared<DecisionEngine>(std::make_shared<StandardDecisionPolicy>(), risk_engine);

    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("dev_1", "fp_1", "192.168.1.1");
    PaymentMethod pm("pm_1", PaymentType::CREDIT_CARD, "4111111111111111", "Nguyen A", 12, 2028);

    // 1. Low risk purchase -> APPROVE
    Transaction tx_ok("tx_ok_1", TransactionType::PURCHASE, "c_1", "m_1", "acc_1", 25.0, "USD",
                      std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_1", pm);
    DecisionResult r_ok = decision_engine->evaluate(tx_ok);
    ASSERT_EQ(r_ok.transaction_id, "tx_ok_1");
    ASSERT_EQ(r_ok.action, DecisionAction::APPROVE);

    // 2. High risk transaction with Blacklist trigger -> BLOCK
    lookup_index->addBlacklist("192.168.1.999");
    Transaction tx_bl("tx_bl_1", TransactionType::PURCHASE, "c_bad", "m_1", "acc_1", 100.0, "USD",
                      std::chrono::system_clock::now(), loc, "192.168.1.999", dev, "m_1", pm);
    DecisionResult r_bl = decision_engine->evaluate(tx_bl);
    ASSERT_EQ(r_bl.transaction_id, "tx_bl_1");
    ASSERT_EQ(r_bl.action, DecisionAction::BLOCK);
    ASSERT_TRUE(!r_bl.blocked_reason.empty());

    // 3. Dynamic Strategy swap: Switch to StrictCompliancePolicy
    decision_engine->setPolicy(std::make_shared<StrictComplianceDecisionPolicy>());
    ASSERT_EQ(decision_engine->getPolicy()->getName(), "StrictComplianceDecisionPolicy");
}
