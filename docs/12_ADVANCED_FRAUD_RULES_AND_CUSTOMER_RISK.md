# 12 — Advanced Fraud Rules & Customer Risk Tiering

## CardTestingRule
Detect many tiny payments in a short window. This differs from generic velocity because amount pattern matters.

## AccountTakeoverSignalRule
Large transaction shortly after sensitive account changes such as password/email/device changes.

## Blacklist/Whitelist
Known bad/good card/device/IP/email signals.

## Customer risk tiering
Baseline:
```text
NEW
ESTABLISHED
TRUSTED
```

Tiering is policy and must be configurable.

## RiskPolicy audit
Record:
- who changed policy
- when
- old → new value
- reason

MVP may simulate one administrator, but architecture should allow future RBAC/four-eyes approval.
