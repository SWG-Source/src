// Service.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include "ConfigSharedNetwork.h"
#include "sharedFoundation/Clock.h"
#include "sharedLog/Log.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetwork/ManagerHandler.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetwork/UdpLibraryMT.h"
#include "TcpServer.h"

//-----------------------------------------------------------------------

ConnectionAllocatorBase::~ConnectionAllocatorBase() 
{
}

//-----------------------------------------------------------------------

//Service::Service(const ConnectionAllocatorBase & c, const unsigned short port, const int m, const int keepAliveDelay, const std::string & interfaceAddress, const bool compress) :
Service::Service(const ConnectionAllocatorBase & c, const NetworkSetupData & setup) :
connectionAllocator(c.clone()),
m_callback(new MessageDispatch::Callback),
m_tcpServer(0)
{
	std::string realAddress;
	if(setup.bindInterface.length() > 0)
	{
		// is the address an interface? If so, change it to an IP address
		const std::vector<std::pair<std::string, std::string> > & ifaces = NetworkHandler::getInterfaceAddresses();
		std::vector<std::pair<std::string, std::string> >::const_iterator i;
		for(i = ifaces.begin(); i != ifaces.end(); ++i)
		{
			if((*i).first == setup.bindInterface)
			{
				realAddress = (*i).second;
				break;
			}
			if((*i).second == setup.bindInterface)
			{
				realAddress = (*i).second;
				break;
			}
		}
		
		if(realAddress.length() == 0)
		{
			// the requested bind interfae does not exactly match one of the
			// interfaces on this host. See if there is a partial match
			// with the supplied address and an interface address
			for(i = ifaces.begin(); i != ifaces.end(); ++i)
			{
				if(setup.bindInterface.length() < (*i).second.length())
				{
					if((*i).second.substr(0, setup.bindInterface.length()) == setup.bindInterface)
					{
						realAddress = (*i).second;
						break;
					}
				}
			}
		}
	}
	if(! setup.useTcp)
	{
		UdpManagerMT::Params p;
		p.handler = m_managerHandler ? m_managerHandler->getManagerHandler() : 0;
		p.crcBytes = setup.crcBytes;
		p.hashTableSize = setup.hashTableSize;
		p.incomingBufferSize = setup.incomingBufferSize;
		p.outgoingBufferSize = setup.outgoingBufferSize;
		p.clockSyncDelay = 0;// Servers don't sync their clocks to clients
		p.keepAliveDelay = setup.keepAliveDelay;
		p.maxConnections = setup.maxConnections;
		p.maxRawPacketSize = setup.maxRawPacketSize;
		p.maxDataHoldTime = setup.maxDataHoldTime;
		p.packetHistoryMax = setup.packetHistoryMax;
		p.port = setup.port;
		p.pooledPacketMax = setup.pooledPacketMax;
		p.pooledPacketSize = setup.pooledPacketSize;
		p.pooledPacketInitial = setup.pooledPacketInitial;
		p.oldestUnacknowledgedTimeout = setup.oldestUnacknowledgedTimeout;
		p.processIcmpErrors = setup.processIcmpErrors;
		p.noDataTimeout = setup.noDataTimeout;
		p.reliableOverflowBytes = setup.reliableOverflowBytes;
		p.icmpErrorRetryPeriod = setup.icmpErrorRetryPeriod;
		p.maxDataHoldSize = setup.maxDataHoldSize;
		p.allowPortRemapping = setup.allowPortRemapping;
       		p.maxConnectionsPerIP = setup.maxConnectionsPerIP;
			
        if (realAddress.length() > 0)
        {
            strncpy(p.bindIpAddress, realAddress.c_str(), sizeof(p.bindIpAddress));
        }

		p.reliable[0].maxInstandingPackets = setup.maxInstandingPackets;
		p.reliable[0].maxOutstandingBytes = setup.maxOutstandingBytes;
		p.reliable[0].maxOutstandingPackets = setup.maxOutstandingPackets;
		p.reliable[0].processOnSend = setup.processOnSend;
		p.reliable[0].congestionWindowMinimum = setup.congestionWindowMinimum;
		p.reliable[0].resendDelayAdjust = setup.resendDelayAdjust;
		p.reliable[0].resendDelayPercent = setup.resendDelayPercent;
		
		for (int j = 1; j < UdpManager::cReliableChannelCount; j++)
			p.reliable[j] = p.reliable[0];
		
		p.userSuppliedEncryptExpansionBytes = 1;
		
		if(setup.compress)
		{
			p.encryptMethod[0] = UdpManager::cEncryptMethodUserSupplied;
			p.encryptMethod[1] = UdpManager::cEncryptMethodXor;
		}
		
		if(setup.port == 0)
		{
			m_udpManager = 0;
			UdpManagerMT * m = 0;
			do
			{
				if(! m)
				{
					m = new UdpManagerMT(&p);
				}
				
				m_udpManager = m;
				setBindPort(static_cast<unsigned short>(m_udpManager->GetLocalPort()));
				if(isPortReserved(getBindPort()))
				{
					m = new UdpManagerMT(&p);
					m_udpManager->Release();
					m_udpManager = 0;
				}
			} while(! m_udpManager);
		}
		else
		{
			m_udpManager = new UdpManagerMT(&p);
			setBindPort(static_cast<unsigned short>(m_udpManager->GetLocalPort()));
		}
		
		newManager(m_udpManager);
	}
	else
	{
		m_tcpServer = new TcpServer(this, realAddress, setup.port);
		setBindPort(m_tcpServer->getBindPort());
	}
	
	if(realAddress.length() > 0)
	{
		setBindAddress(realAddress);
	}
	else
	{
		setBindAddress(NetworkHandler::getHostName());
	}
	
	m_maxConnections = setup.maxConnections;
	newService(this);
	
	m_callback->connect(*this, &Service::onConnectionClosed);
}

//-----------------------------------------------------------------------

Service::~Service()
{
	removeService(this);

	for (std::set<Connection *>::const_iterator i = connections.begin(); i != connections.end(); ++i)
	{
		Connection *c = *i;
		c->setDisconnectReason("Service destroyed");
		c->setService(0);
		c->disconnect();
	}
	
	delete connectionAllocator;
	delete m_callback;

	connections.clear();
	connectionAllocator = nullptr;
	m_callback = nullptr;
	delete m_tcpServer;
}

//-----------------------------------------------------------------------
	
void Service::onConnectionClosed(Connection * c)
{
	if (c)
	{
		std::set<Connection *>::iterator f = connections.find(c);
		if (f != connections.end())
		{
			if (m_tcpServer)
			{
				m_tcpServer->onConnectionClosed(c->getTcpClient());
			}
			connections.erase(f);
		}
		c->setService(0);
	}
}

//-----------------------------------------------------------------------

void Service::onConnectionOpened(Connection * c)
{
	if (c)
	{
	        if (connections.size() < static_cast<size_t>(m_maxConnections))
        	{
			c->setService(this);
			c->onConnectionOpened();
			connections.insert(c);
		}
		else
	     	{
			WARNING(true, ("Service has reached it's maximum connection count (%d).", m_maxConnections));
        	}
	}
}

//-----------------------------------------------------------------------

void Service::onConnectionOpened(UdpConnectionMT * u)
{
	DEBUG_REPORT_LOG(true, ("connection opened for service on port %d\n", getBindPort()));
	Connection * c = connectionAllocator->allocateConnection(u);
	onConnectionOpened(c);
}

//-----------------------------------------------------------------------

void Service::onConnectionOpened(TcpClient * c)
{
	Connection * conn = connectionAllocator->allocateConnection(0, c);
	onConnectionOpened(conn);
}

//-----------------------------------------------------------------------

void Service::removeConnection(Connection * c)
{
	std::set<Connection *>::iterator f = connections.find(c);
	if (f != connections.end())
		connections.erase(f);
	c->setService(0);
}

//-----------------------------------------------------------------------

int Service::flushAndConfirmAllData()
{
	std::set<Connection *>::const_iterator i;

	if(m_udpManager)
	{
		int totalBytesPending = 0;
		unsigned long startTime = Clock::timeMs();
		bool needReport = false;
		unsigned long stallReportDelay = ConfigSharedNetwork::getStallReportDelay();

		do
		{
			if(stallReportDelay > 0)
			{
				if(Clock::timeMs() - stallReportDelay > startTime)
				{
					needReport = true;
				}
			}
			
			m_udpManager->GiveTime();
			totalBytesPending = 0;
			for(i = connections.begin(); i != connections.end(); ++i)
			{
				if(needReport)
				{
					if((*i)->udpConnection->TotalPendingBytes() > 0)
					{
						(*i)->onConnectionStalled(Clock::timeMs() - startTime);
					}
				}
				totalBytesPending += (*i)->udpConnection->TotalPendingBytes();
			}
			
			if(needReport)
			{
				stallReportDelay += ConfigSharedNetwork::getStallReportDelay();
				needReport = false;
			}
		} while(totalBytesPending > 0);
	}
	return 0;
}

//-----------------------------------------------------------------------

void Service::logBackloggedPackets()
{
	static const int threshold = ConfigSharedNetwork::getLogBackloggedPacketThreshold();
	std::set<Connection *>::iterator i;
	for(i = connections.begin(); i != connections.end(); ++i)
	{
		int pendingBytes = (*i)->getPendingBytes();
		if(pendingBytes > threshold)
		{
			LOG("Network", ("Backlog:%s:%s [%d] bytes still pending, congestion window size=%d. ack average ping=%d, oldest unacknowledged=%d, lastSend=%d, lastReceive=%d", (*i)->getRemoteAddress().c_str(), (*i)->getConnectionDescription().c_str(), pendingBytes, (*i)->getCongestionWindowSize(), (*i)->getAckAveragePing(), (*i)->getOldestUnacknowledgedAge(), (*i)->getLastSend(), (*i)->getLastReceive()));
		}
	}
}

//-----------------------------------------------------------------------

void Service::updateTcp()
{
	if(m_tcpServer)
	{
		m_tcpServer->update();
	}
}

// ----------------------------------------------------------------------

void Service::broadcast(Archive::ByteStream const &bs, bool reliable)
{
	for (std::set<Connection *>::const_iterator i = connections.begin(); i != connections.end(); ++i)
		(*i)->send(bs, reliable);
}

// ======================================================================

