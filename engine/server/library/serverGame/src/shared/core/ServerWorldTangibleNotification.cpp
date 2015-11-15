//======================================================================
//
// ServerWorldTangibleNotification.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerWorldTangibleNotification.h"

#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"

//======================================================================

ServerWorldTangibleNotification ServerWorldTangibleNotification::ms_instance;

//----------------------------------------------------------------------

ServerWorldTangibleNotification::ServerWorldTangibleNotification()
: ObjectNotification()
{
}

// ----------------------------------------------------------------------

ServerWorldTangibleNotification::~ServerWorldTangibleNotification()
{
}

// ----------------------------------------------------------------------

void ServerWorldTangibleNotification::addToWorld(Object &object) const
{
	ServerWorld::addTangibleObject(safe_cast<ServerObject *>(&object));
}

// ----------------------------------------------------------------------

void ServerWorldTangibleNotification::removeFromWorld(Object &object) const
{
	ServerWorld::removeTangibleObject(safe_cast<ServerObject *>(&object));
}

//======================================================================
