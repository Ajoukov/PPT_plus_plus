# Create a symlink to the library

ln -s [path/to/ns-3-dev/] ns-3-dev

# Run the dctcp example

./ns-3-dev/ns3 run dctcp-example

# Resources

https://www.nsnam.org/docs/release/3.44/tutorial/html/index.html
https://www.nsnam.org/docs/release/3.44/tutorial/html/conceptual-overview.html

# Make

```bash
cd ns-3-dev
mkdir build-ppt
cd build-ppt
cmake ..
make -j$(nproc)
```

