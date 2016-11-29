
set(SHARED_SOURCES
	shared/ConfigSharedPathfinding.cpp
	shared/ConfigSharedPathfinding.h
	shared/DynamicPathGraph.cpp
	shared/DynamicPathGraph.h
	shared/DynamicPathNode.cpp
	shared/DynamicPathNode.h
	shared/FirstSharedPathfinding.h
	shared/PathEdge.cpp
	shared/PathEdge.h
	shared/Pathfinding.cpp
	shared/PathfindingEnums.h
	shared/Pathfinding.h
	shared/PathGraph.cpp
	shared/PathGraph.h
	shared/PathGraphIterator.cpp
	shared/PathGraphIterator.h
	shared/PathNode.cpp
	shared/PathNode.h
	shared/PathSearch.cpp
	shared/PathSearch.h
	shared/SetupSharedPathfinding.cpp
	shared/SetupSharedPathfinding.h
	shared/SimplePathGraph.cpp
	shared/SimplePathGraph.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedPathfinding.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
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
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMath/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedObject/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
)

add_library(sharedPathfinding STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
