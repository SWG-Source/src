cmake_minimum_required(VERSION 2.8)

project(webAPI)

add_library(webAPI
	webAPI.h
	webAPI.cpp
	json.hpp
)

include_directories(
	${CURL_INCLUDE_DIRS}
)
