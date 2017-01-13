if (${CMAKE_BUILD_TYPE} STREQUAL "MINSIZEREL" AND EXISTS "${PROJECT_SOURCE_DIR}/src/linux/LogServer.profdata")
	add_compile_options(-fprofile-instr-use=${PROJECT_SOURCE_DIR}/src/linux/LogServer.profdata)
endif()

set(SHARED_SOURCES
    shared/ConfigLogServer.cpp
    shared/ConfigLogServer.h
    shared/DatabaseLogObserver.cpp
    shared/DatabaseLogObserver.h
    shared/FirstLogServer.cpp
    shared/FirstLogServer.h
    shared/LoggerConnection.cpp
    shared/LoggerConnection.h
    shared/LoggingServerApi.cpp
    shared/LoggingServerApi.h
    shared/LoggingServerApiObserver.cpp
    shared/LoggingServerApiObserver.h
    shared/LogServer.cpp
    shared/LogServer.h
)

if(WIN32)
	set(PLATFORM_SOURCES
        win32/LoggingServerApiWrapper.cpp
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
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedLog/include/public
	
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetwork/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverUtility/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/singleton/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicodeArchive/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/udplibrary
	${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/webAPI
)

add_executable(LogServer
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(LogServer
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
	webAPI
	${CURL_LIBRARIES}
    ${CMAKE_DL_LIBS}
)
