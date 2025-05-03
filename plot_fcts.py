import matplotlib.pyplot as plt
import numpy as np
import glob
import os
from collections import defaultdict

input_dir = "FCTs"
output_dir = "Plots"
os.makedirs(output_dir, exist_ok=True)

def parse_fcts(filename):
    with open(filename, 'r') as f:
        return [float(line.split(':', 1)[1].replace('ms', '').strip()) for line in f if line.startswith("FCT")]

data = defaultdict(lambda: defaultdict(dict))
for fname in sorted(glob.glob(os.path.join(input_dir, "fct_w*_t*_l*.log"))):
    base = os.path.splitext(os.path.basename(fname))[0]
    _, w_s, t_s, l_s = base.split('_')
    w = int(w_s[1:])
    t = int(t_s[1:])
    l = float(l_s[1:])

    fcts = parse_fcts(fname)
    if not fcts:
        continue

    data[w][l][t] = {
        'mean': np.mean(fcts),
        'p50': np.percentile(fcts, 50),
        'p75': np.percentile(fcts, 75),
        'p95': np.percentile(fcts, 95),
        'p99': np.percentile(fcts, 99),
    }

transport_labels = {
    0: "PPT",
    1: "DCTCP",
    2: "Cubic",
    3: "PPT++"
}

metric_labels = ['mean', 'p50', 'p75', 'p95', 'p99']

for w in data:
    for l in data[w]:
        fig, ax = plt.subplots(figsize=(10, 6))

        all_tr = list(data[w][l].keys())
        # force PPT++ (3) immediately before PPT (0)
        preferred = [3, 0]
        others = [t for t in sorted(all_tr) if t not in preferred]
        transports = [t for t in preferred if t in all_tr] + others

        x = np.arange(len(metric_labels))
        width = 0.8 / len(transports)

        for j, t in enumerate(transports):
            vals = [data[w][l][t][m] for m in metric_labels]
            offset = (j - (len(transports)-1)/2) * width

            bars = ax.bar(
                x + offset,
                vals,
                width,
                label=transport_labels.get(t, f"T{t}")
            )
            for bar in bars:
                h = bar.get_height()
                ax.annotate(
                    f"{h:.1f}",
                    xy=(bar.get_x() + bar.get_width()/2, h),
                    xytext=(0, 3),
                    textcoords="offset points",
                    ha="center",
                    va="bottom",
                    fontsize=8
                )

        ax.set_xticks(x)
        ax.set_xticklabels(metric_labels)
        ax.set_ylabel("FCT (ms)")
        ax.set_title(f"Workload {w}  Load {l}")
        ax.legend(title="Transport")
        ax.grid(True, linestyle='--', alpha=0.5)

        plt.tight_layout()
        plt.savefig(f"{output_dir}/plot_w{w}_l{l}.png")
        plt.close()

