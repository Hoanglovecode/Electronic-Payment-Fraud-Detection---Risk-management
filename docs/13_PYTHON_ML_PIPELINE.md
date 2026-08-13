# 13 — Python ML Pipeline

## Pipeline
```text
Dataset
↓
EDA
↓
Cleaning
↓
Feature Engineering
↓
Train/Validation/Test Split
↓
Leakage Checks
↓
Class Imbalance Analysis
↓
Baseline
↓
Candidate Models
↓
Evaluation
↓
Threshold Analysis
↓
Calibration
↓
Export
```

## Candidate models
1. Logistic Regression — baseline
2. Random Forest — comparison
3. XGBoost — candidate

Do not assume XGBoost is final.

## Metrics
- Precision
- Recall
- F1
- ROC-AUC
- PR-AUC / Average Precision
- Confusion matrix
- False-positive rate
- False-negative rate

Accuracy is not sufficient for an imbalanced fraud problem.

Agent reports experiments; owner selects final model.
