# 06 — Validation & Transaction Services

## TransactionValidator
Validate:
- required identifiers
- amount > 0
- timestamp
- supported transaction type
- known entities
- duplicate constraints
- valid payment state

## Service responsibilities
`TransactionService` should:
1. receive transaction
2. validate
3. submit to analysis
4. update lifecycle state
5. persist result

Do not embed risk-policy logic in the validator.

## Edge cases
Test invalid amount, missing entity, duplicate transaction, malformed data and extreme values.
