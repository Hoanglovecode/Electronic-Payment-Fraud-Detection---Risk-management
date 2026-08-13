# 02 — System Architecture & Module Design

## Target flow
```text
Transaction
↓
Validation
↓
Feature Extraction
↓
Rule-Based Fraud Detection
↓
ML Fraud Detection
↓
Risk Assessment
↓
Risk Aggregation
↓
Risk Level
↓
Decision Engine
↓
APPROVE / REVIEW / CHALLENGE / BLOCK
↓
Logging / Alert / Persistence
```

## Proposed C++ modules
```text
models/
validation/
features/
fraud/
risk/
decision/
ml/
feedback/
database/
services/
utils/
```

## Expected classes
Transaction, Customer, Account, Merchant, PaymentMethod, RiskScore, FraudAlert, FeatureExtractor, FraudRule, RiskRule, RiskEngine, RiskPolicy, DecisionEngine, IModelPredictor, repositories, OutcomeTracker, CaseManager.

Agent may change architecture only with technical justification and approval.
