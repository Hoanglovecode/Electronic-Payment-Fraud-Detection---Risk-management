# EPFD-RAS Design Patterns, SOLID Principles & Custom DSA

## 1. Object-Oriented Programming (OOP) & SOLID Design Principles

EPFD-RAS is architected with strict adherence to the **SOLID** principles:

### A. Single Responsibility Principle (SRP)
- `TransactionValidator` is exclusively responsible for data integrity, balance checks, and signature schema validation.
- `FeatureExtractor` only extracts numerical and behavioral features.
- `FraudDetectorEngine` only coordinates rule evaluations.
- `RiskEngine` calculates multi-factor composite scores without making business routing decisions.
- `DecisionEngine` maps scores to business actions (`APPROVE`, `REVIEW`, `CHALLENGE_3DS`, `BLOCK`).

### B. Open/Closed Principle (OCP)
- **Rules Extension**: Adding new fraud rules requires implementing `IFraudRule` without modifying `FraudDetectorEngine`.
- **Policy Extension**: New risk policies (`IRiskPolicy`) and decision policies (`IDecisionPolicy`) can be injected dynamically at runtime.
- **Model Predictor Extension**: `IModelPredictor` allows plugging in ONNX, XGBoost, Torch, or mock predictors without changing scoring pipelines.

### C. Liskov Substitution Principle (LSP)
- All concrete fraud rules (`LargeAmountRule`, `HighVelocityRule`, `ImpossibleTravelRule`, `MuleAccountSmurfingRule`, `CardTestingRule`) can substitute `IFraudRule` transparently.
- All repository implementations (`InMemoryTransactionRepository`, `FileTransactionRepository`) adhere to `ITransactionRepository`.

### D. Interface Segregation Principle (ISP)
- Segregated interfaces prevent fat abstractions:
  - `ITransactionRepository`, `ICustomerRepository`, `IAccountRepository`, `IFraudAlertRepository`, `IReviewCaseRepository`.
  - `IRiskPolicy`, `IDecisionPolicy`, `IModelPredictor`, `IDecisionObserver`.

### E. Dependency Inversion Principle (DIP)
- High-level orchestrators (`TransactionService`, `RiskEngine`, `DecisionEngine`, `CaseManager`) depend exclusively on abstract interfaces (`std::shared_ptr<IInterface>`), enabling dependency injection and mock testing.

---

## 2. Design Patterns Applied

| Design Pattern | Purpose & Implementation Location |
| :--- | :--- |
| **Strategy Pattern** | `IRiskPolicy` (`StandardWeightedRiskPolicy`, `StrictRiskPolicy`, `FrictionlessPolicy`) and `IDecisionPolicy`. |
| **Observer Pattern** | `IDecisionObserver` and `DecisionEngine::addObserver()` for publishing decision events to alert queues and audit logs. |
| **Composite Pattern** | `FraudDetectorEngine` acting as a composite evaluator over a collection of `IFraudRule` instances. |
| **Repository Pattern** | Decouples data storage from business logic (`ITransactionRepository`, `InMemory*`, `File*`). |
| **Factory Pattern** | `PaymentMethod::createBankTransfer()`, `PaymentMethod::createEWallet()`, `FraudDetectorEngine::createDefaultEngine()`. |
| **State Machine Pattern** | `ReviewCase` managing transitions across `OPEN` $\to$ `IN_INVESTIGATION` $\to$ `RESOLVED_*` $\to$ `CLOSED`. |
| **Circuit Breaker / Resilience Pattern** | `MLResilienceManager` providing transparent fail-open and fail-closed fallbacks upon model outages. |

---

## 3. Custom Data Structures Library (`epfd::dsa`)

Built from scratch with zero dependency on standard containers for core algorithmic hot paths:

```text
include/epfd/dsa/
├── Vector.hpp           (Dynamic resizing array with move semantics, amortized O(1) push_back)
├── Deque.hpp            (Double-ended circular buffer queue with O(1) push/pop front and back)
├── LinkedList.hpp       (Doubly linked list with bidirectional iterators and O(1) splice)
├── HashMap.hpp          (Separate-chaining hash table with automatic rehash at load factor >= 0.75)
├── HashSet.hpp          (Unique element set backed by custom HashMap with O(1) lookup)
├── PriorityQueue.hpp    (Binary max-heap / min-heap with O(log N) push/pop and O(1) top)
├── Queue.hpp            (FIFO queue wrapper backed by custom Deque)
├── TimeWindowBuffer.hpp (Sliding window temporal aggregator for O(1) rolling count & sum)
├── FraudRingGraph.hpp   (Adjacency list graph with BFS/DFS for syndicate detection)
├── FastLookupIndex.hpp  (O(1) in-memory fast indexing for blacklists, whitelists, BIN cards)
└── RiskRankingUtils.hpp (Binary search over sorted timestamp arrays and Top-K risk heaps)
```
