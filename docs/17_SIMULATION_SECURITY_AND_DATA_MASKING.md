# 17 — Transaction Simulation, Resilience & Security Basics

## Purpose
Generate synthetic live transaction streams for testing real-world fraud scenarios and enforce strict PCI-DSS data masking standards across all logs, repositories, and UI layers.

---

## 1. Synthetic Transaction Stream Generator (`TransactionSimulator`)

The `TransactionSimulator` generates high-fidelity synthetic transactions on live domain entities (`Transaction`, `Customer`, `Device`, `PaymentMethod`, `Location`).

### Supported Fraud & Traffic Scenarios:
1. **`NORMAL_LEGITIMATE`**: Standard purchases ($10 - $500) distributed across trusted local merchants.
2. **`BURST_VELOCITY_ATTACK`**: Rapid-fire velocity transactions ($250 - $800) triggering velocity threshold rules.
3. **`IMPOSSIBLE_TRAVEL_ATTACK`**: Multi-country cross-border purchases (Hanoi $\to$ Paris in minutes, speed $> 5000\text{ km/h}$).
4. **`CARD_TESTING_ATTACK`**: Micro-transactions ($1.00 - $4.50) probing card validity.
5. **`MULE_SMURFING_ATTACK`**: Structured transfers ($8,500 - $9,850) deliberately placed below AML $10,000 threshold.
6. **`ACCOUNT_TAKEOVER_ATTACK`**: Transactions initiated from rooted/jailbroken emulator devices with new foreign IPs.
7. **`MIXED_REALISTIC_TRAFFIC`**: Continuous realistic stream with configurable fraud ratio (e.g. 2.5% - 5.0% attack injections).

---

## 2. PCI-DSS Data Masking & Security (`SecurityUtils`)

Strict data protection standards implemented:
- **PAN Masking**: Primary Account Numbers masked leaving only last 4 digits (`**** **** **** 1234`) or BIN + last 4 (`4111 11** **** 1234`).
- **CVV/CVC Elimination**: Complete redaction of security codes from any log payload (`CVV: [REDACTED]`).
- **PII Privacy**: Email anonymization (`n***a@domain.com`) and IP address truncation (`192.168.*.*`).

---

## 3. Resilience & Failure Policies
Explicit failure policies defined and tested:
- **ML Timeout Behavior**: SLA clock $> 50\text{ms}$ automatically triggers fallback.
- **Repository/Database Outage**: Graceful in-memory buffer handling with fallback alerting.
- **Audit Logging**: Zero bypasses; all policy fallbacks logged to append-only audit trail.
