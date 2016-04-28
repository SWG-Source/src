cmake_minimum_required(VERSION 2.8)

project(webAPI)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(webAPI
	webAPI.h
	webAPI.cpp
	json.hpp
	${CURL_LIBRARIES}
)

include_directories(
	${CURL_INCLUDE_DIRS}
)
