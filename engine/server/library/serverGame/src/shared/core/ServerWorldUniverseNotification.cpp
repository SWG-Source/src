//======================================================================
//
// ServerWorldUniverseNotification.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerWorldUniverseNotification.h"

#include "serverGame/ServerWorld.h"
#include "serverGame/UniverseObject.h"

//======================================================================

ServerWorldUniverseNotification ServerWorldUniverseNotification::ms_instance;

//----------------------------------------------------------------------

ServerWorldUniverseNotification::ServerWorldUniverseNotification()
: ObjectNotification()
{
}

// ----------------------------------------------------------------------

ServerWorldUniverseNotification::~ServerWorldUniverseNotification()
{
}

// ----------------------------------------------------------------------

void ServerWorldUniverseNotification::addToWorld(Object &object) const
{
	ServerWorld::addUniverseObject(safe_cast<UniverseObject *>(&object));
}

// ----------------------------------------------------------------------

void ServerWorldUniverseNotification::removeFromWorld(Object &object) const
{
	ServerWorld::removeUniverseObject(safe_cast<UniverseObject *>(&object));
}

//======================================================================
