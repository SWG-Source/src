cmake_policy(SET CMP0003 OLD) # or cmake_policy(VERSION 2.4)

cmake_minimum_required(VERSION 2.8)

project(swgnge C CXX)

if(WIN32)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/win32")
elseif(UNIX)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/linux")
endif()

set(SWG_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SWG_ENGINE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine)
set(SWG_EXTERNALS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(SWG_EXTERNALS_FIND ${CMAKE_CURRENT_SOURCE_DIR}/external/3rd/library)
set(SWG_GAME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/game)

include_directories(/usr/include/i386-linux-gnu)

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

if(UNIX)
	# Maybe use the nuGet version instead? possibly can do the same for many of the other libs...
	find_package(CURL REQUIRED)
	find_package(MULODI REQUIRED)
endif()

if(WIN32)
	find_package(Iconv REQUIRED)
 	#Dont-Build-PDB RELEASE build use the following (by either commenting---uncommenting the line as needed)
 	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:libc.lib /SAFESEH:NO")

	####################################
 	#Do-Build-PDB RELEASE build use the following (by either commenting---uncommenting the line as needed)
	#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DWIN32 -Dwin32 -DUDP_LIBRARY -DDEBUG_LEVEL=0 -DPRODUCTION=1 /Oi /Ot /Oy /O2 /GF /Gy /Zi /MT -D_USE_32BIT_TIME_T=1 -D_MBCS -DPLATFORM_BASE_SINGLE_THREAD -D_CRT_SECURE_NO_WARNINGS /MP /wd4244 /wd4996 /wd4018 /wd4351 /Zc:wchar_t- /Ob1 /FC")
 	#Dont-Build-PDB RELEASE build use the following (by either commenting---uncommenting the line as needed)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DWIN32 -Dwin32 -DUDP_LIBRARY -DDEBUG_LEVEL=0 -DPRODUCTION=1 /Oi /Ot /Oy /O2 /GF /Gy /MT -D_USE_32BIT_TIME_T=1 -D_MBCS -DPLATFORM_BASE_SINGLE_THREAD -D_CRT_SECURE_NO_WARNINGS /MP /wd4244 /wd4996 /wd4018 /wd4351 /Zc:wchar_t- /Ob1 /FC")
 
	####################################
	#Standard DEBUG build use the following	(by either commenting---uncommenting the line as needed)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DWIN32 -Dwin32 -D_DEBUG -DUDP_LIBRARY -DDEBUG_LEVEL=2 -DPRODUCTION=0 /MTd -D_USE_32BIT_TIME_T=1 -D_MBCS -DPLATFORM_BASE_SINGLE_THREAD -D_CRT_SECURE_NO_WARNINGS /MP /wd4244 /wd4996 /wd4018 /wd4351 /Zc:wchar_t- /Ob1 /FC")
 
elseif(UNIX)
	find_package(Curses REQUIRED)
	
	# linker flags
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-O1,--sort-common,--as-needed,-z,relro")
	
	# flto doesn't want to work
	#if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS},-flto")
	#endif()
	
	# don't put anything too crazy in debug...and any common flags go into CMAKE_CXX_FLAGS 
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG_LEVEL=2 -DPRODUCTION=0 -O0")
	
	# funroll loops and everything else potentially crazy for performance here
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEBUG_LEVEL=0 -DPRODUCTION=1 -O3 -funroll-loops")
	
	# TODO: The below flags will enhance security but at the cost of performance, arguably... 
	# currently with them SWG cluster idles at ~17-18% of 24 cpus, and ata round ~14-17% without
	# there is likely also a latency/speed impact as well
	# not sure if needed or not - https://wiki.debian.org/Hardening
	# ============================================================================================
	# -fstack-protector-all -Wstack-protector --param ssp-buffer-size=4 -fPIE -ftrapv
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -D_GLIBCXX_USE_CXX11_ABI=0 \
		-m32 -pipe -march=native -mtune=native \
		-D_FORTIFY_SOURCE=2 -Wformat \
		-Wno-overloaded-virtual -Wno-missing-braces -Wno-format \
		-Wno-write-strings -Wno-unknown-pragmas \
		-Wno-uninitialized -Wno-reorder")
	
	# flto doesn't want to work
	#if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
	#endif()
		
	add_definitions(-DLINUX -D_REENTRANT -Dlinux -D_USING_STL -D__STL_NO_BAD_ALLOC -D_GNU_SOURCE -D_XOPEN_SOURCE=500)
endif()

add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(game)
