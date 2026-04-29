# Portfolio Review Notes

This project is intentionally designed as a public-safe quantitative systems engineering artifact.

## What To Review

- `src/mm_engine/order_book.py`: price-time priority, matching rules, domain types, integer ticks, partial fills, and cancellations.
- `tests/`: edge-case validation for matching behavior.
- `src/mm_engine/benchmark.py`: current benchmark path and future latency-measurement hook.

## What This Demonstrates

- Deterministic execution in a domain where ordering rules matter.
- Correct handling of partial fills, resting orders, market orders, and cancellations.
- Integer price representation to avoid floating-point execution errors.
- A benchmarkable core that can be compared against a lower-level implementation.
- A public-safe alternative to vague financial prediction projects.

## Technical Scope

- Quantitative systems: order-book mechanics, queue priority, market mechanics, and deterministic replay potential.
- Performance engineering: latency histograms, cache-aware data-structure work, and native-core roadmap.
- Backend/systems engineering: clean domain boundaries, dependency-free implementation, and edge-case tests.

## Gaps Worth Closing Next

- Port the core matching loop to C++20 or Rust.
- Add property-based tests for invariants across random order flows.
- Add event replay for pcap/ITCH-style feeds.
- Add latency histograms with p50, p95, p99, and max values.
- Add Python bindings around the native engine for benchmark comparison.
