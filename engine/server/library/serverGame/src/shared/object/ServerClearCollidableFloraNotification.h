//===================================================================
//
// ServerClearCollidableFloraNotification.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ServerClearCollidableFloraNotification_H
#define INCLUDED_ServerClearCollidableFloraNotification_H

//===================================================================

#include "sharedObject/ObjectNotification.h"

//===================================================================

class ServerClearCollidableFloraNotification : public ObjectNotification
{
public:

	static ServerClearCollidableFloraNotification& getInstance ();

public:

	ServerClearCollidableFloraNotification ();
	virtual ~ServerClearCollidableFloraNotification ();

	virtual void addToWorld (Object& object) const;
	virtual void removeFromWorld (Object& object) const;

	virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

private:

	ServerClearCollidableFloraNotification (const ServerClearCollidableFloraNotification&);
	ServerClearCollidableFloraNotification& operator= (const ServerClearCollidableFloraNotification&);

private:

	static ServerClearCollidableFloraNotification ms_instance;
};

//===================================================================

#endif
