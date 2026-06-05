#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace mm_engine {

using Price = std::int64_t;
using Quantity = std::int64_t;

enum class Side {
    buy,
    sell,
};

struct Trade {
    std::string symbol;
    std::string taker_order_id;
    std::string maker_order_id;
    Price price;
    Quantity quantity;

    bool operator==(const Trade&) const = default;
};

struct DepthLevel {
    Price price;
    Quantity quantity;
    std::size_t orders;

    bool operator==(const DepthLevel&) const = default;
};

class OrderBook {
public:
    explicit OrderBook(std::string symbol);

    std::vector<Trade> add_limit_order(
        std::string order_id,
        Side side,
        Price price,
        Quantity quantity
    );
    std::vector<Trade> add_market_order(
        std::string order_id,
        Side side,
        Quantity quantity
    );
    bool cancel(const std::string& order_id);

    [[nodiscard]] std::optional<Price> best_bid() const;
    [[nodiscard]] std::optional<Price> best_ask() const;
    [[nodiscard]] std::optional<Price> spread() const;
    [[nodiscard]] std::vector<DepthLevel> bid_depth(std::size_t levels = 5) const;
    [[nodiscard]] std::vector<DepthLevel> ask_depth(std::size_t levels = 5) const;
    [[nodiscard]] std::size_t resting_order_count() const;

private:
    struct Order {
        std::string order_id;
        Side side;
        Quantity quantity;
        std::optional<Price> price;
        std::uint64_t sequence;
    };

    struct RestingLocation {
        Side side;
        Price price;
    };

    using BidLevels = std::map<Price, std::deque<Order>, std::greater<Price>>;
    using AskLevels = std::map<Price, std::deque<Order>>;

    void validate_new_order(const std::string& order_id, Quantity quantity) const;
    std::vector<Trade> match(Order& taker, bool market_order);
    std::vector<Trade> match_buy(Order& taker, bool market_order);
    std::vector<Trade> match_sell(Order& taker, bool market_order);
    void rest(Order order);

    std::string symbol_;
    BidLevels bids_;
    AskLevels asks_;
    std::unordered_map<std::string, RestingLocation> resting_orders_;
    std::uint64_t sequence_{0};
};

}  // namespace mm_engine
