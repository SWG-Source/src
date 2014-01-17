// TaskConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstPlanetServer.h"
#include "serverNetworkMessages/TaskConnectionIdMessage.h"
#include "serverNetworkMessages/TaskUtilization.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "TaskConnection.h"

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
    static MessageConnectionCallback m("TaskConnectionClosed");
    emitMessage(m);
	PlanetServer::getInstance().setTaskManager(0);
}

//-----------------------------------------------------------------------

void TaskConnection::onConnectionOpened()
{
	PlanetServer::getInstance().setTaskManager(this);

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

	TaskConnectionIdMessage id(TaskConnectionIdMessage::Planet, "", clusterName);
	send(id, true);
	static MessageConnectionCallback m("TaskConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void TaskConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator r(message);
	GameNetworkMessage m(r);
	if(m.isType("TaskUtilization"))
	{
		// generate utilization messages for the task manager
		TaskUtilization sysCpu(static_cast<int>(TaskUtilization::SYSTEM_CPU), 0.5f);
		TaskUtilization sysMem(static_cast<int>(TaskUtilization::SYSTEM_MEMORY), 0.5f);
		TaskUtilization sysNet(static_cast<int>(TaskUtilization::SYSTEM_NETWORK_IO), 50000.0f);
		TaskUtilization procCpu(static_cast<int>(TaskUtilization::PROCESS_CPU), 0.5f);
		TaskUtilization procMem(static_cast<int>(TaskUtilization::PROCESS_MEMORY), 0.5f);
		TaskUtilization procNet(static_cast<int>(TaskUtilization::PROCESS_NETWORK_IO), 50000.0f);

		send(sysCpu, true);
		send(sysMem, true);
		send(sysNet, true);
		send(procCpu, true);
		send(procMem, true);
		send(procNet, true);
	}
}

//-----------------------------------------------------------------------
