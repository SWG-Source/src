// ======================================================================
//
// PlayerShipTurretTargetingSystem.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PlayerShipTurretTargetingSystem_H
#define INCLUDED_PlayerShipTurretTargetingSystem_H

// ======================================================================

#include "serverGame/ShipTurretTargetingSystem.h"

class AiShipController;
class BitArray;
class CachedNetworkId;
class NetworkId;

// ======================================================================

/**
 * Automatically fire some or all of the player's ship's turrets
 */
class PlayerShipTurretTargetingSystem : public ShipTurretTargetingSystem
{
  public:
	PlayerShipTurretTargetingSystem(ShipController & shipController);
	virtual ~PlayerShipTurretTargetingSystem();

	void setWeaponIndexPlayerControlled(int const weaponIndex, bool const playerControlled);

  private:	
	virtual void doShot(int const weaponIndex, NetworkId const & targetId, ShipChassisSlotType::Type targetedComponent, bool const goodShot);
	virtual bool isTurretControlled(int const weaponIndex);
	
  private:
	BitArray * const m_playerControlledWeaponIndices;

  private:
	PlayerShipTurretTargetingSystem(PlayerShipTurretTargetingSystem const &); //disable
	PlayerShipTurretTargetingSystem& operator=(PlayerShipTurretTargetingSystem const &); //disable
};

// ======================================================================

#endif
