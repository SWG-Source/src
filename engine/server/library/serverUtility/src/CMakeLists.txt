
set(SHARED_SOURCES
	shared/AdminAccountManager.cpp
	shared/AdminAccountManager.h
	shared/ChatLogManager.cpp
	shared/ConfigServerUtility.cpp
	shared/ConfigServerUtility.h
	shared/ClusterWideDataManager.cpp
	shared/ClusterWideDataManager.h
	shared/ClusterWideDataManagerList.cpp
	shared/ClusterWideDataManagerList.h
	shared/FirstServerUtility.h
	shared/FreeCtsDataTable.cpp
	shared/FreeCtsDataTable.h
	shared/LocationData.cpp
	shared/LocationData.h
	shared/MissionLocation.cpp
	shared/MissionLocation.h
	shared/PopulationList.cpp
	shared/PopulationList.h
	shared/PvpEnemy.cpp
	shared/PvpEnemy.h
	shared/ResourceFractalData.cpp
	shared/ResourceFractalData.h
	shared/RecentMaxSyncedValue.cpp
	shared/RecentMaxSyncedValue.h
	shared/ServerClock.cpp
	shared/ServerClock.h
	shared/ServerConnection.cpp
	shared/ServerConnection.h
	shared/ServerServiceHandler.h
	shared/SetupServerUtility.cpp
	shared/SetupServerUtility.h
	shared/SystemAssignedProcessId.cpp
	shared/SystemAssignedProcessId.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstServerUtility.cpp
	)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedLog/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMath/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMathArchive/include/public

	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetwork/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
	${SWG_ENGINE_SOURCE_DIR}/server/library/serverNetworkMessages/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localizationArchive/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicodeArchive/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/webAPI
)

add_library(serverUtility STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(serverUtility
	sharedMessageDispatch
	sharedNetworkMessages
	serverNetworkMessages
	sharedUtility
)
