
set(SHARED_SOURCES
	shared/FirstSharedThread.h
	shared/RunThread.cpp
	shared/RunThread.h
	shared/SetupSharedThread.cpp
	shared/SetupSharedThread.h
	shared/ThreadHandle.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedThread.cpp
		win32/Thread.cpp
		win32/Thread.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES
		linux/Thread.cpp
		linux/Thread.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
)

add_library(sharedThread STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(sharedThread
	sharedSynchronization
    ${CMAKE_THREAD_LIBS_INIT}
)
