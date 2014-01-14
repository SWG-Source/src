

set(SHARED_SOURCES
	shared/FirstUnicode.h
	shared/UnicodeBlocks.cpp
	shared/UnicodeBlocks.h
	shared/UnicodeCharacterData.cpp
	shared/UnicodeCharacterData.h
	shared/UnicodeCharacterDataMap.cpp
	shared/UnicodeCharacterDataMap.h
	shared/Unicode.cpp
	shared/Unicode.h
	shared/UnicodeUtils.cpp
	shared/UnicodeUtils.h
	shared/utf8.cpp
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstUnicode.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared)

add_library(unicode
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
