#!/bin/bash

# transports=(0 1 2 3) # PPT, DCTCP, PPT++
# transports=(0 3) # PPT, DCTCP, PPT++ w/ SP
# transports=(4 5) # PPT++ w/ LWD, PPT++ w/ SP&LWD
transports=(0 1 2 3 4 5) # PPT++ w/ LWD, PPT++ w/ SP&LWD
# transports=(0) # PPT, DCTCP, PPT++
# workloads=(0 1) # 0: WebSearch, 1: DataMining
workloads=(0) # 0: WebSearch, 1: DataMining
# loads=(0.3 0.5 0.7)
# loads=(0.33 1.0 3.0)
loads=(0.1)
sim_time=2.5
folder="FCTs"

mkdir -p $folder

for transport in "${transports[@]}"; do
  for workload in "${workloads[@]}"; do
    for load in "${loads[@]}"; do
      output_file="fct_w${workload}_t${transport}_l${load}.log"
      echo "Running workload=${workload}, transport=${transport}, load=${load}"      
      stdbuf -oL \
      ns-3-dev/ns3 run tcp-ppt-example -- \
          --workload=${workload} \
          --transport=${transport} \
          --sim_time=${sim_time} \
           --load=${load} \
           > $folder/${output_file} 2> $folder/${output_file} &
    done
  done
done
