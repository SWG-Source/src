
set(SHARED_SOURCES
	shared/FoundationTypes.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FoundationTypesWin32.h
	)

	set(EXCLUDE_PROJECT "")
else()
	set(PLATFORM_SOURCES 
		linux/FoundationTypesLinux.h
	)

	set(EXCLUDE_PROJECT "EXCLUDE_FROM_ALL")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared)

add_library(sharedFoundationTypes STATIC ${EXCLUDE_PROJECT} 
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

set_target_properties(sharedFoundationTypes PROPERTIES LINKER_LANGUAGE CXX)
