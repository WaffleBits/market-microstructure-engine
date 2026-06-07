#include "mm_engine/order_book.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace mm_engine {

OrderBook::OrderBook(std::string symbol) : symbol_(std::move(symbol)) {}

std::vector<Trade> OrderBook::add_limit_order(
    std::string order_id,
    Side side,
    Price price,
    Quantity quantity
) {
    validate_new_order(order_id, quantity);
    Order order{
        .order_id = std::move(order_id),
        .side = side,
        .quantity = quantity,
        .price = price,
        .sequence = ++sequence_,
    };
    auto trades = match(order, false);
    if (order.quantity > 0) {
        rest(std::move(order));
    }
    return trades;
}

std::vector<Trade> OrderBook::add_market_order(
    std::string order_id,
    Side side,
    Quantity quantity
) {
    validate_new_order(order_id, quantity);
    Order order{
        .order_id = std::move(order_id),
        .side = side,
        .quantity = quantity,
        .price = std::nullopt,
        .sequence = ++sequence_,
    };
    return match(order, true);
}

bool OrderBook::cancel(const std::string& order_id) {
    const auto location_it = resting_orders_.find(order_id);
    if (location_it == resting_orders_.end()) {
        return false;
    }

    const auto location = location_it->second;
    auto erase_from_level = [&order_id](auto& levels, Price price) {
        const auto level_it = levels.find(price);
        if (level_it == levels.end()) {
            throw std::logic_error("resting order index points to a missing price level");
        }

        auto& queue = level_it->second;
        const auto order_it = std::find_if(
            queue.begin(),
            queue.end(),
            [&order_id](const Order& order) { return order.order_id == order_id; }
        );
        if (order_it == queue.end()) {
            throw std::logic_error("resting order index points to a missing order");
        }

        queue.erase(order_it);
        if (queue.empty()) {
            levels.erase(level_it);
        }
    };

    if (location.side == Side::buy) {
        erase_from_level(bids_, location.price);
    } else {
        erase_from_level(asks_, location.price);
    }
    resting_orders_.erase(location_it);
    return true;
}

std::optional<Price> OrderBook::best_bid() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    return bids_.begin()->first;
}

std::optional<Price> OrderBook::best_ask() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    return asks_.begin()->first;
}

std::optional<Price> OrderBook::spread() const {
    const auto bid = best_bid();
    const auto ask = best_ask();
    if (!bid || !ask) {
        return std::nullopt;
    }
    return *ask - *bid;
}

std::vector<DepthLevel> OrderBook::bid_depth(std::size_t levels) const {
    std::vector<DepthLevel> result;
    result.reserve(std::min(levels, bids_.size()));
    for (const auto& [price, queue] : bids_) {
        Quantity quantity = 0;
        for (const auto& order : queue) {
            quantity += order.quantity;
        }
        result.push_back({price, quantity, queue.size()});
        if (result.size() == levels) {
            break;
        }
    }
    return result;
}

std::vector<DepthLevel> OrderBook::ask_depth(std::size_t levels) const {
    std::vector<DepthLevel> result;
    result.reserve(std::min(levels, asks_.size()));
    for (const auto& [price, queue] : asks_) {
        Quantity quantity = 0;
        for (const auto& order : queue) {
            quantity += order.quantity;
        }
        result.push_back({price, quantity, queue.size()});
        if (result.size() == levels) {
            break;
        }
    }
    return result;
}

std::size_t OrderBook::resting_order_count() const {
    return resting_orders_.size();
}

void OrderBook::validate_new_order(const std::string& order_id, Quantity quantity) const {
    if (resting_orders_.contains(order_id)) {
        throw std::invalid_argument("duplicate order_id: " + order_id);
    }
    if (quantity <= 0) {
        throw std::invalid_argument("quantity must be positive");
    }
}

std::vector<Trade> OrderBook::match(Order& taker, bool market_order) {
    if (taker.side == Side::buy) {
        return match_buy(taker, market_order);
    }
    return match_sell(taker, market_order);
}

std::vector<Trade> OrderBook::match_buy(Order& taker, bool market_order) {
    std::vector<Trade> trades;
    while (taker.quantity > 0 && !asks_.empty()) {
        auto level_it = asks_.begin();
        const auto best_price = level_it->first;
        if (!market_order && *taker.price < best_price) {
            break;
        }

        auto& queue = level_it->second;
        while (taker.quantity > 0 && !queue.empty()) {
            auto& maker = queue.front();
            const auto matched_quantity = std::min(taker.quantity, maker.quantity);
            taker.quantity -= matched_quantity;
            maker.quantity -= matched_quantity;
            trades.push_back({
                symbol_,
                taker.order_id,
                maker.order_id,
                best_price,
                matched_quantity,
            });

            if (maker.quantity == 0) {
                const auto maker_order_id = maker.order_id;
                queue.pop_front();
                resting_orders_.erase(maker_order_id);
            }
        }
        if (queue.empty()) {
            asks_.erase(level_it);
        }
    }
    return trades;
}

std::vector<Trade> OrderBook::match_sell(Order& taker, bool market_order) {
    std::vector<Trade> trades;
    while (taker.quantity > 0 && !bids_.empty()) {
        auto level_it = bids_.begin();
        const auto best_price = level_it->first;
        if (!market_order && *taker.price > best_price) {
            break;
        }

        auto& queue = level_it->second;
        while (taker.quantity > 0 && !queue.empty()) {
            auto& maker = queue.front();
            const auto matched_quantity = std::min(taker.quantity, maker.quantity);
            taker.quantity -= matched_quantity;
            maker.quantity -= matched_quantity;
            trades.push_back({
                symbol_,
                taker.order_id,
                maker.order_id,
                best_price,
                matched_quantity,
            });

            if (maker.quantity == 0) {
                const auto maker_order_id = maker.order_id;
                queue.pop_front();
                resting_orders_.erase(maker_order_id);
            }
        }
        if (queue.empty()) {
            bids_.erase(level_it);
        }
    }
    return trades;
}

void OrderBook::rest(Order order) {
    const auto price = *order.price;
    const auto order_id = order.order_id;
    const auto side = order.side;
    if (side == Side::buy) {
        bids_[price].push_back(std::move(order));
    } else {
        asks_[price].push_back(std::move(order));
    }
    resting_orders_.emplace(order_id, RestingLocation{side, price});
}

}  // namespace mm_engine
