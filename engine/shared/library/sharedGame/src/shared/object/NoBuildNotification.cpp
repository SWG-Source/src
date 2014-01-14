//===================================================================
//
// NoBuildNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/NoBuildNotification.h"

#include "sharedObject/LotManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/World.h"

//===================================================================
// PUBLIC NoBuildNotification
//===================================================================

NoBuildNotification::NoBuildNotification () :
	ObjectNotification ()
{
}

//-------------------------------------------------------------------
	
NoBuildNotification::~NoBuildNotification ()
{
}

//-------------------------------------------------------------------

void NoBuildNotification::removeFromWorld (Object& object) const
{
	LotManager* const lotManager = World::getLotManager ();
	NOT_NULL (lotManager);

	lotManager->removeNoBuildEntry (object);
}

//-------------------------------------------------------------------

bool NoBuildNotification::positionChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	rotationChanged (object, false);

	return true;
}

//-------------------------------------------------------------------

void NoBuildNotification::rotationChanged (Object& object, bool /*dueToParentChange*/) const
{
	if (object.isInWorld ())
	{
		removeFromWorld (object);
		addToWorld (object);
	}
}

//-------------------------------------------------------------------

bool NoBuildNotification::positionAndRotationChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	rotationChanged (object, false);

	return true;
}

//===================================================================
// PROTECTED NoBuildNotification
//===================================================================

void NoBuildNotification::addToWorld (Object& object, const float noBuildRadius) const
{
	LotManager* const lotManager = World::getLotManager ();
	NOT_NULL (lotManager);

	lotManager->addNoBuildEntry (object, noBuildRadius);
}

//===================================================================


