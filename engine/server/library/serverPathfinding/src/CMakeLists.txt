
set(SHARED_SOURCES
	shared/CityPathGraph.cpp
	shared/CityPathGraph.h
	shared/CityPathGraphManager.cpp
	shared/CityPathGraphManager.h
	shared/CityPathNode.cpp
	shared/CityPathNode.h
	shared/FirstServerPathfinding.h
	shared/PathAutoGenerator.cpp
	shared/PathAutoGenerator.h
	shared/ServerPathBuilder.cpp
	shared/ServerPathBuilder.h
	shared/ServerPathBuildManager.cpp
	shared/ServerPathBuildManager.h
	shared/ServerPathfindingConstants.cpp
	shared/ServerPathfindingConstants.h
	shared/ServerPathfinding.cpp
	shared/ServerPathfinding.h
	shared/ServerPathfindingMessaging.cpp
	shared/ServerPathfindingMessaging.h
	shared/ServerPathfindingNotification.cpp
	shared/ServerPathfindingNotification.h
	shared/SetupServerPathfinding.cpp
	shared/SetupServerPathfinding.h 
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstServerPathfinding.cpp
	)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedCollision/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedGame/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMath/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMathArchive/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedObject/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedPathfinding/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSkillSystem/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedTerrain/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
	${SWG_ENGINE_SOURCE_DIR}/server/library/serverGame/include/public
	${SWG_ENGINE_SOURCE_DIR}/server/library/serverNetworkMessages/include/public
	${SWG_ENGINE_SOURCE_DIR}/server/library/serverScript/include/public
	${SWG_ENGINE_SOURCE_DIR}/server/library/serverUtility/include/public
	${SWG_GAME_SOURCE_DIR}/shared/library/swgSharedUtility/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localizationArchive/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicodeArchive/include/public
)

add_library(serverPathfinding STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
