cmake_minimum_required(VERSION 2.8)

project(webAPI)

add_library(webAPI
	webAPI.h
	webAPI.cpp
	json.hpp
	webAPIHeartbeat.h
	webAPIHeartbeat.cpp
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CURL_INCLUDE_DIRS}
	${OPENSSL_INCLUDE_DIR}
)
