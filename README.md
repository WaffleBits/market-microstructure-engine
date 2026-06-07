# Market Microstructure Engine

Deterministic limit-order-book and matching-engine implementations in Python and C++20.

The Python engine is the readable correctness oracle. The dependency-free C++20 engine implements the same price-time rules for native performance work. A shared deterministic workload checks final-state parity before comparing throughput and tail latency.

## Why This Exists

Strong quant engineering projects should avoid vague "stock prediction" claims. A matching engine is more concrete: it forces clear rules, deterministic behavior, edge-case tests, and performance measurement.

## Features

- Price-time priority matching.
- Limit and market orders.
- Partial fills and resting residual quantity.
- Order cancellation by ID.
- Top-of-book and depth snapshots.
- Python reference engine and C++20 native core.
- Deterministic cross-language workload generation.
- p50, p95, p99, and maximum per-order latency.
- Python/C++ final-state parity gate across multiple seeds in CI.
- Unit tests for matching rules and edge cases in both languages.
- No third-party runtime dependencies in either engine.

## Engineering Scope

This repo uses a concrete, rule-heavy domain to exercise deterministic execution, correctness under edge cases, integer price representation, and benchmarkable behavior.

Relevant areas:

- Quantitative systems: order-book mechanics, deterministic matching, queue priority, fills, cancels, and market-data replay roadmap.
- Performance engineering: native implementation, latency distributions, deterministic workload comparison, and measured throughput.
- Backend/systems engineering: clean domain model, dependency-free cores, CMake, strict compiler warnings, and tests for edge cases.

## Reviewer Fast Path

- Start with `src/mm_engine/order_book.py` for matching rules and state transitions.
- Compare `native/include/mm_engine/order_book.hpp` and `native/src/order_book.cpp` with the Python oracle.
- Review `tests/` and `native/tests/` for matching behavior in both implementations.
- Run `tools/compare_benchmarks.py` to verify parity and compare latency/throughput.
- Read `docs/BENCHMARKS.md` for methodology and interpretation limits.
- Read `docs/PORTFOLIO_REVIEW.md` for the technical review guide.

## Quick Start

```bash
python -m venv .venv
.venv\Scripts\activate
pip install -e .
python -m unittest discover -s tests
python -m mm_engine.benchmark --orders 25000 --warmup 5000
```

Build and test the C++20 engine:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build --build-config Release --output-on-failure
python tools/compare_benchmarks.py \
  --native build/mm_engine_benchmark \
  --orders 25000 \
  --warmup 5000
```

## Example

```python
from mm_engine import OrderBook, Side

book = OrderBook(symbol="FOO")
book.add_limit_order("ask-1", Side.SELL, price=10100, quantity=100)
trades = book.add_limit_order("buy-1", Side.BUY, price=10100, quantity=40)

print(trades[0].price, trades[0].quantity)
```

Prices are represented as integer ticks to avoid floating-point execution errors.

## Engineering Notes

This project covers:

- Correctness around order matching rules.
- Deterministic, testable systems behavior.
- Awareness of market mechanics: spread, depth, queue priority, fills, cancels.
- Modern C++20 implementation with explicit ownership and integer price/quantity types.
- Cross-language oracle testing before accepting performance results.
- Reproducible latency and throughput measurement on identical generated order flow.

## Next Phase

- Add property-based tests.
- Add pcap/ITCH-style event replay.
- Add Linux `perf` counter capture and cache-aware data-structure comparisons.
- Compare tree-based levels with flat/sorted price-level storage.
- Add Python bindings around the native engine.
