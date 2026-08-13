# 20 — Documentation, Git & Final Demo

## Documentation
Maintain:
```text
README.md
ARCHITECTURE.md
DESIGN.md
ML_PIPELINE.md
RISK_MODEL.md
API.md
TESTING.md
```

README should explain overview, architecture, OOP, DSA, fraud detection, risk management, ML, build/test and example flows.

## Git
Focused commits:
```text
feat: add transaction domain model
feat: add fraud rule engine
feat: add risk scoring engine
feat: add decision engine
feat: add ml predictor
test: add risk engine tests
refactor: improve detector abstraction
```

Do not push without permission.

## Final demo
System should demonstrate:
```text
Payment
→ Validation
→ Feature extraction
→ Rules
→ ML probability
→ Risk score
→ Risk level
→ Decision
→ Logging
→ Alert/Case
→ Outcome
```

Demo question:
> Why was this transaction blocked?

Answer with concrete explainable contributors.

## Final definition of done
- code implemented
- build succeeds
- tests pass
- edge cases considered
- documentation updated
- clean Git state
- no critical unresolved issue
- required human approvals completed
