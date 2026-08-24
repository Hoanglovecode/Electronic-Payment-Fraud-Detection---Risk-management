# EPFD-RAS Testing & Validation Suite Specification

## 1. Test Suite Summary

- **Total Automated C++ Unit & Integration Tests**: **92 Tests**
- **Test Suites Count**: **18 Suites**
- **Pass Rate**: **100% (92 / 92 Passed)**
- **Execution Time**: **~26.0 ms**
- **Python ML Health Audit Checks**: **5/5 Passed (Zero leakage, ECE $0.028\%$, Brier $0.000022$)**

---

## 2. Test Suites Matrix

| # | Test Suite Name | File | Test Count | Key Areas Verified |
| :---: | :--- | :--- | :---: | :--- |
| **1** | `ArchitectureSuite` | `test_architecture.cpp` | 2 | Common types, string conversions, numeric limits, constant invariants. |
| **2** | `DomainModelSuite` | `test_domain_models.cpp` | 9 | `Transaction`, `Customer`, `Account`, `PaymentMethod`, Luhn algorithm, Haversine geo distance, `Device`, `Dispute`. |
| **3** | `SolidSuite` | `test_solid_and_patterns.cpp` | 4 | Strategy pattern, Dependency Inversion, ML Predictor abstraction, Observer event publishing. |
| **4** | `CustomDsaSuite` | `test_dsa_and_algorithms.cpp` | 7 | Custom `Vector`, `Deque`, `LinkedList`, `HashMap` rehashing, `HashSet`, `PriorityQueue`, `Queue`. |
| **5** | `DsaSuite` | `test_dsa_and_algorithms.cpp` | 6 | `TimeWindowBuffer` sliding, `CustomerVelocityTracker`, `FastLookupIndex`, `FraudRingGraph`, Top-K ranking. |
| **6** | `ValidationSuite` | `test_validation_and_services.cpp` | 6 | Required fields, negative amounts, timestamp bounds, duplicate idempotency, account balance. |
| **7** | `ServiceSuite` | `test_validation_and_services.cpp` | 1 | `TransactionService` full end-to-end payment processing lifecycle. |
| **8** | `FeatureSuite` | `test_feature_engineering.cpp` | 5 | 18-D dimension check, velocity sliding window zero-leakage, device diversity, geo speed, merchant risk. |
| **9** | `RuleSuite` | `test_rule_based_fraud_detection.cpp` | 6 | `LargeAmountRule`, `HighVelocityRule`, `NewDeviceRule`, `ImpossibleTravelRule`, `BehaviorDeviationRule`, `BlacklistRule`. |
| **10** | `EngineSuite` | `test_rule_based_fraud_detection.cpp` | 1 | `FraudDetectorEngine` composite rule orchestration. |
| **11** | `RiskSuite` | `test_risk_management_engine.cpp` | 4 | Risk factors, explainable scoring breakdown, concrete risk policies, `RiskEngine` assessment. |
| **12** | `DecisionSuite` | `test_decision_engine.cpp` | 5 | Standard, Strict, Frictionless policies, custom thresholds, observer alerts, end-to-end evaluation. |
| **13** | `PersistenceSuite` | `test_persistence_and_repositories.cpp` | 3 | In-memory repositories, file repository persistence/reloading, PCI-DSS audit trail logger. |
| **14** | `AdvancedRulesSuite` | `test_advanced_fraud_rules_and_tiering.cpp` | 2 | `MuleAccountSmurfingRule`, `DormantAccountRule`, dynamic lists. |
| **15** | `TieringSuite` & `GovernanceSuite` | `test_advanced_fraud_rules_and_tiering.cpp` | 2 | Customer risk tier offsets, 4-Eyes policy change dual authorization. |
| **16** | `MLPipelineSuite` | `test_python_ml_pipeline.cpp` | 3 | Native ML Predictor clean/fraud evaluations, integration with `RiskEngine`. |
| **17** | `LeakageSuite` & `CalibrationSuite` | `test_dataset_leakage_and_calibration.cpp` | 3 | Velocity window temporal hierarchy invariant, future information guard, probability calibration monotonicity. |
| **18** | `MLIntegrationSuite` | `test_ml_cpp_integration.cpp` | 5 | Fail-open / Fail-closed policies, feature dimension safety, transparent success, model registry hot-swapping. |
| **19** | `FeedbackSuite` | `test_feedback_loop_case_management.cpp` | 4 | `ReviewCase` lifecycle, `OutcomeTracker`, `LabelStore` CSV export, `CaseManager` investigation. |
| **20** | `SecuritySuite` & `SimulatorSuite` | `test_simulation_security_and_masking.cpp` | 6 | PAN masking, CVV redaction, PII masking, 7 simulator scenarios (Burst velocity, Impossible travel, Card testing, ATO). |
| **21** | `EndToEndPipelineSuite` | `test_end_to_end_pipeline.cpp` | 4 | Clean transaction, attack dispute, stress stream, resilient predictor fallback. |
| **22** | `BenchmarkSuite` | `test_performance_and_benchmarks.cpp` | 4 | End-to-end latency ($7.40\mu\text{s}$), Linear vs HashMap ($109.66\times$), Full Sort vs Top-K ($20.28\times$), Sliding Window ($49.64\times$). |

---

## 3. How to Run Tests

```bash
# Via Mingw-Make
mingw32-make test

# Via CMake / CTest
cd build
cmake --build .
ctest --output-on-failure
```
