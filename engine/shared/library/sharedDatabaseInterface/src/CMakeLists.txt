
set(SHARED_SOURCES
	shared/core/Bindable.cpp
	shared/core/Bindable.h
	shared/core/BindableImplementation.h
	shared/core/BindableNetworkId.cpp
	shared/core/BindableNetworkId.h
	shared/core/BufferString.cpp
	shared/core/BufferString.h
	shared/core/DbBatchQuery.h
	shared/core/DbBindableBase.cpp
	shared/core/DbBindableBase.h
	shared/core/DbBindableBitArray.cpp
	shared/core/DbBindableBitArray.h
	shared/core/DbBindableBool.cpp
	shared/core/DbBindableBool.h
	shared/core/DbBindableDouble.cpp
	shared/core/DbBindableDouble.h
	shared/core/DbBindableInt64.cpp
	shared/core/DbBindableInt64.h
	shared/core/DbBindableLong.cpp
	shared/core/DbBindableLong.h
	shared/core/DbBindableString.cpp
	shared/core/DbBindableString.h
	shared/core/DbBindableUnicode.cpp
	shared/core/DbBindableUnicode.h
	shared/core/DbBufferRow.cpp
	shared/core/DbBufferRow.h
	shared/core/DbException.cpp
	shared/core/DbException.h
	shared/core/DbModeQuery.cpp
	shared/core/DbModeQuery.h
	shared/core/DbProfiler.cpp
	shared/core/DbProfiler.h
	shared/core/DbProtocol.def
	shared/core/DbQuery.cpp
	shared/core/DbQuery.h
	shared/core/DbQueryImplementation.h
	shared/core/DbRow.cpp
	shared/core/DbRow.h
	shared/core/DbServer.cpp
	shared/core/DbServer.h
	shared/core/DbSession.cpp
	shared/core/DbSession.h
	shared/core/FirstSharedDatabaseInterface.h
	shared/core/NullEncodedUnicodeString.cpp
	shared/core/NullEncodedUnicodeString.h
	shared/core/NullEncodedStandardString.cpp
	shared/core/NullEncodedStandardString.h

	shared/logger/DbLogEvent.cpp
	shared/logger/DbLogEvent.h
	shared/logger/DbLogger.cpp
	shared/logger/DbLogger.h
	shared/logger/DbTaskSaveLog.cpp
	shared/logger/DbTaskSaveLog.h

	shared/queue/DbTaskQueue.cpp
	shared/queue/DbTaskQueue.h
	shared/queue/DbTestQuery.cpp
	shared/queue/DbTestQuery.h
	shared/queue/DbTaskRequest.cpp
	shared/queue/DbTaskRequest.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedDatabaseInterface.cpp
		win32/SQLC_Defs.h
	)
else()
	set(PLATFORM_SOURCES
		linux/SQLC_Defs.h
	)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedLog/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicodeArchive/include/public
)

add_definitions(-DDBLIBRARY_OCI)

add_library(sharedDatabaseInterface STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
