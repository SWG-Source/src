
set(SHARED_SOURCES
    shared/ConfigServerConsole.cpp
    shared/ConfigServerConsole.h
    shared/FirstServerConsole.cpp
    shared/FirstServerConsole.h
    shared/ServerConsoleConnection.cpp
    shared/ServerConsoleConnection.h
    shared/ServerConsole.cpp
    shared/ServerConsole.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/main.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES
		linux/main.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
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
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
)

link_directories(${STLPORT_LIBDIR})

add_executable(ServerConsole
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(ServerConsole
    sharedCompression
    sharedDebug
    sharedFile
    sharedFoundation
    sharedMemoryManager
    sharedMessageDispatch
    sharedNetwork
    sharedNetworkMessages
    sharedThread
    ${STLPORT_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)

if(WIN32)
    target_link_libraries(ServerConsole mswsock ws2_32)
endif()
