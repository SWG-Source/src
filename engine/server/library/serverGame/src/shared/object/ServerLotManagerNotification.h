//===================================================================
//
// ServerLotManagerNotification.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ServerLotManagerNotification_H
#define INCLUDED_ServerLotManagerNotification_H

//===================================================================

#include "sharedGame/LotManagerNotification.h"

//===================================================================

class ServerLotManagerNotification : public LotManagerNotification
{
public:

	static ServerLotManagerNotification& getInstance ();

public:

	ServerLotManagerNotification ();
	virtual ~ServerLotManagerNotification ();

	virtual void addToWorld (Object& object) const;

private:

	ServerLotManagerNotification (const ServerLotManagerNotification&);
	ServerLotManagerNotification& operator= (const ServerLotManagerNotification&);

private:

	static ServerLotManagerNotification ms_instance;
};

//===================================================================

#endif
