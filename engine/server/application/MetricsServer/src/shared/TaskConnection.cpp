// TaskConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstMetricsServer.h"
#include "TaskConnection.h"

#include "ConfigMetricsServer.h"
#include "MetricsServer.h"
#include "serverNetworkMessages/TaskConnectionIdMessage.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/NetworkSetupData.h"

//-----------------------------------------------------------------------

TaskConnection::TaskConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

TaskConnection::TaskConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t)
{
}

//-----------------------------------------------------------------------

TaskConnection::~TaskConnection()
{
}

//-----------------------------------------------------------------------

void TaskConnection::onConnectionClosed()
{
}

//-----------------------------------------------------------------------

void TaskConnection::onConnectionOpened()
{
	std::string cmdLine = MetricsServer::getCommandLine();
	// fix up path
	size_t last = 0;
	while(cmdLine.find_first_of("\\", last + 1) < cmdLine.size())
		last = cmdLine.find_first_of("\\", last + 1);
	cmdLine = cmdLine.substr(last);

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

	TaskConnectionIdMessage id(TaskConnectionIdMessage::Metrics, cmdLine, clusterName);
	send(id, true);
}

//-----------------------------------------------------------------------

void TaskConnection::onReceive(const Archive::ByteStream & message)
{
	UNREF(message);
}

//-----------------------------------------------------------------------


