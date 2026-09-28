Clock regression

Run `sh clock-test.sh SOURCE_DIR BUILD_DIR` against matching headers and an existing build of that source. Repeat for each ABI; set CXX if needed. The runner uses CMake flags and static libraries from BUILD_DIR. Legacy compiler-flag parsing does not support paths containing whitespace.

The test drives the actual Scheduler around 2^32 milliseconds and checks Clock return widths and duration saturation. It does not change the host clock or wait for uptime. It tests shared scheduling contracts, not every gameplay timer changed by this branch. A negative control can use an upstream build and its headers, but its failure modes differ by ABI.

Integer parsing regression

Run `sh integer-parsing-test.sh BUILD_DIR` against an existing matching ABI build. Repeat for 32-bit and 64-bit. Set CXX if the compiler is not g++. The runner uses the build’s CMake flags/libraries. Paths containing whitespace in generated compiler flags are not supported by this legacy runner.

`integer_parsing_expected.txt` is an explicit contract oracle: decimal whole-text values within int32/uint32 limits; bits32 accepts INT32_MIN through UINT32_MAX modulo 2^32; unsigned rejects a leading minus even for -0; leading parsing consumes a decimal prefix; Unicode::toInt clamps that prefix to int32. Failed checked reads preserve their initialized sentinel. The remaining lines specify date offsets, rejected Unicode/NUL inputs, and transactional int-array parsing. Expectations were derived from those rules, not recorded from candidate output. Any disagreement must be investigated rather than updating the expected file automatically.

Optional source-file arguments compile a previous implementation into the test in place of a library object for negative controls. This tests parser contracts, not each console command or live persistence operation.
