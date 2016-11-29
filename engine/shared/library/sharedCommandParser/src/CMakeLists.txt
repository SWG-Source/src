
set(SHARED_SOURCES
	shared/CommandParser.cpp
	shared/CommandParser.h
	shared/CommandParserHistory.cpp
	shared/CommandParserHistory.h
	shared/CommandPermissionManager.cpp
	shared/CommandPermissionManager.h
	shared/FirstSharedCommandParser.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedCommandParser.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
)

add_library(sharedCommandParser STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
