
cmake_minimum_required(VERSION 2.8)

project(swgnge C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(ENABLE_DEBUG "Enable Debug Mode" ON)

set(SWG_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SWG_ENGINE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine)
set(SWG_EXTERNALS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
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
find_package(STLPort REQUIRED)
find_package(Threads)
find_package(ZLIB REQUIRED)

if(ENABLE_DEBUG)
	add_definitions(-D_DEBUG)
endif()

if(WIN32)
	find_package(Iconv REQUIRED)

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_RELEASE} -DPRODUCTION=0")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DPRODUCTION=1")

	add_definitions(/D_CRT_SECURE_NO_WARNINGS /wd4244 /wd4996 /wd4018 /wd4351)
elseif(UNIX)
	find_package(Curses REQUIRED)

	set(CMAKE_CXX_FLAGS "-Wno-write-strings ${CMAKE_CXX_FLAGS}")

	add_definitions(-DLINUX -D_REENTRANT -Dlinux -D_USING_STL -D__STL_NO_BAD_ALLOC -D_GNU_SOURCE -D_XOPEN_SOURCE=500)
endif()

include_directories(${STLPORT_INCLUDE_DIR})
link_directories(${STLPORT_LIBDIR})

add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(game)
