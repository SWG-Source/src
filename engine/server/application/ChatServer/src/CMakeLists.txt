
set(SHARED_SOURCES
    shared/CentralServerConnection.cpp 
    shared/CentralServerConnection.h 
    shared/ChatInterface.cpp 
    shared/ChatInterface.h 
    shared/ChatServerAvatarOwner.cpp 
    shared/ChatServerAvatarOwner.h 
    shared/ChatServer.cpp 
    shared/ChatServer.h 
    shared/ChatServerRoomOwner.cpp 
    shared/ChatServerRoomOwner.h 
    shared/ConfigChatServer.cpp
    shared/ConfigChatServer.h 
    shared/ConnectionServerConnection.cpp 
    shared/ConnectionServerConnection.h 
    shared/CustomerServiceServerConnection.cpp
    shared/CustomerServiceServerConnection.h 
    shared/FirstChatServer.cpp 
    shared/FirstChatServer.h 
    shared/GameServerConnection.cpp 
    shared/GameServerConnection.h 
    shared/PlanetServerConnection.h 
    shared/PlanetServerServerConnection.cpp 
    shared/ChatServerMetricsData.cpp 
    shared/ChatServerMetricsData.h
    shared/VChatInterface.h 
    shared/VChatInterface.cpp
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
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedCommandParser/include/public
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedCompression/include/public
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDatabaseInterface/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedGame/include/public
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedLog/include/public
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMath/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMemoryManager/include/public
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetwork/include/public
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
    #${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
    #${SWG_ENGINE_SOURCE_DIR}/server/library/serverKeyShare/include/public
    #${SWG_ENGINE_SOURCE_DIR}/server/library/serverNetworkMessages/include/public
    #${SWG_ENGINE_SOURCE_DIR}/server/library/serverUtility/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
    #${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
    #${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localizationArchive/include/public
    #${SWG_EXTERNALS_SOURCE_DIR}/ours/library/singleton/include
    #${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
    #${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicodeArchive/include/public
    #${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/platform/projects
    #${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/platform/utils
    #${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/udplibrary
)

link_directories(${STLPORT_LIBDIR})

add_executable(ChatServer
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(ChatServer
#    sharedCommandParser
#    sharedCompression
#    sharedDatabaseInterface
#    sharedDatabaseInterface_oci
#    sharedDebug
#    sharedFile
#    sharedFoundation
#    sharedGame
#    sharedLog
#    sharedMath
#    sharedMemoryManager
#    sharedMessageDispatch
#    sharedNetwork
#    sharedNetworkMessages
#    sharedRandom
#    sharedSynchronization
#    sharedThread
#    sharedUtility
#    serverKeyShare
#    serverNetworkMessages
#    serverUtility
#    archive
#    crypto
#    fileInterface
#    localization
#    localizationArchive
#    unicode
#    unicodeArchive
#    Base
#    CommonAPI
#    LoginAPI
#    MonAPI2
#    udplibrary
#    ${ZLIB_LIBRARY}
#    ${ORACLE_LIBRARY}
)

if(WIN32)
    target_link_libraries(ChatServer mswsock ws2_32)
endif()
