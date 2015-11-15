//===================================================================
//
// ServerNoBuildNotification.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ServerNoBuildNotification_H
#define INCLUDED_ServerNoBuildNotification_H

//===================================================================

#include "sharedGame/NoBuildNotification.h"

//===================================================================

class ServerNoBuildNotification : public NoBuildNotification
{
public:

	static ServerNoBuildNotification& getInstance ();

public:

	ServerNoBuildNotification ();
	virtual ~ServerNoBuildNotification ();

	virtual void addToWorld (Object& object) const;

private:

	ServerNoBuildNotification (const ServerNoBuildNotification&);
	ServerNoBuildNotification& operator= (const ServerNoBuildNotification&);

private:

	static ServerNoBuildNotification ms_instance;
};

//===================================================================

#endif
