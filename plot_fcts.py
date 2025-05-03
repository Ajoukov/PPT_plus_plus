import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import glob
import os
from collections import defaultdict

input_dir = "FCTs"
output_dir = "Plots"
os.makedirs(output_dir, exist_ok=True)

def parse_fcts(filename):
    with open(filename, 'r') as f:
        return [float(line.split(':')[1].replace('ms', '').strip())
                for line in f if line.startswith("FCT")]

data = defaultdict(lambda: defaultdict(dict))

for fname in sorted(glob.glob(os.path.join(input_dir, "fct_w*_t*_l*.log"))):
    parts = os.path.splitext(os.path.basename(fname))[0].split('_')
    w = int(parts[1][1:])
    t = int(parts[2][1:])
    l = float(parts[3][1:])

    fcts = parse_fcts(fname)
    if not fcts:
        continue

    stats = {
        'mean': np.mean(fcts),
        'median': np.median(fcts),
        'p75': np.percentile(fcts, 75),
        'p95': np.percentile(fcts, 95),
        'p99': np.percentile(fcts, 99)
    }

    data[w][l][t] = stats

transport_labels = {
    0: "PPT",
    1: "DCTCP",
    2: "Cubic",
    3: "PPT++"
}
metric_labels = ['mean', 'median', 'p50', 'p75', 'p95', 'p99']

for w in data:
    for l in data[w]:
        fig, ax = plt.subplots(figsize=(10, 6))
        all_stats = []
        x_labels = []

        for t in sorted(data[w][l]):
            stat = data[w][l][t]
            all_stats.append([stat[m] for m in metric_labels])
            x_labels.append(transport_labels.get(t, f"T{t}"))

        all_stats = np.array(all_stats)
        x = np.arange(len(x_labels))
        width = 0.15

        for i, m in enumerate(metric_labels):
            offset = (i - 2) * width
            ax.bar(x + offset, all_stats[:, i], width, label=m)

        ax.set_xticks(x)
        ax.set_xticklabels(x_labels)
        ax.set_ylabel("FCT (ms)")
        ax.set_title(f"Workload {w} Load {l}")
        ax.legend(title="Metric")
        ax.grid(True, linestyle='--', alpha=0.5)
        plt.tight_layout()
        plt.savefig(f"{output_dir}/plot_w{w}_l{l}.png")
        plt.close()

