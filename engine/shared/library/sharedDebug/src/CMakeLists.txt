
set(SHARED_SOURCES
	shared/CallStack.cpp
	shared/CallStack.h
	shared/CallStackCollector.cpp
	shared/CallStackCollector.h
	shared/DataLint.cpp
	shared/DataLint.h
	shared/DebugFlags.cpp
	shared/DebugFlags.h
	shared/DebugKey.cpp
	shared/DebugKey.h
	shared/FirstSharedDebug.h
	shared/InstallTimer.cpp
	shared/InstallTimer.h
	shared/PixCounter.cpp
	shared/PixCounter.h
	shared/Profiler.cpp
	shared/Profiler.h
	shared/RemoteDebug.cpp
	shared/RemoteDebug.h
	shared/RemoteDebug_inner.cpp
	shared/RemoteDebug_inner.h
	shared/Report.cpp
	shared/Report.h
	shared/SetupSharedDebug.cpp
	shared/SetupSharedDebug.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/DebugHelp.cpp
		win32/DebugHelp.h
		win32/DebugMonitor.cpp
		win32/DebugMonitor.h
		win32/FirstSharedDebug.cpp
		win32/PerformanceTimer.cpp
		win32/PerformanceTimer.h
		win32/ProfilerTimer.cpp
		win32/ProfilerTimer.h
		win32/VTune.cpp
		win32/VTune.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES 
		linux/ConfigSharedDebugLinux.cpp
		linux/ConfigSharedDebugLinux.h
		linux/DebugHelp.cpp
		linux/DebugHelp.h
		linux/DebugMonitor.cpp
		linux/DebugMonitor.h
		linux/PerformanceTimer.cpp
		linux/PerformanceTimer.h
		linux/ProfilerTimer.cpp
		linux/ProfilerTimer.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/vtune
)

add_library(sharedDebug STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(sharedDebug
    ${CURSES_LIBRARY}
)
