
set(SHARED_SOURCES
	shared/FirstLocalization.h
	shared/LocalizationManager.cpp
	shared/LocalizationManager.h
	shared/LocalizedString.cpp
	shared/LocalizedString.h
	shared/LocalizedStringTable.cpp
	shared/LocalizedStringTable.h
	shared/LocalizedStringTableReaderWriter.cpp
	shared/LocalizedStringTableReaderWriter.h
	shared/StringId.cpp
	shared/StringId.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstLocalization.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/fileInterface/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
)

add_library(localization
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
