import unittest

from mm_engine.benchmark import _percentile, run_synthetic_benchmark


class BenchmarkTest(unittest.TestCase):
    def test_percentile_uses_nearest_rank(self) -> None:
        values = [50, 10, 30, 20, 40]

        self.assertEqual(_percentile(values, 0.50), 30)
        self.assertEqual(_percentile(values, 0.95), 50)

    def test_workload_state_is_deterministic(self) -> None:
        first = run_synthetic_benchmark(order_count=250, seed=7)
        second = run_synthetic_benchmark(order_count=250, seed=7)
        stable_keys = (
            "orders",
            "warmup_orders",
            "trades",
            "resting_orders",
            "best_bid",
            "best_ask",
        )

        self.assertEqual(
            {key: first[key] for key in stable_keys},
            {key: second[key] for key in stable_keys},
        )
        self.assertGreater(first["latency_p99_ns"], 0)

    def test_rejects_invalid_counts(self) -> None:
        with self.assertRaises(ValueError):
            run_synthetic_benchmark(order_count=0)
        with self.assertRaises(ValueError):
            run_synthetic_benchmark(order_count=1, warmup=-1)


if __name__ == "__main__":
    unittest.main()
