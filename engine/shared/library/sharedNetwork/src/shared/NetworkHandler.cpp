// NetworkHandler.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

//-----------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include "sharedNetwork/NetworkHandler.h"

#include "Archive/Archive.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/PixCounter.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Watcher.h"
#include "sharedLog/Log.h"
#include "sharedLog/NetLogConnection.h"
#include "sharedNetwork/Address.h"
#include "sharedNetwork/ConfigSharedNetwork.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetwork/ManagerHandler.h"
#include "sharedNetwork/Service.h"
#include "sharedNetwork/UdpLibraryMT.h"
#include "sharedSynchronization/Guard.h"

#include "TcpClient.h"

#include <algorithm>
#include <deque>
#include <map>
#include <set>
#include <vector>

//-----------------------------------------------------------------------

namespace NetworkHandlerNamespace
{
	bool   ms_logThrottle = false;

#if PRODUCTION == 0
	PixCounter::ResetInteger s_bytesSentThisFrameForPix;
	PixCounter::ResetInteger s_bytesReceivedThisFrameForPix;
#endif
}

using namespace NetworkHandlerNamespace;

//-----------------------------------------------------------------------

struct IncomingData
{
	Watcher<Connection> connection;
	Archive::ByteStream byteStream;
};

struct Services
{
	Services();
	~Services();
	std::set<UdpManagerMT *>      udpServices;
	std::vector<UdpManagerMT *>   deferredDestroyUdpManagers;
	std::deque<IncomingData>      inputQueue;
	std::vector<Service *>        services;
};

static Services services;
int s_bytesSentThisFrame = 0;
int s_bytesReceivedThisFrame = 0;
int s_packetsSentThisFrame = 0;
int s_packetsReceivedThisFrame = 0;
std::map<std::string, std::pair<int, int> > s_messageCount;
bool s_updating = false;
bool s_logBackloggedPackets = true;
bool s_removing = false;
static std::vector<Connection *>     deferredDisconnects;
uint64 s_currentFrame = 0;

//-----------------------------------------------------------------------

Services::Services()
{
	UdpLibraryMT::getMutex();
}

//-----------------------------------------------------------------------

Services::~Services()
{
	std::set<UdpManagerMT *>::const_iterator i;
	for(i = udpServices.begin(); i != udpServices.end(); ++i)
	{
		(*i)->Release();
	}
	udpServices.clear();
}

//-----------------------------------------------------------------------

NetworkHandler::NetworkHandler() :
m_bindPort(0),
m_bindAddress(""),
m_udpManager(0),
m_managerHandler(0)
{
	m_managerHandler = new ManagerHandler(this);
}

//-----------------------------------------------------------------------

NetworkHandler::~NetworkHandler()
{
	if (m_udpManager)
		m_udpManager->ClearHandler();

	m_managerHandler->setOwner(0);
	m_managerHandler->Release();
	m_managerHandler = 0;

	if (m_udpManager)
	{
		if (s_updating)
			services.deferredDestroyUdpManagers.push_back(m_udpManager);
		else
		{
			std::set<UdpManagerMT *>::iterator f = services.udpServices.find(m_udpManager);
			if (f != services.udpServices.end())
				services.udpServices.erase(f);
			m_udpManager->Release();
			m_udpManager = 0;
		}
	}
}

//-----------------------------------------------------------------------

void NetworkHandler::dispatch()
{
	LogManager::update();

	UdpLibraryMT::mainThreadUpdate();

	if (!services.inputQueue.empty())
	{
		size_t const startQueueSize = services.inputQueue.size();
		unsigned long const startTime = Clock::timeMs();
		bool const throttle =  ConfigSharedNetwork::getNetworkHandlerDispatchThrottle();
		unsigned int const processTimeMilliseconds = static_cast<unsigned int>(ConfigSharedNetwork::getNetworkHandlerDispatchThrottleTimeMilliseconds());
		unsigned int const queueSize = static_cast<unsigned int>(ConfigSharedNetwork::getNetworkHandlerDispatchQueueSize());

		while (!services.inputQueue.empty() && (services.inputQueue.size() > queueSize || (!throttle || (throttle && ((Clock::timeMs() - startTime) < processTimeMilliseconds)))))
		{
			std::deque<IncomingData>::iterator i = services.inputQueue.begin();

			Connection * c = (*i).connection;
			if (c)
			{
				try
				{
					reportBytesReceived((*i).byteStream.getSize());
					int sendSize = (*i).byteStream.getSize();
					static const int packetSizeWarnThreshold = ConfigSharedNetwork::getPacketSizeWarnThreshold();
					if(packetSizeWarnThreshold > 0)
					{
						WARNING(sendSize > packetSizeWarnThreshold, ("large packet received (%d bytes) exceeds warning threshold %d defined as SharedNetwork/packetSizeWarnThreshold", sendSize, packetSizeWarnThreshold));
					}

					c->receive((*i).byteStream);
				}
				catch(const Archive::ReadException & readException)
				{
					WARNING(true, ("Unhandled Archive read error (%s) on connection. Continuing to throw from NetwokrHandler::Dispatch", readException.what()));
					throw(readException);
				}
			}

			services.inputQueue.pop_front();
		}

		REPORT_LOG(ms_logThrottle && throttle && !services.inputQueue.empty(), ("NetworkHandler::dispatch: services.inputQueue has %d/%d messages remaining\n", services.inputQueue.size(), startQueueSize));
	}
}

//-----------------------------------------------------------------------

void NetworkHandler::flushAndConfirmAll()
{
	static bool enabled = ConfigSharedNetwork::getEnableFlushAndConfirmAllData();

	if(enabled)
	{
		std::vector<Service *>::iterator i;
		for(i = services.services.begin(); i != services.services.end(); ++i)
		{
			(*i)->flushAndConfirmAllData();
		}
	}
	else
	{
		update();
	}
}

//-----------------------------------------------------------------------

const std::string & NetworkHandler::getBindAddress() const
{
	return m_bindAddress;
}

//-----------------------------------------------------------------------

const unsigned short NetworkHandler::getBindPort() const
{
	return m_bindPort;
}

//-----------------------------------------------------------------------

void NetworkHandler::newManager(UdpManagerMT * u)
{
	services.udpServices.insert(u);
}

//-----------------------------------------------------------------------

void NetworkHandler::newService(Service * s)
{
	services.services.push_back(s);
}

//-----------------------------------------------------------------------

void NetworkHandler::removeService(Service * s)
{
	std::vector<Service *>::iterator f = std::find(services.services.begin(), services.services.end(), s);
	if(f != services.services.end())
		services.services.erase(f);
}

//-----------------------------------------------------------------------

void NetworkHandler::onConnect(void * callback, UdpConnectionMT * connection)
{
	if(callback)
	{
		NetworkHandler * h = reinterpret_cast<NetworkHandler *>(callback);
		if(connection)
		{
			h->onConnectionOpened(connection);
		}
	}
}

//-----------------------------------------------------------------------

void NetworkHandler::onReceive(Connection * c, const unsigned char * d, int s)
{
	if(c)
	{
		services.inputQueue.push_back(IncomingData());
		services.inputQueue.back().connection = c;
		services.inputQueue.back().byteStream.put(d, s);

		static const bool logAllNetworkTraffic = ConfigSharedNetwork::getLogAllNetworkTraffic();

		if(logAllNetworkTraffic)
		{
			if(! dynamic_cast<const NetLogConnection *>(c))
			{
				std::string logChan = "Network:Recv:" + c->getRemoteAddress() + ":";
				char portBuf[7] = {"\0"};
				snprintf(portBuf, sizeof(portBuf), "%d", c->getRemotePort());
				logChan += portBuf;
				std::string output;
				const unsigned char * uc = d;
				while(uc < d + s)
				{
					if(isalpha(*uc))
					{
						char s = *uc;
						output += s;
					}
					else
					{
						char numBuf[128] = {"\0"};
						snprintf(numBuf, sizeof(numBuf), "[%d]", *uc);
						output += numBuf;
					}
					++uc;
				}
				LOG(logChan, ("%s", output.c_str()));
			}
		}
	}
}

//-----------------------------------------------------------------------

void NetworkHandler::onReceive(void *, UdpConnectionMT * u, const unsigned char * d, int s)
{
	Connection * c = reinterpret_cast<Connection *>(u->GetPassThroughData());
	onReceive(c, d, s);
}

//-----------------------------------------------------------------------

void NetworkHandler::onTerminate(Connection * c)
{
	if (c)
	{
		c->setDisconnectReason("NetworkHandler::onTerminate called");
		c->disconnect();
	}
}

//-----------------------------------------------------------------------

void NetworkHandler::onTerminate(void * m, UdpConnectionMT * u)
{
	if(m)
	{
		if(u)
		{
			u->AddRef();
			Connection * c = reinterpret_cast<Connection *>(u->GetPassThroughData());
			onTerminate(c);
			u->SetPassThroughData(0);
			u->Release();
		}
	}
}

//-----------------------------------------------------------------------

void NetworkHandler::setBindAddress(const std::string & address)
{
	m_bindAddress = address;
}

//-----------------------------------------------------------------------

void NetworkHandler::setBindPort(const unsigned short p)
{
	m_bindPort = p;
}

//-----------------------------------------------------------------------

void NetworkHandler::install()
{
	Connection::install();
	UdpLibraryMT::install();

	DebugFlags::registerFlag(ms_logThrottle, "SharedNetwork", "networkHandlerDispatchLogThrottle");
	DebugFlags::registerFlag(s_logBackloggedPackets, "SharedNetwork", "logBackloggedPackets");

#if PRODUCTION == 0
	s_bytesSentThisFrameForPix.bindToCounter("NetworkBytesSent");
	s_bytesReceivedThisFrameForPix.bindToCounter("NetworkBytesReceived");
#endif
}

//-----------------------------------------------------------------------

void NetworkHandler::remove()
{
	s_removing = true;
	while (!services.services.empty())
	{
		// Service destructor calls removeService, which should the service at
		// begin() from the list
		delete *services.services.begin();
	}

	UdpLibraryMT::remove();
	Connection::remove();

	DebugFlags::unregisterFlag(ms_logThrottle);
	DebugFlags::unregisterFlag(s_logBackloggedPackets);
}

//-----------------------------------------------------------------------

void NetworkHandler::update()
{
	if (!UdpLibraryMT::getUseNetworkThread())
	{
		std::set<UdpManagerMT *>::const_iterator i;
		std::set<UdpManagerMT *>::const_iterator begin = services.udpServices.begin();
		std::set<UdpManagerMT *>::const_iterator end = services.udpServices.end();

		s_updating = true;
		for(i = begin; i != end; ++i)
			(*i)->GiveTime();
		std::vector<Service *>::const_iterator is;
		for(is = services.services.begin(); is != services.services.end(); ++is)
		{
			(*is)->updateTcp();
		}

		Connection::update();
		TcpClient::flushPendingWrites();

		s_updating = false;

		std::vector<UdpManagerMT *>::const_iterator im;
		for(im = services.deferredDestroyUdpManagers.begin(); im != services.deferredDestroyUdpManagers.end(); ++im)
		{
			UdpManagerMT * u = (*im);
			std::set<UdpManagerMT *>::iterator f = services.udpServices.find(u);
			if(f != services.udpServices.end())
				services.udpServices.erase(f);
			if(u)
				u->Release();
		}
		services.deferredDestroyUdpManagers.clear();
	}

	Guard lock(UdpLibraryMT::getMutex());
	while (!deferredDisconnects.empty())
	{
		Connection *c = *deferredDisconnects.begin();
		deferredDisconnects.erase(deferredDisconnects.begin());
		c->onConnectionClosed();
		delete c;
	}
}

//-----------------------------------------------------------------------

const int NetworkHandler::getBytesReceivedThisFrame()
{
	return s_bytesReceivedThisFrame;
}

//-----------------------------------------------------------------------

const int NetworkHandler::getBytesSentThisFrame()
{
	return s_bytesSentThisFrame;
}

//-----------------------------------------------------------------------

void NetworkHandler::reportBytesReceived(const int bytes)
{
	s_bytesReceivedThisFrame+=bytes;
	s_packetsReceivedThisFrame++;
#if PRODUCTION == 0
	s_bytesReceivedThisFrameForPix += bytes;
#endif
}

//-----------------------------------------------------------------------

void NetworkHandler::reportBytesSent(const int bytes)
{
	s_bytesSentThisFrame+=bytes;
	s_packetsSentThisFrame++;
#if PRODUCTION == 0
	s_bytesSentThisFrameForPix += bytes;
#endif
}

//-----------------------------------------------------------------------

const int NetworkHandler::getPacketsReceivedThisFrame()
{
	return s_packetsReceivedThisFrame;
}

//-----------------------------------------------------------------------

const int NetworkHandler::getPacketsSentThisFrame()
{
	return s_packetsSentThisFrame;
}

//-----------------------------------------------------------------------

void NetworkHandler::reportMessage(const std::string & messageName, const int size)
{
	static const bool reportMessages = ConfigSharedNetwork::getReportMessages();
	if(reportMessages)
	{
		std::map<std::string, std::pair<int, int> >::iterator f = s_messageCount.find(messageName);
		if(f != s_messageCount.end())
		{
			(*f).second.first++;
			(*f).second.second+=size;
		}
		else
		{
			std::pair<int, int> newEntry = std::make_pair(1, size);
			s_messageCount[messageName] = newEntry;
		}
	}
}

//-----------------------------------------------------------------------

void NetworkHandler::clearBytesThisFrame()
{
	s_currentFrame++;
	Guard lock(UdpLibraryMT::getMutex());

	bool logMessageCounts = false;
	static const int backlogThreshold = ConfigSharedNetwork::getLogBackloggedPacketThreshold();
	if(s_logBackloggedPackets && backlogThreshold > 0)
	{
		std::vector<Service *>::iterator i;
		for(i = services.services.begin(); i != services.services.end(); ++i)
		{
			(*i)->logBackloggedPackets();
		}
	}

	static const int packetCountWarnThreshold = ConfigSharedNetwork::getPacketCountWarnThreshold();
	if(packetCountWarnThreshold > 0)
	{
		if(s_packetsSentThisFrame > packetCountWarnThreshold)
		{
			LOG("Network:ExcessivePackets:Send", ("packet send count (%d packets) for this frame exceeds warning threshold %d defined as SharedNetwork/packetCountWarnThreshold", NetworkHandler::getPacketsSentThisFrame(), packetCountWarnThreshold));
			logMessageCounts = true;
		}
		if(s_packetsReceivedThisFrame > packetCountWarnThreshold)
		{
			LOG("Network:ExcessivePackets:Recv", ("packet receive count (%d packets) for this frame exceeds warning threshold %d defined as SharedNetwork/packetCountWarnThreshold", s_packetsReceivedThisFrame, packetCountWarnThreshold));
			logMessageCounts = true;
		}
	}

	static const int byteCountWarnThreshold = ConfigSharedNetwork::getByteCountWarnThreshold();
	if(byteCountWarnThreshold > 0)
	{
		if(s_bytesSentThisFrame > byteCountWarnThreshold)
		{
			LOG("Network:ExcessiveBytes:Send", ("byte send count (%d bytes) for this frame exceeds warning threshold %d defined as SharedNetwork/byteCountWarnThreshold", s_bytesSentThisFrame, byteCountWarnThreshold));
			logMessageCounts = true;
		}
		if(s_bytesReceivedThisFrame > byteCountWarnThreshold)
		{
			LOG("Network:ExcessiveBytes:Recv", ("byte receive count (%d bytes) for this frame exceeds warning threshold %d defined as SharedNetwork/byteCountWarnThreshold", s_bytesReceivedThisFrame, byteCountWarnThreshold));
			logMessageCounts = true;
		}
	}


	if(logMessageCounts)
	{
		static const bool reportMessages = ConfigSharedNetwork::getReportMessages();
		if(reportMessages)
		{
			std::map<std::string, std::pair<int, int> >::iterator i;
			static char msgBuf[256];
			for(i = s_messageCount.begin(); i != s_messageCount.end(); ++i)
			{
				snprintf(msgBuf, sizeof(msgBuf), "[%s] - %d messages, %d total bytes", (*i).first.c_str(), (*i).second.first, (*i).second.second);
				LOG("Network:MessageCount", ("%s", msgBuf));
			}
		}
	}

	s_messageCount.clear();
	s_bytesReceivedThisFrame = 0;
	s_bytesSentThisFrame = 0;
	s_packetsReceivedThisFrame = 0;
	s_packetsSentThisFrame = 0;

}

//-----------------------------------------------------------------------

LogicalPacket const *NetworkHandler::createPacket(unsigned char const *data, int size)
{
	if (m_udpManager)
		return m_udpManager->CreatePacket(data, size);
	return UdpMiscMT::CreateQuickLogicalPacket(data, size);
}

//-----------------------------------------------------------------------

void NetworkHandler::releasePacket(LogicalPacket const *p)
{
	p->Release();
}

//-----------------------------------------------------------------------

bool NetworkHandler::removing()
{
	return s_removing;
}

//-----------------------------------------------------------------------

bool NetworkHandler::isPortReserved(unsigned short p)
{
	return(ConfigSharedNetwork::getIsPortReserved(p));
}

//-----------------------------------------------------------------------

void NetworkHandler::disconnectConnection(Connection * c)
{
	if(! s_removing)
	{
		std::vector<Connection *>::iterator f = std::find(deferredDisconnects.begin(), deferredDisconnects.end(), c);
		if(f == deferredDisconnects.end())
		{
			deferredDisconnects.push_back(c);
		}
	}
}

//-----------------------------------------------------------------------

uint64 NetworkHandler::getCurrentFrame()
{
	return s_currentFrame;
}

//-----------------------------------------------------------------------

int NetworkHandler::getRecvTotalCompressedByteCount()
{
	return ManagerHandler::getRecvTotalCompressedByteCount();
}

//-----------------------------------------------------------------------

int NetworkHandler::getRecvTotalUncompressedByteCount()
{
	return ManagerHandler::getRecvTotalUncompressedByteCount();
}

//-----------------------------------------------------------------------

int NetworkHandler::getSendTotalCompressedByteCount()
{
	return ManagerHandler::getSendTotalCompressedByteCount();
}

//-----------------------------------------------------------------------

int NetworkHandler::getSendTotalUncompressedByteCount()
{
	return ManagerHandler::getSendTotalUncompressedByteCount();
}

//-----------------------------------------------------------------------

float NetworkHandler::getTotalCompressionRatio()
{
	return ManagerHandler::getTotalCompressionRatio();
}

//-----------------------------------------------------------------------

bool NetworkHandler::isAddressLocal(const std::string & address)
{
	Address addr(address, 0);

	bool result = false;
	const std::vector<std::pair<std::string, std::string> > & localAddresses = NetworkHandler::getInterfaceAddresses();
	std::vector<std::pair<std::string, std::string> >::const_iterator i;
	for(i = localAddresses.begin(); i != localAddresses.end(); ++i)
	{
		if(i->second == addr.getHostAddress())
		{
			result = true;
			break;
		}
	}
	return result;
}

//-----------------------------------------------------------------------

