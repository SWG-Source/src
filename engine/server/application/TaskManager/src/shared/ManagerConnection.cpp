// ManagerConnection.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "Locator.h"
#include "ManagerConnection.h"
#include "serverNetworkMessages/TaskProcessDiedMessage.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "serverNetworkMessages/TaskSpawnProcessAck.h"
#include "serverNetworkMessages/TaskUtilization.h"
#include "serverNetworkMessages/TaskConnectionIdMessage.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Scheduler.h"
#include "sharedLog/Log.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "ConfigTaskManager.h"
#include "TaskConnection.h"
#include "TaskManager.h"

#include "sharedFoundation/CrcConstexpr.hpp"

//-----------------------------------------------------------------------

namespace ManagerConnectionNamespace
{
	int s_managerConnectionCount = 0;
}

using namespace ManagerConnectionNamespace;

//-----------------------------------------------------------------------

ManagerConnection::ManagerConnection(const std::string & a, const unsigned short p) :
Connection(a,p, NetworkSetupData()),
m_nodeLabel(0),
m_remoteUtilAvg(100000.0f)
{
	
}

//-----------------------------------------------------------------------
ManagerConnection::ManagerConnection(UdpConnectionMT * u, TcpClient * t) :
Connection(u, t),
m_nodeLabel(0),
m_remoteUtilAvg(100000.0f)
{
	
}

//-----------------------------------------------------------------------

ManagerConnection::~ManagerConnection()
{
	if (m_nodeLabel)
	{
		Locator::closed(*m_nodeLabel, this);
		delete m_nodeLabel;
		m_nodeLabel = 0;
	}
}

//-----------------------------------------------------------------------
void ManagerConnection::onConnectionClosed()
{
	DEBUG_REPORT_LOG(true, ("Manager connection %s closed\n", getRemoteAddress().c_str()));
	TaskManager::retryConnection(this);
	if (m_nodeLabel)
	{
		Locator::closed(*m_nodeLabel, this);
		delete m_nodeLabel;
		m_nodeLabel = 0;
	}
	s_managerConnectionCount--;
}

//-----------------------------------------------------------------------

void ManagerConnection::onConnectionOpened()
{
	DEBUG_REPORT_LOG(true, ("Manager connection opened for %s \n", getRemoteAddress().c_str()));
	TaskConnectionIdMessage id(TaskConnectionIdMessage::TaskManager, TaskManager::getNodeLabel(), ConfigTaskManager::getClusterName());
	send(id);
	s_managerConnectionCount++;
}

//-----------------------------------------------------------------------

void ManagerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator r(message);
	GameNetworkMessage m(r);
	r = message.begin();
	
	const uint32 messageType = m.getType();
	
	switch(messageType) {
		case constcrc("SystemTimeCheck") :
		{
			long const currentTime = static_cast<long>(::time(nullptr));
			GenericValueTypeMessage<std::pair<std::string, long > > msg(r);

			if (TaskManager::getNodeLabel() == "node0")
			{
				if ((std::max(currentTime, msg.getValue().second) - std::min(currentTime, msg.getValue().second)) > ConfigTaskManager::getMaximumClockDriftToleranceSeconds())
				{
					LOG("CustomerService", ("system_clock_mismatch:System clock mismatch (%d seconds) by more than %d seconds: master TaskManager epoch (%ld), remote TaskManager %s (%s) epoch (%ld)", (std::max(currentTime, msg.getValue().second) - std::min(currentTime, msg.getValue().second)), ConfigTaskManager::getMaximumClockDriftToleranceSeconds(), currentTime, msg.getValue().first.c_str(), getRemoteAddress().c_str(), msg.getValue().second));

					// tell CentralServer about the system clock mismatch so it can report an alert to SOEMon
					GenericValueTypeMessage<std::string> systemTimeMismatchMessage("SystemTimeMismatchNotification",
						FormattedString<1024>().sprintf("%s: %s (%s) is off by %ld seconds", CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(currentTime)).c_str(), msg.getValue().first.c_str(), getRemoteAddress().c_str(), (std::max(currentTime, msg.getValue().second) - std::min(currentTime, msg.getValue().second))));
					TaskManager::sendToCentralServer(systemTimeMismatchMessage);
				}
			}
			break;
		}
		case constcrc("TaskConnectionIdMessage") :
		{
			static long const clockDriftFatalTimePeriod = static_cast<long>(TaskManager::getStartTime()) + ConfigTaskManager::getClockDriftFatalIntervalSeconds();
			long const currentTime = static_cast<long>(::time(nullptr));
			TaskConnectionIdMessage t(r);
			WARNING_STRICT_FATAL(t.getServerType() != TaskConnectionIdMessage::TaskManager,
								 ("ManagerConnection received wrong type identifier"));
			WARNING_STRICT_FATAL(m_nodeLabel, ("Received new taskconnectionIdMessage with an already named connection"));

			FATAL((ConfigTaskManager::getVerifyClusterName() && (TaskManager::getNodeLabel() == "node0") && (t.getServerType() == TaskConnectionIdMessage::TaskManager) && (t.getClusterName() != std::string(ConfigTaskManager::getClusterName()))), ("Remote TaskManager %s (%s) reported cluster name (%s) that is different from my cluster name (%s)", t.getCommandLine().c_str(), getRemoteAddress().c_str(), t.getClusterName().c_str(), ConfigTaskManager::getClusterName()));

			// don't allow cluster to start if the system clock across the boxes are out of sync
			bool remoteSystemClockInSync = true;
			if (TaskManager::getNodeLabel() == "node0")
			{
				if ((std::max(currentTime, t.getCurrentEpochTime()) - std::min(currentTime, t.getCurrentEpochTime())) > ConfigTaskManager::getMaximumClockDriftToleranceSeconds())
				{
					remoteSystemClockInSync = false;

					// don't bring down the cluster if we lost a box and when the box is
					// restarted, its system clock is out of sync; just ignore that
					// TaskManager, and send it a message to terminate itself
					if (currentTime <= clockDriftFatalTimePeriod)
					{
						FATAL(true, ("System clock mismatch (%d seconds) by more than %d seconds: master TaskManager epoch (%ld), remote TaskManager %s (%s) epoch (%ld)", (std::max(currentTime, t.getCurrentEpochTime()) - std::min(currentTime, t.getCurrentEpochTime())), ConfigTaskManager::getMaximumClockDriftToleranceSeconds(), currentTime, t.getCommandLine().c_str(), getRemoteAddress().c_str(), t.getCurrentEpochTime()));
					}
					else
					{
						LOG("CustomerService", ("system_clock_mismatch:System clock mismatch (%d seconds) by more than %d seconds: master TaskManager epoch (%ld), remote TaskManager %s (%s) epoch (%ld). Telling remote TaskManager to terminate.", (std::max(currentTime, t.getCurrentEpochTime()) - std::min(currentTime, t.getCurrentEpochTime())), ConfigTaskManager::getMaximumClockDriftToleranceSeconds(), currentTime, t.getCommandLine().c_str(), getRemoteAddress().c_str(), t.getCurrentEpochTime()));

						GenericValueTypeMessage<std::pair<std::string, std::pair<long, long> > > systemTimeMismatchMessage("SystemTimeMismatchMessage", std::make_pair(TaskManager::getNodeLabel(), std::make_pair(static_cast<long>(currentTime), t.getCurrentEpochTime())));
						send(systemTimeMismatchMessage);
					}
				}
			}

			if (remoteSystemClockInSync)
			{
				m_nodeLabel = new std::string(t.getCommandLine());
				Locator::opened(*m_nodeLabel, this);
				TaskManager::resendUnacknowledgedSpawnRequests(this, *m_nodeLabel);
			}
			break;
		}
		case constcrc("SystemTimeMismatchMessage") :
		{
			GenericValueTypeMessage<std::pair<std::string, std::pair<long, long> > > msg(r);
			FATAL((msg.getValue().first == "node0"), ("System clock mismatch (%d seconds) by more than %d seconds: master TaskManager epoch (%ld), self epoch (%ld)", (std::max(msg.getValue().second.first, msg.getValue().second.second) - std::min(msg.getValue().second.first, msg.getValue().second.second)), ConfigTaskManager::getMaximumClockDriftToleranceSeconds(), msg.getValue().second.first, msg.getValue().second.second));
			break;
		}
		case constcrc("TaskSpawnProcess") :
		{
			TaskSpawnProcess s(r);
			if (TaskManager::startServer(s.getProcessName(), s.getOptions(), s.getTargetHostAddress(), s.getSpawnDelay()) == 0)
			{
				DEBUG_REPORT_LOG(true, ("Failed to spawn %s on this node\n", s.getProcessName().c_str()));
				//send fail message back
			}
			else
			{
				TaskSpawnProcessAck ack(s.getTransactionId());
				send(ack);
			}
			break;
		}
		case constcrc("TaskUtilization") :
		{
			TaskUtilization util(r);
			switch(util.getUtilType())
			{
				case TaskUtilization::SYSTEM_AVG:
					{
						if (m_nodeLabel)
						{
							m_remoteUtilAvg = util.getUtilAmount();
							Locator::updateServerLoad(*m_nodeLabel, m_remoteUtilAvg);
						}
					}
					break;
				default:
					break;
			}
			break;
		}
		case constcrc("TaskProcessDiedMessage") :
		{
			TaskProcessDiedMessage died(r);
			TaskManager::sendToCentralServer(died);
			break;
		}
		case constcrc("TaskSpawnProcessAck") :
		{
			TaskSpawnProcessAck ack(r);
			TaskManager::removePendingSpawnProcessAck(ack.getTransactionId());
			break;
		}
	}
}

//-----------------------------------------------------------------------

void ManagerConnection::send(const GameNetworkMessage & message)
{
	Archive::ByteStream b;
	message.pack(b);
	Connection::send(b, true);
}

//-----------------------------------------------------------------------

int ManagerConnection::getConnectionCount()
{
	return s_managerConnectionCount;
}

//-----------------------------------------------------------------------
