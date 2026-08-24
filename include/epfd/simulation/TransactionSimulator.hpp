#ifndef EPFD_SIMULATION_TRANSACTION_SIMULATOR_HPP
#define EPFD_SIMULATION_TRANSACTION_SIMULATOR_HPP

#include <vector>
#include <string>
#include <random>
#include <functional>
#include "epfd/models/Transaction.hpp"
#include "epfd/models/Customer.hpp"
#include "epfd/models/Device.hpp"
#include "epfd/models/Location.hpp"
#include "epfd/models/PaymentMethod.hpp"

namespace epfd {

enum class SimulationScenario {
    NORMAL_LEGITIMATE,
    BURST_VELOCITY_ATTACK,
    IMPOSSIBLE_TRAVEL_ATTACK,
    CARD_TESTING_ATTACK,
    MULE_SMURFING_ATTACK,
    ACCOUNT_TAKEOVER_ATTACK,
    MIXED_REALISTIC_TRAFFIC
};

struct SimulationConfig {
    SimulationScenario scenario{SimulationScenario::NORMAL_LEGITIMATE};
    size_t count{100};
    double min_amount{10.0};
    double max_amount{500.0};
    double fraud_ratio{0.03}; // For mixed traffic
    unsigned int random_seed{42};
};

/**
 * @brief Synthetic Transaction Generator for Stress Benchmarks, Live Stream Ingestion, and Rule Testing.
 */
class TransactionSimulator {
public:
    explicit TransactionSimulator(SimulationConfig config = SimulationConfig{});

    std::vector<Transaction> generateBatch();
    std::vector<Transaction> generateBatch(size_t count, SimulationScenario scenario);

    Transaction generateSingleTransaction(SimulationScenario scenario);
    Transaction generateNormalTransaction();
    Transaction generateBurstVelocityTransaction(const std::string& cust_id = "c_burst_sim");
    Transaction generateImpossibleTravelTransaction(const std::string& cust_id = "c_travel_sim");
    Transaction generateCardTestingTransaction(const std::string& cust_id = "c_test_sim");
    Transaction generateMuleSmurfingTransaction(const std::string& cust_id = "c_mule_sim");
    Transaction generateAccountTakeoverTransaction(const std::string& cust_id = "c_ato_sim");

    void streamTransactions(size_t count,
                            SimulationScenario scenario,
                            const std::function<void(const Transaction&)>& consumer);

    void setConfig(const SimulationConfig& config);
    const SimulationConfig& getConfig() const noexcept { return config_; }

private:
    SimulationConfig config_;
    std::mt19937 rng_;
    size_t tx_counter_{1};
};

} // namespace epfd

#endif // EPFD_SIMULATION_TRANSACTION_SIMULATOR_HPP
