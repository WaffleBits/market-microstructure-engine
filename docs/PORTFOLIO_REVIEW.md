# Portfolio Review Notes

This project is intentionally designed as a public-safe quantitative systems engineering artifact.

## What To Review

- `src/mm_engine/order_book.py`: price-time priority, matching rules, domain types, integer ticks, partial fills, and cancellations.
- `native/include/mm_engine/order_book.hpp` and `native/src/order_book.cpp`: dependency-free C++20 implementation.
- `tests/` and `native/tests/`: edge-case validation in both languages.
- `src/mm_engine/benchmark.py` and `native/src/benchmark.cpp`: identical deterministic workload generation and latency measurement.
- `tools/compare_benchmarks.py`: correctness gate and performance comparison.
- `.github/workflows/ci.yml`: Python tests, native build/tests, and parity checks across multiple seeds.

## What This Demonstrates

- Deterministic execution in a domain where ordering rules matter.
- Correct handling of partial fills, resting orders, market orders, and cancellations.
- Integer price representation to avoid floating-point execution errors.
- A native core compared against the Python oracle before performance is reported.
- p50, p95, p99, maximum latency, and throughput measurement.
- Strict C++ compiler warnings and CTest integration.
- A public-safe alternative to vague financial prediction projects.

## Technical Scope

- Quantitative systems: order-book mechanics, queue priority, market mechanics, and deterministic replay potential.
- Performance engineering: native implementation, latency distributions, reproducible workloads, and regression-ready comparison.
- Backend/systems engineering: clean domain boundaries, CMake, dependency-free implementations, and edge-case tests.

## Gaps Worth Closing Next

- Add property-based tests for invariants across random order flows.
- Add event replay for pcap/ITCH-style feeds.
- Add hardware performance-counter capture and cache-aware container comparisons.
- Add Python bindings around the native engine for benchmark comparison.
