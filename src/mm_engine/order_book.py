from __future__ import annotations

from collections import deque
from dataclasses import dataclass
from enum import StrEnum
from typing import Deque


class Side(StrEnum):
    BUY = "buy"
    SELL = "sell"


class OrderType(StrEnum):
    LIMIT = "limit"
    MARKET = "market"


@dataclass
class Order:
    order_id: str
    side: Side
    order_type: OrderType
    quantity: int
    price: int | None
    sequence: int


@dataclass(frozen=True)
class Trade:
    symbol: str
    taker_order_id: str
    maker_order_id: str
    price: int
    quantity: int


class OrderBook:
    def __init__(self, symbol: str) -> None:
        self.symbol = symbol
        self.bids: dict[int, Deque[Order]] = {}
        self.asks: dict[int, Deque[Order]] = {}
        self.orders: dict[str, Order] = {}
        self.sequence = 0

    def add_limit_order(
        self,
        order_id: str,
        side: Side,
        price: int,
        quantity: int,
    ) -> list[Trade]:
        self._validate_new_order(order_id, quantity)
        order = self._new_order(order_id, side, OrderType.LIMIT, quantity, price)
        trades = self._match(order)

        if order.quantity > 0:
            self._rest(order)

        return trades

    def add_market_order(self, order_id: str, side: Side, quantity: int) -> list[Trade]:
        self._validate_new_order(order_id, quantity)
        order = self._new_order(order_id, side, OrderType.MARKET, quantity, None)
        return self._match(order)

    def cancel(self, order_id: str) -> bool:
        order = self.orders.pop(order_id, None)
        if order is None:
            return False

        levels = self.bids if order.side == Side.BUY else self.asks
        assert order.price is not None
        queue = levels.get(order.price)
        if queue is None:
            return False

        levels[order.price] = deque(resting for resting in queue if resting.order_id != order_id)
        if not levels[order.price]:
            del levels[order.price]
        return True

    def best_bid(self) -> int | None:
        return max(self.bids) if self.bids else None

    def best_ask(self) -> int | None:
        return min(self.asks) if self.asks else None

    def spread(self) -> int | None:
        bid = self.best_bid()
        ask = self.best_ask()
        if bid is None or ask is None:
            return None
        return ask - bid

    def depth(self, levels: int = 5) -> dict[str, list[dict[str, int]]]:
        return {
            "bids": self._depth_side(self.bids, reverse=True, levels=levels),
            "asks": self._depth_side(self.asks, reverse=False, levels=levels),
        }

    def _new_order(
        self,
        order_id: str,
        side: Side,
        order_type: OrderType,
        quantity: int,
        price: int | None,
    ) -> Order:
        self.sequence += 1
        return Order(
            order_id=order_id,
            side=side,
            order_type=order_type,
            quantity=quantity,
            price=price,
            sequence=self.sequence,
        )

    def _match(self, taker: Order) -> list[Trade]:
        trades: list[Trade] = []
        opposite = self.asks if taker.side == Side.BUY else self.bids

        while taker.quantity > 0 and opposite:
            best_price = min(opposite) if taker.side == Side.BUY else max(opposite)
            if not self._is_crossing(taker, best_price):
                break

            queue = opposite[best_price]
            while taker.quantity > 0 and queue:
                maker = queue[0]
                matched_quantity = min(taker.quantity, maker.quantity)
                taker.quantity -= matched_quantity
                maker.quantity -= matched_quantity
                trades.append(
                    Trade(
                        symbol=self.symbol,
                        taker_order_id=taker.order_id,
                        maker_order_id=maker.order_id,
                        price=best_price,
                        quantity=matched_quantity,
                    )
                )

                if maker.quantity == 0:
                    queue.popleft()
                    self.orders.pop(maker.order_id, None)

            if not queue:
                del opposite[best_price]

        return trades

    def _is_crossing(self, taker: Order, best_price: int) -> bool:
        if taker.order_type == OrderType.MARKET:
            return True
        assert taker.price is not None
        if taker.side == Side.BUY:
            return taker.price >= best_price
        return taker.price <= best_price

    def _rest(self, order: Order) -> None:
        assert order.price is not None
        levels = self.bids if order.side == Side.BUY else self.asks
        levels.setdefault(order.price, deque()).append(order)
        self.orders[order.order_id] = order

    def _depth_side(
        self,
        side: dict[int, Deque[Order]],
        reverse: bool,
        levels: int,
    ) -> list[dict[str, int]]:
        rows = []
        for price in sorted(side, reverse=reverse)[:levels]:
            rows.append(
                {
                    "price": price,
                    "quantity": sum(order.quantity for order in side[price]),
                    "orders": len(side[price]),
                }
            )
        return rows

    def _validate_new_order(self, order_id: str, quantity: int) -> None:
        if order_id in self.orders:
            raise ValueError(f"duplicate order_id: {order_id}")
        if quantity <= 0:
            raise ValueError("quantity must be positive")

