#include "mm_engine/order_book.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void test_limit_order_crosses_at_resting_price() {
    mm_engine::OrderBook book("FOO");
    book.add_limit_order("ask-1", mm_engine::Side::sell, 10100, 100);

    const auto trades = book.add_limit_order("buy-1", mm_engine::Side::buy, 10200, 40);

    require(trades.size() == 1, "expected one trade");
    require(trades[0].price == 10100, "trade must execute at resting price");
    require(trades[0].quantity == 40, "unexpected matched quantity");
    require(book.best_ask() == 10100, "partial maker should remain at best ask");
    require(book.ask_depth()[0].quantity == 60, "remaining quantity mismatch");
}

void test_price_time_priority() {
    mm_engine::OrderBook book("FOO");
    book.add_limit_order("ask-1", mm_engine::Side::sell, 10100, 50);
    book.add_limit_order("ask-2", mm_engine::Side::sell, 10100, 50);

    const auto trades = book.add_limit_order("buy-1", mm_engine::Side::buy, 10100, 75);

    require(trades.size() == 2, "expected two fills");
    require(trades[0].maker_order_id == "ask-1", "first maker lost FIFO priority");
    require(trades[1].maker_order_id == "ask-2", "second maker lost FIFO priority");
    require(trades[0].quantity == 50 && trades[1].quantity == 25, "fill quantities mismatch");
}

void test_market_order_sweeps_levels() {
    mm_engine::OrderBook book("FOO");
    book.add_limit_order("ask-1", mm_engine::Side::sell, 10100, 50);
    book.add_limit_order("ask-2", mm_engine::Side::sell, 10200, 50);

    const auto trades = book.add_market_order("buy-mkt", mm_engine::Side::buy, 80);

    require(trades.size() == 2, "market order should sweep two levels");
    require(trades[0].price == 10100 && trades[1].price == 10200, "price priority mismatch");
    require(book.ask_depth()[0].quantity == 20, "unexpected residual ask quantity");
}

void test_cancel_and_duplicate_validation() {
    mm_engine::OrderBook book("FOO");
    book.add_limit_order("bid-1", mm_engine::Side::buy, 9900, 100);

    bool duplicate_rejected = false;
    try {
        book.add_limit_order("bid-1", mm_engine::Side::buy, 9800, 1);
    } catch (const std::invalid_argument&) {
        duplicate_rejected = true;
    }

    require(duplicate_rejected, "duplicate resting order id must be rejected");
    require(book.cancel("bid-1"), "existing order should cancel");
    require(!book.cancel("missing"), "missing order should not cancel");
    require(!book.best_bid().has_value(), "cancelled bid level should be removed");
}

}  // namespace

int main() {
    try {
        test_limit_order_crosses_at_resting_price();
        test_price_time_priority();
        test_market_order_sweeps_levels();
        test_cancel_and_duplicate_validation();
        std::cout << "native order book tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "test failure: " << error.what() << "\n";
        return EXIT_FAILURE;
    }
}
