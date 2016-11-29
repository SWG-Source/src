
set(SHARED_SOURCES
	shared/ConfigSharedRegex.cpp
	shared/ConfigSharedRegex.h
	shared/FirstSharedRegex.h
	shared/SetupSharedRegex.cpp
	shared/SetupSharedRegex.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedRegex.cpp
		win32/RegexServices.cpp
		win32/RegexServices.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES
		linux/RegexServices.cpp
		linux/RegexServices.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
	${PCRE_INCLUDE_DIR}
)

add_library(sharedRegex STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(sharedRegex
	${PCRE_LIBRARY}
)
