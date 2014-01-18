
include_directories(
	${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/udplibrary
	${ZLIB_INCLUDE_DIR}
)

add_definitions(-DMonitorAPI=)

add_library(MonAPI2
	MonitorAPI.cpp
	MonitorAPI.h
	MonitorData.cpp
	MonitorData.h
)
