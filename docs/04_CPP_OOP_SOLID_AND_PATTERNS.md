# 04 — C++ OOP, SOLID & Design Patterns

## Encapsulation
Apply to Transaction, Customer, Account, PaymentMethod, RiskAssessment and RiskProfile.

## Abstraction
Potential interfaces:
```cpp
IFraudDetector
IRiskRule
IModelPredictor
IDatabase
IRepository
```

## Polymorphism
Use where interchangeable behavior exists:
```text
RuleBasedDetector
MLDetector
FraudRule
RiskRule
ModelPredictor
Repository
```

## Composition
Prefer:
```text
FraudDetectionService HAS-A FeatureExtractor + Detector + RuleEngine
RiskManagementService HAS-A RiskEngine + RiskPolicy + RiskProfile
```

## SOLID
Apply SRP, OCP, LSP, ISP, DIP pragmatically.

## Patterns
Use only when justified:
- Strategy
- Factory
- Observer
- Repository
- Dependency Injection

Avoid unnecessary Singleton/global mutable state.
