// ======================================================================
//
// AiShipTurretTargetingSystem.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AiShipTurretTargetingSystem_H
#define INCLUDED_AiShipTurretTargetingSystem_H

// ======================================================================

#include "serverGame/AiShipAttackTargetList.h"
#include "serverGame/ShipTurretTargetingSystem.h"
#include "sharedFoundation/Timer.h"

class AiShipController;
class CachedNetworkId;
class NetworkId;

// ======================================================================

/**
 * Fire AI turrets automatically at enemies.
 */
class AiShipTurretTargetingSystem : public ShipTurretTargetingSystem
{
  public:
	AiShipTurretTargetingSystem(ShipController & shipController);
	virtual ~AiShipTurretTargetingSystem();

  private:	
	virtual void doShot(int const weaponIndex, NetworkId const & targetId, ShipChassisSlotType::Type targetedComponent, bool const goodShot);
	virtual bool isTurretControlled(int const weaponIndex);
	
  private:
	AiShipTurretTargetingSystem(AiShipTurretTargetingSystem const &); //disable
	AiShipTurretTargetingSystem& operator=(AiShipTurretTargetingSystem const &); //disable
};

// ======================================================================

#endif
