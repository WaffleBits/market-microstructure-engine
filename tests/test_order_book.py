import unittest

from mm_engine import OrderBook, Side


class OrderBookTest(unittest.TestCase):
    def test_limit_order_crosses_and_trades_at_resting_price(self) -> None:
        book = OrderBook("FOO")
        book.add_limit_order("ask-1", Side.SELL, price=10100, quantity=100)

        trades = book.add_limit_order("buy-1", Side.BUY, price=10200, quantity=40)

        self.assertEqual(len(trades), 1)
        self.assertEqual(trades[0].price, 10100)
        self.assertEqual(trades[0].quantity, 40)
        self.assertEqual(book.best_ask(), 10100)
        self.assertEqual(book.depth()["asks"][0]["quantity"], 60)

    def test_price_time_priority_within_level(self) -> None:
        book = OrderBook("FOO")
        book.add_limit_order("ask-1", Side.SELL, price=10100, quantity=50)
        book.add_limit_order("ask-2", Side.SELL, price=10100, quantity=50)

        trades = book.add_limit_order("buy-1", Side.BUY, price=10100, quantity=75)

        self.assertEqual([trade.maker_order_id for trade in trades], ["ask-1", "ask-2"])
        self.assertEqual([trade.quantity for trade in trades], [50, 25])
        self.assertIn("ask-2", book.orders)

    def test_market_order_consumes_best_prices_until_exhausted(self) -> None:
        book = OrderBook("FOO")
        book.add_limit_order("ask-1", Side.SELL, price=10100, quantity=50)
        book.add_limit_order("ask-2", Side.SELL, price=10200, quantity=50)

        trades = book.add_market_order("buy-mkt", Side.BUY, quantity=80)

        self.assertEqual([trade.price for trade in trades], [10100, 10200])
        self.assertEqual([trade.quantity for trade in trades], [50, 30])
        self.assertEqual(book.depth()["asks"][0]["quantity"], 20)

    def test_non_crossing_limit_order_rests(self) -> None:
        book = OrderBook("FOO")
        trades = book.add_limit_order("bid-1", Side.BUY, price=9900, quantity=100)

        self.assertEqual(trades, [])
        self.assertEqual(book.best_bid(), 9900)
        self.assertEqual(book.spread(), None)

    def test_cancel_removes_resting_order(self) -> None:
        book = OrderBook("FOO")
        book.add_limit_order("bid-1", Side.BUY, price=9900, quantity=100)

        self.assertTrue(book.cancel("bid-1"))
        self.assertFalse(book.cancel("missing"))
        self.assertIsNone(book.best_bid())

    def test_rejects_duplicate_resting_order_id(self) -> None:
        book = OrderBook("FOO")
        book.add_limit_order("bid-1", Side.BUY, price=9900, quantity=100)

        with self.assertRaises(ValueError):
            book.add_limit_order("bid-1", Side.BUY, price=9800, quantity=1)


if __name__ == "__main__":
    unittest.main()

