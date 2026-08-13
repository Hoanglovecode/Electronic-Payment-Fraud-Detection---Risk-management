# 14 — Dataset Selection, Leakage & Calibration

## Dataset gate
Agent must propose at least two concrete dataset candidates with:
- strengths
- weaknesses
- feature realism
- class balance
- preprocessing needs
- compatibility with project goals

Owner chooses final dataset.

## Leakage checks
Explicitly inspect:
- target leakage
- temporal leakage
- future-information leakage
- train/test contamination
- preprocessing fitted on test data
- duplicate entities across splits where inappropriate

For temporal transaction data, prefer time-aware validation when justified.

## Calibration
Fraud probability is used as a risk signal, so assess whether predicted probabilities are reasonably calibrated.

Do not claim calibrated/production-ready without evidence.
