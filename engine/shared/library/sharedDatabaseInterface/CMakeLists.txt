cmake_minimum_required(VERSION 2.8)

project(sharedDatabaseInterface)

if(WIN32)
	add_definitions(/D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include/public
	${CMAKE_CURRENT_SOURCE_DIR}/src_oci
)

add_subdirectory(src)
add_subdirectory(src_oci)
