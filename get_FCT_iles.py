import numpy as np
import pandas as pd
import glob

def parse_fct(file_path):
    fcts = []
    with open(file_path, 'r') as f:
        for line in f:
            if line.startswith("FCT"):
                ms = float(line.split(":")[1].strip().replace("ms", ""))
                fcts.append(ms)
    return fcts

results = []

for file_path in glob.glob("FCTs/fct_*.log"):
    fcts = parse_fct(file_path)
    if not fcts:
        continue

    avg = np.mean(fcts)
    p50 = np.percentile(fcts, 50)
    p75 = np.percentile(fcts, 75)
    p95 = np.percentile(fcts, 95)
    p99 = np.percentile(fcts, 99)

    workload, transport, load = file_path.rstrip('.log').split('_')[1:]
    results.append((workload, transport, float(load[1:]), avg, p50, p75, p95, p99))

df = pd.DataFrame(results, columns=['Workload', 'Transport', 'Load', 'Average(ms)', '50th Percentile(ms)', '75th Percentile(ms)', '95th Percentile(ms)', '99th Percentile(ms)'])
df.sort_values(by=['Workload', 'Transport', 'Load'], inplace=True)

df.to_csv("FCTs.csv", index=False)

