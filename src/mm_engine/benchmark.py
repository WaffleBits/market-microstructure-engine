from __future__ import annotations

import argparse
import json
import math
import time

from mm_engine.order_book import OrderBook, Side


MASK_64 = (1 << 64) - 1
WARMUP_SEED_XOR = 0xD1B54A32D192ED03


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.state = seed & MASK_64

    def next(self) -> int:
        self.state = (self.state + 0x9E3779B97F4A7C15) & MASK_64
        value = self.state
        value = ((value ^ (value >> 30)) * 0xBF58476D1CE4E5B9) & MASK_64
        value = ((value ^ (value >> 27)) * 0x94D049BB133111EB) & MASK_64
        return value ^ (value >> 31)


def _percentile(values: list[int], fraction: float) -> int:
    if not values:
        return 0
    ordered = sorted(values)
    rank = max(1, math.ceil(fraction * len(ordered)))
    return ordered[rank - 1]


def _execute_workload(
    book: OrderBook,
    order_count: int,
    seed: int,
    latencies_ns: list[int] | None,
) -> int:
    random_source = SplitMix64(seed)
    trades = 0
    for index in range(order_count):
        side = Side.BUY if random_source.next() % 2 == 0 else Side.SELL
        price = 10000 + random_source.next() % 51 - 25
        quantity = 1 + random_source.next() % 250

        start_ns = time.perf_counter_ns()
        trades += len(book.add_limit_order(f"ord-{index}", side, price, quantity))
        end_ns = time.perf_counter_ns()
        if latencies_ns is not None:
            latencies_ns.append(end_ns - start_ns)
    return trades


def run_synthetic_benchmark(
    order_count: int,
    seed: int = 42,
    warmup: int = 0,
) -> dict[str, float | int | str]:
    if order_count <= 0:
        raise ValueError("order_count must be positive")
    if warmup < 0:
        raise ValueError("warmup must be non-negative")

    if warmup:
        warmup_book = OrderBook(symbol="WARMUP")
        _execute_workload(warmup_book, warmup, seed ^ WARMUP_SEED_XOR, None)

    book = OrderBook(symbol="SYN")
    latencies_ns: list[int] = []

    start_ns = time.perf_counter_ns()
    trades = _execute_workload(book, order_count, seed, latencies_ns)
    duration_ns = time.perf_counter_ns() - start_ns
    duration = duration_ns / 1_000_000_000
    return {
        "implementation": "python",
        "orders": order_count,
        "warmup_orders": warmup,
        "trades": trades,
        "duration_seconds": round(duration, 9),
        "orders_per_second": round(order_count / duration, 2) if duration > 0 else 0,
        "latency_p50_ns": _percentile(latencies_ns, 0.50),
        "latency_p95_ns": _percentile(latencies_ns, 0.95),
        "latency_p99_ns": _percentile(latencies_ns, 0.99),
        "latency_max_ns": max(latencies_ns),
        "resting_orders": len(book.orders),
        "best_bid": book.best_bid() or 0,
        "best_ask": book.best_ask() or 0,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Run a synthetic order-book benchmark.")
    parser.add_argument("--orders", type=int, default=10000)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--warmup", type=int, default=0)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    result = run_synthetic_benchmark(args.orders, args.seed, args.warmup)
    if args.json:
        print(json.dumps(result, sort_keys=True))
        return
    for key, value in result.items():
        print(f"{key}: {value}")


if __name__ == "__main__":
    main()
