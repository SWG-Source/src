
set(SHARED_SOURCES
	shared/AsynchronousLoader.cpp
	shared/AsynchronousLoader.h
	shared/ConfigSharedFile.cpp
	shared/ConfigSharedFile.h
	shared/FileManifest.cpp
	shared/FileManifest.h
	shared/FileNameUtils.cpp
	shared/FileNameUtils.h
	shared/FileStreamer.cpp
	shared/FileStreamerFile.cpp
	shared/FileStreamerFile.h
	shared/FileStreamer.h
	shared/FileStreamerThread.cpp
	shared/FileStreamerThread.h
	shared/FirstSharedFile.h
	shared/Iff.cpp
	shared/Iff.h
	shared/MemoryFile.cpp
	shared/MemoryFile.h
	shared/SetupSharedFile.cpp
	shared/SetupSharedFile.h
	shared/TreeFile.cpp
	shared/TreeFile.h
	shared/TreeFile_SearchNode.cpp
	shared/TreeFile_SearchNode.h
	shared/ZlibFile.cpp
	shared/ZlibFile.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedFile.cpp
		win32/OsFile.cpp
		win32/OsFile.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES
		linux/OsFile.cpp
		linux/OsFile.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedCompression/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMath/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/fileInterface/include/public
)

add_library(sharedFile STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(sharedFile
	sharedCompression
	sharedDebug
	sharedFoundation
	sharedMath
	fileInterface
)
