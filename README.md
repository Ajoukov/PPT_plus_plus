# Setup

```bash
# Create link to cloned ns3 repository
ln -s [path/to/ns-3-dev/]       ns-3-dev

# Create links to ppt implementation
ln -s tcp-ppt.cc                ns-3-dev/src/internet/model/tcp-ppt.cc
ln -s tcp-ppt.h                 ns-3-dev/src/internet/model/tcp-ppt.h
rm                              ns-3-dev/src/internet/CMakeLists.txt
ln -s CMakeLists.txt            ns-3-dev/src/internet/CMakeLists.txt

# Create links to new socket src files
rm                              ns-3-dev/src/internet/model/tcp-socket-state.h
ln -s tcp-socket-state.h        ns-3-dev/src/internet/model/tcp-socket-state.h
rm                              ns-3-dev/src/internet/model/tcp-socket-base.cc
ln -s tcp-socket-base.cc        ns-3-dev/src/internet/model/tcp-socket-base.cc

# Create links to ppt example
ln -s tcp-ppt-example.cc        ns-3-dev/src/internet/examples/tcp-ppt-example.cc
rm                              ns-3-dev/src/internet/examples/CMakeLists.txt
ln -s examples-CMakeLists.txt   ns-3-dev/src/internet/examples/CMakeLists.txt
```

# Build

```bash
ns-3-dev/ns3 clean
ns-3-dev/ns3 configure --build-profile=debug --enable-examples --enable-tests
ns-3-dev/ns3 build
```

# Run
```bash
ns-3-dev/ns3 run tcp-ppt-example
```

