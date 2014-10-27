// PlanetConnection.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 


//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "PlanetConnection.h"

#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "TaskConnection.h"
#include "TaskManager.h"


//-----------------------------------------------------------------------

PlanetConnection::PlanetConnection(TaskConnection * c) :
TaskHandler(),
connection(c)
{
}

//-----------------------------------------------------------------------

PlanetConnection::PlanetConnection(const PlanetConnection &)
{
	connection = 0;

}

//-----------------------------------------------------------------------

PlanetConnection::~PlanetConnection()
{
}

//-----------------------------------------------------------------------

PlanetConnection & PlanetConnection::operator = (const PlanetConnection & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

void PlanetConnection::receive(const Archive::ByteStream & message)
{
	Archive::ReadIterator r(message);
	GameNetworkMessage m(r);

	r = message.begin();
	if(m.isType("TaskSpawnProcess"))
	{
		TaskSpawnProcess s(r);
		TaskManager::startServer(s.getProcessName(), s.getOptions(), s.getTargetHostAddress(), s.getSpawnDelay());
	}
}

//-----------------------------------------------------------------------
