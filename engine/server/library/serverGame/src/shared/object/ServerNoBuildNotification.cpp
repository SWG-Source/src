//===================================================================
//
// ServerNoBuildNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerNoBuildNotification.h"

#include "serverGame/ServerObject.h"
#include "sharedGame/SharedObjectTemplate.h"

//===================================================================
// STATIC PUBLIC ServerNoBuildNotification
//===================================================================

ServerNoBuildNotification ServerNoBuildNotification::ms_instance;

//-------------------------------------------------------------------

ServerNoBuildNotification& ServerNoBuildNotification::getInstance ()
{
	return ms_instance;
}

//===================================================================
// PUBLIC ServerNoBuildNotification
//===================================================================

ServerNoBuildNotification::ServerNoBuildNotification () :
	NoBuildNotification ()
{
}

//-------------------------------------------------------------------
	
ServerNoBuildNotification::~ServerNoBuildNotification ()
{
}

//-------------------------------------------------------------------

void ServerNoBuildNotification::addToWorld (Object& object) const
{
	const ServerObject* const serverObject = dynamic_cast<const ServerObject*> (&object);
	if (!serverObject)
	{
		DEBUG_WARNING (true, ("ServerNoBuildNotification::addToWorld - [%s] is not a server object", object.getObjectTemplate ()->getName ()));
		return;
	}

	const SharedObjectTemplate* const sharedObjectTemplate = dynamic_cast<const SharedObjectTemplate*> (serverObject->getSharedTemplate ());
	if (!sharedObjectTemplate)
	{
		DEBUG_WARNING (true, ("ServerNoBuildNotification::addToWorld - [%s] does not have a shared object template", object.getObjectTemplate ()->getName ()));
		return;
	}

	const float noBuildRadius = sharedObjectTemplate->getNoBuildRadius ();
	NoBuildNotification::addToWorld (object, noBuildRadius);
}

//===================================================================

