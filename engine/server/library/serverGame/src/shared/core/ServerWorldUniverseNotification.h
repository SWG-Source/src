//======================================================================
//
// ServerWorldUniverseNotification.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ServerWorldUniverseNotification_H
#define INCLUDED_ServerWorldUniverseNotification_H

//======================================================================

#include "sharedObject/ObjectNotification.h"

//----------------------------------------------------------------------

class ServerWorldUniverseNotification : public ObjectNotification
{
public:
	ServerWorldUniverseNotification();
	virtual ~ServerWorldUniverseNotification();
	
	virtual void addToWorld(Object &object) const;
	virtual void removeFromWorld(Object &object) const;
	
	static const ServerWorldUniverseNotification & getInstance ();

private:
	
	ServerWorldUniverseNotification(const ServerWorldUniverseNotification &);
	ServerWorldUniverseNotification &operator =(const ServerWorldUniverseNotification &);

	static ServerWorldUniverseNotification ms_instance;
};

//----------------------------------------------------------------------

inline const ServerWorldUniverseNotification & ServerWorldUniverseNotification::getInstance ()
{
	return ms_instance;
}

//======================================================================

#endif
