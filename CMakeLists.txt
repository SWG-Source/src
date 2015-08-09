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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

find_package(BISON REQUIRED)
find_package(Boost REQUIRED)
find_package(FLEX REQUIRED)
find_package(JNI REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(Oracle REQUIRED)
find_package(PCRE REQUIRED)
find_package(Perl REQUIRED)
find_package(Threads)
find_package(ZLIB REQUIRED)

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

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG_LEVEL=2 -DPRODUCTION=0 -g -pipe -Wall -Wno-unknown-pragmas -Wno-reorder -O0")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEBUG_LEVEL=0 -DPRODUCTION=1 -march=native -pipe -mtune=native -O2 -s")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings")

	add_definitions(-DLINUX -D_REENTRANT -Dlinux -D_USING_STL -D__STL_NO_BAD_ALLOC -D_GNU_SOURCE -D_XOPEN_SOURCE=500)
endif()

add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(game)
