//======================================================================
//
// ServerWorldIntangibleNotification.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ServerWorldIntangibleNotification_H
#define INCLUDED_ServerWorldIntangibleNotification_H

//======================================================================

#include "sharedObject/ObjectNotification.h"

//----------------------------------------------------------------------

class ServerWorldIntangibleNotification : public ObjectNotification
{
public:
	ServerWorldIntangibleNotification();
	virtual ~ServerWorldIntangibleNotification();
	
	virtual void addToWorld(Object &object) const;
	virtual void removeFromWorld(Object &object) const;
	
	static const ServerWorldIntangibleNotification & getInstance ();

private:
	
	ServerWorldIntangibleNotification (const ServerWorldIntangibleNotification &);
	ServerWorldIntangibleNotification &operator =(const ServerWorldIntangibleNotification &);

	static ServerWorldIntangibleNotification ms_instance;
};

//----------------------------------------------------------------------

inline const ServerWorldIntangibleNotification & ServerWorldIntangibleNotification::getInstance ()
{
	return ms_instance;
}

//======================================================================

#endif
