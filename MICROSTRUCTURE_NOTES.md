# Microstructure Notes

## Concepts Modeled

- Bid: resting buy liquidity.
- Ask: resting sell liquidity.
- Spread: best ask minus best bid.
- Marketable limit order: limit order that crosses the opposite side.
- Queue priority: earlier orders at the same price fill first.
- Partial fill: an order is filled in part and the residual may continue matching or rest.

## Why This Is Better Than A Stock Predictor Repo

Prediction notebooks are easy to overfit and hard to evaluate. Matching engines expose concrete engineering questions: deterministic state transitions, order priority, invariant testing, latency, memory layout, replay, and risk controls.

