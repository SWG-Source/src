// ======================================================================
//
// AiShipControllerInterface.h
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_AiShipControllerInterface_H
#define INCLUDED_AiShipControllerInterface_H

#include "serverGame/AiShipController.h"

class NetworkId;
class Object;
class SpacePath;
class Vector;

// ----------------------------------------------------------------------
class AiShipControllerInterface
{
public:

	static bool addDamageTaken(NetworkId const & unit, NetworkId const & attackingUnit, float const damage, bool const notifySquad, bool const checkPlayerAttacker);
	static bool setAttackOrders(NetworkId const & unit, AiShipController::AttackOrders attackOrders);
	static bool setLeashRadius(NetworkId const & unit, float radius);
	static bool idle(NetworkId const & unit);
	static bool track(NetworkId const & unit, Object const & target);
	static bool follow(NetworkId const & unit, NetworkId const & followedUnit, Vector const & direction, float offset);
	static bool addPatrolPath(NetworkId const & unit, SpacePath * const path);
	static bool clearPatrolPath(NetworkId const & unit);
	static bool moveTo(NetworkId const & unit, SpacePath * const path);

private:

	// Disabled

	AiShipControllerInterface();
	AiShipControllerInterface(AiShipControllerInterface const &);
	AiShipControllerInterface & operator =(AiShipControllerInterface const &);
};

// ======================================================================

#endif // INCLUDED_AiShipControllerInterface_H
