
set(SHARED_SOURCES
	shared/ConfigSharedLog.cpp
	shared/ConfigSharedLog.h
	shared/TailFileLogObserver.cpp
	shared/TailFileLogObserver.h
	shared/FileLogObserver.cpp
	shared/FileLogObserver.h
	shared/FirstSharedLog.cpp
	shared/FirstSharedLog.h
	shared/Log.h
	shared/LogManager.cpp
	shared/LogManager.h
	shared/LogObserver.cpp
	shared/LogObserver.h
	shared/NetLogConnection.cpp
	shared/NetLogConnection.h
	shared/NetLogObserver.cpp
	shared/NetLogObserver.h
	shared/SetupSharedLog.cpp
	shared/SetupSharedLog.h
	shared/StderrLogger.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/StderrLogger.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES
		linux/StderrLogger.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetwork/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/fileInterface/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicodeArchive/include/public
)

add_library(sharedLog STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(sharedLog
	sharedNetworkMessages
)
