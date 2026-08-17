#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <chrono>
#include <thread>

using namespace epfd;
using namespace std::chrono_literals;

// ==========================================
// 1. Sliding Window Buffer Suite
// ==========================================
EPFD_TEST(DsaSuite, TimeWindowBufferSliding) {
    TimeWindowBuffer buffer(10s);
    auto now = std::chrono::system_clock::now();

    buffer.add(now - 12s, 100.0); // Outside window
    buffer.add(now - 8s, 50.0);   // Inside window
    buffer.add(now - 4s, 150.0);  // Inside window
    buffer.add(now, 200.0);       // Inside window

    ASSERT_EQ(buffer.getCount(now), 3);
    ASSERT_EQ(buffer.getSum(now), 400.0);
    ASSERT_NEAR(buffer.getAverage(now), 400.0 / 3.0, 0.001);
    ASSERT_EQ(buffer.getMax(now), 200.0);

    // Advance time past all entries
    auto future = now + 15s;
    ASSERT_EQ(buffer.getCount(future), 0);
    ASSERT_EQ(buffer.getSum(future), 0.0);
}

// ==========================================
// 2. Customer Velocity Tracker Suite
// ==========================================
EPFD_TEST(DsaSuite, CustomerVelocityMultiTier) {
    CustomerVelocityTracker tracker;
    auto now = std::chrono::system_clock::now();
    std::string cust_id = "cust_velocity_1";

    // 2 transactions in last 2 minutes
    tracker.recordTransaction(cust_id, now - 2min, 100.0);
    tracker.recordTransaction(cust_id, now - 1min, 200.0);

    // 1 transaction 30 minutes ago (within 1h and 24h, outside 5m)
    tracker.recordTransaction(cust_id, now - 30min, 300.0);

    // 1 transaction 5 hours ago (within 24h, outside 1h and 5m)
    tracker.recordTransaction(cust_id, now - 5h, 400.0);

    auto m = tracker.getMetrics(cust_id, now);
    ASSERT_EQ(m.count_5m, 2);
    ASSERT_EQ(m.sum_5m, 300.0);

    ASSERT_EQ(m.count_1h, 3);
    ASSERT_EQ(m.sum_1h, 600.0);

    ASSERT_EQ(m.count_24h, 4);
    ASSERT_EQ(m.sum_24h, 1000.0);
    ASSERT_NEAR(m.average_amount_24h, 250.0, 0.001);
}

// ==========================================
// 3. Fast Lookup Index Suite
// ==========================================
EPFD_TEST(DsaSuite, FastLookupIndexAssociationsAndLists) {
    FastLookupIndex index;

    // Device sharing across 3 accounts (Bot farm / Card testing signal)
    index.recordDeviceUsage("dev_fingerprint_abc", "cust_1");
    index.recordDeviceUsage("dev_fingerprint_abc", "cust_2");
    index.recordDeviceUsage("dev_fingerprint_abc", "cust_3");

    ASSERT_EQ(index.getAccountCountOnDevice("dev_fingerprint_abc"), 3);
    ASSERT_EQ(index.getAccountCountOnDevice("dev_clean"), 0);

    // Blacklist & Whitelist
    index.addBlacklist("IP:185.220.101.5");
    index.addWhitelist("MERCHANT:M_TRUSTED_99");

    ASSERT_TRUE(index.isBlacklisted("IP:185.220.101.5"));
    ASSERT_FALSE(index.isBlacklisted("IP:127.0.0.1"));

    ASSERT_TRUE(index.isWhitelisted("MERCHANT:M_TRUSTED_99"));
    ASSERT_FALSE(index.isWhitelisted("MERCHANT:M_UNKNOWN"));

    index.removeBlacklist("IP:185.220.101.5");
    ASSERT_FALSE(index.isBlacklisted("IP:185.220.101.5"));
}

// ==========================================
// 4. Priority Queue Triaging Suite
// ==========================================
EPFD_TEST(DsaSuite, InvestigationPriorityQueueOrdering) {
    InvestigationPriorityQueue pq;

    InvestigationCase c_low{"c1", "tx1", "u1", 20.0, 50.0, RiskLevel::LOW};
    InvestigationCase c_crit{"c2", "tx2", "u2", 95.0, 5000.0, RiskLevel::CRITICAL};
    InvestigationCase c_med{"c3", "tx3", "u3", 55.0, 200.0, RiskLevel::MEDIUM};
    InvestigationCase c_high{"c4", "tx4", "u4", 80.0, 1500.0, RiskLevel::HIGH};

    pq.push(c_low);
    pq.push(c_crit);
    pq.push(c_med);
    pq.push(c_high);

    ASSERT_EQ(pq.size(), 4);

    InvestigationCase top_case;
    // 1st pop must be CRITICAL
    ASSERT_TRUE(pq.pop(top_case));
    ASSERT_EQ(top_case.case_id, "c2");
    ASSERT_EQ(top_case.severity, RiskLevel::CRITICAL);

    // 2nd pop must be HIGH
    ASSERT_TRUE(pq.pop(top_case));
    ASSERT_EQ(top_case.case_id, "c4");
    ASSERT_EQ(top_case.severity, RiskLevel::HIGH);

    // 3rd pop must be MEDIUM
    ASSERT_TRUE(pq.pop(top_case));
    ASSERT_EQ(top_case.case_id, "c3");

    // 4th pop must be LOW
    ASSERT_TRUE(pq.pop(top_case));
    ASSERT_EQ(top_case.case_id, "c1");

    ASSERT_TRUE(pq.empty());
}

// ==========================================
// 5. Fraud Ring Graph Suite
// ==========================================
EPFD_TEST(DsaSuite, FraudRingGraphConnectedComponents) {
    FraudRingGraph graph;

    // Ring network: CustA - Dev1 - CustB - CardX - CustC
    graph.addEdge("CustA", NodeType::CUSTOMER, "Dev1", NodeType::DEVICE);
    graph.addEdge("CustB", NodeType::CUSTOMER, "Dev1", NodeType::DEVICE);
    graph.addEdge("CustB", NodeType::CUSTOMER, "CardX", NodeType::CARD);
    graph.addEdge("CustC", NodeType::CUSTOMER, "CardX", NodeType::CARD);
    graph.addEdge("CustD", NodeType::CUSTOMER, "Dev1", NodeType::DEVICE); // 3rd customer on Dev1

    // Total unique nodes: CustA, CustB, CustC, CustD, Dev1, CardX = 6
    ASSERT_EQ(graph.nodeCount(), 6);
    ASSERT_EQ(graph.edgeCount(), 5);

    // Dev1 is shared by CustA, CustB, CustD (>= 3 customers)
    ASSERT_TRUE(graph.isSharedSuspiciously("Dev1", 3));
    ASSERT_FALSE(graph.isSharedSuspiciously("CardX", 3)); // Only CustB, CustC = 2

    // BFS Component Discovery from CustA
    auto ring = graph.findConnectedComponent("CustA", 5);
    ASSERT_EQ(ring.size(), 6); // Entire connected component found!
    ASSERT_TRUE(ring.find("CustA") != ring.end());
    ASSERT_TRUE(ring.find("CustB") != ring.end());
    ASSERT_TRUE(ring.find("CustC") != ring.end());
    ASSERT_TRUE(ring.find("CustD") != ring.end());
    ASSERT_TRUE(ring.find("Dev1") != ring.end());
}

// ==========================================
// 6. Risk Ranking & Binary Search Suite
// ==========================================
EPFD_TEST(DsaSuite, RiskRankingAndBinarySearchTimeRange) {
    // 1. Risk sorting test
    std::vector<RiskAssessment> assessments;
    assessments.emplace_back("a1", "tx1", 20.0, 10.0, 30.0, RiskLevel::LOW, DecisionAction::APPROVE);
    assessments.emplace_back("a2", "tx2", 90.0, 95.0, 92.5, RiskLevel::CRITICAL, DecisionAction::BLOCK);
    assessments.emplace_back("a3", "tx3", 60.0, 70.0, 65.0, RiskLevel::HIGH, DecisionAction::REVIEW);

    RiskRankingUtils::sortByRiskDescending(assessments);
    ASSERT_EQ(assessments[0].getAssessmentId(), "a2"); // 92.5
    ASSERT_EQ(assessments[1].getAssessmentId(), "a3"); // 65.0
    ASSERT_EQ(assessments[2].getAssessmentId(), "a1"); // 30.0

    // 2. Binary search time-series range test
    auto t0 = std::chrono::system_clock::now();
    Location loc(0, 0, "City", "Country");
    Device dev("d", "f", "1.1.1.1");
    PaymentMethod pm("pm", PaymentType::CREDIT_CARD, "4111111111111111", "Name", 12, 2028);

    std::vector<Transaction> history;
    history.emplace_back("tx_10", TransactionType::PURCHASE, "u", "m", "a", 10.0, "USD", t0 + 10s, loc, "1.1.1.1", dev, "m", pm);
    history.emplace_back("tx_20", TransactionType::PURCHASE, "u", "m", "a", 20.0, "USD", t0 + 20s, loc, "1.1.1.1", dev, "m", pm);
    history.emplace_back("tx_30", TransactionType::PURCHASE, "u", "m", "a", 30.0, "USD", t0 + 30s, loc, "1.1.1.1", dev, "m", pm);
    history.emplace_back("tx_40", TransactionType::PURCHASE, "u", "m", "a", 40.0, "USD", t0 + 40s, loc, "1.1.1.1", dev, "m", pm);
    history.emplace_back("tx_50", TransactionType::PURCHASE, "u", "m", "a", 50.0, "USD", t0 + 50s, loc, "1.1.1.1", dev, "m", pm);

    // Find transactions in range [t0 + 15s, t0 + 45s] -> tx_20, tx_30, tx_40
    auto results = RiskRankingUtils::findTransactionsInTimeRange(history, t0 + 15s, t0 + 45s);
    ASSERT_EQ(results.size(), 3);
    ASSERT_EQ(results[0].getTransactionId(), "tx_20");
    ASSERT_EQ(results[1].getTransactionId(), "tx_30");
    ASSERT_EQ(results[2].getTransactionId(), "tx_40");
}
