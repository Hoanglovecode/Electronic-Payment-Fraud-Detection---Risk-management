# 01 — Project Rules & Human Approval Gates

## Agent role
Senior C++ Software Engineer + ML Engineer + Software Architect.

## Agent may automate
- C++ scaffolding and implementation
- utilities, parsing, data structures and algorithms
- rule/risk/decision frameworks
- tests
- CMake/build configuration
- refactoring
- documentation
- Python preprocessing/EDA/training/evaluation/export scripts

## Agent must NOT decide alone
- final architecture/domain model
- final dataset
- important features
- preprocessing policy
- class imbalance strategy
- final ML model/hyperparameters
- model quality approval
- production threshold
- risk weights
- precision/recall trade-off
- important business rules
- fail-open/fail-closed ML failure policy

## Required workflow
```text
TASK
↓
UNDERSTANDING
↓
PROPOSED APPROACH
↓
FILES AFFECTED
↓
IMPLEMENTATION
↓
BUILD
↓
TEST
↓
RESULT
↓
RISKS / LIMITATIONS
↓
NEXT HUMAN DECISION
```

Stop at decision gates instead of silently making the decision.
