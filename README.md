# Setup

```bash
# Create link to cloned ns3 repository
ln -s [path/to/ns-3-dev/]   ns-3-dev

# Create links to ppt implementation
ln -s [path/to/ns-3-dev/src/internet/model/tcp-ppt.cc]  tcp-ppt.cc
ln -s [path/to/ns-3-dev/src/internet/model/tcp-ppt.h]   tcp-ppt.h
rm    [path/to/ns-3-dev/src/internet/CMakeLists.txt]
ln -s [path/to/ns-3-dev/src/internet/CMakeLists.txt]    CMakeLists.txt

# Create links to ppt example
ln -s [path/to/ns-3-dev/src/internet/examples/tcp-ppt-example.cc]   tcp-ppt-example.cc
rm    [path/to/ns-3-dev/src/internet/examples/CMakeLists.txt]
ln -s [path/to/ns-3-dev/src/internet/examples/CMakeLists.txt]       examples-CMakeLists.txt
```

# Build

```bash
./ns3 clean
./ns3 configure --build-profile=debug --enable-examples --enable-tests
./ns3 build
```

