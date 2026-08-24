# 15 — ML Integration into C++ & Failure Policy Resilience

## Runtime Flow
```text
Approved Python Model (Trained & Calibrated)
               │
               ▼
   JSON Metadata & Weights Export (`models/fraud_ml_model.json`)
               │
               ▼
   ResilientModelPredictor (Decorator / Proxy Pattern)
               │
               ▼
   NativeMLModelPredictor / MLModelRegistry (`IModelPredictor`)
               │
               ▼
   Calibrated `fraud_probability` [0.0, 1.0]
               │
               ▼
   RiskEngine (Aggregates Rule Engine, ML Probability, Contextual Signals)
               │
               ▼
   DecisionEngine (APPROVE / CHALLENGE_3DS / REVIEW / BLOCK)
```

The system uses `IModelPredictor` interface (Dependency Inversion Principle) to completely decouple the C++ risk engine from underlying inference runtime technologies.

---

## Runtime Technology Evaluation & Architectural Trade-Offs

| Technology Candidate | Inference Latency | Build & Runtime Complexity | Deployment & Portability | Failure Safety | Decision |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Native C++ Inference Engine (`NativeMLModelPredictor`)** | **$< 0.005\text{ ms}$ ($< 5\mu\text{s}$)** | **Zero external dependencies** (Pure C++17) | **100% Portable** across Windows MinGW, Linux, macOS | Exception-free, deterministic memory footprint | **CHAMPION (Adopted)** |
| **ONNX Runtime C++ API** | $\sim 0.1 - 0.5\text{ ms}$ | High (Heavy dynamic `.dll` / `.so`, complex CMake linkage) | Moderate (Requires bundling large ONNX runtime libraries) | ABI compatibility risks | *Alternative for deep neural nets* |
| **Embedded Python C-API / IPC Subprocess** | $\sim 15 - 50\text{ ms}$ | High (Python environment dependency, GIL contention) | Poor (Requires installed Python environment on target server) | High risk of process crash & latency spikes | *Rejected (Breaches payment gateway SLAs)* |

---

## ML Failure & Resilience Policies (`MLResilienceManager`)

Fail-open vs. fail-closed is a critical business and risk governance decision:

1. **`FAIL_OPEN`**:
   - In case of ML failure, sets `fraud_probability = 0.0` with an explicit failure audit note.
   - The transaction is evaluated safely by the **Rule Engine** (which enforces deterministic safety rules like velocity bursts, blacklists, high amounts, and impossible travel).
   - Recommended for low-risk established customer tiers to prevent checkout drop-offs during partial ML outages.

2. **`FAIL_CLOSED`**:
   - In case of ML failure, assumes maximum risk (`fraud_probability = 1.0`).
   - Automatically escalates the transaction to step-up authentication (`CHALLENGE_3DS`) or analyst queue (`REVIEW`).
   - Recommended for high-value cross-border transactions or high-risk accounts.

3. **`FALLBACK_SCORE` (Default: 30.0%)**:
   - Assigns a moderate configurable baseline risk score.

### Failure Guard Matrix

| Failure Condition | Detection Mechanism | System Behavior & Mitigation |
| :--- | :--- | :--- |
| **Model Unavailable** | `!isReady()` check before execution | Triggers configured failure policy + logs failure reason. |
| **Latency Timeout Breach** | Clock duration $> 50\text{ms}$ limit | Aborts prediction, applies fallback policy, emits SLA alert. |
| **Invalid Output (NaN / Inf / Out-of-bounds)** | `isnan`, `isinf`, or $p \notin [0.0, 1.0]$ | Drops invalid prediction, applies safe fallback bounds. |
| **Feature Dimension Mismatch** | `features.size() != 18` | Intercepts mismatch before memory access, prevents segfault. |
| **Model Version Mismatch** | `getModelVersion() != expected_version` | Logs version incompatibility, prevents scoring with stale weights. |

> [!IMPORTANT]
> **Zero Silent Bypasses**: The system **never silently ignores or bypasses** ML failure. Every failure triggers an audit event logged into `AuditTrailLogger`.

---

## Model Hot-Swapping Registry (`MLModelRegistry`)

The `MLModelRegistry` supports dynamic zero-downtime hot-swapping between model versions:
- Multiple model versions (e.g. `v1.0.0`, `v2.0.0`) can be registered simultaneously.
- Thread-safe atomic version activation (`setActiveVersion`) enables seamless live updates without service restarts.
