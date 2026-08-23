#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <cstdio>
#include <fstream>
#include <chrono>

using namespace epfd;

// ==========================================
// 1. Extended In-Memory Repositories Suite
// ==========================================
EPFD_TEST(PersistenceSuite, InMemoryAlertAndCaseRepositories) {
    // 1. Fraud Alert Repository
    InMemoryFraudAlertRepository alert_repo;
    FraudAlert a1("a1", "tx_10", "RULE_1", "Rule 1", FraudRuleCategory::AMOUNT_DEVIATION, 30.0, "Amount deviation", RiskLevel::MEDIUM);
    FraudAlert a2("a2", "tx_10", "RULE_2", "Rule 2", FraudRuleCategory::VELOCITY, 50.0, "Velocity burst", RiskLevel::HIGH);
    FraudAlert a3("a3", "tx_20", "RULE_1", "Rule 1", FraudRuleCategory::AMOUNT_DEVIATION, 30.0, "Amount deviation", RiskLevel::MEDIUM);

    alert_repo.save(a1);
    alert_repo.save(a2);
    alert_repo.save(a3);
    ASSERT_EQ(alert_repo.count(), 3);

    auto tx10_alerts = alert_repo.findByTransactionId("tx_10");
    ASSERT_EQ(tx10_alerts.size(), 2);

    auto rule1_alerts = alert_repo.findByRuleId("RULE_1");
    ASSERT_EQ(rule1_alerts.size(), 2);

    auto high_alerts = alert_repo.findBySeverity(RiskLevel::HIGH);
    ASSERT_EQ(high_alerts.size(), 1);

    ASSERT_TRUE(alert_repo.remove("a3"));
    ASSERT_EQ(alert_repo.count(), 2);

    // 2. Fraud Case (Dispute) Repository
    InMemoryFraudCaseRepository case_repo;
    Dispute d1("disp_1", "tx_10", "cust_1", 250.0, "Unauthorized transaction");
    Dispute d2("disp_2", "tx_11", "cust_1", 500.0, "Item not received");
    case_repo.save(d1);
    case_repo.save(d2);

    ASSERT_EQ(case_repo.count(), 2);
    auto cust1_cases = case_repo.findByCustomerId("cust_1");
    ASSERT_EQ(cust1_cases.size(), 2);

    // 3. Risk Assessment Repository
    InMemoryRiskAssessmentRepository risk_repo;
    RiskAssessment asm1("asm_1", "tx_10", 30.0, 20.0, 25.0, RiskLevel::LOW, DecisionAction::APPROVE);
    RiskAssessment asm2("asm_2", "tx_11", 80.0, 90.0, 85.0, RiskLevel::CRITICAL, DecisionAction::BLOCK);
    risk_repo.save(asm1);
    risk_repo.save(asm2);

    ASSERT_EQ(risk_repo.count(), 2);
    auto blocked_asms = risk_repo.findByDecision(DecisionAction::BLOCK);
    ASSERT_EQ(blocked_asms.size(), 1);
    ASSERT_EQ(blocked_asms[0].getTransactionId(), "tx_11");
}

// ==========================================
// 2. Persistent FileTransactionRepository Suite
// ==========================================
EPFD_TEST(PersistenceSuite, FileTransactionRepositoryPersistenceAndReload) {
    const std::string test_csv = "bin/test_repo_transactions.csv";
    std::remove(test_csv.c_str()); // Clean start

    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("dev_1", "fp_1", "192.168.1.1");
    PaymentMethod pm("pm_1", PaymentType::CREDIT_CARD, "4111111111111111", "Nguyen A", 12, 2028);

    Transaction tx1("tx_file_1", TransactionType::PURCHASE, "c_1", "m_1", "acc_1", 100.0, "USD",
                    std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_1", pm);
    Transaction tx2("tx_file_2", TransactionType::TRANSFER, "c_1", "c_2", "acc_1", 200.0, "USD",
                    std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_2", pm);

    {
        FileTransactionRepository repo1(test_csv);
        repo1.save(tx1);
        repo1.save(tx2);
        ASSERT_EQ(repo1.count(), 2);
    } // repo1 closes and flushes to disk

    // Cold-start reload with fresh repository instance pointing to the same file
    {
        FileTransactionRepository repo2(test_csv);
        ASSERT_EQ(repo2.count(), 2);

        auto loaded_tx1 = repo2.findById("tx_file_1");
        ASSERT_TRUE(loaded_tx1.has_value());
        ASSERT_EQ(loaded_tx1.value().getTransactionId(), "tx_file_1");
        ASSERT_NEAR(loaded_tx1.value().getAmount(), 100.0, 0.01);
        ASSERT_EQ(loaded_tx1.value().getCustomerId(), "c_1");

        auto c1_txs = repo2.findByCustomerId("c_1");
        ASSERT_EQ(c1_txs.size(), 2);

        // Delete tx2
        ASSERT_TRUE(repo2.remove("tx_file_2"));
        ASSERT_EQ(repo2.count(), 1);
    }

    // Verify deletion persisted
    {
        FileTransactionRepository repo3(test_csv);
        ASSERT_EQ(repo3.count(), 1);
        ASSERT_FALSE(repo3.findById("tx_file_2").has_value());
    }

    std::remove(test_csv.c_str()); // Clean up
}

// ==========================================
// 3. AuditTrailLogger Suite
// ==========================================
EPFD_TEST(PersistenceSuite, AuditTrailLoggerCompliance) {
    const std::string audit_log = "bin/test_audit_trail.log";
    std::remove(audit_log.c_str());

    {
        AuditTrailLogger logger(audit_log);
        DecisionResult dr(DecisionAction::BLOCK, "StandardDecisionPolicy", "Critical fraud alert triggered");
        dr.decision_id = "DEC_999";
        dr.transaction_id = "tx_999";
        dr.risk_score = 92.5;
        dr.risk_level = RiskLevel::CRITICAL;

        ASSERT_TRUE(logger.logDecision(dr, "ANALYST_AGENT_01"));
        ASSERT_EQ(logger.getLoggedCount(), 1);
    }

    // Verify file content on disk
    std::ifstream ifs(audit_log);
    ASSERT_TRUE(ifs.is_open());
    std::string line;
    ASSERT_TRUE(std::getline(ifs, line));
    ASSERT_TRUE(line.find("TX_ID=tx_999") != std::string::npos);
    ASSERT_TRUE(line.find("ACTION=BLOCK") != std::string::npos);
    ASSERT_TRUE(line.find("ANALYST_AGENT_01") != std::string::npos);

    std::remove(audit_log.c_str());
}
