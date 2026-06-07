#include "mm_engine/order_book.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr std::uint64_t kMask = ~std::uint64_t{0};
constexpr std::uint64_t kWarmupSeedXor = 0xD1B54A32D192ED03ULL;

class SplitMix64 {
public:
    explicit SplitMix64(std::uint64_t seed) : state_(seed) {}

    std::uint64_t next() {
        state_ = (state_ + 0x9E3779B97F4A7C15ULL) & kMask;
        auto value = state_;
        value = ((value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL) & kMask;
        value = ((value ^ (value >> 27U)) * 0x94D049BB133111EBULL) & kMask;
        return value ^ (value >> 31U);
    }

private:
    std::uint64_t state_;
};

struct Options {
    std::size_t orders{10000};
    std::size_t warmup{0};
    std::uint64_t seed{42};
    bool json{false};
};

struct BenchmarkResult {
    std::size_t orders;
    std::size_t warmup_orders;
    std::size_t trades;
    double duration_seconds;
    double orders_per_second;
    std::int64_t latency_p50_ns;
    std::int64_t latency_p95_ns;
    std::int64_t latency_p99_ns;
    std::int64_t latency_max_ns;
    std::size_t resting_orders;
    mm_engine::Price best_bid;
    mm_engine::Price best_ask;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string_view argument{argv[index]};
        if (argument == "--json") {
            options.json = true;
            continue;
        }
        if (index + 1 >= argc) {
            throw std::invalid_argument("missing value for " + std::string(argument));
        }
        const std::string value{argv[++index]};
        if (argument == "--orders") {
            options.orders = std::stoull(value);
        } else if (argument == "--warmup") {
            options.warmup = std::stoull(value);
        } else if (argument == "--seed") {
            options.seed = std::stoull(value);
        } else {
            throw std::invalid_argument("unknown argument: " + std::string(argument));
        }
    }
    if (options.orders == 0) {
        throw std::invalid_argument("--orders must be positive");
    }
    return options;
}

std::int64_t percentile(std::vector<std::int64_t> values, double fraction) {
    if (values.empty()) {
        return 0;
    }
    std::sort(values.begin(), values.end());
    const auto rank = static_cast<std::size_t>(
        std::ceil(fraction * static_cast<double>(values.size()))
    );
    return values[std::max<std::size_t>(1, rank) - 1];
}

std::size_t execute_workload(
    mm_engine::OrderBook& book,
    std::size_t order_count,
    std::uint64_t seed,
    std::vector<std::int64_t>* latencies
) {
    SplitMix64 random_source(seed);
    std::size_t trades = 0;
    for (std::size_t index = 0; index < order_count; ++index) {
        const auto side = random_source.next() % 2 == 0
            ? mm_engine::Side::buy
            : mm_engine::Side::sell;
        const auto price = static_cast<mm_engine::Price>(
            10000 + static_cast<std::int64_t>(random_source.next() % 51) - 25
        );
        const auto quantity = static_cast<mm_engine::Quantity>(
            1 + random_source.next() % 250
        );

        const auto start = std::chrono::steady_clock::now();
        trades += book.add_limit_order(
            "ord-" + std::to_string(index),
            side,
            price,
            quantity
        ).size();
        const auto end = std::chrono::steady_clock::now();
        if (latencies != nullptr) {
            latencies->push_back(
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            );
        }
    }
    return trades;
}

BenchmarkResult run_benchmark(const Options& options) {
    if (options.warmup > 0) {
        mm_engine::OrderBook warmup_book("WARMUP");
        execute_workload(
            warmup_book,
            options.warmup,
            options.seed ^ kWarmupSeedXor,
            nullptr
        );
    }

    mm_engine::OrderBook book("SYN");
    std::vector<std::int64_t> latencies;
    latencies.reserve(options.orders);

    const auto start = std::chrono::steady_clock::now();
    const auto trades = execute_workload(
        book,
        options.orders,
        options.seed,
        &latencies
    );
    const auto end = std::chrono::steady_clock::now();
    const auto duration_seconds = std::chrono::duration<double>(end - start).count();

    return {
        .orders = options.orders,
        .warmup_orders = options.warmup,
        .trades = trades,
        .duration_seconds = duration_seconds,
        .orders_per_second = static_cast<double>(options.orders) / duration_seconds,
        .latency_p50_ns = percentile(latencies, 0.50),
        .latency_p95_ns = percentile(latencies, 0.95),
        .latency_p99_ns = percentile(latencies, 0.99),
        .latency_max_ns = *std::max_element(latencies.begin(), latencies.end()),
        .resting_orders = book.resting_order_count(),
        .best_bid = book.best_bid().value_or(0),
        .best_ask = book.best_ask().value_or(0),
    };
}

void print_json(const BenchmarkResult& result) {
    std::cout << std::fixed << std::setprecision(9)
              << "{"
              << "\"implementation\":\"cpp20\","
              << "\"orders\":" << result.orders << ","
              << "\"warmup_orders\":" << result.warmup_orders << ","
              << "\"trades\":" << result.trades << ","
              << "\"duration_seconds\":" << result.duration_seconds << ","
              << "\"orders_per_second\":" << result.orders_per_second << ","
              << "\"latency_p50_ns\":" << result.latency_p50_ns << ","
              << "\"latency_p95_ns\":" << result.latency_p95_ns << ","
              << "\"latency_p99_ns\":" << result.latency_p99_ns << ","
              << "\"latency_max_ns\":" << result.latency_max_ns << ","
              << "\"resting_orders\":" << result.resting_orders << ","
              << "\"best_bid\":" << result.best_bid << ","
              << "\"best_ask\":" << result.best_ask
              << "}\n";
}

void print_human(const BenchmarkResult& result) {
    std::cout << std::fixed << std::setprecision(3)
              << "implementation: cpp20\n"
              << "orders: " << result.orders << "\n"
              << "warmup_orders: " << result.warmup_orders << "\n"
              << "trades: " << result.trades << "\n"
              << "duration_seconds: " << result.duration_seconds << "\n"
              << "orders_per_second: " << result.orders_per_second << "\n"
              << "latency_p50_ns: " << result.latency_p50_ns << "\n"
              << "latency_p95_ns: " << result.latency_p95_ns << "\n"
              << "latency_p99_ns: " << result.latency_p99_ns << "\n"
              << "latency_max_ns: " << result.latency_max_ns << "\n"
              << "resting_orders: " << result.resting_orders << "\n"
              << "best_bid: " << result.best_bid << "\n"
              << "best_ask: " << result.best_ask << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto result = run_benchmark(options);
        if (options.json) {
            print_json(result);
        } else {
            print_human(result);
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n";
        return EXIT_FAILURE;
    }
}
