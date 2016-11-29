
set(SHARED_SOURCES
	shared/FirstServerBase.h

	shared/core/ServerBase.cpp
	shared/core/ServerBase.h
	shared/core/ConfigServerBase.cpp
	shared/core/ConfigServerBase.h
	shared/core/ServerBaseImpl.cpp
	shared/core/ServerBaseImpl.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedLog/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetwork/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
)

add_library(serverBase STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
