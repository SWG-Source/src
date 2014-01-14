cmake_minimum_required(VERSION 2.8)

project(unicode)

if(WIN32)
	add_definitions(/D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(src)
