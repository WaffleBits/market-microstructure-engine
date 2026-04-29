# Market Microstructure Engine

Deterministic limit-order-book and matching-engine reference implementation for market microstructure study.

This repo focuses on deterministic market mechanics: price-time priority, partial fills, market orders, cancellations, snapshots, deterministic simulations, and benchmarkable execution.

## Why This Exists

Strong quant engineering projects should avoid vague "stock prediction" claims. A matching engine is more concrete: it forces clear rules, deterministic behavior, edge-case tests, and performance measurement.

## Features

- Price-time priority matching.
- Limit and market orders.
- Partial fills and resting residual quantity.
- Order cancellation by ID.
- Top-of-book and depth snapshots.
- Deterministic simulation and benchmark runner.
- Unit tests for matching rules and edge cases.
- No third-party runtime dependencies.

## Engineering Scope

This repo uses a concrete, rule-heavy domain to exercise deterministic execution, correctness under edge cases, integer price representation, and benchmarkable behavior.

Relevant areas:

- Quantitative systems: order-book mechanics, deterministic matching, queue priority, fills, cancels, and market-data replay roadmap.
- Performance engineering: benchmark runner, latency-measurement roadmap, cache-aware data-structure comparison path.
- Backend/systems engineering: clean domain model, dependency-free core, tests for edge cases, and a C++20 or Rust extension path.

## Reviewer Fast Path

- Start with `src/mm_engine/order_book.py` for matching rules and state transitions.
- Review `tests/` for edge-case behavior around fills, cancels, and market orders.
- Run `python -m mm_engine.benchmark --orders 25000` for the current benchmark path.
- Read `docs/PORTFOLIO_REVIEW.md` for the technical review guide.

## Quick Start

```bash
python -m venv .venv
.venv\Scripts\activate
pip install -e .
python -m unittest discover -s tests
python -m mm_engine.benchmark --orders 25000
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
- A credible foundation for a future low-latency C++ or Rust implementation.

## Next Phase

- Port the matching core to C++20 or Rust.
- Add property-based tests.
- Add pcap/ITCH-style event replay.
- Add latency histograms and cache-aware data-structure comparisons.
- Add Python bindings around the native engine.
