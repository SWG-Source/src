//======================================================================
//
// ServerWorldIntangibleNotification.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerWorldIntangibleNotification.h"

#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"

//======================================================================

ServerWorldIntangibleNotification ServerWorldIntangibleNotification::ms_instance;

//----------------------------------------------------------------------

ServerWorldIntangibleNotification::ServerWorldIntangibleNotification()
: ObjectNotification()
{
}

// ----------------------------------------------------------------------

ServerWorldIntangibleNotification::~ServerWorldIntangibleNotification()
{
}

// ----------------------------------------------------------------------

void ServerWorldIntangibleNotification::addToWorld(Object &object) const
{
	ServerWorld::addIntangibleObject(safe_cast<ServerObject *>(&object));
}

// ----------------------------------------------------------------------

void ServerWorldIntangibleNotification::removeFromWorld(Object &object) const
{
	ServerWorld::removeIntangibleObject(safe_cast<ServerObject *>(&object));
}

//======================================================================
