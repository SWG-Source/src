//======================================================================
//
// ServerWorldTangibleNotification.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ServerWorldTangibleNotification_H
#define INCLUDED_ServerWorldTangibleNotification_H

//======================================================================

#include "sharedObject/ObjectNotification.h"

//----------------------------------------------------------------------

class ServerWorldTangibleNotification : public ObjectNotification
{
public:
	ServerWorldTangibleNotification();
	virtual ~ServerWorldTangibleNotification();
	
	virtual void addToWorld(Object &object) const;
	virtual void removeFromWorld(Object &object) const;
	
	static const ServerWorldTangibleNotification & getInstance ();

private:
	
	ServerWorldTangibleNotification(const ServerWorldTangibleNotification &);
	ServerWorldTangibleNotification &operator =(const ServerWorldTangibleNotification &);

	static ServerWorldTangibleNotification ms_instance;
};

//----------------------------------------------------------------------

inline const ServerWorldTangibleNotification & ServerWorldTangibleNotification::getInstance ()
{
	return ms_instance;
}

//======================================================================

#endif
