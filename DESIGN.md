# Design Notes

## Matching Rules

- Buy orders match the lowest available ask at or below the buy limit.
- Sell orders match the highest available bid at or above the sell limit.
- Market orders cross the book until the requested quantity is filled or liquidity is exhausted.
- Resting orders preserve FIFO priority within each price level.
- Trades execute at the resting order price.
- Prices are integer ticks, not floats.

## Dual-Implementation Strategy

The Python implementation remains the correctness-first reference model because its state transitions are easy to inspect. The C++20 implementation applies the same rules using explicit native types and standard-library containers.

Both benchmark runners use the same SplitMix64 workload generator. The comparison tool rejects a run when trade count, resting-order count, or top-of-book state differs. Performance numbers are only reported after that parity check passes.

## Data Structures

The Python engine uses dictionaries of price levels backed by FIFO queues. The C++20 engine uses ordered maps for price priority, deques for FIFO order priority, and an unordered index for cancellation lookup.

Cancellation is O(n) within a price level because orders are stored in a deque. This is deliberate for a readable baseline. Future comparisons should measure intrusive lists, flat price arrays, and arena-backed order storage rather than claiming exchange-grade characteristics from the baseline.

## Benchmark Scope

The benchmark measures deterministic engine throughput and per-order p50/p95/p99/max latency on synthetic limit-order flow. Warmup uses a separate book and seed so it does not alter the measured final state.

The comparison is useful for regression detection and implementation tradeoffs. It is not a claim about exchange-grade latency: the workload excludes network I/O, persistence, risk checks, market-data decoding, CPU pinning, NUMA controls, and kernel-bypass networking.
