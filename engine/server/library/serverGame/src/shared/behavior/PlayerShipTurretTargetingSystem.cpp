// ======================================================================
//
// PlayerShipTurretTargetingSystem.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PlayerShipTurretTargetingSystem.h"

#include "serverGame/ShipObject.h"

// ======================================================================

PlayerShipTurretTargetingSystem::PlayerShipTurretTargetingSystem(ShipController & shipController) :
		ShipTurretTargetingSystem(shipController),
		m_playerControlledWeaponIndices(new BitArray)
{
}

// ----------------------------------------------------------------------

PlayerShipTurretTargetingSystem::~PlayerShipTurretTargetingSystem()
{
	delete m_playerControlledWeaponIndices;
}

// ----------------------------------------------------------------------

void PlayerShipTurretTargetingSystem::doShot(int const weaponIndex, NetworkId const & targetId, ShipChassisSlotType::Type targetedComponent, bool const goodShot)
{
	ShipObject & shipObject = getShipOwner();
	shipObject.fireShotTurretServer(weaponIndex, targetId, targetedComponent, goodShot, true);
}

// ----------------------------------------------------------------------

bool PlayerShipTurretTargetingSystem::isTurretControlled(int const weaponIndex)
{
	return !m_playerControlledWeaponIndices->testBit(weaponIndex);
}

// ----------------------------------------------------------------------

void PlayerShipTurretTargetingSystem::setWeaponIndexPlayerControlled(int const weaponIndex, bool const playerControlled)
{
	DEBUG_FATAL((weaponIndex < 0), ("Invalid weaponIndex(%d)", weaponIndex));

	if (playerControlled)
	{
		m_playerControlledWeaponIndices->setBit(weaponIndex);
	}
	else
	{
		m_playerControlledWeaponIndices->clearBit(weaponIndex);
	}
}

// ======================================================================
