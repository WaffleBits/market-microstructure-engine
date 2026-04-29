# Design Notes

## Matching Rules

- Buy orders match the lowest available ask at or below the buy limit.
- Sell orders match the highest available bid at or above the sell limit.
- Market orders cross the book until the requested quantity is filled or liquidity is exhausted.
- Resting orders preserve FIFO priority within each price level.
- Trades execute at the resting order price.
- Prices are integer ticks, not floats.

## Why Python First

This implementation is a correctness-first reference model. It is easy to test, inspect, and compare against a future native implementation. For a quant-facing portfolio, the next major upgrade should be a C++20 or Rust port with the Python model retained as an oracle.

## Data Structures

The MVP uses dictionaries of price levels backed by FIFO queues. This is simple and correct for review. Production-style versions should benchmark heaps, sorted arrays, intrusive lists, and arena allocation depending on the target workload.

## Benchmark Scope

The benchmark measures deterministic engine throughput on synthetic order flow. It is not a claim about exchange-grade latency.

