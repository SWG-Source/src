cmake_minimum_required(VERSION 2.8)

project(stellabellum C CXX)

if (WIN32)
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/win32")
elseif (UNIX)
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/linux")
endif ()

set(SWG_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SWG_ENGINE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine)
set(SWG_EXTERNALS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(SWG_EXTERNALS_FIND ${CMAKE_CURRENT_SOURCE_DIR}/external/3rd/library)
set(SWG_GAME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/game)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

include_directories(/usr/include/i386-linux-gnu)

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

# c++17 yeah!
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# shame on you cmake for auto-setting optimization flags!
set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
set(CMAKE_CXX_FLAGS_MINSIZEREL "")

# I'll be honest, win32 is probably very very broken and won't build - it will need worked on so that MSVC uses
# the vs2013+ STL instead of stlport, and probably will thus need removal/modification/addition of TONS of the ifdef blocks for WIN32
if (WIN32)
    find_package(Iconv REQUIRED)

    # Dont-Build-PDB RELEASE build use the following (by either commenting---uncommenting the line as needed)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:libc.lib /SAFESEH:NO")

    # Do-Build-PDB RELEASE build use the following (by either commenting---uncommenting the line as needed)
    #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DWIN32 -Dwin32 -DUDP_LIBRARY -DDEBUG_LEVEL=0 \
    #	-DPRODUCTION=1 /Oi /Ot /Oy /O2 /GF /Gy /Zi /MT -D_USE_32BIT_TIME_T=1 -D_MBCS \
    #	-DPLATFORM_BASE_SINGLE_THREAD -D_CRT_SECURE_NO_WARNINGS /MP /wd4244 /wd4996 /wd4018 /wd4351 \
    #	/Zc:wchar_t- /Ob1 /FC")

    # Dont-Build-PDB RELEASE build use the following (by either commenting---uncommenting the line as needed)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DWIN32 -Dwin32 -DUDP_LIBRARY -DDEBUG_LEVEL=0 -DPRODUCTION=1 \
		/Oi /Ot /Oy /O2 /GF /Gy /MT -D_USE_32BIT_TIME_T=1 -D_MBCS -DPLATFORM_BASE_SINGLE_THREAD -D_CRT_SECURE_NO_WARNINGS \
		/MP /wd4244 /wd4996 /wd4018 /wd4351 /Zc:wchar_t- /Ob1 /FC")

    # Standard DEBUG build use the following (by either commenting---uncommenting the line as needed)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DWIN32 -Dwin32 -D_DEBUG -DUDP_LIBRARY -DDEBUG_LEVEL=2 -DPRODUCTION=0 /MTd \
		-D_USE_32BIT_TIME_T=1 -D_MBCS -DPLATFORM_BASE_SINGLE_THREAD -D_CRT_SECURE_NO_WARNINGS /MP /wd4244 /wd4996 /wd4018 /wd4351 \
		/Zc:wchar_t- /Ob1 /FC")
elseif (UNIX)
    find_package(Curses REQUIRED)

    # linker flags
    if (${CMAKE_BUILD_TYPE} STREQUAL "RELWITHDEBINFO" OR ${CMAKE_BUILD_TYPE} STREQUAL "MINSIZEREL")
        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z,norelro,-O3,--sort-common,--as-needed,--relax,-z,combreloc,-z,global,--no-omagic")
	set(CMAKE_EXE_LINKER_FLAGS "-Wl,-z,norelro,-O3,--sort-common,--as-needed,--relax,-z,combreloc,-z,global,--no-omagic")
    else ()
        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z,norelro,-O3,--sort-common,--as-needed,--relax,-z,combreloc,-z,global,--no-omagic,-x,-s")
	set(CMAKE_EXE_LINKER_FLAGS "-Wl,-z,norelro,-O3,--sort-common,--as-needed,--relax,-z,combreloc,-z,global,--no-omagic,-x,-s")
    endif ()

    # don't put anything too crazy in debug...and any common flags go into CMAKE_CXX_FLAGS
    set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -DDEBUG_LEVEL=2 -DPRODUCTION=0 -O0 -g3")

    # release specific cflags - optimizations beyond the default for both types is mostly just -O flags
    set(CMAKE_CXX_FLAGS_RELEASE "-DDEBUG_LEVEL=0 -DPRODUCTION=1")

    # we use fewer, if any, crazy optimizations for profile generation, and likewise release flags for the minsizerel
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_RELEASE}")

    # Ofast doesn't work with gcc builds
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast \
		-fno-threadsafe-statics -fslp-vectorize \
		-fno-stack-protector -fstrict-enums -finline-hint-functions \
		-fno-coverage-mapping -fno-spell-checking \
		-mno-retpoline")

        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")

        # RELWITHDEBINFO is used for building bins that produce profdata files
	# we only need the basics of our heavy optimizations here, i think - that and one of these flags
	# breaks JNI when we profile
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Ofast -finline-hint-functions -fprofile-instr-generate")

        # MINSIZEREL is used for profiled, flto builds
        set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_RELEASE} -flto -fwhole-program-vtables")
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        # O3 and Ofast include one or more flags that cause java to crash when using gcc6
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -fno-signed-zeros -freciprocal-math -fno-unroll-loops -fno-tree-loop-optimize -fno-plt")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    endif ()

    # our "always on" flags - build by default for the system we're on but include all instruction sets
    set(CMAKE_CXX_FLAGS "-m32 -pipe -march=native -mtune=native \
		-Wformat -Wno-overloaded-virtual -Wno-missing-braces -Wno-format \
		-Wno-write-strings -Wno-unknown-pragmas \
		-Wno-uninitialized -Wno-reorder -Wno-tautological-constant-out-of-range-compare")

    # if you'd like to opt in to our statistics, remove "-DSTELLA_INTERNAL" and your server will send us a heartbeat at startup
    add_definitions(-DLINUX -D_REENTRANT -Dlinux -D_USING_STL -D_GNU_SOURCE -D_XOPEN_SOURCE=500 -U_FORTIFY_SOURCE -DSTELLA_INTERNAL)

    # this is so some profile specific stuff is turned on in the code
    if (${CMAKE_BUILD_TYPE} STREQUAL "RELWITHDEBINFO")
        add_definitions(-DENABLE_PROFILING)
    endif ()
endif ()

add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(game)
