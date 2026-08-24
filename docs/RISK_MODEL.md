# EPFD-RAS Risk Model & Decision Policies Specification

## 1. Multi-Factor Risk Scoring Architecture

The risk scoring subsystem computes an explainable composite risk score $S \in [0.0, 100.0]$:

$$S = \min\left(100.0, \; w_{\text{rules}} \cdot S_{\text{rules}} + w_{\text{ml}} \cdot S_{\text{ml}} + w_{\text{velocity}} \cdot S_{\text{velocity}} + w_{\text{geo}} \cdot S_{\text{geo}} + w_{\text{device}} \cdot S_{\text{device}} + \Delta_{\text{tier}}\right)$$

### Default Weight Profile (`StandardWeightedRiskPolicy`):
- $w_{\text{rules}} = 0.35$ (Rule-based deterministic signals)
- $w_{\text{ml}} = 0.30$ (Calibrated ML fraud probability $\times 100$)
- $w_{\text{velocity}} = 0.15$ (Short-term and medium-term transaction bursts)
- $w_{\text{geo}} = 0.10$ (Haversine distance and impossible travel speed)
- $w_{\text{device}} = 0.10$ (Rooted status, emulator flags, device diversity)

---

## 2. Customer Risk Tier Offsets ($\Delta_{\text{tier}}$)

`CustomerRiskTierManager` dynamically adjusts baseline scores based on KYC status and account age:

| Customer Risk Tier | Scoring Offset ($\Delta_{\text{tier}}$) | Behavior & SLA |
| :--- | :---: | :--- |
| **VIP / Whitelisted** | **$-15.0$ pts** | Frictionless experience for trusted high-net-worth customers. |
| **STANDARD** | **$0.0$ pts** | Normal baseline risk evaluation. |
| **NEW_ACCOUNT** | **$+10.0$ pts** | Extra vigilance during initial 30 days of account creation. |
| **HIGH_RISK** | **$+25.0$ pts** | Prior dispute / chargeback history; requires stricter step-up verification. |
| **BLOCKED** | **$+100.0$ pts** | Immediate hard block. |

---

## 3. Risk Levels & Decision Action Policies

| Risk Score Range ($S$) | Risk Level | Standard Decision Action | Frictionless Policy Action | Strict Policy Action |
| :---: | :---: | :---: | :---: | :---: |
| **$[0, 30)$** | `VERY_LOW` / `LOW` | **`APPROVE`** | `APPROVE` | `APPROVE` |
| **$[30, 60)$** | `MEDIUM` | **`REVIEW`** | `APPROVE` | `CHALLENGE_3DS` |
| **$[60, 80)$** | `HIGH` | **`CHALLENGE_3DS`** | `CHALLENGE_3DS` | `BLOCK` |
| **$[80, 100]$** | `CRITICAL` | **`BLOCK`** | `BLOCK` | `BLOCK` |

---

## 4. Four-Eyes Principle & Policy Governance

`PolicyChangeAuditManager` guarantees that no single administrator can alter risk weights or decision thresholds in production without dual-authorized review:
1. Administrator A proposes a parameter update (`PolicyChangeProposal`).
2. Proposal enters `PENDING_REVIEW` state.
3. Administrator B reviews the impact and signs off with cryptographic authorization.
4. Parameters are atomically activated and logged to immutable audit trail.
