import numpy as np
import pandas as pd
import glob
import os

X = 2.4
Y = 5.0

def parse_fct(file_path):
    hits = []
    with open(file_path, 'r') as f:
        for line in f:
            # line looks like: "Start @4.48857s: FCT 2317: 1.23248ms"
            if "FCT" in line:
                parts = line.split()
                start_s = float(parts[1].lstrip('@').rstrip('s:'))
                fct_ms = float(parts[-1].rstrip('ms'))
                hits.append((start_s, fct_ms))
    return hits

results = []

for file_path in glob.glob("FCTs/fct_*.log"):
    measurements = parse_fct(file_path)
    fcts = [f for (t, f) in measurements if X <= t <= Y]
    if not fcts:
        continue

    avg  = np.mean(fcts)
    p50  = np.percentile(fcts, 50)
    p75  = np.percentile(fcts, 75)
    p95  = np.percentile(fcts, 95)
    p99  = np.percentile(fcts, 99)

    base = os.path.basename(file_path).removesuffix('.log')
    _, w, t, l = base.split('_')
    results.append((w, t, float(l[1:]), avg, p50, p75, p95, p99))

df = pd.DataFrame(results,
    columns=[
      'Workload','Transport','Load',
      'Average(ms)','50th Percentile(ms)',
      '75th Percentile(ms)','95th Percentile(ms)',
      '99th Percentile(ms)'
    ])
df.sort_values(by=['Workload','Transport','Load'], inplace=True)
df.to_csv("FCTs.csv", index=False)

