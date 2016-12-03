
set(SHARED_SOURCES
	shared/BlockingPointer.h
	shared/BlockingQueue.h
	shared/CountingSemaphore.h
	shared/Guard.h
	shared/WriteOnce.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/ConditionVariable.cpp
		win32/ConditionVariable.h
		win32/FirstSharedSynchronization.cpp
		win32/Gate.cpp
		win32/Gate.h
		win32/InterlockedInteger.h
		win32/InterlockedVoidPointer.h
		win32/Mutex.cpp
		win32/Mutex.h
		win32/RecursiveMutex.cpp
		win32/RecursiveMutex.h
		win32/Semaphore.cpp
		win32/Semaphore.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES
		linux/ConditionVariable.cpp
		linux/ConditionVariable.h
		linux/Gate.cpp
		linux/Gate.h
		linux/InterlockedInteger.h
		linux/InterlockedVoidPointer.h
		linux/Mutex.cpp
		linux/Mutex.h
		linux/RecursiveMutex.cpp
		linux/RecursiveMutex.h
		linux/Semaphore.cpp
		linux/Semaphore.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
)

add_library(sharedSynchronization STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
