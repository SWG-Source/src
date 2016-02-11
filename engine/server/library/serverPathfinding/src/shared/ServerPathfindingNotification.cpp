// ======================================================================
//
// ServerPathfindingNotification.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverPathfinding/FirstServerPathfinding.h"
#include "serverPathfinding/ServerPathfindingNotification.h"

#include "serverGame/ServerObject.h"
#include "serverGame/BuildingObject.h"

#include "serverPathfinding/CityPathGraphManager.h"
#include "serverPathfinding/ServerPathfindingConstants.h"

#include "sharedObject/Object.h"

namespace ServerPathfindingNotificationNamespace
{
	ServerPathfindingNotification g_notification;
};

using namespace ServerPathfindingNotificationNamespace;

// ======================================================================
// CityPathGraphManager notification 

ServerPathfindingNotification::ServerPathfindingNotification()
{
}

ServerPathfindingNotification::~ServerPathfindingNotification()
{
}

// ----------------------------------------------------------------------

void ServerPathfindingNotification::addToWorld ( Object & object ) const
{
	BuildingObject * building = dynamic_cast<BuildingObject*>(&object);

	if(building)
	{
		CityPathGraphManager::addBuilding( building );
	}
	else if(object.asServerObject() != nullptr && object.asServerObject()->isWaypoint()) 
	{
		CityPathGraphManager::addWaypoint( object.asServerObject() );
	}
	else
	{
	}
}

void ServerPathfindingNotification::removeFromWorld ( Object & object ) const
{
	BuildingObject * building = dynamic_cast<BuildingObject*>(&object);

	if(building)
	{
		CityPathGraphManager::removeBuilding( building );
	}
	else if(object.asServerObject() != nullptr && object.asServerObject()->isWaypoint())
	{
		CityPathGraphManager::removeWaypoint( object.asServerObject() );
	}
	else
	{
		CityPathGraphManager::destroyPathGraph(object.asServerObject());
	}
}

bool ServerPathfindingNotification::positionChanged ( Object & object, bool /*dueToParentChange*/, Vector const & oldPosition) const
{
	BuildingObject * building = dynamic_cast<BuildingObject*>(&object);

	if(building)
	{
		CityPathGraphManager::moveBuilding( building, oldPosition );
	}
	else if(object.asServerObject() != nullptr && object.asServerObject()->isWaypoint()) 
	{
		CityPathGraphManager::moveWaypoint( object.asServerObject(), oldPosition );
	}

	return true;
}

bool ServerPathfindingNotification::positionAndRotationChanged ( Object & object, bool /*dueToParentChange*/, Vector const & oldPosition ) const
{
	BuildingObject * building = dynamic_cast<BuildingObject*>(&object);

	if(building)
	{
		CityPathGraphManager::moveBuilding( building, oldPosition );
	}
	else if(object.asServerObject() != nullptr && object.asServerObject()->isWaypoint()) 
	{
		CityPathGraphManager::moveWaypoint( object.asServerObject(), oldPosition );
	}

	return true;
}

// ----------------------------------------------------------------------

void ServerPathfindingNotification::destroyBuilding ( BuildingObject * building )
{
	CityPathGraphManager::destroyBuilding(building);
}

// ----------------------------------------------------------------------

void ServerPathfindingNotification::destroyWaypoint ( ServerObject * building )
{
	CityPathGraphManager::destroyWaypoint(building);
}

// ----------------------------------------------------------------------

ServerPathfindingNotification & ServerPathfindingNotification::getInstance ( void )
{
	return g_notification;
}

// ======================================================================
