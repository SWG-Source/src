//===================================================================
//
// ServerClearCollidableFloraNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerClearCollidableFloraNotification.h"

#include "serverGame/ServerObject.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainObject.h"

//===================================================================
// STATIC PUBLIC ServerClearCollidableFloraNotification
//===================================================================

ServerClearCollidableFloraNotification ServerClearCollidableFloraNotification::ms_instance;

//-------------------------------------------------------------------

ServerClearCollidableFloraNotification& ServerClearCollidableFloraNotification::getInstance ()
{
	return ms_instance;
}

//===================================================================
// PUBLIC ServerClearCollidableFloraNotification
//===================================================================

ServerClearCollidableFloraNotification::ServerClearCollidableFloraNotification () :
	ObjectNotification ()
{
}

//-------------------------------------------------------------------
	
ServerClearCollidableFloraNotification::~ServerClearCollidableFloraNotification ()
{
}

//-------------------------------------------------------------------

void ServerClearCollidableFloraNotification::addToWorld (Object& object) const
{
	const ServerObject* const serverObject = dynamic_cast<const ServerObject*> (&object);
	if (!serverObject)
	{
		DEBUG_WARNING (true, ("ServerClearCollidableFloraNotification::addToWorld - [%s] is not a server object", object.getObjectTemplate ()->getName ()));
		return;
	}

	const SharedObjectTemplate* const objectTemplate = dynamic_cast<const SharedObjectTemplate*> (serverObject->getSharedTemplate ());
	if (!objectTemplate)
	{
		DEBUG_WARNING (true, ("ServerClearCollidableFloraNotification::addToWorld - [%s] does not have a shared object template", object.getObjectTemplate ()->getName ()));
		return;
	}

	const float clearFloraRadius = objectTemplate->getClearFloraRadius ();
	if (clearFloraRadius <= 0.f)
	{
		DEBUG_WARNING (true, ("ServerClearCollidableFloraNotification::addToWorld - [%s] has a clearFloraRadius (%1.2f) <= 0", object.getObjectTemplate ()->getName (), clearFloraRadius));
		return;
	}

	TerrainObject* const terrainObject = TerrainObject::getInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING (true, ("ServerClearCollidableFloraNotification::addToWorld - no terrain"));
		return;		
	}

	terrainObject->addClearCollidableFloraObject (&object, object.getPosition_w (), clearFloraRadius);
}

//-------------------------------------------------------------------

void ServerClearCollidableFloraNotification::removeFromWorld (Object& object) const
{
	TerrainObject* const terrainObject = TerrainObject::getInstance ();
	if (!terrainObject)
	{
		DEBUG_WARNING (true, ("ServerClearCollidableFloraNotification::addToWorld - no terrain"));
		return;		
	}

	terrainObject->removeClearCollidableFloraObject (&object);
}

//-------------------------------------------------------------------

bool ServerClearCollidableFloraNotification::positionChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	removeFromWorld (object);
	addToWorld (object);

	return true;
}

//-------------------------------------------------------------------

bool ServerClearCollidableFloraNotification::positionAndRotationChanged (Object& object, bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	removeFromWorld (object);
	addToWorld (object);

	return true;
}

//===================================================================
