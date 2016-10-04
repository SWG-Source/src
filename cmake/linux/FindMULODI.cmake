# when using clang with m32, mulodi4 is missing...this checks for it/not
# if we're using linux/not

include(CheckCXXSourceCompiles)
set(BUILD_MULODI_LIB FALSE)

set(find_mulodi_test_program
"#include <stdint.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
	int64_t x = ((int64_t)atoi(argv[1])) * (int64_t)atoi(argv[2])
	* (int64_t)atoi(argv[3]);
	return x == 9;
}")

set(CMAKE_REQUIRED_FLAGS "-Wl,-O1,--sort-common,--as-needed,-z,relro,-flto -ftrapv -m32")
set(CMAKE_REQUIRED_LIBRARIES "")
CHECK_CXX_SOURCE_COMPILES(
  "${find_mulodi_test_program}"
  mulodi_found
)

if(NOT mulodi_found)
  set(BUILD_MULODI_LIB TRUE)
endif()
