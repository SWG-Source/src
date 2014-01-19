
cmake_minimum_required(VERSION 2.8)

project(swgnge CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(SWG_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SWG_ENGINE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine)
set(SWG_EXTERNALS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(SWG_GAME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/game)

find_package(Boost REQUIRED)
find_package(JNI REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(Oracle REQUIRED)
find_package(PCRE REQUIRED)
find_package(Perl REQUIRED)
find_package(STLPort REQUIRED)
find_package(ZLIB REQUIRED)

if(WIN32)
	find_package(Iconv REQUIRED)
	add_definitions(/D_CRT_SECURE_NO_WARNINGS /wd4244 /wd4996 /wd4018 /wd4351)
elseif(UNIX)
	add_definitions(-DLINUX -D_REENTRANT -Dlinux -D_USING_STL -D__STL_NO_BAD_ALLOC -D_GNU_SOURCE)
	set(CMAKE_CXX_FLAGS "-Wno-write-strings ${CMAKE_CXX_FLAGS}")
endif()

include_directories(${STLPORT_INCLUDE_DIR})

add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(game)
