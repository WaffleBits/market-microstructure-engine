from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path

from mm_engine.benchmark import run_synthetic_benchmark


PARITY_KEYS = (
    "orders",
    "warmup_orders",
    "trades",
    "resting_orders",
    "best_bid",
    "best_ask",
)


def load_native_result(
    executable: Path,
    orders: int,
    seed: int,
    warmup: int,
) -> dict[str, float | int | str]:
    completed = subprocess.run(
        [
            str(executable),
            "--orders",
            str(orders),
            "--seed",
            str(seed),
            "--warmup",
            str(warmup),
            "--json",
        ],
        check=True,
        capture_output=True,
        text=True,
    )
    return json.loads(completed.stdout)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Compare deterministic Python and C++20 matching-engine benchmarks."
    )
    parser.add_argument("--native", type=Path, required=True)
    parser.add_argument("--orders", type=int, default=25000)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--warmup", type=int, default=5000)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    native_executable = args.native.resolve()
    if not native_executable.is_file():
        raise SystemExit(f"native benchmark not found: {native_executable}")

    python_result = run_synthetic_benchmark(args.orders, args.seed, args.warmup)
    native_result = load_native_result(
        native_executable,
        args.orders,
        args.seed,
        args.warmup,
    )

    mismatches = {
        key: {"python": python_result[key], "cpp20": native_result[key]}
        for key in PARITY_KEYS
        if python_result[key] != native_result[key]
    }
    if mismatches:
        raise SystemExit(f"Python/C++ state mismatch: {json.dumps(mismatches, sort_keys=True)}")

    speedup = (
        float(native_result["orders_per_second"])
        / float(python_result["orders_per_second"])
    )
    comparison = {
        "parity": "pass",
        "orders": args.orders,
        "seed": args.seed,
        "warmup_orders": args.warmup,
        "python": python_result,
        "cpp20": native_result,
        "throughput_speedup": round(speedup, 2),
    }

    if args.json:
        print(json.dumps(comparison, indent=2, sort_keys=True))
        return

    print("parity: pass")
    print(
        "python: "
        f"{python_result['orders_per_second']:.2f} orders/s, "
        f"p99 {python_result['latency_p99_ns']} ns"
    )
    print(
        "cpp20: "
        f"{native_result['orders_per_second']:.2f} orders/s, "
        f"p99 {native_result['latency_p99_ns']} ns"
    )
    print(f"throughput_speedup: {speedup:.2f}x")


if __name__ == "__main__":
    main()
