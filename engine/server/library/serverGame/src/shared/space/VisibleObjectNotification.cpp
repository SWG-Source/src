// ======================================================================
//
// VisibleObjectNotification.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/VisibleObjectNotification.h"

#include "serverGame/ServerObject.h"
#include "serverGame/SpaceVisibilityManager.h"

// ======================================================================

VisibleObjectNotification VisibleObjectNotification::ms_instance;

// ======================================================================

VisibleObjectNotification::VisibleObjectNotification() :
		ObjectNotification()
{
}

// ----------------------------------------------------------------------

VisibleObjectNotification::~VisibleObjectNotification ()
{
}

// ----------------------------------------------------------------------

bool VisibleObjectNotification::positionAndRotationChanged(Object &object, bool /* dueToParentChange */, const Vector & /* oldPosition */) const
{
	ServerObject *asServerObject = object.asServerObject();
	NOT_NULL(asServerObject);
	SpaceVisibilityManager::moveObject(*asServerObject);
	return true;
}

// ----------------------------------------------------------------------

void VisibleObjectNotification::removeFromWorld(Object &object) const
{
	ServerObject *asServerObject = object.asServerObject();
	NOT_NULL(asServerObject);
	SpaceVisibilityManager::removeObject(*asServerObject);
}

// ----------------------------------------------------------------------

void VisibleObjectNotification::cellChanged(Object &object, bool /* dueToParentChange */) const
{
	if (!object.isInWorldCell())
	{
		ServerObject *asServerObject = object.asServerObject();
		NOT_NULL(asServerObject);
		SpaceVisibilityManager::removeObject(*asServerObject);
	}
}

// ======================================================================
