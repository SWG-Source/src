
set(SHARED_SOURCES
    shared/ConfigLoginPing.cpp
    shared/ConfigLoginPing.h
    shared/FirstLoginPing.cpp
    shared/FirstLoginPing.h
    shared/LoginConnection.cpp
    shared/LoginConnection.h
    shared/LoginPing.cpp
    shared/LoginPing.h
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
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMemoryManager/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetwork/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverNetworkMessages/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
)

link_directories(${STLPORT_LIBDIR})

add_executable(LoginPing
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(LoginPing
    sharedCompression
    sharedDebug
    sharedFile
    sharedFoundation
    #sharedLog
    #sharedMath
    sharedMemoryManager
    sharedMessageDispatch
    sharedNetwork
    sharedNetworkMessages
    sharedRandom
    sharedSynchronization
    sharedThread
    #sharedUtility
    serverNetworkMessages
    #serverUtility
    archive
    #fileInterface
    #localization
    #localizationArchive
    #unicode
    #unicodeArchive
    #udplibrary
    ${STLPORT_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)

if(WIN32)
    target_link_libraries(LoginPing mswsock ws2_32)
endif()
