
set(SHARED_SOURCES
    shared/CentralServerConnection.cpp
    shared/CentralServerConnection.h
    shared/StationPlayersCollector.cpp
    shared/StationPlayersCollector.h
    shared/StationPlayersCollectorImpl.cpp
    shared/StationPlayersCollectorImpl.h
    shared/ConfigStationPlayersCollector.cpp
    shared/ConfigStationPlayersCollector.h
    shared/DatabaseConnection.cpp
    shared/DatabaseConnection.h
    shared/TaskUpdateSPCharacterProfileData.cpp
    shared/TaskUpdateSPCharacterProfileData.h
)

if(WIN32)
	set(PLATFORM_SOURCES "")
else()
	set(PLATFORM_SOURCES
		linux/main.cpp
	)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedCompression/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDatabaseInterface/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedLog/include/public
    
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetwork/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverBase/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverNetworkMessages/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverUtility/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
)

add_executable(StationPlayersCollector
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(StationPlayersCollector
    sharedCompression
    sharedDatabaseInterface
    sharedDatabaseInterface_oci
    sharedDebug
    sharedFile
    sharedFoundation
    sharedLog
    
    sharedMessageDispatch
    sharedNetwork
    sharedNetworkMessages
    sharedRandom
    sharedThread
    serverBase
    serverNetworkMessages
    serverUtility
    archive
    unicode
    ${CMAKE_DL_LIBS}
)
