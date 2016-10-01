// CentralConnection.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "CentralConnection.h"
#include "TaskConnection.h"
#include "TaskManager.h"
#include "serverNetworkMessages/PreloadFinishedMessage.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedFoundation/Clock.h"
#include "sharedMessageDispatch/Transceiver.h"

#include "sharedFoundation/CrcConstexpr.hpp"

//-----------------------------------------------------------------------

CentralConnection::CentralConnection(TaskConnection * c, const std::string & n) :
TaskHandler(),
callback(),
commandLine(n),
connected(true),
connection(c),
lastSpawnTime(Clock::timeMs())
{
	callback.connect(c->getTransceiverClosed(), *this, &CentralConnection::closed); //lint !e1025 !e1703 !e1514 !e64
	callback.connect(*this, &CentralConnection::onProcessKilled); //lint !e1025 !e1703 !e1514 !e64 !e1058 !e118
//	TaskManager::startServer("ConnectionServer", "");
//	TaskManager::startServer("SwgDatabaseServer", "");
}

//-----------------------------------------------------------------------

CentralConnection::~CentralConnection()
{
	connection = 0;
}

//-----------------------------------------------------------------------

void CentralConnection::closed(const Closed &)
{
	if(connected)
	{
		ProcessAborted a;
		a.commandLine = commandLine;
		a.hostName = NetworkHandler::getHostName();
		MessageDispatch::Transceiver<const ProcessAborted &> b;
		b.emitMessage(a);

		unsigned long t = Clock::timeMs();
		if(t - lastSpawnTime > 5000) // stop spawning if it bombs immediately
		{
			lastSpawnTime = t;
			// need to restart it!
			//IGNORE_RETURN(TaskManager::startServer(commandLine));
		}
		else
		{
			// emit the killed message
			ProcessKilled k;
			k.commandLine = a.commandLine;
			k.hostName = a.hostName;
			MessageDispatch::Transceiver<const ProcessKilled &> pk;
			pk.emitMessage(k);
		}
		connected = false;
		TaskManager::setCentralConnection(0);
	}
}

//-----------------------------------------------------------------------

void CentralConnection::onProcessKilled(const ProcessKilled & k)
{
	if(k.commandLine == commandLine)
		lastSpawnTime = Clock::timeMs(); // force it to stay dead
}

//-----------------------------------------------------------------------

void CentralConnection::receive(const Archive::ByteStream & message)
{
	Archive::ReadIterator r(message);
	GameNetworkMessage m(r);
	r = message.begin();
	
	const uint32 messageType = m.getType();

	switch (messageType) {
		case constcrc("TaskSpawnProcess") :
		{
			TaskSpawnProcess s(r);
			IGNORE_RETURN(TaskManager::startServer(s.getProcessName(), s.getOptions(), s.getTargetHostAddress(), s.getSpawnDelay()));
			break;
		}
		case constcrc("PreloadFinishedMessage") :
		{
			PreloadFinishedMessage m(r);
			if (m.getFinished())
				TaskManager::onPreloadFinished();
			break;
		}
		case constcrc("ConGenericMessage") :
		{
			ConGenericMessage taskConsoleCommand(r);

			static std::string command = taskConsoleCommand.getMsg();
			while(command.rfind(' ') != command.npos || command.rfind('\n') != command.npos || command.rfind('\r') != command.npos)
			{
				command = command.substr(0, command.length() - 1);
			}

			TaskManager::executeCommand(command);
			break;
		}
	}
}

//-----------------------------------------------------------------------

