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
  - Test ideas:
    - [X] Linear window decrease
    - [ ] Exponential window decrease in proportion to state variables rather than simply halving every RTT.

  - [X] Create scripts to automate the below.

  - [ ] Collect cwnd sizes. Create a graph that demonstrates superposition.

  - Foreach transport in PPT orig, PPT w/ superposition, DCTCP, and Homa:
    - Foreach workload in DataMining and WebSearch
      - Foreach load in 0.3, 0.5, 0.7
        - [X] Modify \*example.cc to spit out all FCTs.
        - [X] Determine average FCT and 95th and 99th percentiles.

  - Results needed:
    - [ ] Show that sending packets at BDP is bad (don't decrease LCP and increase load 10x)
    - [ ] Show that LWD > EWD
    - [ ] Show that superposition > not
    - [ ] Show that LWD + superposition > EWD + not
    - [ ] Compare overall performance of PPT++ with PPT, DCTCP, and Cubic
    - [X] Show why superposition is important
    - [X] Show where LCP can be used + show sum of LCP & HCP



