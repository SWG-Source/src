

set(SHARED_SOURCES
	shared/FirstLocalizationArchive.h
	shared/StringIdArchive.cpp
	shared/StringIdArchive.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstLocalizationArchive.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
)

add_library(localizationArchive
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
