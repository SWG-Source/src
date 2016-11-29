
set(SHARED_SOURCES
	shared/Emitter.cpp
	shared/Emitter.h
	shared/FirstSharedMessageDispatch.h
	shared/Message.cpp
	shared/Message.h
	shared/MessageManager.cpp
	shared/MessageManager.h
	shared/Receiver.cpp
	shared/Receiver.h
	shared/Transceiver.cpp
	shared/Transceiver.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedMessageDispatch.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
)

add_library(sharedMessageDispatch STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
