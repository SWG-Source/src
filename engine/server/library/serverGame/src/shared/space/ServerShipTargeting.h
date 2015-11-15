//======================================================================
//
// ServerShipTargeting.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ServerShipTargeting_H
#define INCLUDED_ServerShipTargeting_H

#include "sharedGame/ShipTargeting.h"

//======================================================================

class Object;

//----------------------------------------------------------------------

class ServerShipTargeting : public ShipTargeting
{
public:

	static Object * closestShip(Object const * targetingObject);
	static Object * closestCapShip(Object const * targetingObject);
	static Object * closestUnderReticle(Object const * targetingObject);
	static Object * closestPlayer(Object const * targetingObject);
	static Object * closestNPC(Object const * targetingObject);
	static Object * closestEnemy(Object const * targetingObject);
	static Object * closestEnemyPlayer(Object const * targetingObject);
	static Object * closestFriend(Object const * targetingObject);
	static Object * closestFriendPlayer(Object const * targetingObject);

	static Object * getNextFriendlyTarget(Object const * targetingObject, Object const * currentTarget);
	static Object * getPreviousFriendlyTarget(Object const * targetingObject, Object const * currentTarget);

	static Object * getNextEnemyTarget(Object const * targetingObject, Object const * currentTarget);
	static Object * getPreviousEnemyTarget(Object const * targetingObject, Object const * currentTarget);

private:

	ServerShipTargeting();
	ServerShipTargeting(ServerShipTargeting const & copy);
	ServerShipTargeting & operator=(ServerShipTargeting const & copy);
};

//======================================================================

#endif
