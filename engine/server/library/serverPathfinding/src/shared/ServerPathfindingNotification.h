// ======================================================================
//
// ServerPathfindingNotification.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_ServerPathfindingNotification_H
#define	INCLUDED_ServerPathfindingNotification_H

#include "sharedObject/ObjectNotification.h"

class Object;
class Vector;
class BuildingObject;
class ServerObject;

// ======================================================================

class ServerPathfindingNotification : public ObjectNotification
{
public:

	ServerPathfindingNotification();
	virtual ~ServerPathfindingNotification();

	virtual void    addToWorld                  ( Object & object ) const;
	virtual void    removeFromWorld             ( Object & object ) const;

	virtual bool    positionChanged             ( Object & object, bool dueToParentChange, Vector const & oldPosition) const;
	virtual bool    positionAndRotationChanged  ( Object & object, bool dueToParentChange, Vector const & oldPosition ) const;

	// ----------
	
	static void destroyBuilding ( BuildingObject * building );
	static void destroyWaypoint ( ServerObject * waypoint );

	static ServerPathfindingNotification & getInstance ( void );

protected:

private:

	ServerPathfindingNotification(const ServerPathfindingNotification &);
	ServerPathfindingNotification &operator =(const ServerPathfindingNotification &);
};

// ======================================================================

#endif

