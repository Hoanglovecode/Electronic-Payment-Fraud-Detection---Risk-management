# 03 — Domain Model & Transaction Processing

## Domain entities
- Customer
- Account
- Transaction
- Merchant
- PaymentMethod
- Device
- RiskAssessment
- FraudAlert
- Chargeback
- Dispute

## Transaction lifecycle
```text
Created → Validated → Analyzed → Risk Assessed → Decision → Logged/Persisted
```

## Transaction fields
transaction_id, type, customer/sender, recipient, amount, timestamp, location, IP, device, merchant and payment method.

## Services
`TransactionService` coordinates validation, analysis, state update and persistence.

Keep domain objects independent from ML/runtime details.
