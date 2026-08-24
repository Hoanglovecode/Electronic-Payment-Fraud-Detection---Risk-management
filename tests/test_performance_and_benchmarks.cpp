#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace epfd;

// ============================================================================
// Phase 19: Performance Optimization & Empirical Micro-benchmarking Suite
// ============================================================================

EPFD_TEST(BenchmarkSuite, EndToEndProcessingLatencyAndThroughput) {
    auto tx_repo = std::make_shared<InMemoryTransactionRepository>();
    auto cust_repo = std::make_shared<InMemoryCustomerRepository>();
    auto acc_repo = std::make_shared<InMemoryAccountRepository>();
    auto validator = std::make_shared<TransactionValidator>(tx_repo, acc_repo);
    TransactionService service(validator, tx_repo, acc_repo, cust_repo);

    auto lookup_index = std::make_shared<FastLookupIndex>();
    auto fraud_engine = FraudDetectorEngine::createDefaultEngine(lookup_index);
    auto extractor = std::make_shared<FeatureExtractor>();
    auto risk_policy = std::make_shared<StandardWeightedRiskPolicy>();
    auto risk_aggregator = std::make_shared<RiskAggregator>();
    auto ml_predictor = std::make_shared<MockModelPredictor>("MockRiskMLModel", "v1.0", 18, true, 0.05);

    auto risk_engine = std::make_shared<RiskEngine>(risk_policy, risk_aggregator, ml_predictor, fraud_engine, extractor);
    auto decision_engine = std::make_shared<DecisionEngine>(std::make_shared<StandardDecisionPolicy>(), risk_engine);

    TransactionSimulator sim;
    const size_t N_TX = 1000;
    auto batch = sim.generateBatch(N_TX, SimulationScenario::MIXED_REALISTIC_TRAFFIC);

    // Seed database
    for (const auto& tx : batch) {
        if (!acc_repo->findById(tx.getAccountId()).has_value()) {
            acc_repo->save(Account(tx.getAccountId(), tx.getCustomerId(), 100000.0, "USD"));
        }
        if (!cust_repo->findById(tx.getCustomerId()).has_value()) {
            cust_repo->save(Customer(tx.getCustomerId(), "Bench User", "bench@bank.com", "0900000000"));
        }
    }

    BenchmarkTimer timer;

    for (const auto& tx : batch) {
        auto t_start = std::chrono::high_resolution_clock::now();

        // 1. Validation & Persistence
        auto svc_res = service.processTransaction(tx);
        (void)svc_res;

        // 2. Feature Extraction & Risk Decision
        auto dec_res = decision_engine->evaluate(tx);
        (void)dec_res;

        auto t_end = std::chrono::high_resolution_clock::now();
        double elapsed_us = std::chrono::duration<double, std::micro>(t_end - t_start).count();
        timer.recordMicroseconds(elapsed_us);
    }

    auto stats = timer.computeStats();
    std::cout << "\n[BENCHMARK] End-to-End Transaction Pipeline:" << std::endl;
    std::cout << "  " << stats.toString() << std::endl;

    ASSERT_EQ(stats.sample_count, N_TX);
    ASSERT_TRUE(stats.p99_us < 5000.0); // P99 sub-5ms SLA
    ASSERT_TRUE(stats.tps > 500.0);     // High throughput
}

EPFD_TEST(BenchmarkSuite, LinearSearchVsCustomHashMapLookup) {
    const size_t NUM_ITEMS = 5000;
    const size_t NUM_LOOKUPS = 2000;

    std::vector<std::pair<std::string, std::string>> linear_table;
    dsa::HashMap<std::string, std::string> custom_map;

    for (size_t i = 0; i < NUM_ITEMS; ++i) {
        std::string key = "key_card_" + std::to_string(i);
        std::string val = "customer_id_" + std::to_string(i);
        linear_table.emplace_back(key, val);
        custom_map.insert(key, val);
    }

    // 1. Linear Search Benchmark (O(N))
    auto t0 = std::chrono::high_resolution_clock::now();
    size_t linear_found = 0;
    for (size_t i = 0; i < NUM_LOOKUPS; ++i) {
        std::string target_key = "key_card_" + std::to_string((i * 7) % NUM_ITEMS);
        for (const auto& item : linear_table) {
            if (item.first == target_key) {
                linear_found++;
                break;
            }
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double linear_time_us = std::chrono::duration<double, std::micro>(t1 - t0).count();

    // 2. Custom HashMap Benchmark (O(1))
    auto t2 = std::chrono::high_resolution_clock::now();
    size_t map_found = 0;
    for (size_t i = 0; i < NUM_LOOKUPS; ++i) {
        std::string target_key = "key_card_" + std::to_string((i * 7) % NUM_ITEMS);
        if (custom_map.contains(target_key)) {
            map_found++;
        }
    }
    auto t3 = std::chrono::high_resolution_clock::now();
    double map_time_us = std::chrono::duration<double, std::micro>(t3 - t2).count();

    std::cout << "\n[BENCHMARK] Linear Search vs Custom HashMap (" << NUM_LOOKUPS << " lookups over " << NUM_ITEMS << " items):" << std::endl;
    std::cout << "  - Linear Search (O(N)): " << linear_time_us << " us (" << linear_time_us / NUM_LOOKUPS << " us/lookup)" << std::endl;
    std::cout << "  - Custom HashMap (O(1)): " << map_time_us << " us (" << map_time_us / NUM_LOOKUPS << " us/lookup)" << std::endl;
    std::cout << "  - Speedup Ratio: " << (linear_time_us / (map_time_us > 0 ? map_time_us : 1.0)) << "x" << std::endl;

    ASSERT_EQ(linear_found, NUM_LOOKUPS);
    ASSERT_EQ(map_found, NUM_LOOKUPS);
    ASSERT_TRUE(map_time_us < linear_time_us);
}

EPFD_TEST(BenchmarkSuite, FullSortVsTopKPriorityQueue) {
    const size_t NUM_ALERTS = 10000;
    const size_t TOP_K = 10;

    std::vector<std::pair<double, std::string>> alert_list;
    alert_list.reserve(NUM_ALERTS);

    for (size_t i = 0; i < NUM_ALERTS; ++i) {
        double score = static_cast<double>((i * 37) % 1000) / 10.0;
        alert_list.emplace_back(score, "alert_" + std::to_string(i));
    }

    // 1. Full Sort Benchmark (O(N log N))
    auto t0 = std::chrono::high_resolution_clock::now();
    auto sorted_copy = alert_list;
    std::sort(sorted_copy.begin(), sorted_copy.end(), [](const auto& a, const auto& b) {
        return a.first > b.first; // Descending
    });
    std::vector<std::string> top_k_full;
    for (size_t i = 0; i < TOP_K; ++i) {
        top_k_full.push_back(sorted_copy[i].second);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double full_sort_us = std::chrono::duration<double, std::micro>(t1 - t0).count();

    // 2. Custom Min-Heap Top-K Benchmark (O(N log K))
    auto t2 = std::chrono::high_resolution_clock::now();
    dsa::PriorityQueue<std::pair<double, std::string>, std::greater<std::pair<double, std::string>>> min_heap;

    for (const auto& item : alert_list) {
        if (min_heap.size() < TOP_K) {
            min_heap.push(item);
        } else if (item.first > min_heap.top().first) {
            min_heap.pop();
            min_heap.push(item);
        }
    }
    auto t3 = std::chrono::high_resolution_clock::now();
    double top_k_heap_us = std::chrono::duration<double, std::micro>(t3 - t2).count();

    std::cout << "\n[BENCHMARK] Full Sort O(N log N) vs Top-K Heap O(N log K) (" << NUM_ALERTS << " alerts, Top-" << TOP_K << "):" << std::endl;
    std::cout << "  - Full std::sort: " << full_sort_us << " us" << std::endl;
    std::cout << "  - Top-K Min-Heap: " << top_k_heap_us << " us" << std::endl;
    std::cout << "  - Speedup Ratio: " << (full_sort_us / (top_k_heap_us > 0 ? top_k_heap_us : 1.0)) << "x" << std::endl;

    ASSERT_EQ(min_heap.size(), TOP_K);
    ASSERT_TRUE(top_k_heap_us <= full_sort_us || top_k_heap_us < 5000.0);
}

EPFD_TEST(BenchmarkSuite, SlidingWindowVsRecalculateFromScratch) {
    const size_t NUM_EVENTS = 2000;
    auto t_base = std::chrono::system_clock::now();

    // 1. Sliding Window Deque (O(1) amortized update and sum)
    TimeWindowBuffer window_5m(std::chrono::minutes(5));

    auto t0 = std::chrono::high_resolution_clock::now();
    double total_slide_sum = 0.0;
    for (size_t i = 0; i < NUM_EVENTS; ++i) {
        auto t_event = t_base + std::chrono::seconds(i * 2);
        window_5m.add(t_event, 10.0 + (i % 50));
        total_slide_sum += window_5m.getSum(t_event);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double sliding_window_us = std::chrono::duration<double, std::micro>(t1 - t0).count();

    // 2. Full History Iteration (O(N) per event)
    std::vector<std::pair<std::chrono::system_clock::time_point, double>> history;
    auto t2 = std::chrono::high_resolution_clock::now();
    double total_recalc_sum = 0.0;
    for (size_t i = 0; i < NUM_EVENTS; ++i) {
        auto t_event = t_base + std::chrono::seconds(i * 2);
        history.emplace_back(t_event, 10.0 + (i % 50));

        // Scan full history to find elements in [t_event - 5min, t_event]
        auto window_start = t_event - std::chrono::minutes(5);
        double cur_sum = 0.0;
        for (const auto& h : history) {
            if (h.first >= window_start && h.first <= t_event) {
                cur_sum += h.second;
            }
        }
        total_recalc_sum += cur_sum;
    }
    auto t3 = std::chrono::high_resolution_clock::now();
    double full_scan_us = std::chrono::duration<double, std::micro>(t3 - t2).count();

    std::cout << "\n[BENCHMARK] Sliding Window Deque O(1) vs Full Scan O(N) (" << NUM_EVENTS << " sliding updates):" << std::endl;
    std::cout << "  - Sliding Window Deque (O(1)): " << sliding_window_us << " us" << std::endl;
    std::cout << "  - Full History Scan (O(N)):    " << full_scan_us << " us" << std::endl;
    std::cout << "  - Speedup Ratio: " << (full_scan_us / (sliding_window_us > 0 ? sliding_window_us : 1.0)) << "x" << std::endl;

    ASSERT_NEAR(total_slide_sum, total_recalc_sum, 0.01);
    ASSERT_TRUE(sliding_window_us < full_scan_us);
}
