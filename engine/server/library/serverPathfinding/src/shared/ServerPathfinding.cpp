// ======================================================================
//
// ServerPathfinding.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverPathfinding/FirstServerPathfinding.h"
#include "serverPathfinding/ServerPathfinding.h"

#include "serverPathfinding/CityPathGraphManager.h"
#include "serverPathfinding/ServerPathBuildManager.h"

#ifdef _DEBUG

#include "sharedDebug/PerformanceTimer.h"

#endif

// ======================================================================

void ServerPathfinding::update ( float timeBudget )
{
	CityPathGraphManager::update(0.0f);

#ifdef _DEBUG
	extern float pathfindingTime;

	PerformanceTimer timer;

	timer.start();

#endif

	ServerPathBuildManager::update(timeBudget);

#ifdef _DEBUG

	timer.stop();

	pathfindingTime = timer.getElapsedTime();

#endif
}

// ======================================================================
