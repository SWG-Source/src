//=======================================================================
//
// ShipTargeting.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//=======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerShipTargeting.h"

#include "serverGame/Pvp.h"
#include "serverGame/ServerSharedObjectTemplateInterface.h"
#include "serverGame/ServerObject.h"
#include "serverGame/TangibleObject.h"

#include <vector>

//=======================================================================

namespace ServerShipTargetingNamespace
{
	static ServerSharedObjectTemplateInterface const cs_sharedObjectTemplateInterface;

	bool actorAndTargetAreFriends(Object const * actor, Object const * target);
	bool actorAndTargetAreEnemies(Object const * actor, Object const * target);
	bool actorAndTargetAreEnemiesAndTargetIsPlayer(Object const * actor, Object const * target);
	bool actorAndTargetAreFriendsAndTargetIsPlayer(Object const * actor, Object const * target);
}

// ----------------------------------------------------------------------

bool ServerShipTargetingNamespace::actorAndTargetAreFriends(Object const * const actor, Object const * const target)
{
	return !actorAndTargetAreEnemies(actor, target);
}

// ----------------------------------------------------------------------

bool ServerShipTargetingNamespace::actorAndTargetAreEnemies(Object const * const actor, Object const * const target)
{
	TangibleObject const * tangibleActor = 0;

	if (actor != 0)
	{
		ServerObject const * const serverObject = actor->asServerObject();
		if (serverObject != 0)
		{
			tangibleActor = serverObject->asTangibleObject();
		}
	}

	TangibleObject const * tangibleTarget = 0;

	if (target != 0)
	{
		ServerObject const * const serverObject = target->asServerObject();
		if (serverObject != 0)
		{
			tangibleTarget = serverObject->asTangibleObject();
		}
	}

	if ((tangibleActor != 0) && (tangibleTarget != 0))
	{
		return Pvp::isEnemy(*tangibleActor, *tangibleTarget);
	}

	return false;
}

// ----------------------------------------------------------------------

bool ServerShipTargetingNamespace::actorAndTargetAreEnemiesAndTargetIsPlayer(Object const * const actor, Object const * const target)
{
	if (actorAndTargetAreEnemies(actor, target))
	{
		return ShipTargeting::isPlayerShip(cs_sharedObjectTemplateInterface, target);
	}

	return false;
}

// ----------------------------------------------------------------------

bool ServerShipTargetingNamespace::actorAndTargetAreFriendsAndTargetIsPlayer(Object const * const actor, Object const * const target)
{
	if (actorAndTargetAreFriends(actor, target))
	{
		return ShipTargeting::isPlayerShip(cs_sharedObjectTemplateInterface, target);
	}

	return false;
}

// ----------------------------------------------------------------------

using namespace ServerShipTargetingNamespace;

//=======================================================================

Object * ServerShipTargeting::closestShip(Object const * const targetingObject)
{
	return ShipTargeting::sweepForClosestShip(cs_sharedObjectTemplateInterface, targetingObject);
}

// ----------------------------------------------------------------------

Object * ServerShipTargeting::closestCapShip(Object const * const targetingObject)
{
	return ShipTargeting::sweepForClosestCapShip(cs_sharedObjectTemplateInterface, targetingObject);
}

// ----------------------------------------------------------------------

Object * ServerShipTargeting::closestUnderReticle(Object const * const targetingObject)
{
	return ShipTargeting::sweepForClosestUnderReticle(cs_sharedObjectTemplateInterface, targetingObject);
}

// ----------------------------------------------------------------------

Object * ServerShipTargeting::closestPlayer(Object const * const targetingObject)
{
	return ShipTargeting::sweepForClosestPlayer(cs_sharedObjectTemplateInterface, targetingObject);
}

// ----------------------------------------------------------------------

Object * ServerShipTargeting::closestNPC(Object const * const targetingObject)
{
	return ShipTargeting::sweepForClosestNPC(cs_sharedObjectTemplateInterface, targetingObject);
}

// ----------------------------------------------------------------------

Object * ServerShipTargeting::closestEnemy(Object const * const targetingObject)
{
	return sweepForClosestShipWithRelationShip(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreEnemies);
}

// ----------------------------------------------------------------------

Object * ServerShipTargeting::closestEnemyPlayer(Object const * const targetingObject)
{
	return sweepForClosestShipWithRelationShip(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreEnemiesAndTargetIsPlayer);
}

// ----------------------------------------------------------------------

Object * ServerShipTargeting::closestFriend(Object const * const targetingObject)
{
	return sweepForClosestShipWithRelationShip(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreFriends);
}

// ----------------------------------------------------------------------

Object * ServerShipTargeting::closestFriendPlayer(Object const * const targetingObject)
{
	return sweepForClosestShipWithRelationShip(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreFriendsAndTargetIsPlayer);
}

// ----------------------------------------------------------------------

Object * ServerShipTargeting::getNextFriendlyTarget(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::sweepForShipsWithRelationship(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreFriends, resultObjects);
	return getNextTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

Object * ServerShipTargeting::getPreviousFriendlyTarget(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::sweepForShipsWithRelationship(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreFriends, resultObjects);
	return getPreviousTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

Object * ServerShipTargeting::getNextEnemyTarget(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::sweepForShipsWithRelationship(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreEnemies, resultObjects);
	return getNextTarget(resultObjects, currentTarget);
}

// ----------------------------------------------------------------------

Object * ServerShipTargeting::getPreviousEnemyTarget(Object const * const targetingObject, Object const * const currentTarget)
{
	ShipTargeting::ObjectVector resultObjects;
	ShipTargeting::sweepForShipsWithRelationship(cs_sharedObjectTemplateInterface, targetingObject, actorAndTargetAreEnemies, resultObjects);
	return getPreviousTarget(resultObjects, currentTarget);
}

//=======================================================================
