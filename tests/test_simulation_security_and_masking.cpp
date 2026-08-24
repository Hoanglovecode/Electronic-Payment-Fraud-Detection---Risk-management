#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <vector>

using namespace epfd;

// ==========================================
// 1. PCI-DSS Security & Data Masking Suite
// ==========================================
EPFD_TEST(SecuritySuite, PanMaskingLast4Compliance) {
    std::string pan_16 = "4111111111111234";
    std::string masked = SecurityUtils::maskPan(pan_16);
    ASSERT_EQ(masked, "**** **** **** 1234");

    std::string pan_spaces = "4111 2222 3333 4444";
    std::string masked_spaces = SecurityUtils::maskPan(pan_spaces);
    ASSERT_EQ(masked_spaces, "**** **** **** 4444");

    // Short invalid pan
    std::string short_pan = "12";
    ASSERT_EQ(SecurityUtils::maskPan(short_pan), "****");
}

EPFD_TEST(SecuritySuite, PanMaskingBinAndLast4) {
    std::string pan = "4111119988771234";
    std::string masked = SecurityUtils::maskPanBinAndLast4(pan);
    ASSERT_EQ(masked, "4111 11** **** 1234");
}

EPFD_TEST(SecuritySuite, CvvRedactionAndPayloadSanitization) {
    std::string payload = "Transaction failed for card 4111111111111234 with CVV: 789 and CVC=456";
    std::string sanitized = SecurityUtils::sanitizeLogPayload(payload);

    ASSERT_TRUE(sanitized.find("789") == std::string::npos);
    ASSERT_TRUE(sanitized.find("456") == std::string::npos);
    ASSERT_TRUE(sanitized.find("4111111111111234") == std::string::npos);
    ASSERT_TRUE(sanitized.find("[REDACTED]") != std::string::npos);
    ASSERT_TRUE(sanitized.find("****-****-****-XXXX") != std::string::npos);
}

EPFD_TEST(SecuritySuite, EmailAndIpMaskingPrivacy) {
    std::string email = "nguyen.van.a@paymentgateway.com";
    std::string masked_email = SecurityUtils::maskEmail(email);
    ASSERT_EQ(masked_email, "n***a@paymentgateway.com");

    std::string ip = "192.168.1.204";
    std::string masked_ip = SecurityUtils::maskIp(ip);
    ASSERT_EQ(masked_ip, "192.168.*.*");
}

// ==========================================
// 2. Transaction Simulation Suite
// ==========================================
EPFD_TEST(SimulationSuite, SimulationAllScenariosGeneration) {
    TransactionSimulator sim;

    auto normal_tx = sim.generateNormalTransaction();
    ASSERT_TRUE(normal_tx.getAmount() >= 10.0 && normal_tx.getAmount() <= 500.0);
    ASSERT_EQ(normal_tx.getLocation().getCountry(), "Vietnam");

    auto burst_tx = sim.generateBurstVelocityTransaction("c_burst_target");
    ASSERT_EQ(burst_tx.getCustomerId(), "c_burst_target");
    ASSERT_TRUE(burst_tx.getAmount() >= 250.0 && burst_tx.getAmount() <= 800.0);

    auto travel_tx = sim.generateImpossibleTravelTransaction();
    ASSERT_EQ(travel_tx.getLocation().getCity(), "Paris");
    ASSERT_EQ(travel_tx.getLocation().getCountry(), "France");

    auto test_tx = sim.generateCardTestingTransaction();
    ASSERT_TRUE(test_tx.getAmount() >= 1.0 && test_tx.getAmount() <= 5.0);

    auto mule_tx = sim.generateMuleSmurfingTransaction();
    ASSERT_TRUE(mule_tx.getAmount() >= 8500.0 && mule_tx.getAmount() <= 9900.0);
    ASSERT_EQ(mule_tx.getType(), TransactionType::TRANSFER);

    auto ato_tx = sim.generateAccountTakeoverTransaction();
    ASSERT_TRUE(ato_tx.getDevice().isRootedOrJailbroken());
    ASSERT_TRUE(ato_tx.getDevice().isEmulator());
    ASSERT_EQ(ato_tx.getLocation().getCountry(), "Russia");
}

EPFD_TEST(SimulationSuite, SimulationBatchAndStreamingStream) {
    SimulationConfig cfg;
    cfg.count = 50;
    cfg.scenario = SimulationScenario::MIXED_REALISTIC_TRAFFIC;
    cfg.fraud_ratio = 0.10; // 10% fraud

    TransactionSimulator sim(cfg);
    auto batch = sim.generateBatch();
    ASSERT_EQ(batch.size(), 50);

    size_t streamed_count = 0;
    sim.streamTransactions(30, SimulationScenario::MIXED_REALISTIC_TRAFFIC, [&](const Transaction& tx) {
        ASSERT_FALSE(tx.getTransactionId().empty());
        streamed_count++;
    });

    ASSERT_EQ(streamed_count, 30);
}
