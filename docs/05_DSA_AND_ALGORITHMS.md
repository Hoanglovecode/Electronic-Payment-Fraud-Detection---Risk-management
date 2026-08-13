# 05 — DSA & Algorithms

DSA must solve real system problems.

## unordered_map
Fast lookup:
```text
customer_id → profile
card_id → history
device_id → accounts
IP → transactions
```

## deque + sliding window
Velocity detection over recent transaction windows.

## priority_queue
High-risk transaction/investigation queues.

## sorting
Risk ranking and suspicious-transaction ranking.

## binary search
Use only where ordered data makes it appropriate.

## graph — advanced
Model:
```text
Customer — Device — IP — Card — Merchant
```
for suspicious relationship analysis/fraud rings.

Do not implement graph before MVP needs it.
