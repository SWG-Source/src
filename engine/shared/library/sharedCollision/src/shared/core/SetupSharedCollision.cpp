// ======================================================================
//
// SetupSharedCollision.cpp
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/SetupSharedCollision.h"

#include "sharedCollision/BoxTree.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/SimpleExtent.h"
#include "sharedCollision/SpatialDatabase.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"

// ----------------------------------------------------------------------

namespace SetupSharedCollisionNamespace
{
	bool ms_installed = false;

	void remove();
}

using namespace SetupSharedCollisionNamespace;

// ----------------------------------------------------------------------

void SetupSharedCollision::install ( SetupSharedCollision::Data const & data )
{
	InstallTimer const installTimer("SetupSharedCollision::install");

	DEBUG_FATAL(ms_installed, ("SetupSharedCollision::install - Collision is already installed"));
	ms_installed = true;

	SimpleExtent::install();

	// ----------

	//@todo - FIXME - ExtentList::install is currently called by SetupSharedObject::install
//	if(data.installExtents)	ExtentList::install();

	if(data.installCollisionWorld)
	{
		CollisionWorld::install(data.serverSide);
		BoxTree::install();
	}

	if (!data.serverSide)
	{
		ConfigSharedCollision::setSpatialSweepAndResolveDefaultMask(SpatialDatabase::Q_Physicals);
	}

	ConfigSharedCollision::setPlayEffectHook(data.playEffect);
	ConfigSharedCollision::setIsPlayerHouseHook(data.isPlayerHouse);

	// ----------

	ExitChain::add (remove, "SetupSharedCollision");
}

// ----------------------------------------------------------------------

void SetupSharedCollisionNamespace::remove()
{
	ms_installed = false;

	ConfigSharedCollision::setPlayEffectHook(0);
	ConfigSharedCollision::setIsPlayerHouseHook(0);
}

// ======================================================================

