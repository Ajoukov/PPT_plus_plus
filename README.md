# Setup

```bash
# Create link to cloned ns3 repository
ln -s [path/to/ns-3-dev/]   ns-3-dev

# Create links to ppt implementation
ln -s tcp-ppt.cc       [path/to/ns-3-dev/src/internet/model/tcp-ppt.cc]
ln -s tcp-ppt.h        [path/to/ns-3-dev/src/internet/model/tcp-ppt.h]
rm                     [path/to/ns-3-dev/src/internet/CMakeLists.txt]
ln -s CMakeLists.txt   [path/to/ns-3-dev/src/internet/CMakeLists.txt]

# Create links to ppt example
ln -s tcp-ppt-example.cc      [path/to/ns-3-dev/src/internet/examples/tcp-ppt-example.cc]
rm                            [path/to/ns-3-dev/src/internet/examples/CMakeLists.txt]
ln -s examples-CMakeLists.txt [path/to/ns-3-dev/src/internet/examples/CMakeLists.txt]
```

# Build

```bash
./ns3 clean
./ns3 configure --build-profile=debug --enable-examples --enable-tests
./ns3 build
```

