# Set up

```bash
# Create link to cloned ns3 repository
ln -s [path/to/ns-3-dev/]       ns-3-dev

# Create links to the ppt implementation
ln -s tcp-ppt.cc                ns-3-dev/src/internet/model/tcp-ppt.cc
ln -s tcp-ppt.h                 ns-3-dev/src/internet/model/tcp-ppt.h
rm                              ns-3-dev/src/internet/CMakeLists.txt
ln -s CMakeLists.txt            ns-3-dev/src/internet/CMakeLists.txt

# Create links to new socket src files
rm                              ns-3-dev/src/internet/model/tcp-socket-state.h
ln -s tcp-socket-state.h        ns-3-dev/src/internet/model/tcp-socket-state.h
rm                              ns-3-dev/src/internet/model/tcp-socket-base.cc
ln -s tcp-socket-base.cc        ns-3-dev/src/internet/model/tcp-socket-base.cc

# Create links to the ppt workloads
ln -s tcp-ppt-example.cc        ns-3-dev/src/internet/examples/tcp-ppt-example.cc
rm                              ns-3-dev/src/internet/examples/CMakeLists.txt
ln -s examples-CMakeLists.txt   ns-3-dev/src/internet/examples/CMakeLists.txt

# Create links to the new tx buffer files
rm                              ns-3-dev/src/internet/model/tcp-tx-buffer.h
ln -s tcp-tx-buffer.h           ns-3-dev/src/internet/model/tcp-tx-buffer.h
rm                              ns-3-dev/src/internet/model/tcp-tx-buffer.cc
ln -s tcp-tx-buffer.cc          ns-3-dev/src/internet/model/tcp-tx-buffer.cc
```

# Build

```bash
ns-3-dev/ns3 clean
ns-3-dev/ns3 configure --build-profile=debug --enable-examples --enable-tests
ns-3-dev/ns3 build
```

Note: you only need build once; rebuilding after making modifications will be done automatically when you run `./ns3 run tcp-ppt-example ...args`.

# Run

Run `ns-3-dev/ns3 run tcp-ppt-example -- --help` to see the argument options.

An example runtime is provided below.
```bash
ns-3-dev/ns3 run tcp-ppt-example -- \
    --sim_time=60 \
    --transport=0 \
    --workload=0 \
    --load=0.5 \
    --n_sources=15 \
    --n_dests=15
```

To analyze cwnd sizes after running a simulation, run the following.
```bash
python3 plot_cwnd.py ns-3-dev/cwnd_sizes.log img.png --rank 1
```


# TODO
  - [X] Linear window decrease
  - [ ] Determine LWD decay rate based on growth rate
  - [X] Explain that the throughput is worse than DCTCP, but that we improve on PPT nonetheless
    - Optimizing for FCT while failing to reduce starvation
    - Essentially borrowing an idea from CPU scheduling
  - [ ] Explain why congestion collapse won't occur

  - [X] Collect cwnd sizes. Create a graph that demonstrates superposition.
  - [X] Modify \*example.cc to spit out all FCTs.
  - [X] Foreach transport in PPT orig, PPT w/ superposition, DCTCP, and Homa:
    - Foreach workload in DataMining and WebSearch
      - Foreach load in 0.3, 0.7
        - Determine average FCT 50th, 75th, 95th, and 99th percentiles.

  - Results needed:
    - [X] Show where LCP can be used + show sum of LCP & HCP
    - [ ] Show that sending packets at BDP is bad (don't decrease LCP and increase load 10x)
      - If we get RC3 working we won't need to do this
      - future Sasha: RC3 doesn't work---no open-source NS-3 implementation exists, will need to test
    - [X] Compare LWD and SP to PPT
    - [X] Compare overall performance of PPT++ with PPT, and DCTCP
    - [X] Show why superposition is important

  - Setup:
    - [X] Make table

  - Digging deeper:
    - [X] Show that throughput is worse
    - [X] Show that PPT++ has higher FCT when the network is oversubscribed
    - [ ] Show that PPT++ does not have lower throughput when we just remove the dual loop mechanism
    - [ ] Get number of dropped packets

  - [ ] Get optimal LWD rate
  - [ ] Explain mathematically why priorities lead to lower FCTs but higher starvation

  - [ ] Ensure that (cwnd) comes before any usages of ``cwnd''


