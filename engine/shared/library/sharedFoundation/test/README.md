Clock regression

Run `sh clock-test.sh SOURCE_DIR BUILD_DIR` against matching headers and an existing build of that source. Repeat for each ABI; set CXX if needed. The runner uses CMake flags and static libraries from BUILD_DIR. Legacy compiler-flag parsing does not support paths containing whitespace.

The test drives the actual Scheduler around 2^32 milliseconds and checks Clock return widths and duration saturation. It does not change the host clock or wait for uptime. It tests shared scheduling contracts, not every gameplay timer changed by this branch. A negative control can use an upstream build and its headers, but its failure modes differ by ABI.
