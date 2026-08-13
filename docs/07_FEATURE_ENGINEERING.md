# 07 — Feature Engineering

## C++ FeatureExtractor candidates
```text
transaction_amount
transaction_frequency
transactions_last_5min
transactions_last_1hour
average_transaction_amount
amount_deviation
new_device
new_country
distance_from_previous_transaction
failed_attempt_count
merchant_frequency
customer_risk_history
```

Each feature must document:
- business meaning
- data type
- calculation
- source
- whether it supports rules, ML, or both

Avoid meaningless feature proliferation.

## Important ML rule
Features must be generated without target leakage and without using future information relative to the transaction timestamp.
