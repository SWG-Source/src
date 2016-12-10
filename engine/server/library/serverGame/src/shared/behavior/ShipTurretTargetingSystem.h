// ======================================================================
//
// ShipTurretTargetingSystem.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipTurretTargetingSystem_H
#define INCLUDED_ShipTurretTargetingSystem_H

// ======================================================================

#include "serverGame/AiShipAttackTargetList.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/ShipChassisSlotType.h"

class CachedNetworkId;
class NetworkId;
class ShipController;
class ShipObject;

// ======================================================================

/**
 * A system that automatically fires ship turrets at enemies.
 * This can be attached to a ship controller, which will cause
 * the ship to automatically fire its turrets.
 *
 * Derive from this class to make targeting systems for AI or for player
 * ships with autopilot.
 */
class ShipTurretTargetingSystem
{
  public:
	ShipTurretTargetingSystem(ShipController & shipController);
	virtual ~ShipTurretTargetingSystem() = 0;

	virtual void alter(float const deltaTime);
	ShipController const & getShipController() const;
	ShipController & getShipController();
	ShipObject & getShipOwner();

	void clearTargets();
	void onTargetChanged(NetworkId const & target);
	void onTargetLost(NetworkId const & target);

  protected:
	bool m_targetsDirty;

  private:	
	CachedNetworkId onTurretLostTarget(int const weaponIndex, CachedNetworkId const & oldTarget);
	virtual void doShot(int const weaponIndex, NetworkId const & targetId, ShipChassisSlotType::Type targetedComponent, bool const goodShot) = 0;
	virtual bool isTurretControlled(int const weaponIndex) = 0;
	bool buildTargetList();

  protected:
	typedef std::map<NetworkId, int> TurretsPerTargetType;

  private:
	ShipController & m_shipController;

  protected:
	TurretsPerTargetType * const m_turretsPerTarget;
	AiShipAttackTargetList::SortedTargetList * const m_targetList;
	int m_idealTurretsPerTarget;
	Timer m_retargetTimer;

  private:
	ShipTurretTargetingSystem(ShipTurretTargetingSystem const &); //disable
	ShipTurretTargetingSystem & operator=(ShipTurretTargetingSystem const &); //disable
};

// ======================================================================

#endif
