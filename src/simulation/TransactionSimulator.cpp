#include "epfd/simulation/TransactionSimulator.hpp"
#include <iomanip>
#include <sstream>
#include <chrono>

namespace epfd {

TransactionSimulator::TransactionSimulator(SimulationConfig config)
    : config_(config),
      rng_(config.random_seed) {}

void TransactionSimulator::setConfig(const SimulationConfig& config) {
    config_ = config;
    rng_.seed(config_.random_seed);
}

std::vector<Transaction> TransactionSimulator::generateBatch() {
    return generateBatch(config_.count, config_.scenario);
}

std::vector<Transaction> TransactionSimulator::generateBatch(size_t count, SimulationScenario scenario) {
    std::vector<Transaction> batch;
    batch.reserve(count);

    if (scenario == SimulationScenario::MIXED_REALISTIC_TRAFFIC) {
        std::uniform_real_distribution<double> dist_p(0.0, 1.0);
        std::uniform_int_distribution<int> dist_attack(1, 5);

        for (size_t i = 0; i < count; ++i) {
            if (dist_p(rng_) < config_.fraud_ratio) {
                int attack_type = dist_attack(rng_);
                switch (attack_type) {
                    case 1: batch.push_back(generateBurstVelocityTransaction()); break;
                    case 2: batch.push_back(generateImpossibleTravelTransaction()); break;
                    case 3: batch.push_back(generateCardTestingTransaction()); break;
                    case 4: batch.push_back(generateMuleSmurfingTransaction()); break;
                    case 5: batch.push_back(generateAccountTakeoverTransaction()); break;
                }
            } else {
                batch.push_back(generateNormalTransaction());
            }
        }
    } else {
        for (size_t i = 0; i < count; ++i) {
            batch.push_back(generateSingleTransaction(scenario));
        }
    }

    return batch;
}

Transaction TransactionSimulator::generateSingleTransaction(SimulationScenario scenario) {
    switch (scenario) {
        case SimulationScenario::BURST_VELOCITY_ATTACK:
            return generateBurstVelocityTransaction();
        case SimulationScenario::IMPOSSIBLE_TRAVEL_ATTACK:
            return generateImpossibleTravelTransaction();
        case SimulationScenario::CARD_TESTING_ATTACK:
            return generateCardTestingTransaction();
        case SimulationScenario::MULE_SMURFING_ATTACK:
            return generateMuleSmurfingTransaction();
        case SimulationScenario::ACCOUNT_TAKEOVER_ATTACK:
            return generateAccountTakeoverTransaction();
        case SimulationScenario::NORMAL_LEGITIMATE:
        default:
            return generateNormalTransaction();
    }
}

Transaction TransactionSimulator::generateNormalTransaction() {
    std::ostringstream oss;
    oss << "tx_sim_" << std::setw(7) << std::setfill('0') << tx_counter_++;
    std::string tx_id = oss.str();

    std::uniform_real_distribution<double> dist_amt(config_.min_amount, config_.max_amount);
    std::uniform_int_distribution<int> dist_cust(1, 500);
    std::uniform_int_distribution<int> dist_mch(1, 100);

    std::string cust_id = "c_sim_" + std::to_string(dist_cust(rng_));
    std::string mch_id = "m_sim_" + std::to_string(dist_mch(rng_));
    std::string acc_id = "acc_" + cust_id;

    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("dev_sim_known", "fp_sim_clean", "192.168.1.50");
    PaymentMethod pm("pm_sim_clean", PaymentType::CREDIT_CARD, "4111111111111234", "Legit User", 12, 2028);

    return Transaction(tx_id, TransactionType::PURCHASE, cust_id, mch_id, acc_id,
                       dist_amt(rng_), "USD", std::chrono::system_clock::now(),
                       loc, "192.168.1.50", dev, mch_id, pm);
}

Transaction TransactionSimulator::generateBurstVelocityTransaction(const std::string& cust_id) {
    std::ostringstream oss;
    oss << "tx_burst_" << std::setw(7) << std::setfill('0') << tx_counter_++;
    std::string tx_id = oss.str();

    std::uniform_real_distribution<double> dist_amt(250.0, 800.0);
    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("dev_burst_1", "fp_burst_1", "10.0.0.1");
    PaymentMethod pm("pm_burst_1", PaymentType::CREDIT_CARD, "4000123456789010", "Burst Attacker", 10, 2027);

    return Transaction(tx_id, TransactionType::PURCHASE, cust_id, "m_target_1", "acc_" + cust_id,
                       dist_amt(rng_), "USD", std::chrono::system_clock::now(),
                       loc, "10.0.0.1", dev, "m_target_1", pm);
}

Transaction TransactionSimulator::generateImpossibleTravelTransaction(const std::string& cust_id) {
    std::ostringstream oss;
    oss << "tx_travel_" << std::setw(7) << std::setfill('0') << tx_counter_++;
    std::string tx_id = oss.str();

    // Cross-border coordinates (Paris, France: 48.8566, 2.3522) from Hanoi home
    Location foreign_loc(48.8566, 2.3522, "Paris", "France");
    Device foreign_dev("dev_foreign_ip", "fp_foreign_ip", "195.154.120.10");
    PaymentMethod pm("pm_travel_1", PaymentType::CREDIT_CARD, "5105105105105100", "Travel Target", 11, 2029);

    return Transaction(tx_id, TransactionType::PURCHASE, cust_id, "m_paris_store", "acc_" + cust_id,
                       1250.0, "USD", std::chrono::system_clock::now(),
                       foreign_loc, "195.154.120.10", foreign_dev, "m_paris_store", pm);
}

Transaction TransactionSimulator::generateCardTestingTransaction(const std::string& cust_id) {
    std::ostringstream oss;
    oss << "tx_test_" << std::setw(7) << std::setfill('0') << tx_counter_++;
    std::string tx_id = oss.str();

    std::uniform_real_distribution<double> dist_micro(1.0, 4.5);
    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Device dev("dev_botnet_test", "fp_botnet", "45.33.32.156");
    PaymentMethod pm("pm_test_card", PaymentType::CREDIT_CARD, "4242424242424242", "Bot Testing", 5, 2028);

    return Transaction(tx_id, TransactionType::PURCHASE, cust_id, "m_digital_goods", "acc_" + cust_id,
                       dist_micro(rng_), "USD", std::chrono::system_clock::now(),
                       loc, "45.33.32.156", dev, "m_digital_goods", pm);
}

Transaction TransactionSimulator::generateMuleSmurfingTransaction(const std::string& cust_id) {
    std::ostringstream oss;
    oss << "tx_mule_" << std::setw(7) << std::setfill('0') << tx_counter_++;
    std::string tx_id = oss.str();

    std::uniform_real_distribution<double> dist_aml(8500.0, 9850.0); // Below $10k threshold
    Location loc(10.8231, 106.6297, "Ho Chi Minh City", "Vietnam");
    Device dev("dev_mule_layer", "fp_mule_layer", "118.69.180.22");
    PaymentMethod pm("pm_mule_wire", PaymentType::BANK_TRANSFER, "9876543210123456", "Mule Target", 1, 2030);

    return Transaction(tx_id, TransactionType::TRANSFER, cust_id, "c_mule_dest", "acc_" + cust_id,
                       dist_aml(rng_), "USD", std::chrono::system_clock::now(),
                       loc, "118.69.180.22", dev, "c_mule_dest", pm);
}

Transaction TransactionSimulator::generateAccountTakeoverTransaction(const std::string& cust_id) {
    std::ostringstream oss;
    oss << "tx_ato_" << std::setw(7) << std::setfill('0') << tx_counter_++;
    std::string tx_id = oss.str();

    Location foreign_loc(55.7558, 37.6173, "Moscow", "Russia");
    Device rooted_dev("dev_ato_rooted", "fp_ato_rooted", "185.220.101.5", "Android Emulator 13.0", true, true);
    PaymentMethod pm("pm_ato_card", PaymentType::CREDIT_CARD, "378282246310005", "Compromised Customer", 8, 2026);

    return Transaction(tx_id, TransactionType::PURCHASE, cust_id, "m_crypto_exchange", "acc_" + cust_id,
                       4200.0, "USD", std::chrono::system_clock::now(),
                       foreign_loc, "185.220.101.5", rooted_dev, "m_crypto_exchange", pm);
}

void TransactionSimulator::streamTransactions(size_t count,
                                              SimulationScenario scenario,
                                              const std::function<void(const Transaction&)>& consumer) {
    auto batch = generateBatch(count, scenario);
    for (const auto& tx : batch) {
        if (consumer) {
            consumer(tx);
        }
    }
}

} // namespace epfd
