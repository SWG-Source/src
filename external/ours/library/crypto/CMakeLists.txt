cmake_minimum_required(VERSION 2.8)

project(crypto)

if(WIN32)
	add_definitions(/D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)
