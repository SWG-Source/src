// ======================================================================
//
// AiShipTurretTargetingSystem.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipTurretTargetingSystem.h"

#include "serverGame/ShipObject.h"

// ======================================================================

AiShipTurretTargetingSystem::AiShipTurretTargetingSystem(ShipController & shipController) :
		ShipTurretTargetingSystem(shipController)
{
}

// ----------------------------------------------------------------------

AiShipTurretTargetingSystem::~AiShipTurretTargetingSystem()
{
}

// ----------------------------------------------------------------------

void AiShipTurretTargetingSystem::doShot(int const weaponIndex, NetworkId const & targetId, ShipChassisSlotType::Type targetedComponent, bool const goodShot)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiShipTurretTargetingSystem::doShot");
	ShipObject & shipObject = getShipOwner();
	shipObject.fireShotTurretServer(weaponIndex, targetId, targetedComponent, goodShot, false);
}

// ----------------------------------------------------------------------

bool AiShipTurretTargetingSystem::isTurretControlled(int const /*weaponIndex*/)
{
	return true;
}

// ======================================================================
