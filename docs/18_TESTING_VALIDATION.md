# 18 — Testing & Validation

## Unit tests
Cover:
- domain objects
- validation
- features
- each fraud rule
- risk factors
- RiskEngine
- DecisionEngine
- repositories
- feedback/case management

## Integration test
```text
Transaction
→ Feature
→ Fraud
→ Risk
→ Decision
→ Persistence
→ Feedback
```

## ML validation
Check:
- split correctness
- leakage
- class imbalance
- overfitting
- suspicious metrics
- threshold trade-offs
- calibration

## Reporting
Agent must report:
```text
Tests passed: X
Tests failed: Y
Coverage if available: Z
```

Never claim "works" without actual build/test evidence.
