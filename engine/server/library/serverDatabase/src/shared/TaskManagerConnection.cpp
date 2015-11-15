// TaskManagerConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverDatabase/FirstServerDatabase.h"
#include "ConfigServerDatabase.h"
#include "serverDatabase/TaskManagerConnection.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "sharedNetwork/NetworkSetupData.h"

//-----------------------------------------------------------------------

TaskManagerConnection::TaskManagerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

TaskManagerConnection::~TaskManagerConnection()
{
	DEBUG_REPORT_LOG(true, ("Deleting task server connection\n"));
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
	ServerConnection::onConnectionClosed();
	static MessageConnectionCallback m("TaskConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

