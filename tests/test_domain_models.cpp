#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <cmath>

using namespace epfd;

// ==========================================
// 1. Location Suite
// ==========================================
EPFD_TEST(DomainModelSuite, LocationValidationAndHaversine) {
    Location loc1(21.0285, 105.8542, "Hanoi", "Vietnam");
    Location loc2(10.8231, 106.6297, "Ho Chi Minh City", "Vietnam");

    ASSERT_TRUE(loc1.isValid());
    ASSERT_TRUE(loc2.isValid());

    double dist_km = loc1.distanceKmTo(loc2);
    // Distance between Hanoi and HCMC is roughly ~1140-1160 km
    ASSERT_TRUE(dist_km > 1100.0 && dist_km < 1200.0);

    // Invalid coordinates should throw
    try {
        Location invalid_loc(95.0, 200.0, "Invalid", "Nowhere");
        ASSERT_TRUE(false); // Should not reach here
    } catch (const std::invalid_argument&) {
        ASSERT_TRUE(true);
    }
}

// ==========================================
// 2. Device Suite
// ==========================================
EPFD_TEST(DomainModelSuite, DeviceRiskEnvironment) {
    Location loc(37.7749, -122.4194, "San Francisco", "USA");
    Device normal_dev("dev_1", "fp_12345", "192.168.1.1", "Mozilla/5.0", false, false, loc);
    ASSERT_FALSE(normal_dev.isHighRiskEnvironment());

    Device emulator_dev("dev_2", "fp_67890", "10.0.0.1", "Dalvik/2.1.0", true, false, loc);
    ASSERT_TRUE(emulator_dev.isHighRiskEnvironment());

    Device rooted_dev("dev_3", "fp_99999", "172.16.0.1", "Android", false, true, loc);
    ASSERT_TRUE(rooted_dev.isHighRiskEnvironment());
}

// ==========================================
// 3. PaymentMethod Suite (PAN Masking & Luhn)
// ==========================================
EPFD_TEST(DomainModelSuite, PaymentMethodMaskingAndLuhn) {
    // Valid Visa test card: 4111 1111 1111 1111 -> Luhn valid
    ASSERT_TRUE(PaymentMethod::validateLuhn("4111111111111111"));
    // Invalid Luhn
    ASSERT_FALSE(PaymentMethod::validateLuhn("4111111111111112"));

    PaymentMethod card("pm_01", PaymentType::CREDIT_CARD, "4111111111111111", "Nguyen Van A", 12, 2028, "Vietnam");
    ASSERT_TRUE(card.isCard());
    ASSERT_EQ(card.getCardBin(), "411111");
    ASSERT_EQ(card.getLast4(), "1111");
    ASSERT_EQ(card.getMaskedCardNumber(), "411111******1111");
    
    // Expiry check
    ASSERT_FALSE(card.isExpired(2026, 8));
    ASSERT_TRUE(card.isExpired(2029, 1));

    // E-wallet & Bank transfer
    PaymentMethod wallet = PaymentMethod::createEWallet("pm_w1", "user_wallet_9999", "MoMo");
    ASSERT_FALSE(wallet.isCard());
    ASSERT_EQ(wallet.getLast4(), "9999");
}

// ==========================================
// 4. Merchant Suite
// ==========================================
EPFD_TEST(DomainModelSuite, MerchantHighRiskMCC) {
    Merchant normal_merch("m_01", "Grocery Store", "5411", "USA", 0.05);
    ASSERT_FALSE(normal_merch.isHighRiskMCC());

    Merchant casino("m_02", "Royal Casino", "7995", "Malta", 0.85);
    ASSERT_TRUE(casino.isHighRiskMCC());

    Merchant crypto("m_03", "Crypto Exchange", "6051", "Cyprus", 0.70);
    ASSERT_TRUE(crypto.isHighRiskMCC());
}

// ==========================================
// 5. Account Suite
// ==========================================
EPFD_TEST(DomainModelSuite, AccountBalanceOperations) {
    Account acc("acc_01", "cust_01", 1000.0, "USD");
    ASSERT_EQ(acc.getBalance(), 1000.0);
    ASSERT_TRUE(acc.hasSufficientBalance(500.0));
    ASSERT_FALSE(acc.hasSufficientBalance(1500.0));

    acc.deposit(200.0);
    ASSERT_EQ(acc.getBalance(), 1200.0);

    bool ok = acc.withdraw(400.0);
    ASSERT_TRUE(ok);
    ASSERT_EQ(acc.getBalance(), 800.0);

    // Freeze account
    acc.freeze();
    ASSERT_TRUE(acc.isFrozen());
    ASSERT_FALSE(acc.withdraw(100.0));
    ASSERT_FALSE(acc.hasSufficientBalance(100.0));
}

// ==========================================
// 6. Customer Suite
// ==========================================
EPFD_TEST(DomainModelSuite, CustomerRiskTieringAndDevices) {
    Location home(21.0285, 105.8542, "Hanoi", "Vietnam");
    Customer cust("cust_100", "Tran Van B", "tranb@example.com", "+84901234567", home, true, true);

    cust.addAccountId("acc_01");
    cust.addKnownDeviceId("dev_1");
    cust.addKnownPaymentMethodId("pm_01");

    ASSERT_TRUE(cust.isKnownDevice("dev_1"));
    ASSERT_FALSE(cust.isKnownDevice("dev_unknown"));

    cust.setRiskScore(15.0);
    ASSERT_EQ(cust.getRiskLevel(), RiskLevel::VERY_LOW);

    cust.setRiskScore(85.0);
    ASSERT_EQ(cust.getRiskLevel(), RiskLevel::CRITICAL);
}

// ==========================================
// 7. FraudAlert & RiskAssessment Suite
// ==========================================
EPFD_TEST(DomainModelSuite, FraudAlertAndRiskAssessment) {
    FraudAlert alert1("alt_01", "tx_999", "R01", "HighVelocityRule", FraudRuleCategory::VELOCITY, 35.0, "3 transactions in 1 minute", RiskLevel::HIGH);
    FraudAlert alert2("alt_02", "tx_999", "R02", "ImpossibleTravel", FraudRuleCategory::GEO_LOCATION, 50.0, "Speed exceeds 1000 km/h", RiskLevel::CRITICAL);

    ASSERT_EQ(alert1.getScoreContribution(), 35.0);
    ASSERT_EQ(alert2.getSeverity(), RiskLevel::CRITICAL);

    RiskAssessment assessment("asmt_01", "tx_999", 85.0, 90.0, 87.5, RiskLevel::CRITICAL, DecisionAction::BLOCK);
    assessment.addAlert(alert1);
    assessment.addAlert(alert2);
    assessment.addReason("Multiple severe fraud signals detected");

    ASSERT_TRUE(assessment.isBlocked());
    ASSERT_FALSE(assessment.isApproved());
    ASSERT_EQ(assessment.getAlerts().size(), 2);
    ASSERT_EQ(assessment.getReasons().size(), 1);
}

// ==========================================
// 8. Dispute Suite
// ==========================================
EPFD_TEST(DomainModelSuite, DisputeLifecycle) {
    Dispute dispute("disp_01", "tx_999", "cust_100", 500.0, "FRAUD_UNAUTHORIZED");
    ASSERT_EQ(dispute.getStatus(), CaseStatus::OPEN);
    ASSERT_FALSE(dispute.isResolved());

    dispute.resolve(true, "Chargeback confirmed by issuing bank");
    ASSERT_TRUE(dispute.isResolved());
    ASSERT_EQ(dispute.getStatus(), CaseStatus::RESOLVED_CONFIRMED_FRAUD);
}

// ==========================================
// 9. Transaction Suite
// ==========================================
EPFD_TEST(DomainModelSuite, TransactionLifecycleAndCrossBorder) {
    Location loc(1.3521, 103.8198, "Singapore", "Singapore");
    Device dev("dev_sg", "fp_sg", "128.0.0.1", "Safari", false, false, loc);
    PaymentMethod pm("pm_card", PaymentType::CREDIT_CARD, "4111111111111111", "Tran Van B", 10, 2027, "Vietnam");

    Transaction tx("tx_001", TransactionType::PURCHASE, "cust_100", "m_01", "acc_01", 250.0, "USD",
                    std::chrono::system_clock::now(), loc, "128.0.0.1", dev, "m_01", pm);

    ASSERT_EQ(tx.getStatus(), TransactionStatus::PENDING);
    ASSERT_TRUE(tx.isCrossBorder("Vietnam")); // Home is Vietnam, tx is Singapore
    ASSERT_FALSE(tx.isCrossBorder("Singapore"));

    // State transition to Review
    ASSERT_TRUE(tx.markReview());
    ASSERT_EQ(tx.getStatus(), TransactionStatus::REVIEW);

    // Review -> Approved
    ASSERT_TRUE(tx.markApproved());
    ASSERT_EQ(tx.getStatus(), TransactionStatus::APPROVED);

    // Approved -> Settled
    ASSERT_TRUE(tx.markSettled());
    ASSERT_EQ(tx.getStatus(), TransactionStatus::SETTLED);

    // Settled -> Disputed
    ASSERT_TRUE(tx.markDisputed());
    ASSERT_EQ(tx.getStatus(), TransactionStatus::DISPUTED);
}
