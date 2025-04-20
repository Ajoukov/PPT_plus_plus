# Setup

```bash
ln -s [path/to/ns-3-dev/] ns-3-dev
ln -s [path/to/ns-3-dev/src/internet/model/tcp-ppt.cc] tcp-ppt.cc
ln -s [path/to/ns-3-dev/src/internet/model/tcp-ppt.h]  tcp-ppt.h
```

Append `tcp-ppt.h` and `tcp-ppt.cc` to
`ns-3-dev/src/internet/model/CMakeLists.txt`

Append

```cmake
build_lib_example(
  NAME           tcp-ppt-example
  SOURCE_FILES   tcp-ppt-example.cc
  LIBRARIES_TO_LINK
    ${libinternet}
    ${libnetwork}        # if you use point‑to‑point/helper
    ${libapplications}   # for BulkSendHelper, PacketSinkHelper
    ${libtraffic-control} # for TrafficControlHelper/PrioQueueDisc
)```

to examples/CMakeLists.txt
