"""Plot per-order latency percentiles from a committed benchmark result."""
import json
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

BG = "#0d1117"
FG = "#e6edf3"
BAR = "#58a6ff"
GRID = "#30363d"


def main(result_path: str, out_path: str) -> None:
    data = json.loads(Path(result_path).read_text())
    labels = ["p50", "p95", "p99", "max"]
    values = [
        data["latency_p50_ns"],
        data["latency_p95_ns"],
        data["latency_p99_ns"],
        data["latency_max_ns"],
    ]
    sysinfo = data.get("system", {})

    fig, ax = plt.subplots(figsize=(10, 5.2), dpi=120)
    fig.patch.set_facecolor(BG)
    ax.set_facecolor(BG)

    bars = ax.bar(labels, values, color=BAR, width=0.6, log=True)
    for rect, value in zip(bars, values):
        ax.text(
            rect.get_x() + rect.get_width() / 2,
            value,
            f"{value:,} ns",
            ha="center",
            va="bottom",
            color=FG,
            fontsize=11,
        )

    ax.set_ylabel("Latency per order (ns, log scale)", color=FG, fontsize=11)
    title = "Python matching engine: per-order latency"
    subtitle = (
        f"{data['orders']:,} orders, {data['warmup_orders']:,} warmup, seed "
        f"{sysinfo.get('seed', '?')}  |  {sysinfo.get('cpu', 'CPU')}, "
        f"Python {sysinfo.get('python', '')}"
    )
    ax.set_title(f"{title}\n{subtitle}", color=FG, fontsize=13, pad=14)

    ax.tick_params(colors=FG)
    for spine in ax.spines.values():
        spine.set_color(GRID)
    ax.yaxis.grid(True, color=GRID, linewidth=0.6)
    ax.set_axisbelow(True)

    fig.tight_layout()
    fig.savefig(out_path, facecolor=BG)
    print(f"wrote {out_path}")


if __name__ == "__main__":
    src = sys.argv[1] if len(sys.argv) > 1 else "results/python_benchmark_ryzen9800x3d.json"
    dst = sys.argv[2] if len(sys.argv) > 2 else "assets/latency_percentiles.png"
    main(src, dst)
