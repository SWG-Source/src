
set(SHARED_SOURCES
	shared/BitStream.cpp
	shared/BitStream.h
	shared/Compressor.cpp
	shared/Compressor.h
	shared/FirstSharedCompression.h
	shared/Lz77.cpp
	shared/Lz77.h
	shared/SetupSharedCompression.cpp
	shared/SetupSharedCompression.h
	shared/ZlibCompressor.cpp
	shared/ZlibCompressor.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedCompression.cpp
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
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
	${ZLIB_INCLUDE_DIR}
)

add_library(sharedCompression STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(sharedCompression
	${ZLIB_LIBRARY}
)
