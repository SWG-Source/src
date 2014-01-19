

set(SHARED_SOURCES
	Archive.cpp
	Archive.h
	AutoLog.cpp
	AutoLog.h
	Base.cpp
	Base.h
	BlockAllocator.h
	Config.cpp
	Config.h
	Event.h
	Logger.h
	MD5.cpp
	MD5.h
	Mutex.h
	Platform.h
	ScopeLock.cpp
	ScopeLock.h
	Statistics.cpp
	Statistics.h
	TemplateBlockAllocator.h
	TemplateObjectAllocator.h
	Thread.h
	Types.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/Archive.h
		win32/BlockAllocator.cpp
		win32/Event.cpp
		win32/Event.h
		win32/File.cpp
		win32/File.h
		win32/Logger.cpp
		win32/Mutex.cpp
		win32/Mutex.h
		win32/Platform.cpp
		win32/Platform.h
		win32/Thread.cpp
		win32/Thread.h
		win32/Types.h
	)

	#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES
		linux/Archive.h
		linux/BlockAllocator.cpp
		linux/Event.cpp
		linux/Event.h
		linux/Logger.cpp
		linux/Mutex.cpp
		linux/Mutex.h
		linux/Platform.cpp
		linux/Platform.h
		linux/Thread.cpp
		linux/Thread.h
		linux/Types.h
	)

	#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
endif()

add_definitions(-D_REENTRANT)

add_library(Base STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
