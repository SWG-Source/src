// TaskConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "ConfigCentralServer.h"
#include "serverNetworkMessages/TaskConnectionIdMessage.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "serverNetworkMessages/TaskUtilization.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "TaskConnection.h"

//-----------------------------------------------------------------------

TaskConnection::TaskConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData()),
callback(new MessageDispatch::Callback),
m_connected(false)
{

}

//-----------------------------------------------------------------------

TaskConnection::TaskConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t),
callback(new MessageDispatch::Callback),
m_connected(false)
{
}

//-----------------------------------------------------------------------

TaskConnection::~TaskConnection()
{
	delete callback;
}

//-----------------------------------------------------------------------

void TaskConnection::onConnectionClosed()
{
	m_connected = false;

	static MessageConnectionCallback m("TaskConnectionClosed");
	emitMessage(m);
	CentralServer::getInstance().setTaskManager(0);
}

//-----------------------------------------------------------------------

void TaskConnection::onConnectionOpened()
{
	m_connected = true;

	static MessageConnectionCallback m("TaskConnectionOpened");
	emitMessage(m);
	std::string cmdLine = CentralServer::getInstance().getCommandLine();
	// fix up path
	size_t last = 0;
	while(cmdLine.find_first_of("\\", last + 1) < cmdLine.size())
		last = cmdLine.find_first_of("\\", last + 1);
	cmdLine = cmdLine.substr(last);
	TaskConnectionIdMessage id(TaskConnectionIdMessage::Central, cmdLine, ConfigCentralServer::getClusterName());
	send(id, true);
	CentralServer::getInstance().setTaskManager(this);

	// start launching processes
	CentralServer::getInstance().launchStartingProcesses();
}

//-----------------------------------------------------------------------

void TaskConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator r(message);
	GameNetworkMessage m(r);
	if(m.isType("TaskUtilization"))
	{
		// generate utilization messages for the task manager
		TaskUtilization sysCpu(TaskUtilization::SYSTEM_CPU, 0.5f); //lint !e641 Converting enum to int (it's ok, this is for a network message)
		TaskUtilization sysMem(TaskUtilization::SYSTEM_MEMORY, 0.5f); //lint !e641 Converting enum to int (it's ok, this is for a network message)
		TaskUtilization sysNet(TaskUtilization::SYSTEM_NETWORK_IO, 50000.0f); //lint !e641 Converting enum to int (it's ok, this is for a network message)
		TaskUtilization procCpu(TaskUtilization::PROCESS_CPU, 0.5f); //lint !e641 Converting enum to int (it's ok, this is for a network message)
		TaskUtilization procMem(TaskUtilization::PROCESS_MEMORY, 0.5f); //lint !e641 Converting enum to int (it's ok, this is for a network message)
		TaskUtilization procNet(TaskUtilization::PROCESS_NETWORK_IO, 50000.0f); //lint !e641 Converting enum to int (it's ok, this is for a network message)

		send(sysCpu, true);
		send(sysMem, true);
		send(sysNet, true);
		send(procCpu, true);
		send(procMem, true);
		send(procNet, true);
	}
	else
	{
		emitMessage(m);
	}
}

//-----------------------------------------------------------------------


