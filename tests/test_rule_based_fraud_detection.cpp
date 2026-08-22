#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <chrono>
#include <memory>

using namespace epfd;
using namespace std::chrono_literals;

// ==========================================
// 1. LargeAmountRule Test
// ==========================================
EPFD_TEST(RuleSuite, LargeAmountRuleEvaluation) {
    LargeAmountRule rule(5000.0, 35.0);

    Location loc(0, 0, "Hanoi", "Vietnam");
    Device dev("d1", "fp1", "1.1.1.1");
    PaymentMethod pm("pm1", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);

    // Normal amount ($500)
    Transaction tx_normal("tx1", TransactionType::PURCHASE, "c1", "r1", "a1", 500.0, "USD",
                          std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);
    auto alert1 = rule.evaluate(tx_normal);
    ASSERT_FALSE(alert1.has_value());

    // Large amount ($6000) -> MEDIUM
    Transaction tx_large("tx2", TransactionType::PURCHASE, "c1", "r1", "a1", 6000.0, "USD",
                         std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);
    auto alert2 = rule.evaluate(tx_large);
    ASSERT_TRUE(alert2.has_value());
    ASSERT_EQ(alert2->getSeverity(), RiskLevel::MEDIUM);
    ASSERT_EQ(alert2->getScoreContribution(), 35.0);

    // Extreme amount ($15,000 >= 2x threshold) -> HIGH
    Transaction tx_extreme("tx3", TransactionType::PURCHASE, "c1", "r1", "a1", 15000.0, "USD",
                           std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);
    auto alert3 = rule.evaluate(tx_extreme);
    ASSERT_TRUE(alert3.has_value());
    ASSERT_EQ(alert3->getSeverity(), RiskLevel::HIGH);
}

// ==========================================
// 2. HighVelocityRule Test
// ==========================================
EPFD_TEST(RuleSuite, HighVelocityRuleEvaluation) {
    HighVelocityRule rule(3, 8, 50.0);
    Location loc(0, 0, "City", "Country");
    Device dev("d1", "fp1", "1.1.1.1");
    PaymentMethod pm("pm1", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);
    Transaction tx("tx_v", TransactionType::PURCHASE, "c1", "r1", "a1", 100.0, "USD",
                   std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);

    TransactionFeatures f_normal;
    f_normal.transactions_last_5min = 1.0;
    f_normal.transactions_last_1hour = 4.0;
    ASSERT_FALSE(rule.evaluate(tx, f_normal).has_value());

    TransactionFeatures f_burst;
    f_burst.transactions_last_5min = 4.0; // >= 3 threshold
    f_burst.transactions_last_1hour = 5.0;
    auto alert = rule.evaluate(tx, f_burst);
    ASSERT_TRUE(alert.has_value());
    ASSERT_EQ(alert->getSeverity(), RiskLevel::HIGH);
    ASSERT_EQ(alert->getScoreContribution(), 50.0);
}

// ==========================================
// 3. NewDevice & ForeignCountry Rules Test
// ==========================================
EPFD_TEST(RuleSuite, NewDeviceAndForeignCountryRules) {
    NewDeviceRule dev_rule(25.0);
    ForeignCountryRule foreign_rule(30.0);

    Location loc(0, 0, "Paris", "France");
    Device dev("d1", "fp_unseen", "1.1.1.1");
    PaymentMethod pm("pm1", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);
    Transaction tx("tx_dev", TransactionType::PURCHASE, "c1", "r1", "a1", 100.0, "USD",
                   std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);

    TransactionFeatures f;
    f.is_new_device = 1.0;
    f.is_new_country = 1.0;

    auto alert_dev = dev_rule.evaluate(tx, f);
    ASSERT_TRUE(alert_dev.has_value());
    ASSERT_EQ(alert_dev->getScoreContribution(), 25.0);

    auto alert_foreign = foreign_rule.evaluate(tx, f);
    ASSERT_TRUE(alert_foreign.has_value());
    ASSERT_EQ(alert_foreign->getScoreContribution(), 30.0);
}

// ==========================================
// 4. ImpossibleTravelRule Test
// ==========================================
EPFD_TEST(RuleSuite, ImpossibleTravelRuleEvaluation) {
    ImpossibleTravelRule rule(800.0, 70.0);
    Location loc(0, 0, "City", "Country");
    Device dev("d", "f", "1.1.1.1");
    PaymentMethod pm("pm", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);
    Transaction tx("tx_spd", TransactionType::PURCHASE, "c1", "r1", "a1", 100.0, "USD",
                   std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);

    // Normal car/train speed (120 km/h)
    TransactionFeatures f_car;
    f_car.speed_from_last_tx_kmh = 120.0;
    ASSERT_FALSE(rule.evaluate(tx, f_car).has_value());

    // Impossible travel (3500 km/h)
    TransactionFeatures f_impossible;
    f_impossible.speed_from_last_tx_kmh = 3500.0;
    auto alert = rule.evaluate(tx, f_impossible);
    ASSERT_TRUE(alert.has_value());
    ASSERT_EQ(alert->getSeverity(), RiskLevel::CRITICAL);
    ASSERT_EQ(alert->getScoreContribution(), 70.0);
}

// ==========================================
// 5. BehaviorDeviation & CardTesting Rules Test
// ==========================================
EPFD_TEST(RuleSuite, BehaviorDeviationAndCardTestingRules) {
    BehaviorDeviationRule dev_rule(3.0, 40.0);
    CardTestingRule card_test_rule(5.0, 60.0);

    Location loc(0, 0, "City", "Country");
    Device dev("d", "f", "1.1.1.1");
    PaymentMethod pm("pm", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);

    // Case 1: Behavior Deviation (5x average)
    Transaction tx_spike("tx_sp", TransactionType::PURCHASE, "c1", "r1", "a1", 1000.0, "USD",
                         std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);
    TransactionFeatures f_dev;
    f_dev.transactions_last_24hours = 3.0;
    f_dev.amount_deviation_ratio = 5.0; // 5x
    auto alert_dev = dev_rule.evaluate(tx_spike, f_dev);
    ASSERT_TRUE(alert_dev.has_value());
    ASSERT_EQ(alert_dev->getScoreContribution(), 40.0);

    // Case 2: Card Testing micro probing ($1.50 with 4 rapid transactions)
    Transaction tx_micro("tx_mic", TransactionType::PURCHASE, "c1", "r1", "a1", 1.50, "USD",
                         std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m1", pm);
    TransactionFeatures f_card;
    f_card.transactions_last_5min = 4.0;
    auto alert_card = card_test_rule.evaluate(tx_micro, f_card);
    ASSERT_TRUE(alert_card.has_value());
    ASSERT_EQ(alert_card->getSeverity(), RiskLevel::HIGH);
    ASSERT_EQ(alert_card->getScoreContribution(), 60.0);
}

// ==========================================
// 6. Blacklist & Whitelist Rules Test
// ==========================================
EPFD_TEST(RuleSuite, BlacklistAndWhitelistRules) {
    auto lookup_index = std::make_shared<FastLookupIndex>();
    lookup_index->addBlacklist("IP:185.220.101.5");
    lookup_index->addWhitelist("MERCHANT:M_SUPER_TRUSTED");

    BlacklistRule bl_rule(lookup_index, 100.0);
    WhitelistRule wl_rule(lookup_index, 0.0);

    Location loc(0, 0, "City", "Country");
    Device dev("d", "f", "185.220.101.5");
    PaymentMethod pm("pm", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);

    // Blacklisted IP
    Transaction tx_black("tx_bl", TransactionType::PURCHASE, "c1", "r1", "a1", 100.0, "USD",
                         std::chrono::system_clock::now(), loc, "185.220.101.5", dev, "m1", pm);
    auto alert_bl = bl_rule.evaluate(tx_black);
    ASSERT_TRUE(alert_bl.has_value());
    ASSERT_EQ(alert_bl->getSeverity(), RiskLevel::CRITICAL);
    ASSERT_EQ(alert_bl->getScoreContribution(), 100.0);

    // Whitelisted Merchant
    Transaction tx_white("tx_wl", TransactionType::PURCHASE, "c1", "r1", "a1", 100.0, "USD",
                         std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "M_SUPER_TRUSTED", pm);
    auto alert_wl = wl_rule.evaluate(tx_white);
    ASSERT_TRUE(alert_wl.has_value());
    ASSERT_EQ(alert_wl->getRuleId(), "RULE_WHITELIST");
}

// ==========================================
// 7. FraudDetectorEngine Composite Evaluation
// ==========================================
EPFD_TEST(EngineSuite, FraudDetectorEngineCompositeDetection) {
    auto lookup_index = std::make_shared<FastLookupIndex>();
    auto engine = FraudDetectorEngine::createDefaultEngine(lookup_index);

    ASSERT_TRUE(engine->ruleCount() >= 10);

    Location loc(0, 0, "City", "Country");
    Device dev("d_compromised", "fp_new", "1.1.1.1");
    PaymentMethod pm("pm", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);

    // High risk composite scenario: Large amount + New Device + Velocity Burst
    Transaction tx("tx_composite", TransactionType::PURCHASE, "c1", "r1", "a1", 8000.0, "USD",
                   std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m_casino", pm);

    TransactionFeatures f;
    f.is_new_device = 1.0;
    f.transactions_last_5min = 4.0;
    f.amount_deviation_ratio = 4.5;
    f.is_high_risk_mcc = 1.0;

    auto alerts = engine->detect(tx, f);
    ASSERT_TRUE(alerts.size() >= 4); // Large amount, High velocity, New device, Suspicious merchant, ATO signal

    double fraud_score = engine->computeFraudScore(tx, f);
    ASSERT_TRUE(fraud_score >= 85.0); // High aggregated risk

    // Test dynamic rule disable
    ASSERT_TRUE(engine->enableRule("RULE_LARGE_AMOUNT", false));
    auto rule = engine->getRule("RULE_LARGE_AMOUNT");
    ASSERT_TRUE(rule != nullptr);
    ASSERT_FALSE(rule->isEnabled());
}
