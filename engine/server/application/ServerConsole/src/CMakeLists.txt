
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
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
)

add_executable(ServerConsole
    ${SHARED_SOURCES}
    ${PLATFORM_SOURCES}
)

target_link_libraries(ServerConsole
    sharedCompression
    sharedDebug
    sharedFile
    sharedFoundation
    
    sharedMessageDispatch
    sharedNetwork
    sharedNetworkMessages
    sharedThread
    ${CMAKE_DL_LIBS}
)
