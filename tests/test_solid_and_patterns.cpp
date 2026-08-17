#include "test_framework.hpp"
#include "epfd/epfd.hpp"
#include <memory>
#include <vector>

using namespace epfd;

// ==========================================
// 1. Concrete Rules for Strategy Pattern Test
// ==========================================
class DummyLargeAmountRule : public BaseFraudRule {
public:
    DummyLargeAmountRule(double threshold = 5000.0, double weight = 25.0)
        : BaseFraudRule("R_LARGE_AMT", "LargeAmountRule", FraudRuleCategory::AMOUNT_DEVIATION, weight),
          threshold_(threshold) {}

    std::optional<FraudAlert> evaluate(const Transaction& tx) const override {
        if (!isEnabled()) return std::nullopt;
        if (tx.getAmount() >= threshold_) {
            return FraudAlert("alt_" + tx.getTransactionId(),
                              tx.getTransactionId(),
                              id_,
                              name_,
                              category_,
                              weight_,
                              "Transaction amount $" + std::to_string(tx.getAmount()) + " exceeds threshold $" + std::to_string(threshold_),
                              RiskLevel::HIGH);
        }
        return std::nullopt;
    }

private:
    double threshold_;
};

class DummyCrossBorderRule : public BaseFraudRule {
public:
    DummyCrossBorderRule(std::string home_country = "Vietnam", double weight = 15.0)
        : BaseFraudRule("R_CROSS_BORDER", "CrossBorderRule", FraudRuleCategory::GEO_LOCATION, weight),
          home_country_(std::move(home_country)) {}

    std::optional<FraudAlert> evaluate(const Transaction& tx) const override {
        if (!isEnabled()) return std::nullopt;
        if (tx.isCrossBorder(home_country_)) {
            return FraudAlert("alt_cb_" + tx.getTransactionId(),
                              tx.getTransactionId(),
                              id_,
                              name_,
                              category_,
                              weight_,
                              "Transaction country " + tx.getLocation().getCountry() + " differs from home " + home_country_,
                              RiskLevel::MEDIUM);
        }
        return std::nullopt;
    }

private:
    std::string home_country_;
};

// ==========================================
// 2. Strategy & OCP Test
// ==========================================
EPFD_TEST(SolidSuite, StrategyPatternAndOCP) {
    std::vector<std::unique_ptr<IFraudRule>> rules;
    rules.push_back(std::make_unique<DummyLargeAmountRule>(1000.0, 30.0));
    rules.push_back(std::make_unique<DummyCrossBorderRule>("Vietnam", 20.0));

    Location loc_foreign(48.8566, 2.3522, "Paris", "France");
    Device dev("d1", "fp1", "1.1.1.1", "Chrome", false, false, loc_foreign);
    PaymentMethod pm("pm1", PaymentType::CREDIT_CARD, "4111111111111111", "User A", 12, 2028);

    // Large foreign transaction (Triggers both rules)
    Transaction tx1("tx_solid_1", TransactionType::PURCHASE, "cust_01", "m_01", "acc_01", 1500.0, "USD",
                    std::chrono::system_clock::now(), loc_foreign, "1.1.1.1", dev, "m_01", pm);

    std::vector<FraudAlert> alerts;
    for (const auto& rule : rules) {
        auto res = rule->evaluate(tx1);
        if (res.has_value()) {
            alerts.push_back(*res);
        }
    }

    ASSERT_EQ(alerts.size(), 2);
    ASSERT_EQ(alerts[0].getRuleId(), "R_LARGE_AMT");
    ASSERT_EQ(alerts[1].getRuleId(), "R_CROSS_BORDER");

    // Disable first rule and re-evaluate
    rules[0]->setEnabled(false);
    alerts.clear();
    for (const auto& rule : rules) {
        auto res = rule->evaluate(tx1);
        if (res.has_value()) {
            alerts.push_back(*res);
        }
    }
    ASSERT_EQ(alerts.size(), 1);
    ASSERT_EQ(alerts[0].getRuleId(), "R_CROSS_BORDER");
}

// ==========================================
// 3. Dependency Inversion (Repository DIP)
// ==========================================
EPFD_TEST(SolidSuite, DependencyInversionRepositories) {
    std::unique_ptr<ITransactionRepository> txRepo = std::make_unique<InMemoryTransactionRepository>();
    std::unique_ptr<ICustomerRepository> custRepo = std::make_unique<InMemoryCustomerRepository>();

    Location loc(21.0285, 105.8542, "Hanoi", "Vietnam");
    Customer cust("cust_solid_1", "Le Van C", "levanc@example.com", "+84988888888", loc, true, false);
    cust.setRiskScore(75.0); // HIGH risk

    custRepo->save(cust);
    ASSERT_EQ(custRepo->count(), 1);

    auto retrieved_cust = custRepo->findById("cust_solid_1");
    ASSERT_TRUE(retrieved_cust.has_value());
    ASSERT_EQ(retrieved_cust->getFullName(), "Le Van C");

    auto high_risk_customers = custRepo->findByRiskLevel(RiskLevel::HIGH);
    ASSERT_EQ(high_risk_customers.size(), 1);

    // Save Transactions
    Device dev("d1", "fp1", "192.168.1.1", "Mozilla", false, false, loc);
    PaymentMethod pm("pm1", PaymentType::CREDIT_CARD, "4111111111111111", "Le Van C", 10, 2027);

    Transaction tx("tx_dip_100", TransactionType::PURCHASE, "cust_solid_1", "m_01", "acc_01", 300.0, "USD",
                    std::chrono::system_clock::now(), loc, "192.168.1.1", dev, "m_01", pm);

    txRepo->save(tx);
    ASSERT_EQ(txRepo->count(), 1);

    auto cust_txs = txRepo->findByCustomerId("cust_solid_1");
    ASSERT_EQ(cust_txs.size(), 1);
    ASSERT_EQ(cust_txs[0].getAmount(), 300.0);
}

// ==========================================
// 4. ML Predictor Abstraction & Failure Policy
// ==========================================
EPFD_TEST(SolidSuite, MLPredictorAbstractionAndResilience) {
    std::unique_ptr<IModelPredictor> predictor = std::make_unique<MockModelPredictor>("MockRandomForest", "2.1.0", 5, true, 0.85);

    ASSERT_TRUE(predictor->isReady());
    ASSERT_EQ(predictor->getModelName(), "MockRandomForest");
    ASSERT_EQ(predictor->getExpectedFeatureCount(), 5);

    std::vector<double> valid_features = {1.5, 300.0, 1.0, 0.0, 12.0};
    auto pred = predictor->predict(valid_features);
    ASSERT_TRUE(pred.is_success);
    ASSERT_NEAR(pred.probability, 0.85, 0.001);

    // Feature dimension mismatch
    std::vector<double> invalid_features = {1.5, 300.0};
    auto pred_invalid = predictor->predict(invalid_features);
    ASSERT_FALSE(pred_invalid.is_success);

    // Simulated runtime error
    auto mock_ptr = dynamic_cast<MockModelPredictor*>(predictor.get());
    mock_ptr->setShouldSimulateError(true);
    auto pred_error = predictor->predict(valid_features);
    ASSERT_FALSE(pred_error.is_success);
}

// ==========================================
// 5. Observer Pattern
// ==========================================
struct FraudAlertEvent {
    std::string transaction_id;
    std::string rule_name;
    double score;
};

class TestAuditObserver : public IObserver<FraudAlertEvent> {
public:
    void onNotify(const FraudAlertEvent& event) override {
        notified_events.push_back(event);
    }
    std::vector<FraudAlertEvent> notified_events;
};

EPFD_TEST(SolidSuite, ObserverPatternEventPublishing) {
    Observable<FraudAlertEvent> fraudPublisher;
    auto observer = std::make_shared<TestAuditObserver>();

    fraudPublisher.subscribe(observer);
    ASSERT_EQ(fraudPublisher.observerCount(), 1);

    FraudAlertEvent evt{"tx_obs_1", "HighVelocityRule", 45.0};
    fraudPublisher.notifyAll(evt);

    ASSERT_EQ(observer->notified_events.size(), 1);
    ASSERT_EQ(observer->notified_events[0].transaction_id, "tx_obs_1");
    ASSERT_EQ(observer->notified_events[0].score, 45.0);

    fraudPublisher.unsubscribe(observer);
    ASSERT_EQ(fraudPublisher.observerCount(), 0);
}
