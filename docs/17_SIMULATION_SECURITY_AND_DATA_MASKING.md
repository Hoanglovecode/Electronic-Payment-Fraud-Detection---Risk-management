# 17 — Transaction Simulation, Resilience & Security Basics

## TransactionSimulator
Generate synthetic sequential transactions for demos and rule testing.

Parameters:
- frequency
- amount range
- normal/suspicious pattern
- device behavior
- location behavior

This is separate from the training dataset.

## Data masking
Never log full sensitive payment data.

Example:
```text
PAN: **** **** **** 1234
```

Never log CVV.

## Resilience
Explicitly test failures around ModelPredictor, repository/database and configuration.

Important policy:
- ML timeout behavior
- invalid prediction behavior
- unavailable dependency behavior

These policies require human approval when they affect transaction safety.
