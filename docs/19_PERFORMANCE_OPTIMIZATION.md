# 19 — Performance & Optimization

Measure before optimizing.

## Metrics
- transaction latency
- risk calculation latency
- ML inference latency
- throughput
- P95/P99
- memory usage

## Dataset sizes
Potential benchmarks:
100K, 500K, 1M, 5M transactions.

## Meaningful comparisons
- linear search vs unordered_map
- full sort vs priority_queue Top-K
- uncached vs cached access

Avoid premature optimization.
