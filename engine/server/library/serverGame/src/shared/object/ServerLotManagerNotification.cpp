//===================================================================
//
// ServerLotManagerNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerLotManagerNotification.h"

#include "serverGame/ServerObject.h"
#include "sharedGame/SharedTangibleObjectTemplate.h"

//===================================================================
// STATIC PUBLIC ServerLotManagerNotification
//===================================================================

ServerLotManagerNotification ServerLotManagerNotification::ms_instance;

//-------------------------------------------------------------------

ServerLotManagerNotification& ServerLotManagerNotification::getInstance ()
{
	return ms_instance;
}

//===================================================================
// PUBLIC ServerLotManagerNotification
//===================================================================

ServerLotManagerNotification::ServerLotManagerNotification () :
	LotManagerNotification ()
{
}

//-------------------------------------------------------------------
	
ServerLotManagerNotification::~ServerLotManagerNotification ()
{
}

//-------------------------------------------------------------------

void ServerLotManagerNotification::addToWorld (Object& object) const
{
	const ServerObject* const serverObject = dynamic_cast<const ServerObject*> (&object);
	if (!serverObject)
	{
		DEBUG_WARNING (true, ("ServerLotManagerNotification::addToWorld - [%s] is not a server object", object.getObjectTemplate ()->getName ()));
		return;
	}

	const SharedTangibleObjectTemplate* const sharedTangibleObjectTemplate = dynamic_cast<const SharedTangibleObjectTemplate*> (serverObject->getSharedTemplate ());
	if (!sharedTangibleObjectTemplate)
	{
		DEBUG_WARNING (true, ("ServerLotManagerNotification::addToWorld - [%s] does not have a shared tangible object template", object.getObjectTemplate ()->getName ()));
		return;
	}

	const StructureFootprint* const structureFootprint = sharedTangibleObjectTemplate->getStructureFootprint ();
	NOT_NULL (structureFootprint);

	if (structureFootprint)
		LotManagerNotification::addToWorld (object, *structureFootprint);
}

//===================================================================

