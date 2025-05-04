import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import os
from matplotlib.patches import Patch

plt.rcParams['hatch.linewidth'] = 1.0

input_file = "FCTs.csv"
output_dir = "Plots"
os.makedirs(output_dir, exist_ok=True)

df = pd.read_csv(input_file)
df['TransportCode'] = df['Transport'].apply(lambda x: int(str(x).lstrip('t')))

transport_labels = {
    0: "PPT",
    1: "DCTCP",
    2: "Cubic",
    3: "PPT++ (Superposition)"
    4: "PPT++ (LWD)"
    5: "PPT++ (Superposition & LWD)"
}

metric_labels = [
    'Average(ms)',
    '50th Percentile(ms)',
    '75th Percentile(ms)',
    '95th Percentile(ms)',
    '99th Percentile(ms)'
]

hatches = [
    '\\\\\\\\\\',
    '/////',
    'xxxxxx',
    '++++++',
    '......',
    'oooooo',
    '**'
    'OO',
]

for (w, l), group in df.groupby(['Workload', 'Load']):
    fig, ax = plt.subplots(figsize=(10, 6))
    fig.patch.set_facecolor('#e0e0e0')
    ax.set_facecolor('#e0e0e0')

    codes = sorted(group['TransportCode'].unique())
    preferred = [3, 0]
    others    = [c for c in codes if c not in preferred]
    transports = [c for c in preferred if c in codes] + others

    x = np.arange(len(metric_labels))
    width = 0.8 / len(transports)

    legend_handles = []

    for j, t in enumerate(transports):
        row = group[group['TransportCode'] == t]
        if row.empty:
            continue
        vals = row.iloc[0][metric_labels].values.astype(float)
        offset = (j - (len(transports)-1)/2) * width
        hatch = hatches[j % len(hatches)]
        label = transport_labels.get(t, f"T{t}")

        bars = ax.bar(
            x + offset,
            vals,
            width,
            facecolor='#cccccc',
            edgecolor='black',
            hatch=hatch,
            linewidth=1.2
        )
        for bar in bars:
            h = bar.get_height()
            ax.annotate(
                f"{h:.1f}",
                xy=(bar.get_x()+bar.get_width()/2, h),
                xytext=(0, 3),
                textcoords="offset points",
                ha="center",
                va="bottom",
                fontsize=8)
        legend_handles.append(Patch(facecolor='#cccccc', edgecolor='black', hatch=hatch, label=label))

    ax.set_xticks(x)
    ax.set_xticklabels(metric_labels)
    ax.set_ylabel("FCT (ms)")
    w2wl = {"w0":"Web Search", "w1":"Data Mining"}
    ax.set_title(f"{w2wl[w]} at Load {l}")
    ax.legend(handles=legend_handles, title="Transport", frameon=False)
    ax.grid(True, linestyle='--', alpha=0.5, color='black')

    plt.tight_layout()
    plt.savefig(f"{output_dir}/plot_w{w}_l{l}.pdf")
    plt.close()

