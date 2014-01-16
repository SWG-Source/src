
include_directories(
	${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/udplibrary
)

add_library(CommonAPI
	CommonAPI.cpp
	CommonAPI.h
	CommonAPIStrings.h
	CommonClient.cpp
	CommonClient.h
	CommonMessages.cpp
	CommonMessages.h
)
