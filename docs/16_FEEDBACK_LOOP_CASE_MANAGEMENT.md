# 16 — Feedback Loop & Fraud Case Management

## Purpose
Track what happened after a decision so outcomes can later support model evaluation, dispute management, and offline retraining dataset generation.

---

## Core Architecture & Components

```text
Decision / Dispute Event
           │
           ▼
    CaseManager (Orchestrator)
    ├── ReviewCase (Case lifecycle & Analyst investigation notes)
    ├── OutcomeTracker (Post-decision outcomes: Chargebacks, 3DS pass/fail, legit confirmations)
    └── LabelStore (Verified GroundTruthRecord storage + Dataset CSV export)
```

1. **`ReviewCase`**: Domain model capturing the investigation lifecycle:
   - `case_id`, `transaction_id`, `customer_id`, `initial_risk_score`, `original_decision`
   - `assigned_analyst_id`, `evidence_list`, `notes`, `resolution_summary`
   - State transition: `OPEN` ➔ `IN_INVESTIGATION` ➔ `RESOLVED_CONFIRMED_FRAUD` / `RESOLVED_FALSE_POSITIVE` ➔ `CLOSED`.

2. **`OutcomeTracker`**:
   - Records real-world business outcomes post-decision.
   - Event types: `CHARGEBACK_RECEIVED`, `CUSTOMER_CONFIRMED_LEGIT`, `AUTH_3DS_PASSED`, `AUTH_3DS_FAILED`, `MERCHANT_REFUND`, `FRAUD_REPORTED_BY_ISSUER`.
   - Tracks chargeback loss amounts and event counts.

3. **`LabelStore`**:
   - Thread-safe repository storing verified `GroundTruthRecord` objects (`transaction_id`, `GroundTruthLabel`, `LabelSource`, `confidence`, `features`, `notes`).
   - Exports clean labeled CSV datasets for offline retraining without automatic retraining in the MVP.

4. **`CaseManager`**:
   - Manages review cases and seamlessly bridges analyst resolutions directly into `LabelStore`.

---

## Governance & Safety Rule
> [!IMPORTANT]
> **No Unsupervised Automatic Retraining**: In compliance with banking risk governance standards, the system stores verified ground-truth labels in `LabelStore` and exports them for offline validation, but **never automatically retrains production models in real time without human data-scientist approval**.
