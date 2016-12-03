
set(SHARED_SOURCES
	shared/FirstSharedIoWin.h
	shared/ConfigSharedIoWin.cpp
	shared/ConfigSharedIoWin.h
	shared/IoWin.cpp
	shared/IoWin.def
	shared/IoWin.h
	shared/IoWinManager.cpp
	shared/IoWinManager.h
	shared/SetupSharedIoWin.cpp
	shared/SetupSharedIoWin.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedIoWin.cpp
	)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
)

add_library(sharedIoWin STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
