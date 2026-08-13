# 16 — Feedback Loop & Fraud Case Management

## Purpose
Track what happened after a decision so outcomes can later support evaluation/retraining.

## Components
```text
OutcomeTracker
ReviewCase
CaseManager
LabelStore
```

## Case lifecycle
```text
OPEN
→ INVESTIGATING
→ CONFIRMED_FRAUD
or
→ FALSE_POSITIVE
→ RESOLVED
```

## Feedback
Store:
- transaction
- original decision
- risk score
- evidence
- investigator outcome
- final label
- timestamp

MVP requirement: outcome tracking + label storage.

Do not automatically retrain the production model from feedback in the MVP.
