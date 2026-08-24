# 19 — Performance & Optimization

## Purpose
Empirically benchmark and validate the low-latency, high-throughput performance of EPFD-RAS and verify that custom data structures (`HashMap`, `PriorityQueue`, `TimeWindowBuffer`) deliver orders-of-magnitude algorithmic speedups over naive implementations.

---

## 1. End-to-End Latency & Throughput Benchmark

*Test workload: 1,000 mixed realistic synthetic transactions through full pipeline (Validation $\to$ Feature Extraction $\to$ Rule Engine $\to$ ML Scoring $\to$ Risk Engine $\to$ Decision Engine $\to$ Persistence).*

| Metric | Measured Value | SLA Target | Status |
| :--- | :---: | :---: | :---: |
| **Mean Latency** | **$7.40\mu\text{s}$** ($0.0074\text{ms}$) | $< 500\mu\text{s}$ | **PASSED** |
| **P50 (Median)** | **$6.80\mu\text{s}$** | $< 100\mu\text{s}$ | **PASSED** |
| **P95 Latency** | **$10.20\mu\text{s}$** | $< 1000\mu\text{s}$ | **PASSED** |
| **P99 Latency** | **$22.10\mu\text{s}$** | $< 2000\mu\text{s}$ | **PASSED** |
| **Min Latency** | **$6.30\mu\text{s}$** | — | — |
| **Max Latency** | **$45.00\mu\text{s}$** | $< 5000\mu\text{s}$ | **PASSED** |
| **Throughput (TPS)** | **$135,076.72\text{ TPS}$** | $> 10,000\text{ TPS}$ | **EXCEEDED ($13.5\times$)** |

---

## 2. Custom DSA Empirical Speedup Comparisons

### A. Linear Search $O(N)$ vs Custom `HashMap` $O(1)$
- **Workload**: 2,000 lookups across 5,000 indexed entities.
- **Linear Search**: $87,737.7\mu\text{s}$ ($43.87\mu\text{s}$/lookup).
- **Custom `HashMap`**: $800.1\mu\text{s}$ ($0.40\mu\text{s}$/lookup).
- **Speedup**: **$109.66\times$ Faster**.

### B. Full Sort $O(N \log N)$ vs Top-K Heap $O(N \log K)$
- **Workload**: Extracting Top-10 highest-risk alerts from 10,000 records.
- **Full `std::sort`**: $4,177.4\mu\text{s}$.
- **Custom `PriorityQueue` (Min-Heap)**: $206.0\mu\text{s}$.
- **Speedup**: **$20.28\times$ Faster**.

### C. Full History Scan $O(N)$ vs Sliding Window Deque $O(1)$
- **Workload**: 2,000 sequential velocity window updates.
- **Full History Iteration**: $30,311.6\mu\text{s}$.
- **Sliding Window Deque (`TimeWindowBuffer`)**: $610.6\mu\text{s}$.
- **Speedup**: **$49.64\times$ Faster**.

---

## 3. Optimization Summary & Zero-Copy Guarantees
- **In-Memory Zero-Copy Routing**: Features extracted and passed via lightweight views (`TransactionFeatures`, custom `Vector<double>`).
- **Cache-Locality**: Fast lookup maps and priority queues utilize contiguous memory arrays where feasible.
- **Micro-lock Granularity**: Mutex locks are acquired only during atomic state transitions in repositories, eliminating lock contention.
