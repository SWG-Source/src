// DatabaseConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "DatabaseConnection.h"
#include "TaskManager.h"
#include "serverNetworkMessages/GameTaskManagerMessages.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"

#include "sharedFoundation/CrcConstexpr.hpp"

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
	
	const uint32 messageType = m.getType();
	
	switch(messageType) {
		case constcrc("TaskSpawnProcess") :
		{
			TaskSpawnProcess s(r);
			IGNORE_RETURN(TaskManager::startServer(s.getProcessName(), s.getOptions(), s.getTargetHostAddress(), s.getSpawnDelay()));
			break;
		}
		case constcrc("ServerIdleMessage") :
		{
			ServerIdleMessage msg(r);
			TaskManager::onDatabaseIdle(msg.getIsIdle());
			break;
		}
	}
}

//-----------------------------------------------------------------------

