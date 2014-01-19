
include_directories(
	${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/udplibrary
)

add_library(LoginAPI
	Client.cpp
	Client.h
	ClientCore.cpp
	ClientCore.h
	Messages.cpp
	Messages.h
)

target_link_libraries(LoginAPI
	Base
	CommonAPI
)
