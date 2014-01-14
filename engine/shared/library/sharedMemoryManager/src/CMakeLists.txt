
set(SHARED_SOURCES
	shared/FirstSharedMemoryManager.cpp
	shared/FirstSharedMemoryManager.h
	shared/MemoryManager.cpp
	shared/MemoryManager.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/OsMemory.cpp
		win32/OsMemory.h
		win32/OsNewDel.cpp
		win32/OsNewDel.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES 
		linux/OsMemory.cpp
		linux/OsMemory.h
		linux/OsNewDel.cpp
		linux/OsNewDel.h
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

add_library(sharedMemoryManager STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
