# 09 — Risk Management Engine

Risk Management asks:
> How risky is the transaction and what action should follow?

## Risk sources
```text
ML Risk
Transaction Risk
Behavioral Risk
Velocity Risk
Device Risk
Location Risk
Merchant Risk
Rule Risk
```

## Classes
RiskFactor, RiskAssessment, RiskProfile, RiskRule, RiskAggregator, RiskEngine, RiskPolicy.

## Baseline levels
```text
0–20   VERY_LOW
21–40  LOW
41–60  MEDIUM
61–80  HIGH
81–100 CRITICAL
```

These are configurable policy defaults, not business truth.

## Weighted scoring
Weights must be configurable, not scattered as magic numbers.

Every score must be explainable:
```text
High Amount       +15
New Device        +10
High Velocity     +20
Behavior Risk     +12
ML Risk           +30
Total              87
```

Final weights/thresholds require human approval.
