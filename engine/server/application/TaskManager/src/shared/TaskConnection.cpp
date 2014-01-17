// TaskConnection.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------
#include "FirstTaskManager.h"

#include "CentralConnection.h"
#include "ConfigTaskManager.h"
#include "DatabaseConnection.h"
#include "Locator.h"
#include "GameConnection.h"
#include "ManagerConnection.h"
#include "MetricsServerConnection.h"
#include "PlanetConnection.h"
#include "TaskConnection.h"
#include "TaskManager.h"
#include "serverNetworkMessages/ExcommunicateGameServerMessage.h"
#include "serverNetworkMessages/TaskConnectionIdMessage.h"
#include "serverNetworkMessages/TaskKillProcess.h"
#include "sharedNetwork/NetworkSetupData.h"

//-----------------------------------------------------------------------

TaskConnection::TaskConnection(const std::string & a, const unsigned short p) :
Connection(a, p, NetworkSetupData()),
closed(),
failed(),
identified(),
opened(),
overflowing(),
receiveMessage(),
reset(),
handler(0)
{
}

//-----------------------------------------------------------------------

TaskConnection::TaskConnection(UdpConnectionMT * u, TcpClient * t) :
Connection(u, t),
closed(),
failed(),
identified(),
opened(),
overflowing(),
receiveMessage(),
reset(),
handler(0)
{
}

//-----------------------------------------------------------------------

TaskConnection::~TaskConnection()
{
	delete handler;
}

//-----------------------------------------------------------------------

void TaskConnection::onConnectionClosed()
{
	static Closed msg;
	closed.emitMessage(msg); //lint !e738 Symbol 'msg' not explicitly initialized
}

//-----------------------------------------------------------------------

void TaskConnection::onConnectionOpened()
{
	DEBUG_REPORT_LOG(true, ("Task Connection opened %s\n", getRemoteAddress().c_str()));
	static Opened msg;
	opened.emitMessage(msg); //lint !e738 Symbol 'msg' not explicitly initialized
}

//-----------------------------------------------------------------------

void TaskConnection::onConnectionOverflowing(const unsigned int bytesPending)
{
	static Overflowing msg;
	msg.bytesPending = bytesPending;
	overflowing.emitMessage(msg);
}

//-----------------------------------------------------------------------

void TaskConnection::onReceive(const Archive::ByteStream & message)
{
	static Receive msg;

	if(handler)
		handler->receive(message);

	Archive::ReadIterator r(message);
	GameNetworkMessage m(r);
	if(m.isType("TaskConnectionIdMessage"))
	{
		r = message.begin();
		TaskConnectionIdMessage t(r);

		FATAL((ConfigTaskManager::getVerifyClusterName() && (TaskManager::getNodeLabel() == "node0") && (t.getServerType() == TaskConnectionIdMessage::TaskManager) && (t.getClusterName() != std::string(ConfigTaskManager::getClusterName()))), ("Remote TaskManager %s (%s) reported cluster name (%s) that is different from my cluster name (%s)", t.getCommandLine().c_str(), getRemoteAddress().c_str(), t.getClusterName().c_str(), ConfigTaskManager::getClusterName()));

		Identified i = {this,  t.getServerType() };
		identified.emitMessage(i);

		switch(i.id)
		{
		case TaskConnectionIdMessage::Central:
			{
				REPORT_LOG(true, ("New Central Server connection active\n"));
				handler = new CentralConnection(this, t.getCommandLine());
				TaskManager::setCentralConnection(this);
			}
			break;
		case TaskConnectionIdMessage::Game:
			{
				REPORT_LOG(true, ("New Game Server connection active\n"));
				handler = new GameConnection(this);
			}
			break;
		case TaskConnectionIdMessage::Database:
			{
				REPORT_LOG(true, ("New Database Server connection active\n"));
				handler = new DatabaseConnection();
			}
			break;
		case TaskConnectionIdMessage::Metrics:
			{
				REPORT_LOG(true, ("New Metrics Server connection active\n"));
				handler = new MetricsServerConnection(this, t.getCommandLine());
			}
			break;
		case TaskConnectionIdMessage::Planet:
			{
				REPORT_LOG(true, ("New Planet Server connection active\n"));
				handler = new PlanetConnection(this);
			}
			break;

		default:
			WARNING_STRICT_FATAL(true, ("Unknown id (%d) received on task connection", i.id));
			break;
		}
	}
	else if(m.isType("ExcommunicateGameServerMessage"))
	{
		r = message.begin();
		ExcommunicateGameServerMessage ex(r);
		TaskKillProcess k(ex.getHostName(), ex.getProcessId(), true);
		TaskManager::killProcess(k);
		
		// broadcast to other task managers
		Locator::sendToAllTaskManagers(k);
	}
	else if(m.isType("TaskKillProcess"))
	{
		r = message.begin();
		TaskKillProcess k(r);
		TaskManager::killProcess(k);
	}

	msg.message = &message;
	receiveMessage.emitMessage(msg);
}

//-----------------------------------------------------------------------

void TaskConnection::send(const GameNetworkMessage & message)
{
	Archive::ByteStream b;
	message.pack(b);
	Connection::send(b, true);
}

//-----------------------------------------------------------------------

MessageDispatch::Transceiver<const Closed &> & TaskConnection::getTransceiverClosed()
{
	return closed; //lint !e1536 Exposing low access member
}

//-----------------------------------------------------------------------

MessageDispatch::Transceiver<const Failed &> & TaskConnection::getTransceiverFailed()
{
	return failed; //lint !e1536 Exposing low access member
}

//-----------------------------------------------------------------------

MessageDispatch::Transceiver<const Identified &> & TaskConnection::getTransceiverIdentified()
{
	return identified; //lint !e1536 Exposing low access member
}

//-----------------------------------------------------------------------

MessageDispatch::Transceiver<const Opened &> & TaskConnection::getTransceiverOpened()
{
	return opened; //lint !e1536 Exposing low access member
}

//-----------------------------------------------------------------------

MessageDispatch::Transceiver<const Overflowing &> &  TaskConnection::getTransceiverOverflowing()
{
	return overflowing; //lint !e1536 Exposing low access member
}

//-----------------------------------------------------------------------

MessageDispatch::Transceiver<const Receive &> & TaskConnection::getTransceiverReceive()
{
	return receiveMessage; //lint !e1536 Exposing low access member
}

//-----------------------------------------------------------------------

MessageDispatch::Transceiver<const Reset &> & TaskConnection::getTransceiverReset()
{
	return reset; //lint !e1536 Exposing low access member
}

//-----------------------------------------------------------------------
