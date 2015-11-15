// DatabaseConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "DatabaseConnection.h"
#include "TaskManager.h"
#include "serverNetworkMessages/GameTaskManagerMessages.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"

//-----------------------------------------------------------------------

DatabaseConnection::DatabaseConnection() :
TaskHandler()
{
//	TaskManager::startServer("PlanetServer", "");
//	TaskManager::startServer("SwgGameServer", "");
}

//-----------------------------------------------------------------------

DatabaseConnection::~DatabaseConnection()
{
}

//-----------------------------------------------------------------------

DatabaseConnection & DatabaseConnection::operator = (const DatabaseConnection & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

void DatabaseConnection::receive(const Archive::ByteStream & message)
{
	Archive::ReadIterator r(message);
	GameNetworkMessage m(r);

	r = message.begin();
	if(m.isType("TaskSpawnProcess"))
	{
		TaskSpawnProcess s(r);
		IGNORE_RETURN(TaskManager::startServer(s.getProcessName(), s.getOptions(), s.getTargetHostAddress(), s.getSpawnDelay()));
	}
	else if(m.isType("ServerIdleMessage"))
	{
		ServerIdleMessage msg(r);
		TaskManager::onDatabaseIdle(msg.getIsIdle());
	}
}

//-----------------------------------------------------------------------

