

set(SHARED_SOURCES
	shared/FirstUnicodeArchive.h
	shared/UnicodeArchive.cpp
	shared/UnicodeArchive.h
	shared/UnicodeAutoDeltaPackedMap.cpp
	shared/UnicodeAutoDeltaPackedMap.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstUnicodeArchive.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
)

add_library(unicodeArchive
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
