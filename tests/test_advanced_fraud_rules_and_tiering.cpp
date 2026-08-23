#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <chrono>
#include <memory>

using namespace epfd;
using namespace std::chrono_literals;

// ==========================================
// 1. Advanced Fraud Rules Suite
// ==========================================
EPFD_TEST(AdvancedRulesSuite, MuleAccountSmurfingRuleEvaluation) {
    MuleAccountSmurfingRule smurf_rule(8000.0, 9999.0, 65.0);

    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("dev_1", "fp_1", "192.168.1.1");
    PaymentMethod pm("pm_1", PaymentType::CREDIT_CARD, "4111111111111111", "Nguyen A", 12, 2028);

    // Case 1: Transaction $8,500 with high velocity (2 in 24h) -> AML Smurfing alert
    Transaction tx_smurf("tx_smurf_1", TransactionType::TRANSFER, "c_1", "c_2", "acc_1", 8500.0, "USD",
                         std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_1", pm);
    TransactionFeatures f_smurf;
    f_smurf.transactions_last_24hours = 2.0;

    auto alert_opt = smurf_rule.evaluate(tx_smurf, f_smurf);
    ASSERT_TRUE(alert_opt.has_value());
    ASSERT_EQ(alert_opt.value().getSeverity(), RiskLevel::HIGH);
    ASSERT_TRUE(alert_opt.value().getReason().find("AML Structuring") != std::string::npos);

    // Case 2: Normal small transaction $500 -> No alert
    Transaction tx_normal("tx_norm", TransactionType::PURCHASE, "c_1", "m_1", "acc_1", 500.0, "USD",
                          std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_1", pm);
    auto alert_norm = smurf_rule.evaluate(tx_normal, f_smurf);
    ASSERT_FALSE(alert_norm.has_value());
}

EPFD_TEST(AdvancedRulesSuite, DormantAccountAndDynamicListRules) {
    // Dormant account reactivation
    DormantAccountReactivationRule dormant_rule(1000.0, 55.0);
    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("dev_1", "fp_1", "192.168.1.1");
    PaymentMethod pm("pm_1", PaymentType::CREDIT_CARD, "4111111111111111", "Nguyen A", 12, 2028);

    Transaction tx_dormant("tx_dorm_1", TransactionType::PURCHASE, "c_dorm", "m_1", "acc_1", 1500.0, "USD",
                           std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_1", pm);
    TransactionFeatures f_dormant;
    f_dormant.transactions_last_24hours = 0.0;
    f_dormant.amount_deviation_ratio = 4.0;

    auto alert_dorm = dormant_rule.evaluate(tx_dormant, f_dormant);
    ASSERT_TRUE(alert_dorm.has_value());
    ASSERT_EQ(alert_dorm.value().getSeverity(), RiskLevel::MEDIUM);

    // Dynamic list matching
    auto lookup_index = std::make_shared<FastLookupIndex>();
    lookup_index->addBlacklist("BAD_CARD_BIN_411111");
    DynamicListMatchingRule list_rule(lookup_index);

    Transaction tx_clean("tx_cln", TransactionType::PURCHASE, "c_1", "m_1", "acc_1", 50.0, "USD",
                         std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_1", pm);
    ASSERT_FALSE(list_rule.evaluate(tx_clean, TransactionFeatures{}).has_value());

    lookup_index->addBlacklist("192.168.1.1");
    ASSERT_TRUE(list_rule.evaluate(tx_clean, TransactionFeatures{}).has_value());
}

// ==========================================
// 2. Customer Risk Tiering Suite
// ==========================================
EPFD_TEST(TieringSuite, CustomerRiskTierEvaluation) {
    CustomerRiskTierManager tier_manager;

    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");

    // 1. New customer (< 30 days, 2 transactions)
    Customer new_cust("c_new", "New User", "new@example.com", "0123456789", loc, false, false, std::chrono::system_clock::now());
    RiskProfile new_prof("c_new", 10.0, 1.0);
    new_prof.recordTransaction(false);
    new_prof.recordTransaction(false);

    auto res_new = tier_manager.evaluateCustomer(new_cust, new_prof);
    ASSERT_EQ(res_new.tier, CustomerRiskTier::NEW);
    ASSERT_NEAR(res_new.trust_multiplier, 1.20, 0.01);
    ASSERT_EQ(res_new.velocity_5m_limit, 3);

    // 2. Established customer (>= 5 clean txs)
    RiskProfile est_prof("c_est", 10.0, 1.0);
    for (int i = 0; i < 6; ++i) est_prof.recordTransaction(false);
    auto res_est = tier_manager.evaluateCustomer(new_cust, est_prof);
    ASSERT_EQ(res_est.tier, CustomerRiskTier::ESTABLISHED);
    ASSERT_NEAR(res_est.trust_multiplier, 1.00, 0.01);

    // 3. Trusted VIP customer (KYC verified + 25 clean transactions)
    Customer vip_cust("c_vip", "VIP User", "vip@example.com", "0987654321", loc, true, true, std::chrono::system_clock::now());
    RiskProfile vip_prof("c_vip", 5.0, 1.0);
    for (int i = 0; i < 25; ++i) vip_prof.recordTransaction(false);
    auto res_vip = tier_manager.evaluateCustomer(vip_cust, vip_prof);
    ASSERT_EQ(res_vip.tier, CustomerRiskTier::TRUSTED);
    ASSERT_NEAR(res_vip.trust_multiplier, 0.75, 0.01);

    // 4. Restricted customer (Dispute history >= 2 or fraud)
    RiskProfile bad_prof("c_bad", 30.0, 1.0);
    bad_prof.recordDispute();
    bad_prof.recordDispute();
    auto res_bad = tier_manager.evaluateCustomer(new_cust, bad_prof);
    ASSERT_EQ(res_bad.tier, CustomerRiskTier::RESTRICTED);
    ASSERT_NEAR(res_bad.trust_multiplier, 2.0, 0.01);
}

// ==========================================
// 3. Policy Change Governance & Four-Eyes Suite
// ==========================================
EPFD_TEST(GovernanceSuite, PolicyChangeAuditAndFourEyesEnforcement) {
    PolicyChangeAuditManager audit_mgr;

    // 1. Propose change to Large Amount threshold
    std::string prop_id = audit_mgr.proposeChange("LargeAmountRule", "threshold", "5000", "7500",
                                                  "Adjust threshold due to holiday inflation", "ADMIN_ALICE");
    ASSERT_TRUE(!prop_id.empty());
    ASSERT_EQ(audit_mgr.getPendingProposals().size(), 1);

    // 2. Four-Eyes Principle: Proposer ALICE cannot approve own proposal!
    bool self_approval = audit_mgr.approveChange(prop_id, "ADMIN_ALICE");
    ASSERT_FALSE(self_approval); // Must fail due to dual control violation

    // 3. Independent second reviewer (Risk Officer BOB) approves
    bool peer_approval = audit_mgr.approveChange(prop_id, "RISK_OFFICER_BOB");
    ASSERT_TRUE(peer_approval);
    ASSERT_EQ(audit_mgr.getPendingProposals().size(), 0);

    // 4. Verify audit history record
    auto log = audit_mgr.getAuditLog();
    ASSERT_EQ(log.size(), 1);
    ASSERT_EQ(log[0].proposer_id, "ADMIN_ALICE");
    ASSERT_EQ(log[0].approver_id, "RISK_OFFICER_BOB");
    ASSERT_EQ(log[0].status, PolicyProposalStatus::APPROVED);

    // 5. Rollback approved change
    ASSERT_TRUE(audit_mgr.rollbackChange(prop_id, "CHIEF_RISK_OFFICER", "Reverting holiday adjustment"));
    auto log_post = audit_mgr.getAuditLog();
    ASSERT_EQ(log_post[0].status, PolicyProposalStatus::ROLLED_BACK);
}
