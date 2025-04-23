# Run the dctcp example

./ns-3-dev/ns3 run dctcp-example

# Resources

https://www.nsnam.org/docs/release/3.44/tutorial/html/index.html
https://www.nsnam.org/docs/release/3.44/tutorial/html/conceptual-overview.html

# Make

./ns3 clean
./ns3 configure --build-profile=debug --enable-examples --enable-tests
./ns3 build

# TODO

1) Create 2 queues instead of switching to LCP entirely
2) How to stress test?
3) Linear?



