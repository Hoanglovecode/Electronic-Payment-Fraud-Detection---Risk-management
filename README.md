# EPFD-RAS — Electronic Payment Fraud Detection & Risk Management System

> **High-Performance C++ Core & Machine Learning Hybrid System**  
> An educational, production-grade simulation system for real-time payment fraud detection, multi-factor risk scoring, automated decision engine, and continuous feedback loop.

*Project created and developed by team members: Hoang, Khiem, and Triet.*

---

## 🛠️ Technology Stack & System Breakdown

- **C++ (85%)**: High-performance core runtime engine, DSA buffers, Feature Extractor, Rule Engine, Risk Engine, Decision Engine, and ML Predictor interface. (Standard: **C++17 / C++20**, Build: **CMake**).
- **Python (15%)**: Offline Machine Learning pipeline (EDA, preprocessing, temporal split, class imbalance handling, baseline to ensemble models, probability calibration, SHAP explainability, and model export).

---

## 🏗️ Target Transaction Processing Flow

```text
Transaction
    ↓
Validation (TransactionValidator, Idempotency Check)
    ↓
Feature Extraction (Velocity, Amount Deviation, Geo/Device signals)
    ↓
Rule-Based Fraud Detection (8 Core + 4 Advanced Rules)
    ↓
ML Fraud Detection (ML Predictor Probability)
    ↓
Risk Assessment (Multi-factor Risk Score Calculation: 0 – 100)
    ↓
Risk Level Mapping (VERY_LOW, LOW, MEDIUM, HIGH, CRITICAL)
    ↓
Decision Engine (APPROVE, REVIEW, CHALLENGE 3DS, BLOCK)
    ↓
Logging / Alerting / Persistence (Repository Layer)
    ↓
Feedback Loop (OutcomeTracker, LabelStore, ReviewCase Management)
```

---

## 📂 Implementation Plan & Documentation (`docs/`)

The implementation of this project is divided into **20 focused steps**, fully documented in the [`docs/`](file:///d:/ProjectOOP/docs) directory:

| Step | File Link | Focus Area |
| :---: | :--- | :--- |
| **00** | [00_MASTER_INDEX.md](file:///d:/ProjectOOP/docs/00_MASTER_INDEX.md) | Master Index & Implementation Plan v2 Overview |
| **01** | [01_PROJECT_RULES_AND_GATES.md](file:///d:/ProjectOOP/docs/01_PROJECT_RULES_AND_GATES.md) | Project Rules, Agent Automation Boundaries & 12 Human Approval Gates |
| **02** | [02_ARCHITECTURE_AND_MODULES.md](file:///d:/ProjectOOP/docs/02_ARCHITECTURE_AND_MODULES.md) | Target System Architecture & C++ Module Structure (`src/`) |
| **03** | [03_DOMAIN_MODEL_AND_TRANSACTION.md](file:///d:/ProjectOOP/docs/03_DOMAIN_MODEL_AND_TRANSACTION.md) | Core Domain Entities & Transaction Lifecycle |
| **04** | [04_CPP_OOP_SOLID_AND_PATTERNS.md](file:///d:/ProjectOOP/docs/04_CPP_OOP_SOLID_AND_PATTERNS.md) | C++ OOP, SOLID Principles & Design Patterns |
| **05** | [05_DSA_AND_ALGORITHMS.md](file:///d:/ProjectOOP/docs/05_DSA_AND_ALGORITHMS.md) | Data Structures (`unordered_map`, Sliding Window `deque`, `priority_queue`, Graph) |
| **06** | [06_VALIDATION_AND_TRANSACTION_SERVICES.md](file:///d:/ProjectOOP/docs/06_VALIDATION_AND_TRANSACTION_SERVICES.md) | `TransactionValidator` & `TransactionService` Orchestration |
| **07** | [07_FEATURE_ENGINEERING.md](file:///d:/ProjectOOP/docs/07_FEATURE_ENGINEERING.md) | Feature Extraction & Leakage Prevention Rules |
| **08** | [08_RULE_BASED_FRAUD_DETECTION.md](file:///d:/ProjectOOP/docs/08_RULE_BASED_FRAUD_DETECTION.md) | Core & Advanced Rule Abstraction (`IFraudDetector`, `FraudRule`) |
| **09** | [09_RISK_MANAGEMENT_ENGINE.md](file:///d:/ProjectOOP/docs/09_RISK_MANAGEMENT_ENGINE.md) | Weighted Risk Scoring, Risk Levels & Explainable Breakdown |
| **10** | [10_DECISION_ENGINE.md](file:///d:/ProjectOOP/docs/10_DECISION_ENGINE.md) | Decision Policy & Action Mapping (`APPROVE`/`REVIEW`/`CHALLENGE`/`BLOCK`) |
| **11** | [11_PERSISTENCE_AND_REPOSITORIES.md](file:///d:/ProjectOOP/docs/11_PERSISTENCE_AND_REPOSITORIES.md) | Persistence Progression & Repository Pattern |
| **12** | [12_ADVANCED_FRAUD_RULES_AND_CUSTOMER_RISK.md](file:///d:/ProjectOOP/docs/12_ADVANCED_FRAUD_RULES_AND_CUSTOMER_RISK.md) | Card Testing, Account Takeover, Blacklist/Whitelist & Customer Tiering |
| **13** | [13_PYTHON_ML_PIPELINE.md](file:///d:/ProjectOOP/docs/13_PYTHON_ML_PIPELINE.md) | Offline ML Pipeline, Model Candidates & Metric Comparisons |
| **14** | [14_ML_DATASET_LEAKAGE_AND_CALIBRATION.md](file:///d:/ProjectOOP/docs/14_ML_DATASET_LEAKAGE_AND_CALIBRATION.md) | Dataset Selection Gate, Leakage Checks & Probability Calibration |
| **15** | [15_ML_CPP_INTEGRATION.md](file:///d:/ProjectOOP/docs/15_ML_CPP_INTEGRATION.md) | C++ ML Runtime (`IModelPredictor`), Resilience & Fail Policy |
| **16** | [16_FEEDBACK_LOOP_CASE_MANAGEMENT.md](file:///d:/ProjectOOP/docs/16_FEEDBACK_LOOP_CASE_MANAGEMENT.md) | Feedback Loop, `OutcomeTracker`, `LabelStore` & `ReviewCase` SLA |
| **17** | [17_SIMULATION_SECURITY_AND_DATA_MASKING.md](file:///d:/ProjectOOP/docs/17_SIMULATION_SECURITY_AND_DATA_MASKING.md) | `TransactionSimulator`, Sensitive Data Masking (PAN/CVV) & Fault Tolerance |
| **18** | [18_TESTING_VALIDATION.md](file:///d:/ProjectOOP/docs/18_TESTING_VALIDATION.md) | Testing Strategy, Edge Cases Test Matrix & Verification |
| **19** | [19_PERFORMANCE_OPTIMIZATION.md](file:///d:/ProjectOOP/docs/19_PERFORMANCE_OPTIMIZATION.md) | Performance Profiling (p50/p95/p99 Latency & RAM Footprint) |
| **20** | [20_DOCUMENTATION_GIT_AND_FINAL_DEMO.md](file:///d:/ProjectOOP/docs/20_DOCUMENTATION_GIT_AND_FINAL_DEMO.md) | Documentation Suite, Git Conventional Commits & Final Demo Flow |

---

## 🛑 Human Approval Gates

Per project governance rules, **Human Approval** is strictly required for:
1. Final System Architecture & Domain Model changes.
2. Final Dataset Selection.
3. Feature Set Selection & Preprocessing policy.
4. Class Imbalance Strategy & Threshold tuning.
5. Final Model Selection (comparing Logistic Regression, RF, XGBoost).
6. Risk Weights & Decision Policy thresholds.
7. Fail-open / Fail-closed policies for ML failures.

---

## ⚡ Quick Start & Build Instructions

```bash
# Clone the repository
git clone https://github.com/Hoanglovecode/Electronic-Payment-Fraud-Detection---Risk-management.git
cd Electronic-Payment-Fraud-Detection---Risk-management

# Configure CMake build
mkdir build && cd build
cmake ..

# Build the project
cmake --build .

# Run Unit Tests
ctest --output-on-failure
```
