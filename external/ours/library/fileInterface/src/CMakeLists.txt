

set(SHARED_SOURCES
	shared/AbstractFile.cpp
	shared/AbstractFile.h
	shared/FirstFileInterface.h
	shared/StdioFile.cpp
	shared/StdioFile.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstFileInterface.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared)

add_library(fileInterface
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
