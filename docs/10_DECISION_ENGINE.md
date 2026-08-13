# 10 — Decision Engine

Convert `RiskAssessment` into an action.

## Actions
```text
APPROVE
REVIEW
CHALLENGE
BLOCK
```

Baseline:
```text
VERY_LOW → APPROVE
LOW      → APPROVE
MEDIUM   → MONITOR / APPROVE
HIGH     → REVIEW / CHALLENGE
CRITICAL → BLOCK
```

Decision policy must be configurable.

RiskEngine calculates risk; DecisionEngine applies policy.

Every decision must include score, level, contributors, policy threshold and action.
