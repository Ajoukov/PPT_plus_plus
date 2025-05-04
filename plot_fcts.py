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
    1: "DCTCP",
    0: "PPT",
    3: "PPT++ (Superposition)",
    4: "PPT++ (LWD)",
    5: "PPT++ (Superposition & LWD)",
}

ordered_transports = [1, 0, 3, 4, 5]

metric_labels = [
    'Average(ms)',
    '50th Percentile(ms)',
    '75th Percentile(ms)',
    '95th Percentile(ms)',
    '99th Percentile(ms)'
]

hatch_map = {
    1: '',
    0: '',
    3: '/////',
    4: '\\\\\\\\',
    5: 'xxxxxx',
}

color_map = {
    1: '#666666',  # DCTCP – darker gray
    0: '#cccccc',  # others – light gray
    3: '#cccccc',
    4: '#cccccc',
    5: '#cccccc',
}

for (w, l), group in df.groupby(['Workload', 'Load']):
    fig, ax = plt.subplots(figsize=(10, 6), facecolor='white')
    ax.set_facecolor('#f5f5f5')

    codes = group['TransportCode'].unique()
    transports = [t for t in ordered_transports if t in codes]

    x = np.arange(len(metric_labels))
    width = 0.8 / len(transports)

    legend_handles = []

    for j, t in enumerate(transports):
        row = group[group['TransportCode'] == t]
        if row.empty:
            continue
        vals = row.iloc[0][metric_labels].values.astype(float)
        offset = (j - (len(transports) - 1) / 2) * width
        hatch = hatch_map.get(t, '')
        color = color_map.get(t, '#cccccc')
        label = transport_labels.get(t, f"T{t}")

        bars = ax.bar(
            x + offset,
            vals,
            width,
            facecolor=color,
            edgecolor='black',
            hatch=hatch,
            linewidth=1.2,
            zorder=3
        )
        for bar in bars:
            h = bar.get_height()
            ax.annotate(
                f"{int(round(h))}",
                xy=(bar.get_x() + bar.get_width() / 2, h),
                xytext=(0, 3),
                textcoords="offset points",
                ha="center",
                va="bottom",
                fontsize=6)

        legend_handles.append(Patch(facecolor=color, edgecolor='black', hatch=hatch, label=label))

    ax.set_xticks(x)
    ax.set_xticklabels(metric_labels)
    ax.set_ylabel("FCT (ms)")

    legend_box = ax.legend(
        handles=legend_handles,
        title="Transport",
        frameon=True,
        loc='upper left'
    )
    legend_box.get_frame().set_facecolor('#ffffff')
    legend_box.get_frame().set_edgecolor('#000000')

    ax.grid(True, linestyle='--', alpha=0.5, color='black', zorder=0)

    fig.subplots_adjust(left=0, right=1, top=1, bottom=0)

    plt.savefig(f"{output_dir}/plot_w{w}_l{l}.pdf", bbox_inches='tight', pad_inches=0)
    plt.close()

