// ======================================================================
//
// SetupServerPathfinding.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverPathfinding/FirstServerPathfinding.h"
#include "serverPathfinding/SetupServerPathfinding.h"

#include "serverPathfinding/CityPathGraphManager.h"
#include "serverPathfinding/ServerPathfindingMessaging.h"

#include "sharedFoundation/ExitChain.h"

#include "sharedPathfinding/SetupSharedPathfinding.h"

void SetupServerPathfinding::install ( void )
{
	SetupSharedPathfinding::install();

	ServerPathfindingMessaging::install();

	CityPathGraphManager::install();

	ExitChain::add( SetupServerPathfinding::remove, "SetupServerPathfinding" );
}

void SetupServerPathfinding::remove ( void )
{
	CityPathGraphManager::remove();
	
	ServerPathfindingMessaging::remove();

	SetupSharedPathfinding::remove();
}

