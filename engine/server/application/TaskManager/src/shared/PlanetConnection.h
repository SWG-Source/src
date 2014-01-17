// PlanetConnection.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 


#ifndef	_INCLUDED_PlanetConnection_H
#define	_INCLUDED_PlanetConnection_H

//-----------------------------------------------------------------------

#include "TaskHandler.h"

class TaskConnection;

//-----------------------------------------------------------------------

class PlanetConnection : public TaskHandler
{
public:
	PlanetConnection(TaskConnection * connection);
	~PlanetConnection();

	void receive(const Archive::ByteStream & message);

private:
	PlanetConnection & operator = (const PlanetConnection & rhs);
	PlanetConnection(const PlanetConnection & source);
	TaskConnection *  connection;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_PlanetConnection_H
