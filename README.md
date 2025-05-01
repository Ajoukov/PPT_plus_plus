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
