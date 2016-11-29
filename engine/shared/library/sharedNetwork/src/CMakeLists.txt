
set(SHARED_SOURCES
	shared/Address.h
	shared/BroadcastSock.cpp
	shared/BroadcastSock.h
	shared/ConfigSharedNetwork.cpp
	shared/ConfigSharedNetwork.h
	shared/Connection.cpp
	shared/Connection.h
	shared/ConnectionHandler.cpp
	shared/ConnectionHandler.h
	shared/FirstSharedNetwork.h
	shared/ManagerHandler.cpp
	shared/ManagerHandler.h
	shared/NetworkHandler.cpp
	shared/NetworkHandler.h
	shared/NetworkSetupData.cpp
	shared/NetworkSetupData.h
	shared/Service.cpp
	shared/Service.h
	shared/SetupSharedNetwork.cpp
	shared/SetupSharedNetwork.h
	shared/UdpSock.h

	shared/UdpLibraryMT/Events.cpp
	shared/UdpLibraryMT/Events.h
	shared/UdpLibraryMT/UdpConnectionHandlerMT.cpp
	shared/UdpLibraryMT/UdpConnectionHandlerMT.h
	shared/UdpLibraryMT/UdpConnectionMT.cpp
	shared/UdpLibraryMT/UdpConnectionMT.h
	shared/UdpLibraryMT/UdpHandlerMT.h
	shared/UdpLibraryMT/UdpLibraryMT.cpp
	shared/UdpLibraryMT/UdpLibraryMT.h
	shared/UdpLibraryMT/UdpManagerHandlerMT.cpp
	shared/UdpLibraryMT/UdpManagerHandlerMT.h
	shared/UdpLibraryMT/UdpManagerMT.cpp
	shared/UdpLibraryMT/UdpManagerMT.h
	shared/UdpLibraryMT/UdpMiscMT.cpp
	shared/UdpLibraryMT/UdpMiscMT.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/Address.cpp
		win32/FirstSharedNetwork.cpp
		win32/NetworkGetHostName.cpp
		win32/OverlappedTcp.cpp
		win32/OverlappedTcp.h
		win32/Sock.cpp
		win32/Sock.h
		win32/TcpClient.cpp
		win32/TcpClient.h
		win32/TcpServer.cpp
		win32/TcpServer.h
		win32/UdpSock.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES
		linux/Address.cpp
		linux/NetworkGetHostName.cpp
		linux/Sock.cpp
		linux/Sock.h
		linux/TcpClient.cpp
		linux/TcpClient.h
		linux/TcpServer.cpp
		linux/TcpServer.h
		linux/UdpSock.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedCompression/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedLog/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/udplibrary
)

add_library(sharedNetwork STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(sharedNetwork
	sharedFoundation
	sharedLog
	sharedMessageDispatch
	udplibrary
)

if(WIN32)
    target_link_libraries(sharedNetwork mswsock ws2_32)
endif()
