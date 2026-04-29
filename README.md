# Market Microstructure Engine

Deterministic limit-order-book and matching-engine reference implementation for market microstructure study.

This repo is built to signal systems thinking for quantitative trading engineering: price-time priority, partial fills, market orders, cancellations, snapshots, deterministic simulations, and benchmarkable execution.

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

## Portfolio Signal

This project is meant to show:

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

