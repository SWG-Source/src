

set(SHARED_SOURCES
	shared/Archive.h
	shared/AutoByteStream.cpp
	shared/AutoByteStream.h
	shared/AutoDeltaByteStream.cpp
	shared/AutoDeltaByteStream.h
	shared/AutoDeltaMap.h
	shared/AutoDeltaObserverOps.h
	shared/AutoDeltaPackedMap.cpp
	shared/AutoDeltaPackedMap.h
	shared/AutoDeltaQueue.h
	shared/AutoDeltaSet.h
	shared/AutoDeltaSetObserver.h
	shared/AutoDeltaVariableCallback.h
	shared/AutoDeltaVariableObserver.h
	shared/AutoDeltaVector.h
	shared/AutoDeltaVectorObserver.h
	shared/ByteStream.cpp
	shared/ByteStream.h
	shared/FirstArchive.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/ArchiveMutex.cpp
		win32/ArchiveMutex.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES
		linux/ArchiveMutex.cpp
		linux/ArchiveMutex.h
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared)

add_library(archive
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
