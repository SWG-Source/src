
set(SHARED_SOURCES
	shared/ConfigServerMetrics.cpp
	shared/ConfigServerMetrics.h
	shared/FirstServerMetrics.h
	shared/MetricsConnection.cpp
	shared/MetricsConnection.h
	shared/MetricsData.cpp
	shared/MetricsData.h
	shared/MetricsManager.cpp
	shared/MetricsManager.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstServerMetrics.cpp
	)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetwork/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
	${SWG_ENGINE_SOURCE_DIR}/server/library/serverNetworkMessages/include/public
	${SWG_ENGINE_SOURCE_DIR}/server/library/serverUtility/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/singleton/include
)

add_library(serverMetrics STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
