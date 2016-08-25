cmake_minimum_required(VERSION 2.8)

project(mulodi)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(mulodi
	mulodi4.c
)
