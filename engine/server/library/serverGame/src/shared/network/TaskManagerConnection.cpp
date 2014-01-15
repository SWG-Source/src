// TaskManagerConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/TaskManagerConnection.h"
#include "serverNetworkMessages/TaskConnectionIdMessage.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/NetworkSetupData.h"

//-----------------------------------------------------------------------

TaskManagerConnection::TaskManagerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

TaskManagerConnection::~TaskManagerConnection()
{
}

//-----------------------------------------------------------------------

void TaskManagerConnection::onConnectionClosed() 
{
	ServerConnection::onConnectionClosed();
	static MessageConnectionCallback m("TaskConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void TaskManagerConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();
	static MessageConnectionCallback m("TaskConnectionOpened");
	emitMessage(m);

	// get cluster name
	std::string clusterName;
	ConfigFile::Section const * const sec = ConfigFile::getSection("TaskManager");
	if (sec)
	{
		ConfigFile::Key const * const ky = sec->findKey("clusterName");
		if (ky)
		{
			clusterName = ky->getAsString(ky->getCount()-1, "");
		}
	}

	TaskConnectionIdMessage id(TaskConnectionIdMessage::Game, "", clusterName);
	send(id, true);
}

//-----------------------------------------------------------------------

void TaskManagerConnection::onReceive(const Archive::ByteStream & message)
{
	ServerConnection::onReceive(message);

	// special TaskManagerConnection-specific handler here
}

//-----------------------------------------------------------------------
