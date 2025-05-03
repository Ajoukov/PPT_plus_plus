#!/bin/bash

transports=(0 1 3) # PPT, DCTCP, PPT++
workloads=(0 1) # 0: WebSearch, 1: DataMining
loads=(0.3 0.5 0.7)
sim_time=5.0
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
