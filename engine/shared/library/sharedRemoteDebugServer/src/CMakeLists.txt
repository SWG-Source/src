
set(SHARED_SOURCES
	shared/FirstSharedRemoteDebugServer.cpp
	shared/FirstSharedRemoteDebugServer.h
	shared/SharedRemoteDebugServerConnection.cpp
	shared/SharedRemoteDebugServerConnection.h
	shared/SharedRemoteDebugServer.cpp
	shared/SharedRemoteDebugServer.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public	
	#${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetwork/include/public
	#${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	#${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
)

add_library(sharedRemoteDebugServer STATIC
	${SHARED_SOURCES}
)
