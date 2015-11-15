//======================================================================
//
// ServerWorldTerrainObjectNotification.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerWorldTerrainObjectNotification.h"

#include "serverGame/ServerWorld.h"
#include "sharedTerrain/TerrainObject.h"

//======================================================================

ServerWorldTerrainObjectNotification ServerWorldTerrainObjectNotification::ms_instance;

//----------------------------------------------------------------------

ServerWorldTerrainObjectNotification::ServerWorldTerrainObjectNotification()
: ObjectNotification()
{
}

// ----------------------------------------------------------------------

ServerWorldTerrainObjectNotification::~ServerWorldTerrainObjectNotification()
{
}

// ----------------------------------------------------------------------

void ServerWorldTerrainObjectNotification::addToWorld(Object &object) const
{
	ServerWorld::addIntangibleObject(safe_cast<TerrainObject *>(&object));
}

// ----------------------------------------------------------------------

void ServerWorldTerrainObjectNotification::removeFromWorld(Object &object) const
{
	ServerWorld::removeIntangibleObject(safe_cast<TerrainObject *>(&object));
}
//======================================================================
