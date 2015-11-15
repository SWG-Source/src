// ======================================================================
//
// ServerCollisionProperty.cpp
//
// Copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerCollisionProperty.h"

#include "serverGame/CreatureObject.h"

// ======================================================================

ServerCollisionProperty::ServerCollisionProperty(ServerObject &owner) :
	CollisionProperty(owner)
{
}

// ----------------------------------------------------------------------

ServerCollisionProperty::ServerCollisionProperty(ServerObject &owner, SharedObjectTemplate const *objTemplate) :
	CollisionProperty(owner, objTemplate)
{
}

// ----------------------------------------------------------------------

bool ServerCollisionProperty::canCollideWith (CollisionProperty const *otherCollision) const
{
	if (!otherCollision)
		return false;

	if (isMobile() && otherCollision->isMobile())
	{
		ServerObject const * const ownerServerObject = getOwner().asServerObject();
		ServerObject const * const otherOwnerServerObject = otherCollision->getOwner().asServerObject();

		CreatureObject const * const ownerA = (ownerServerObject != 0) ? ownerServerObject->asCreatureObject() : 0;
		CreatureObject const * const ownerB = (otherOwnerServerObject != 0) ? otherOwnerServerObject->asCreatureObject() : 0;

		if (ownerA && (ownerA->isDead() || ownerA->isIncapacitated()))
			return false;
		if (ownerB && (ownerB->isDead() || ownerB->isIncapacitated()))
			return false;
	}

	return CollisionProperty::canCollideWith(otherCollision);
}

// ======================================================================

