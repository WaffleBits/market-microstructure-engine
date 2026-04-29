from __future__ import annotations

import argparse
import random
import time

from mm_engine.order_book import OrderBook, Side


def run_synthetic_benchmark(order_count: int, seed: int = 42) -> dict[str, float | int]:
    random_source = random.Random(seed)
    book = OrderBook(symbol="SYN")

    start = time.perf_counter()
    trades = 0
    for index in range(order_count):
        side = Side.BUY if random_source.random() < 0.5 else Side.SELL
        price = 10000 + random_source.randint(-25, 25)
        quantity = random_source.randint(1, 250)
        order_id = f"ord-{index}"

        trades += len(book.add_limit_order(order_id, side, price, quantity))

    duration = time.perf_counter() - start
    return {
        "orders": order_count,
        "trades": trades,
        "duration_seconds": round(duration, 6),
        "orders_per_second": round(order_count / duration, 2) if duration > 0 else 0,
        "resting_orders": len(book.orders),
        "best_bid": book.best_bid() or 0,
        "best_ask": book.best_ask() or 0,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Run a synthetic order-book benchmark.")
    parser.add_argument("--orders", type=int, default=10000)
    parser.add_argument("--seed", type=int, default=42)
    args = parser.parse_args()

    result = run_synthetic_benchmark(args.orders, args.seed)
    for key, value in result.items():
        print(f"{key}: {value}")


if __name__ == "__main__":
    main()

