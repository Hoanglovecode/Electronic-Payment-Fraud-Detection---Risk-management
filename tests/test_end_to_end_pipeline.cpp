#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <memory>
#include <vector>

using namespace epfd;

// ============================================================================
// Phase 18: Master End-to-End Pipeline & Integration Validation Suite
// ============================================================================

EPFD_TEST(EndToEndPipelineSuite, FullCleanTransactionLifecycle) {
    auto tx_repo = std::make_shared<InMemoryTransactionRepository>();
    auto cust_repo = std::make_shared<InMemoryCustomerRepository>();
    auto acc_repo = std::make_shared<InMemoryAccountRepository>();
    auto validator = std::make_shared<TransactionValidator>(tx_repo, acc_repo);

    // Seed customer and account with $1,000 balance
    Customer cust("c_e2e_clean", "Nguyen Van Clean", "clean@bank.com", "0901234567");
    cust_repo->save(cust);

    Account acc("acc_c_e2e_clean", "c_e2e_clean", 1000.0, "USD");
    acc_repo->save(acc);

    TransactionService service(validator, tx_repo, acc_repo, cust_repo);

    // Create a normal low-risk purchase of $50
    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("dev_clean_1", "fp_clean_1", "192.168.1.100");
    PaymentMethod pm("pm_clean_1", PaymentType::CREDIT_CARD, "4111111111111234", "Nguyen Van Clean", 12, 2028);

    Transaction tx("tx_e2e_001", TransactionType::PURCHASE, "c_e2e_clean", "m_groceries", "acc_c_e2e_clean",
                   50.0, "USD", std::chrono::system_clock::now(),
                   loc, "192.168.1.100", dev, "m_groceries", pm);

    // 1. Feature Extraction & Risk Assessment
    auto extractor = std::make_shared<FeatureExtractor>();
    auto features = extractor->extract(tx);
    ASSERT_EQ(features.toVector().size(), 18);

    auto lookup_index = std::make_shared<FastLookupIndex>();
    auto fraud_engine = FraudDetectorEngine::createDefaultEngine(lookup_index);
    auto fraud_alerts = fraud_engine->detect(tx, features);
    ASSERT_TRUE(fraud_alerts.empty());

    auto risk_policy = std::make_shared<StandardWeightedRiskPolicy>();
    auto risk_aggregator = std::make_shared<RiskAggregator>();
    auto ml_predictor = std::make_shared<MockModelPredictor>("MockRiskMLModel", "v1.0", 18, true, 0.02);

    auto risk_engine = std::make_shared<RiskEngine>(risk_policy, risk_aggregator, ml_predictor, fraud_engine, extractor);
    auto decision_engine = std::make_shared<DecisionEngine>(std::make_shared<StandardDecisionPolicy>(), risk_engine);

    auto decision_eval = decision_engine->evaluate(tx);
    ASSERT_EQ(decision_eval.action, DecisionAction::APPROVE);

    // 2. Transaction Service Execution & Balance update
    auto result = service.processTransaction(tx);
    ASSERT_TRUE(result.is_success);
    ASSERT_EQ(result.transaction.getStatus(), TransactionStatus::APPROVED);

    // Verify account balance was deducted
    auto updated_acc = acc_repo->findById("acc_c_e2e_clean");
    ASSERT_TRUE(updated_acc.has_value());
    ASSERT_NEAR(updated_acc->getBalance(), 950.0, 0.001);

    // Verify transaction stored in repository
    auto saved_tx = tx_repo->findById("tx_e2e_001");
    ASSERT_TRUE(saved_tx.has_value());
    ASSERT_EQ(saved_tx->getStatus(), TransactionStatus::APPROVED);
}

EPFD_TEST(EndToEndPipelineSuite, FullFraudDetectionAndDisputeWorkflow) {
    auto tx_repo = std::make_shared<InMemoryTransactionRepository>();
    auto cust_repo = std::make_shared<InMemoryCustomerRepository>();
    auto acc_repo = std::make_shared<InMemoryAccountRepository>();

    Customer cust("c_e2e_victim", "Pham Thi Victim", "victim@bank.com", "0909888777");
    cust_repo->save(cust);

    Account acc("acc_c_e2e_victim", "c_e2e_victim", 5000.0, "USD");
    acc_repo->save(acc);

    // Simulate high-risk attack transaction (Impossible Travel & Rooted Device)
    Location foreign_loc(48.8566, 2.3522, "Paris", "France");
    Device rooted_dev("dev_paris_attack", "fp_paris_attack", "195.154.120.10", "Android Emulator 13.0", true, true);
    PaymentMethod pm("pm_victim_card", PaymentType::CREDIT_CARD, "4000123456789010", "Pham Thi Victim", 10, 2027);

    Transaction attack_tx("tx_e2e_attack_001", TransactionType::PURCHASE, "c_e2e_victim", "m_crypto_paris", "acc_c_e2e_victim",
                          3500.0, "USD", std::chrono::system_clock::now(),
                          foreign_loc, "195.154.120.10", rooted_dev, "m_crypto_paris", pm);

    // Pipeline Execution
    auto lookup_index = std::make_shared<FastLookupIndex>();
    lookup_index->addBlacklist("195.154.120.10"); // Blacklisted Paris IP

    auto fraud_engine = FraudDetectorEngine::createDefaultEngine(lookup_index);
    auto extractor = std::make_shared<FeatureExtractor>();
    auto features = extractor->extract(attack_tx);

    auto risk_policy = std::make_shared<StandardWeightedRiskPolicy>();
    auto risk_aggregator = std::make_shared<RiskAggregator>();
    auto ml_predictor = std::make_shared<MockModelPredictor>("MockRiskMLModel", "v1.0", 18, true, 0.95);

    auto risk_engine = std::make_shared<RiskEngine>(risk_policy, risk_aggregator, ml_predictor, fraud_engine, extractor);
    auto decision_engine = std::make_shared<DecisionEngine>(std::make_shared<StandardDecisionPolicy>(), risk_engine);

    auto decision_eval = decision_engine->evaluate(attack_tx);

    // Verify Decision is BLOCK or REVIEW
    ASSERT_TRUE(decision_eval.action == DecisionAction::BLOCK || decision_eval.action == DecisionAction::REVIEW);

    // ==========================================
    // Feedback Loop & Analyst Resolution Flow
    // ==========================================
    auto label_store = std::make_shared<LabelStore>();
    auto outcome_tracker = std::make_shared<OutcomeTracker>();
    CaseManager case_manager(label_store, outcome_tracker);

    // Open investigation case
    std::string case_id = case_manager.createCase("tx_e2e_attack_001", "c_e2e_victim", 95.0, DecisionAction::BLOCK);
    ASSERT_FALSE(case_id.empty());

    case_manager.assignCase(case_id, "analyst_sarah");
    case_manager.addEvidence(case_id, "Rooted emulator device fingerprint matched known carding syndicate.");
    case_manager.addEvidence(case_id, "Paris IP jump within 2 minutes of Hanoi transaction.");

    // Resolve case as confirmed fraud
    case_manager.resolveCase(case_id, CaseStatus::RESOLVED_CONFIRMED_FRAUD,
                             "Confirmed ATO fraud by cyber forensics",
                             features);

    // Verify GroundTruthRecord in LabelStore
    auto record = label_store->getLabel("tx_e2e_attack_001");
    ASSERT_TRUE(record.has_value());
    ASSERT_TRUE(record->label == GroundTruthLabel::FRAUD);
    ASSERT_TRUE(record->source == LabelSource::ANALYST_INVESTIGATION);

    // Export dataset for ML retraining verification
    bool export_ok = label_store->exportLabeledDatasetCsv("test_e2e_labeled_export.csv");
    ASSERT_TRUE(export_ok);
}

EPFD_TEST(EndToEndPipelineSuite, MultiScenarioStressSimulationValidation) {
    TransactionSimulator sim;
    auto batch = sim.generateBatch(100, SimulationScenario::MIXED_REALISTIC_TRAFFIC);
    ASSERT_EQ(batch.size(), 100);

    auto tx_repo = std::make_shared<InMemoryTransactionRepository>();
    auto cust_repo = std::make_shared<InMemoryCustomerRepository>();
    auto acc_repo = std::make_shared<InMemoryAccountRepository>();
    auto validator = std::make_shared<TransactionValidator>(tx_repo, acc_repo);

    TransactionService service(validator, tx_repo, acc_repo, cust_repo);

    size_t processed_count = 0;
    for (const auto& tx : batch) {
        // Pre-seed account so validator doesn't fail on missing account
        if (!acc_repo->findById(tx.getAccountId()).has_value()) {
            acc_repo->save(Account(tx.getAccountId(), tx.getCustomerId(), 10000.0, "USD"));
        }
        if (!cust_repo->findById(tx.getCustomerId()).has_value()) {
            cust_repo->save(Customer(tx.getCustomerId(), "Sim User", "sim@bank.com", "0900000000"));
        }

        auto res = service.processTransaction(tx);
        ASSERT_TRUE(res.is_success);
        processed_count++;
    }

    ASSERT_EQ(processed_count, 100);
}

EPFD_TEST(EndToEndPipelineSuite, ResilientPredictorFallbackIntegration) {
    // Crashing model mock
    class CrashingPredictor : public IModelPredictor {
    public:
        const std::string& getModelName() const noexcept override { static std::string n = "crash_model"; return n; }
        const std::string& getModelVersion() const noexcept override { static std::string v = "v_crash"; return v; }
        bool isReady() const noexcept override { return true; }
        size_t getExpectedFeatureCount() const noexcept override { return 18; }
        PredictionResult predict(const std::vector<double>&) override {
            throw std::runtime_error("Simulated ML Service Core Dump");
        }
    };

    auto crashing_model = std::make_shared<CrashingPredictor>();

    // Test FAIL_OPEN (score 0.0)
    MLResilienceConfig cfg_open;
    cfg_open.failure_policy = MLFailurePolicy::FAIL_OPEN;
    ResilientModelPredictor fail_open_predictor(crashing_model, cfg_open);
    std::vector<double> dummy_feat(18, 0.1);
    auto res_open = fail_open_predictor.predict(dummy_feat);
    ASSERT_FALSE(res_open.is_success);
    ASSERT_NEAR(res_open.fraud_probability, 0.0, 0.001);

    // Test FAIL_CLOSED (score 1.0)
    MLResilienceConfig cfg_closed;
    cfg_closed.failure_policy = MLFailurePolicy::FAIL_CLOSED;
    ResilientModelPredictor fail_closed_predictor(crashing_model, cfg_closed);
    auto res_closed = fail_closed_predictor.predict(dummy_feat);
    ASSERT_FALSE(res_closed.is_success);
    ASSERT_NEAR(res_closed.fraud_probability, 1.0, 0.001);
}
