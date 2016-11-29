
set(SHARED_SOURCES
	shared/ConfigSharedRandom.cpp
	shared/ConfigSharedRandom.h
	shared/FastRandomGenerator.h
	shared/FirstSharedRandom.h
	shared/Random.cpp
	shared/Random.h
	shared/RandomGenerator.cpp
	shared/RandomGenerator.h
	shared/SetupSharedRandom.cpp
	shared/SetupSharedRandom.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedRandom.cpp
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

add_library(sharedRandom STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
