
set(SHARED_SOURCES
	CTServiceAPI.cpp
	CTServiceAPI.h
	CTServiceAPICore.cpp
	CTServiceAPICore.h
	Request.cpp
	Request.h
	Response.cpp
	Response.h

	Base/Archive.cpp

	CTCommon/CTEnum.h
	CTCommon/CTServiceCharacter.cpp
	CTCommon/CTServiceCharacter.h
	CTCommon/CTServiceCustomer.h
	CTCommon/CTServiceDBOrder.h
	CTCommon/CTServiceDBTransaction.h
	CTCommon/CTServiceObjects.h
	CTCommon/CTServiceServer.cpp
	CTCommon/CTServiceServer.h
	CTCommon/CTServiceWebAPITransaction.h
	CTCommon/RequestStrings.h

	CTGenericAPI/GenericApiCore.cpp
	CTGenericAPI/GenericApiCore.h
	CTGenericAPI/GenericConnection.cpp
	CTGenericAPI/GenericConnection.h
	CTGenericAPI/GenericMessage.cpp
	CTGenericAPI/GenericMessage.h

	TcpLibrary/Clock.cpp
	TcpLibrary/Clock.h
	TcpLibrary/IPAddress.cpp
	TcpLibrary/IPAddress.h
	TcpLibrary/TcpBlockAllocator.cpp
	TcpLibrary/TcpBlockAllocator.h
	TcpLibrary/TcpConnection.cpp
	TcpLibrary/TcpConnection.h
	TcpLibrary/TcpHandlers.h
	TcpLibrary/TcpManager.cpp
	TcpLibrary/TcpManager.h

	Unicode/FirstUnicode.cpp
	Unicode/FirstUnicode.h
	Unicode/Unicode.cpp
	Unicode/Unicode.h
	Unicode/UnicodeBlocks.cpp
	Unicode/UnicodeBlocks.h
	Unicode/UnicodeCharacterData.cpp
	Unicode/UnicodeCharacterData.h
	Unicode/UnicodeCharacterDataMap.cpp
	Unicode/UnicodeCharacterDataMap.h
	Unicode/UnicodeUtils.cpp
	Unicode/UnicodeUtils.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		Base/win32/Platform.cpp
	)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Base/win32)
else()
	set(PLATFORM_SOURCES 
		Base/linux/Platform.cpp
	)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Base/linux)
endif()

add_definitions(-DEXTERNAL_DISTRO -DNAMESPACE=CTService -DUSE_TCP_LIBRARY)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(CTServiceGameAPI
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)