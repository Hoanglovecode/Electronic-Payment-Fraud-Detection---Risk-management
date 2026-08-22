#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <chrono>
#include <memory>

using namespace epfd;
using namespace std::chrono_literals;

// ==========================================
// 1. Basic Feature Extraction & Dimension
// ==========================================
EPFD_TEST(FeatureSuite, BasicFeatureExtractionAndDimension) {
    auto extractor = std::make_shared<FeatureExtractor>();

    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("dev_1", "fp_1", "192.168.1.1", "Mozilla/5.0", false, false, loc);
    PaymentMethod pm("pm_1", PaymentType::CREDIT_CARD, "4111111111111111", "Nguyen Van A", 12, 2028);

    Transaction tx("tx_f1", TransactionType::PURCHASE, "cust_01", "m_01", "acc_01", 250.0, "USD",
                    std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_01", pm);

    TransactionFeatures f = extractor->extract(tx);
    ASSERT_EQ(f.transaction_amount, 250.0);
    ASSERT_TRUE(f.hour_of_day >= 0.0 && f.hour_of_day <= 23.0);
    ASSERT_TRUE(f.is_weekend == 0.0 || f.is_weekend == 1.0);

    // Vector conversion
    auto vec = f.toVector();
    ASSERT_EQ(vec.size(), TransactionFeatures::FEATURE_DIMENSION);
    ASSERT_EQ(vec.size(), 18);
    ASSERT_EQ(vec[0], 250.0); // 1st element is transaction_amount
}

// ==========================================
// 2. Velocity Extraction & Zero-Leakage Guarantee
// ==========================================
EPFD_TEST(FeatureSuite, VelocityExtractionAndZeroLeakage) {
    auto velocity_tracker = std::make_shared<CustomerVelocityTracker>();
    auto extractor = std::make_shared<FeatureExtractor>(velocity_tracker);

    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("dev_1", "fp_1", "192.168.1.1");
    PaymentMethod pm("pm_1", PaymentType::CREDIT_CARD, "4111111111111111", "User A", 12, 2028);

    auto t0 = std::chrono::system_clock::now();

    // Transaction 1: Arrives at t0
    Transaction tx1("tx_v1", TransactionType::PURCHASE, "cust_vel", "m_01", "acc_01", 100.0, "USD",
                    t0, loc, "192.168.1.1", dev, "m_01", pm);

    // Extracting before updating state: must see 0 prior transactions (NO LEAKAGE)
    TransactionFeatures f1 = extractor->extract(tx1);
    ASSERT_EQ(f1.transactions_last_5min, 0.0);
    ASSERT_EQ(f1.amount_sum_last_5min, 0.0);
    ASSERT_EQ(f1.transactions_last_24hours, 0.0);

    // Now update state after analysis
    extractor->updateState(tx1);

    // Transaction 2: Arrives at t0 + 1 min
    Transaction tx2("tx_v2", TransactionType::PURCHASE, "cust_vel", "m_01", "acc_01", 200.0, "USD",
                    t0 + 1min, loc, "192.168.1.1", dev, "m_01", pm);

    TransactionFeatures f2 = extractor->extract(tx2);
    ASSERT_EQ(f2.transactions_last_5min, 1.0);
    ASSERT_EQ(f2.amount_sum_last_5min, 100.0);
    ASSERT_EQ(f2.average_amount_24h, 100.0);
    ASSERT_NEAR(f2.amount_deviation_ratio, 2.0, 0.001); // 200 / 100 = 2.0

    extractor->updateState(tx2);

    // Transaction 3: Arrives at t0 + 3 min with spike $600
    Transaction tx3("tx_v3", TransactionType::PURCHASE, "cust_vel", "m_01", "acc_01", 600.0, "USD",
                    t0 + 3min, loc, "192.168.1.1", dev, "m_01", pm);

    TransactionFeatures f3 = extractor->extract(tx3);
    ASSERT_EQ(f3.transactions_last_5min, 2.0);
    ASSERT_EQ(f3.amount_sum_last_5min, 300.0); // 100 + 200
    ASSERT_NEAR(f3.average_amount_24h, 150.0, 0.001); // (100 + 200) / 2
    ASSERT_NEAR(f3.amount_deviation_ratio, 4.0, 0.001); // 600 / 150 = 4.0
}

// ==========================================
// 3. Device & Multi-Accounting Features
// ==========================================
EPFD_TEST(FeatureSuite, DeviceAndMultiAccountingSignals) {
    auto lookup_index = std::make_shared<FastLookupIndex>();
    auto cust_repo = std::make_shared<InMemoryCustomerRepository>();
    auto extractor = std::make_shared<FeatureExtractor>(nullptr, lookup_index, cust_repo);

    Location home(21.0285, 105.8542, "Hanoi", "Vietnam");
    Customer cust("cust_dev_test", "Tran C", "tranc@example.com", "+84911111111", home, true, false);
    cust.addKnownDeviceId("dev_trusted_phone");
    cust.setRiskScore(45.0);
    cust_repo->save(cust);

    // Device sharing setup (3 accounts on same device)
    lookup_index->recordDeviceUsage("fp_shared_device", "cust_dev_test");
    lookup_index->recordDeviceUsage("fp_shared_device", "cust_other_1");
    lookup_index->recordDeviceUsage("fp_shared_device", "cust_other_2");

    // Case 1: Emulator on shared device (Unseen device for customer)
    Device root_dev("dev_new_phone", "fp_shared_device", "10.0.0.1", "Android", true, true, home);
    PaymentMethod pm("pm_1", PaymentType::CREDIT_CARD, "4111111111111111", "Tran C", 12, 2028);

    Transaction tx1("tx_d1", TransactionType::PURCHASE, "cust_dev_test", "m_01", "acc_01", 100.0, "USD",
                    std::chrono::system_clock::now(), home, "10.0.0.1", root_dev, "m_01", pm);

    TransactionFeatures f1 = extractor->extract(tx1);
    ASSERT_EQ(f1.is_new_device, 1.0);
    ASSERT_EQ(f1.is_high_risk_device, 1.0);
    ASSERT_EQ(f1.accounts_on_device_count, 3.0);
    ASSERT_EQ(f1.customer_historical_risk_score, 45.0);

    // Case 2: Known trusted phone (Clean device)
    Device trusted_dev("dev_trusted_phone", "fp_clean", "192.168.1.1", "iOS", false, false, home);
    Transaction tx2("tx_d2", TransactionType::PURCHASE, "cust_dev_test", "m_01", "acc_01", 100.0, "USD",
                    std::chrono::system_clock::now(), home, "192.168.1.1", trusted_dev, "m_01", pm);

    TransactionFeatures f2 = extractor->extract(tx2);
    ASSERT_EQ(f2.is_new_device, 0.0);
    ASSERT_EQ(f2.is_high_risk_device, 0.0);
    ASSERT_EQ(f2.accounts_on_device_count, 1.0);
}

// ==========================================
// 4. Geographic Distance & Impossible Speed
// ==========================================
EPFD_TEST(FeatureSuite, GeoDistanceAndImpossibleSpeed) {
    auto cust_repo = std::make_shared<InMemoryCustomerRepository>();
    auto extractor = std::make_shared<FeatureExtractor>(nullptr, nullptr, cust_repo);

    Location home_hanoi(21.0285, 105.8542, "Hanoi", "Vietnam");
    Customer cust("cust_travel", "Le D", "led@example.com", "+84922222222", home_hanoi);
    cust_repo->save(cust);

    Location loc_hcmc(10.8231, 106.6297, "Ho Chi Minh City", "Vietnam");
    Location loc_paris(48.8566, 2.3522, "Paris", "France");

    Device dev("d1", "fp1", "1.1.1.1");
    PaymentMethod pm("pm_1", PaymentType::CREDIT_CARD, "4111111111111111", "Le D", 12, 2028);

    auto t0 = std::chrono::system_clock::now();

    // Transaction 1: in Hanoi at t0
    Transaction tx1("tx_t1", TransactionType::PURCHASE, "cust_travel", "m_01", "acc_01", 50.0, "USD",
                    t0, home_hanoi, "1.1.1.1", dev, "m_01", pm);
    extractor->updateState(tx1);

    // Transaction 2: in HCMC at t0 + 15 min (0.25h). Distance ~1150km. Speed ~4600 km/h!
    Transaction tx2("tx_t2", TransactionType::PURCHASE, "cust_travel", "m_01", "acc_01", 100.0, "USD",
                    t0 + 15min, loc_hcmc, "1.1.1.1", dev, "m_01", pm);

    TransactionFeatures f2 = extractor->extract(tx2);
    ASSERT_EQ(f2.is_new_country, 0.0); // Both in Vietnam
    ASSERT_TRUE(f2.distance_from_home_km > 1100.0 && f2.distance_from_home_km < 1200.0);
    // Speed should be ~4600 km/h (exceeds supersonic airplane speed)
    ASSERT_TRUE(f2.speed_from_last_tx_kmh > 4000.0);

    // Transaction 3: in Paris (Cross-border foreign transaction)
    Transaction tx3("tx_t3", TransactionType::PURCHASE, "cust_travel", "m_01", "acc_01", 500.0, "EUR",
                    t0 + 1h, loc_paris, "2.2.2.2", dev, "m_01", pm);
    TransactionFeatures f3 = extractor->extract(tx3);
    ASSERT_EQ(f3.is_new_country, 1.0); // France != Vietnam
    ASSERT_TRUE(f3.distance_from_home_km > 9000.0); // Hanoi to Paris > 9000 km
}

// ==========================================
// 5. Merchant Risk Categorization
// ==========================================
EPFD_TEST(FeatureSuite, MerchantRiskFeatureExtraction) {
    auto extractor = std::make_shared<FeatureExtractor>();

    Merchant normal_m("m_normal", "Supermarket", "5411", "Vietnam", 0.05);
    Merchant casino_m("m_casino", "Crypto Casino", "7995", "Malta", 0.90);

    extractor->registerMerchant(normal_m);
    extractor->registerMerchant(casino_m);

    Location loc(0, 0, "City", "Country");
    Device dev("d", "f", "1.1.1.1");
    PaymentMethod pm("pm", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);

    Transaction tx_normal("tx_m1", TransactionType::PURCHASE, "c1", "r1", "a1", 50.0, "USD",
                          std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m_normal", pm);
    TransactionFeatures f_normal = extractor->extract(tx_normal);
    ASSERT_EQ(f_normal.is_high_risk_mcc, 0.0);
    ASSERT_NEAR(f_normal.merchant_risk_rating, 0.05, 0.001);

    Transaction tx_casino("tx_m2", TransactionType::PURCHASE, "c1", "r1", "a1", 1000.0, "USD",
                         std::chrono::system_clock::now(), loc, "1.1.1.1", dev, "m_casino", pm);
    TransactionFeatures f_casino = extractor->extract(tx_casino);
    ASSERT_EQ(f_casino.is_high_risk_mcc, 1.0);
    ASSERT_NEAR(f_casino.merchant_risk_rating, 0.90, 0.001);
}
