# EPFD-RAS C++ Core API Reference

## 1. Umbrella Header & Namespace

Include the entire engine via a single umbrella header:
```cpp
#include "epfd/epfd.hpp"
using namespace epfd;
```

---

## 2. Core Service & Evaluation API

### `TransactionValidator`
```cpp
class TransactionValidator {
public:
    TransactionValidator(std::shared_ptr<ITransactionRepository> tx_repo,
                         std::shared_ptr<IAccountRepository> acc_repo);

    ValidationResult validate(const Transaction& tx);
};
```

### `FeatureExtractor`
```cpp
class FeatureExtractor {
public:
    FeatureExtractor() = default;
    TransactionFeatures extract(const Transaction& tx, 
                                const std::vector<Transaction>& customer_history = {});
};
```

### `FraudDetectorEngine`
```cpp
class FraudDetectorEngine {
public:
    static std::shared_ptr<FraudDetectorEngine> createDefaultEngine(
        std::shared_ptr<FastLookupIndex> lookup_index = nullptr);

    void addRule(std::shared_ptr<IFraudRule> rule);
    std::vector<FraudAlert> detect(const Transaction& tx, const TransactionFeatures& features);
};
```

### `RiskEngine`
```cpp
class RiskEngine {
public:
    RiskEngine(std::shared_ptr<IRiskPolicy> policy,
               std::shared_ptr<RiskAggregator> aggregator,
               std::shared_ptr<IModelPredictor> ml_predictor,
               std::shared_ptr<FraudDetectorEngine> fraud_engine,
               std::shared_ptr<FeatureExtractor> extractor);

    RiskAssessment assessRisk(const Transaction& tx, 
                              const std::vector<Transaction>& customer_history = {});
};
```

### `DecisionEngine`
```cpp
class DecisionEngine {
public:
    DecisionEngine(std::shared_ptr<IDecisionPolicy> policy,
                   std::shared_ptr<RiskEngine> risk_engine);

    DecisionResult evaluate(const Transaction& tx);
    void addObserver(std::shared_ptr<IDecisionObserver> observer);
};
```

### `CaseManager` & `LabelStore`
```cpp
class CaseManager {
public:
    CaseManager(std::shared_ptr<LabelStore> label_store,
                std::shared_ptr<OutcomeTracker> outcome_tracker);

    std::string createCase(const std::string& tx_id, const std::string& cust_id,
                           double risk_score, DecisionAction original_decision);
    bool assignCase(const std::string& case_id, const std::string& analyst_id);
    bool addEvidence(const std::string& case_id, const std::string& evidence);
    bool resolveCase(const std::string& case_id, CaseStatus resolution,
                     const std::string& resolution_summary, const TransactionFeatures& features);
};
```

### `SecurityUtils`
```cpp
class SecurityUtils {
public:
    static std::string maskPan(const std::string& raw_pan);
    static std::string maskPanBinAndLast4(const std::string& raw_pan);
    static std::string redactCvv(const std::string& raw_payload);
    static std::string maskEmail(const std::string& email);
    static std::string maskIp(const std::string& ip);
};
```
