
cmake_minimum_required(VERSION 2.8)

project(swgnge)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(SWG_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SWG_ENGINE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine)
set(SWG_EXTERNALS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(SWG_GAME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/game)

find_package(Boost REQUIRED)
find_package(Iconv REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(STLPort REQUIRED)
find_package(ZLib REQUIRED)

include_directories(${STLPORT_INCLUDE_DIR})

add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(game)
