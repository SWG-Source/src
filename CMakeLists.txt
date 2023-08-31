cmake_minimum_required(VERSION 2.8)
include(CMakePrintHelpers)

project(stellabellum C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/linux")

set(SWG_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SWG_ENGINE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine)
set(SWG_EXTERNALS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(SWG_EXTERNALS_FIND ${CMAKE_CURRENT_SOURCE_DIR}/external/3rd/library)
set(SWG_GAME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/game)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(JNI REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(Oracle REQUIRED)
find_package(PCRE REQUIRED)
find_package(Perl REQUIRED)
find_package(Threads)
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)
find_package(Curses REQUIRED)

# c++17 yeah!
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Global CMake CXX Flags
# our "always on" flags - build by default for the system we're on but include all instruction sets
set(CMAKE_CXX_FLAGS "\
	${CMAKE_CXX_FLAGS} \
	-pipe \
	-march=native \
	-maes \
	-Wformat \
	-Wno-overloaded-virtual \
	-Wno-missing-braces \
	-Wno-format \
	-Wno-write-strings \
	-Wno-unknown-pragmas \
	-Wno-uninitialized \
	-Wno-reorder \
	-Wno-tautological-constant-out-of-range-compare \
	-Wno-address-of-packed-member \
	-Wfatal-errors \
	-ffast-math \
")

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

# Global Linker flags
set(CMAKE_SHARED_LINKER_FLAGS 	"${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,norelro,-O3,--sort-common,--as-needed,--relax,-z,combreloc,-z,global,--no-omagic")
set(CMAKE_EXE_LINKER_FLAGS 		"${CMAKE_EXE_LINKER_FLAGS}    -Wl,-z,norelro,-O3,--sort-common,--as-needed,--relax,-z,combreloc,-z,global,--no-omagic")

# Add on flags to strip debug symbols if RELEASE or MINSIZEREL
if (${CMAKE_BUILD_TYPE} STREQUAL "RELEASE" OR ${CMAKE_BUILD_TYPE} STREQUAL "MINSIZEREL")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS},-x,-s")
	set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS},-x,-s")
endif ()

# Compiler specific settings.
# GNU - These are for GCC 7.5 (the version on the 3.0.x VM)
# Clang - These are for CLang 11 (current version as of Jan-2022), but it is not installed on the 3.0.x VM
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS					"${CMAKE_CXX_FLAGS} -Wno-stringop-overflow")
	set(CMAKE_CXX_FLAGS_DEBUG 			"${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG_LEVEL=2 -DPRODUCTION=0 -pg -ggdb")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 	"${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DDEBUG_LEVEL=0 -DPRODUCTION=0 -pg -ggdb -no-pie")
	set(CMAKE_CXX_FLAGS_MINSIZEREL 		"${CMAKE_CXX_FLAGS_MINSIZEREL}     -DDEBUG_LEVEL=0 -DPRODUCTION=1 -no-pie -flto -fwhole-program-vtables")
	set(CMAKE_CXX_FLAGS_RELEASE 		"${CMAKE_CXX_FLAGS_RELEASE}        -DDEBUG_LEVEL=0 -DPRODUCTION=1")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "CLang")
	set(CMAKE_CXX_FLAGS_DEBUG 			"${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG_LEVEL=2 -DPRODUCTION=0 -pg -ggdb -g -fprofile-instr-generate -fcoverage-mapping")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 	"${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DDEBUG_LEVEL=0 -DPRODUCTION=0 -Ofast -pg -ggdb")
	set(CMAKE_CXX_FLAGS_MINSIZEREL 		"${CMAKE_CXX_FLAGS_MINSIZEREL}     -DDEBUG_LEVEL=0 -DPRODUCTION=1 -no-pie -flto -fwhole-program-vtables")
	set(CMAKE_CXX_FLAGS_RELEASE 		"${CMAKE_CXX_FLAGS_RELEASE}        -DDEBUG_LEVEL=0 -DPRODUCTION=1 -Ofast")
endif()

# Remove whitespace from flags
string(REGEX REPLACE "[ \t\r\n]+" " " CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
string(REGEX REPLACE "[ \t\r\n]+" " " CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
string(REGEX REPLACE "[ \t\r\n]+" " " CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_CXX_FLAGS_MINSIZEREL})
string(REGEX REPLACE "[ \t\r\n]+" " " CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})

message("Using Compiler: ${CMAKE_CXX_COMPILER_ID}")

add_definitions(-DLINUX -D_REENTRANT -Dlinux -D_USING_STL -D_GNU_SOURCE -D_XOPEN_SOURCE=500 -U_FORTIFY_SOURCE)

# this is so some profile specific stuff is turned on in the code
if (${CMAKE_BUILD_TYPE} STREQUAL "RELWITHDEBINFO" OR ${CMAKE_BUILD_TYPE} STREQUAL "DEBUG")
	add_definitions(-DENABLE_PROFILING)
endif ()

if (${CMAKE_BUILD_TYPE} STREQUAL "DEBUG")
	cmake_print_variables(CMAKE_CXX_FLAGS_DEBUG)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "RELWITHDEBINFO")
	cmake_print_variables(CMAKE_CXX_FLAGS_RELWITHDEBINFO)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "MINSIZEREL")
	cmake_print_variables(CMAKE_CXX_FLAGS_MINSIZEREL)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "RELEASE")
	cmake_print_variables(CMAKE_CXX_FLAGS_RELEASE)
endif ()

add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(game)
