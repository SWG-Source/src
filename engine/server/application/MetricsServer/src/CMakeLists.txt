
set(SHARED_SOURCES
    shared/ConfigMetricsServer.cpp
    shared/ConfigMetricsServer.h
    shared/FirstMetricsServer.cpp
    shared/FirstMetricsServer.h
    shared/MetricsGatheringConnection.cpp
    shared/MetricsGatheringConnection.h
    shared/MetricsServer.cpp
    shared/MetricsServer.h
    shared/TaskConnection.cpp
    shared/TaskConnection.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/WinMain.cpp
	)
else()
	set(PLATFORM_SOURCES
		linux/main.cpp
	)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedCompression/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
    
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetwork/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverNetworkMessages/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverUtility/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
    ${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/platform/projects
    ${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/udplibrary
)

add_executable(MetricsServer
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(MetricsServer
    sharedCompression
    sharedDebug
    sharedFile
    sharedFoundation
    sharedLog
    sharedMath
    
    sharedMessageDispatch
    sharedNetwork
    sharedNetworkMessages
    sharedRandom
    sharedSynchronization
    sharedThread
    sharedUtility
    serverNetworkMessages
    serverUtility
    archive
    fileInterface
    localization
    localizationArchive
    unicode
    unicodeArchive
    MonAPI2
    ${CMAKE_DL_LIBS}
)
