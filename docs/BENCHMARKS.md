# Benchmark Methodology

The benchmark exists to compare implementation behavior and detect regressions. It does not claim exchange-grade latency.

## Workload

- SplitMix64 produces the same side, integer-tick price, and quantity sequence in Python and C++20.
- Warmup runs against a separate order book and seed.
- The measured workload uses limit orders over a 51-tick price range.
- Each implementation reports throughput plus p50, p95, p99, and maximum per-order latency.

## Correctness Gate

`tools/compare_benchmarks.py` checks these fields before reporting a speedup:

- order count and warmup count
- trade count
- resting-order count
- best bid and best ask

CI repeats the gate with multiple seeds. A state mismatch fails the workflow.

## Example Local Run

Environment: WSL 2, GCC 13.3, CMake Release build, 5,000 measured orders, 500 warmup orders, June 5, 2026.

```text
parity: pass
python: 287181.21 orders/s, p99 7361 ns
cpp20: 5520884.40 orders/s, p99 500 ns
throughput_speedup: 19.22x
```

These numbers are environment-specific. Use the comparison command on the target host and compare repeated runs before drawing performance conclusions.

## Deliberate Omissions

The current benchmark does not include market-data decoding, sockets, persistence, risk checks, CPU affinity, NUMA placement, huge pages, kernel bypass, or hardware performance counters. Those belong in later experiments with explicit methodology.
