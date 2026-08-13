# 11 — Persistence & Repository Layer

## Progression
```text
CSV/JSON
→ in-memory repositories
→ SQLite/PostgreSQL
```

## Interfaces
- ITransactionRepository
- CustomerRepository
- AccountRepository
- FraudAlertRepository
- FraudCaseRepository

Repository abstraction demonstrates dependency inversion and testability.

Database-specific code must not leak into domain classes.

Use mock/in-memory repositories for unit tests.
