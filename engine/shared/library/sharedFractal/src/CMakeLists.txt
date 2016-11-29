
set(SHARED_SOURCES
	shared/FirstSharedFractal.h
	shared/MultiFractal.cpp
	shared/MultiFractal.h
	shared/MultiFractalReaderWriter.cpp
	shared/MultiFractalReaderWriter.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedFractal.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
)

add_library(sharedFractal STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
