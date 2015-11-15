//======================================================================
//
// ServerWorldTerrainObjectNotification.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ServerWorldTerrainObjectNotification_H
#define INCLUDED_ServerWorldTerrainObjectNotification_H

//======================================================================

#include "sharedObject/ObjectNotification.h"

//----------------------------------------------------------------------

class ServerWorldTerrainObjectNotification : public ObjectNotification
{
public:
	ServerWorldTerrainObjectNotification();
	virtual ~ServerWorldTerrainObjectNotification();
	
	virtual void addToWorld(Object &object) const;
	virtual void removeFromWorld(Object &object) const;
	
	static const ServerWorldTerrainObjectNotification & getInstance ();
	
private:
	
	ServerWorldTerrainObjectNotification(const ServerWorldTerrainObjectNotification &);
	ServerWorldTerrainObjectNotification &operator =(const ServerWorldTerrainObjectNotification &);

	static ServerWorldTerrainObjectNotification ms_instance;
};

//----------------------------------------------------------------------

inline const ServerWorldTerrainObjectNotification & ServerWorldTerrainObjectNotification::getInstance ()
{
	return ms_instance;
}

//======================================================================

#endif
