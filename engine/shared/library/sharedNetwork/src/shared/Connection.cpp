// Connection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

//-----------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"
#include "sharedNetwork/Connection.h"

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedLog/Log.h"
#include "sharedLog/NetLogConnection.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetwork/ConfigSharedNetwork.h"
#include "sharedNetwork/ConnectionHandler.h"
#include "sharedNetwork/ManagerHandler.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetwork/UdpLibraryMT.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "../../../../../../external/3rd/library/udplibrary/UdpLibrary.hpp"

#include <algorithm>

//-----------------------------------------------------------------------

namespace ConnectionServerNamespace
{
	std::vector<Connection *> s_connections;
	std::vector<Connection *> s_clientConnections;
}

using namespace ConnectionServerNamespace;

//-----------------------------------------------------------------------

class DeferredSend
{
public:
	DeferredSend();
	virtual ~DeferredSend() = 0;
	virtual void flush(const Connection * to) = 0;
	virtual const int  getSize() const = 0;
};

//-----------------------------------------------------------------------

class DeferredSendArchive : public DeferredSend
{
public:
	DeferredSendArchive(const Archive::ByteStream & source);
	~DeferredSendArchive();


	void flush(const Connection * to);
	virtual const int  getSize() const;

private:
	Archive::ByteStream data;
};

//-----------------------------------------------------------------------

class DeferredSendLogicalPacket : public DeferredSend
{
public:
	DeferredSendLogicalPacket(const LogicalPacket * data);
	~DeferredSendLogicalPacket();

	void flush(const Connection * to);
	virtual const int  getSize() const;

private:
	const LogicalPacket * data;
};

//-----------------------------------------------------------------------

DeferredSend::DeferredSend()
{
}

//-----------------------------------------------------------------------

DeferredSend::~DeferredSend()
{
}

//-----------------------------------------------------------------------

DeferredSendArchive::DeferredSendArchive(const Archive::ByteStream & source) :
data(source)
{
}

//-----------------------------------------------------------------------

DeferredSendArchive::~DeferredSendArchive()
{
}

//-----------------------------------------------------------------------

void DeferredSendArchive::flush(const Connection * to)
{
  if (to->getUdpConnection())
    {
      UdpChannel c = cUdpChannelReliable1;
      const_cast<Connection *>(to)->getUdpConnection()->Send(c, data.getBuffer(), data.getSize());
    }
}

//-----------------------------------------------------------------------

const int DeferredSendArchive::getSize() const
{
	return data.getSize();
}

//-----------------------------------------------------------------------

DeferredSendLogicalPacket::DeferredSendLogicalPacket(const LogicalPacket * p) :
data(p)
{
	data->AddRef();
}

//-----------------------------------------------------------------------

DeferredSendLogicalPacket::~DeferredSendLogicalPacket()
{
	data->Release();
}

//-----------------------------------------------------------------------

void DeferredSendLogicalPacket::flush(const Connection * to)
{
  if (to->getUdpConnection())
    {
      UdpChannel c = cUdpChannelReliable1;
      const_cast<Connection *>(to)->getUdpConnection()->Send(c, data);
    }
}

//-----------------------------------------------------------------------

const int DeferredSendLogicalPacket::getSize() const
{
	return data->GetDataLen();
}

//-----------------------------------------------------------------------


//-----------------------------------------------------------------------

Connection::Connection(const std::string & a, const unsigned short p, const NetworkSetupData & setup) :
udpConnection(0),
m_pendingPackets(),
m_currentFrame(0),
m_rawTCP( false ),
m_remoteAddress(a),
m_remotePort(p),
m_deferredDataSize(0),
m_overflowLimit(ConfigSharedNetwork::getOverflowLimit()),
m_bytesReceived(0),
m_bytesSent(0),
m_sendPeakBytesPerSecond(0),
m_sendAverageBytesPerSecond(0),
m_connectionStartTime(Clock::getFrameStartTimeMs()),
m_recvPeakBytesPerSecond(0),
m_recvAverageBytesPerSecond(0),
m_lastRecvTime(Clock::getFrameStartTimeMs()),
m_lastSendTime(Clock::getFrameStartTimeMs()),
m_lastRecvReportTime(Clock::getFrameStartTimeMs()),
m_lastSendReportTime(Clock::getFrameStartTimeMs()),
m_recvBytesReportInterval(0),
m_sendBytesReportInterval(0),
m_service(0),
m_connectionHandler(0),
m_tcpClient(0),
m_tcpHeader(0),
m_tcpInput(0),
m_disconnecting(false),
m_disconnectReason()
{
	m_connectionHandler = new ConnectionHandler(this);
//	Network::connect(this);
	if (m_remoteAddress.length())
	{
		if (m_remotePort)
		{
			if (!setup.useTcp)
			{
				UdpManagerMT::Params p;

				p.handler = m_managerHandler ? m_managerHandler->getManagerHandler() : 0;
				p.crcBytes = setup.crcBytes;
				p.hashTableSize = 1;
				p.incomingBufferSize = setup.incomingBufferSize;
				p.outgoingBufferSize = setup.outgoingBufferSize;
				p.clockSyncDelay = setup.clockSyncDelay;
				p.keepAliveDelay = setup.keepAliveDelay;
				p.maxConnections = 1;
				p.maxConnectionsPerIP = setup.maxConnectionsPerIP;
				p.maxRawPacketSize = setup.maxRawPacketSize;
				p.maxDataHoldSize = setup.maxDataHoldSize;
				p.reliable[0].maxInstandingPackets = setup.maxInstandingPackets;
				p.reliable[0].maxOutstandingBytes = setup.maxOutstandingBytes;
				p.reliable[0].maxOutstandingPackets = setup.maxOutstandingPackets;
				p.reliable[0].processOnSend = setup.processOnSend;
				p.reliable[0].fragmentSize = setup.fragmentSize;
				p.reliable[0].congestionWindowMinimum = setup.congestionWindowMinimum;
				p.reliable[0].resendDelayAdjust=setup.resendDelayAdjust;
				p.userSuppliedEncryptExpansionBytes = 1;

				for (int j = 1; j < UdpManager::cReliableChannelCount; j++)
					p.reliable[j] = p.reliable[0];
				p.pooledPacketMax = setup.pooledPacketMax;
				p.pooledPacketSize = setup.pooledPacketSize;
				p.oldestUnacknowledgedTimeout = setup.oldestUnacknowledgedTimeout;
				p.processIcmpErrors = setup.processIcmpErrors;
				p.noDataTimeout = setup.noDataTimeout;
				p.reliableOverflowBytes = setup.reliableOverflowBytes;
				p.allowPortRemapping = setup.allowPortRemapping;

				m_udpManager = 0;
				do
				{
					UdpManagerMT *m = new UdpManagerMT(&p);
					m_udpManager = m;
					setBindPort(static_cast<unsigned short>(m_udpManager->GetLocalPort()));
					if (isPortReserved(getBindPort()))
					{
						m_udpManager->Release();
						m_udpManager = 0;
					}
				} while (!m_udpManager);

				udpConnection = m_udpManager->EstablishConnection(m_remoteAddress.c_str(), m_remotePort);
				if (udpConnection)
				{
					udpConnection->SetPassThroughData(this);
					udpConnection->SetHandler(m_connectionHandler);
					newManager(m_udpManager);
				}
			}
			else
			{
				do
				{
					TcpClient *t = new TcpClient(a, p);
					t->addRef();
					m_tcpClient = t;
					setBindPort(t->getBindPort());
					if (isPortReserved(getBindPort()) || (getBindPort() == p))
					{
						t->release();
						m_tcpClient = 0;
					}
				} while (!m_tcpClient);
				m_tcpClient->setConnection(this);
				s_clientConnections.push_back(this);
				m_tcpHeader = new Archive::ByteStream();
				m_tcpInput = new Archive::ByteStream();
			}
		}
	}

	s_connections.push_back(this);
}

//-----------------------------------------------------------------------

Connection::Connection(UdpConnectionMT * newConnection, TcpClient * tcpClient) :
NetworkHandler (),
udpConnection(newConnection),
m_rawTCP( false ),
m_remoteAddress(),
m_remotePort(0),
m_deferredMessages(),
m_deferredDataSize(0),
m_watchedByList(),
m_overflowLimit(ConfigSharedNetwork::getOverflowLimit()),
m_bytesReceived(0),
m_bytesSent(0),
m_sendPeakBytesPerSecond(0),
m_sendAverageBytesPerSecond(0),
m_connectionStartTime(Clock::getFrameStartTimeMs()),
m_recvPeakBytesPerSecond(0),
m_recvAverageBytesPerSecond(0),
m_lastRecvTime(Clock::getFrameStartTimeMs()),
m_lastSendTime(Clock::getFrameStartTimeMs()),
m_lastRecvReportTime(Clock::getFrameStartTimeMs()),
m_lastSendReportTime(Clock::getFrameStartTimeMs()),
m_recvBytesReportInterval(0),
m_sendBytesReportInterval(0),
m_service(0),
m_connectionHandler(0),
m_tcpClient(tcpClient),
m_tcpHeader(0),
m_tcpInput(0),
m_disconnecting(false),
m_disconnectReason()
{
	if (!m_tcpClient)
	{
		m_connectionHandler = new ConnectionHandler(this);
		udpConnection->SetHandler(m_connectionHandler);
		udpConnection->SetPassThroughData(this);

		udpConnection->AddRef();

		if (newConnection)
		{
			char addrBuf[1024] = {"\0"};
			newConnection->GetDestinationIp().GetAddress(addrBuf);
			m_remoteAddress = addrBuf;
			m_remotePort = static_cast<unsigned short>(newConnection->GetDestinationPort());
		}
	}
	else
	{
		m_tcpHeader = new Archive::ByteStream;
		m_tcpInput = new Archive::ByteStream;
		m_tcpClient->addRef();
		m_tcpClient->setConnection(this);
		m_remoteAddress = m_tcpClient->getRemoteAddress();
		m_remotePort = m_tcpClient->getRemotePort();
	}

	if (ConfigSharedNetwork::getLogConnectionConstructionDestruction())
	{
		LOG("Connection", ("constructor %s(%d) [%s:%d %s]",
		Os::getProgramName(),
		static_cast<int>(Os::getProcessId()),
		m_remoteAddress.c_str(),
		static_cast<int>(m_remotePort),
		m_description.c_str()));
	}

	s_connections.push_back(this);
}

//-----------------------------------------------------------------------

Connection::~Connection()
{
	if (ConfigSharedNetwork::getLogConnectionConstructionDestruction())
	{
		LOG("Connection", ("destructor %s(%d) [%s:%d %s]",
		Os::getProgramName(),
		static_cast<int>(Os::getProcessId()),
		m_remoteAddress.c_str(),
		static_cast<int>(m_remotePort),
		m_description.c_str()));
	}

	std::vector<Connection *>::iterator f = std::find(s_connections.begin(), s_connections.end(), this);
	if (f != s_connections.end())
		s_connections.erase(f);

	f = std::find(s_clientConnections.begin(), s_clientConnections.end(), this);
	if (f != s_clientConnections.end())
		s_clientConnections.erase(f);

	if (udpConnection)
	{
		UdpConnectionMT *oldUdpConnection = udpConnection;
		udpConnection = 0;
		oldUdpConnection->SetHandler(0);
		oldUdpConnection->SetPassThroughData(0);
		oldUdpConnection->Release();
	}

	if (m_service)
	{
		m_service->removeConnection(this);
		m_service = 0;
	}

	if (m_connectionHandler)
	{
		m_connectionHandler->setOwner(0);
		m_connectionHandler->Release();
		m_connectionHandler = 0;
	}

	std::for_each(m_deferredMessages.begin(), m_deferredMessages.end(), PointerDeleter());
	m_deferredMessages.clear();

	static MessageDispatch::Transceiver<Connection *> emitter;
	emitter.emitMessage(this);
	delete m_tcpHeader;
	delete m_tcpInput;
	if (m_tcpClient)
	{
		// set connection to 0 before calling release, since this connection
		// has been partially destructed and some vtable entries may no
		// longer be available
		m_tcpClient->setConnection(0);

		// release the TcpClient (which may destroy it)
		m_tcpClient->release();
	}
}

//-----------------------------------------------------------------------

void Connection::setDisconnectReason(char const *fmt, ...)
{
	if (m_disconnectReason.empty())
	{
		va_list ap;
		va_start(ap, fmt);
		char buf[512];
		_vsnprintf(buf, sizeof(buf), fmt, ap);
		buf[sizeof(buf)-1] = '\0';
		va_end(ap);
		m_disconnectReason = buf;
	}
}

//-----------------------------------------------------------------------

void Connection::setDisconnecting()
{
	m_disconnecting = true;
}

//-----------------------------------------------------------------------

void Connection::disconnect()
{
	if (m_disconnecting)
		return;

	setDisconnecting();
	setDisconnectReason("Connection::disconnect explicitly called");

	if (   (udpConnection && ConfigSharedNetwork::getReportUdpDisconnects())
	    || (!udpConnection && ConfigSharedNetwork::getReportTcpDisconnects()))
	{

		LOG(
			"Network",
			(
				"Disconnect: %s(%d) [%s:%d %s]: %s",
				Os::getProgramName(),
				static_cast<int>(Os::getProcessId()),
				m_remoteAddress.c_str(),
				static_cast<int>(m_remotePort),
				m_description.c_str(),
				m_disconnectReason.c_str()));
	}

	onConnectionClosed(this);

	if (udpConnection)
	{
		//-- Because multiple objects can keep a reference to the UdpConnection or the UdpConnectionMT,
		//   we need to set the Connection passthrough data for this connection to zero so we don't get
		//   a callback on an already-deleted Connection instance.  (this will be deleted very soon).
		udpConnection->SetHandler(0);
		udpConnection->SetPassThroughData(0);

		//-- Clear the udpConnection data member so we don't think we can refer to it anymore since we're about to release our reference to it
		UdpConnectionMT *const oldUdpConnection = udpConnection;
		udpConnection = 0;

		//-- Do the disconnect and release.
		oldUdpConnection->Disconnect();
		oldUdpConnection->Release();
	}

	//-- Nuke the Connection instance.
	NetworkHandler::disconnectConnection(this);
}

//-----------------------------------------------------------------------

void Connection::flush()
{
	if (udpConnection && udpConnection->GetStatus() == UdpConnection::cStatusConnected)
	{
		if (!m_deferredMessages.empty())
		{

			for (std::vector<DeferredSend *>::const_iterator i = m_deferredMessages.begin(); i != m_deferredMessages.end(); ++i)
			{
				DeferredSend * d = (*i);
				m_deferredDataSize -= d->getSize();
				d->flush(this);
				delete d;
			}
			m_deferredMessages.clear();
		}
	}
}

//-----------------------------------------------------------------------

int Connection::flushAndConfirmAllData()
{
	int result = 0;
	if (udpConnection)
		result = udpConnection->TotalPendingBytes();
	return result;
}

//-----------------------------------------------------------------------

const std::string & Connection::getRemoteAddress() const
{
	return m_remoteAddress;
}

//-----------------------------------------------------------------------

const unsigned short Connection::getRemotePort() const
{
	return m_remotePort;
}

//-----------------------------------------------------------------------

const UdpConnectionMT * Connection::getUdpConnection() const
{
  return udpConnection;
}

//-----------------------------------------------------------------------

UdpConnectionMT * Connection::getUdpConnection()
{
  return udpConnection;
}

//-----------------------------------------------------------------------

uint16 Connection::getServerSyncStampShort() const
{
	if (udpConnection)
		return udpConnection->ServerSyncStampShort();
	return 0;
}

//-----------------------------------------------------------------------

uint32 Connection::getServerSyncStampLong() const
{
	if (udpConnection)
		return udpConnection->ServerSyncStampLong();
	return 0;
}

//-----------------------------------------------------------------------

void Connection::install()
{
	TcpClient::install();
}

//-----------------------------------------------------------------------

void Connection::onConnectionOverflowing(unsigned int totalBytes)
{
	WARNING(true, ("Connection with %s is overflowing. There are %lu bytes pending! The overflow limit is set to %d", m_remoteAddress.c_str(), totalBytes, m_overflowLimit));
	setDisconnectReason("Connection::onConnectionOverflowing called");
	disconnect();
}

//-----------------------------------------------------------------------

void Connection::onConnectionClosed(Connection *)
{
        if (getService())
        {
                getService()->onConnectionClosed(this);
        }


	if (udpConnection)
	{
        	UdpConnection::DisconnectReason reason = udpConnection->GetDisconnectReason();
		setDisconnectReason("Connection::onConnectionClosed (udplibrary:%s)", UdpConnection::DisconnectReasonText(reason));
		FATAL(ConfigSharedNetwork::getFatalOnConnectionClosed(), ("Connection closed %s", UdpConnection::DisconnectReasonText(reason)));
	}
	else
	{
        	setDisconnectReason("Connection::onConnectionClosed called");
	}

	if (ConfigSharedNetwork::getLogConnectionOpenedClosed())
	{
		LOG("Connection", ("onConnectionClosed() %s(%d) [%s:%d %s]",
		Os::getProgramName(),
		static_cast<int>(Os::getProcessId()),
		m_remoteAddress.c_str(),
		static_cast<int>(m_remotePort),
		m_description.c_str()));
	}
}

//-----------------------------------------------------------------------

void Connection::onConnectionOpened(UdpConnectionMT * u)
{
	if (udpConnection && udpConnection != u)
	{
		UdpConnectionMT *oldUdpConnection = u;
		udpConnection = 0;
		oldUdpConnection->SetHandler(0);
		oldUdpConnection->SetPassThroughData(0);
		oldUdpConnection->Release();
	}

	udpConnection = u;
	if (udpConnection)
	{
		udpConnection->SetPassThroughData(this);
		udpConnection->SetHandler(m_connectionHandler);
		onConnectionOpened();
		flush();
	}

	if (ConfigSharedNetwork::getLogConnectionOpenedClosed())
	{
		LOG("Connection", ("onConnectionOpened() %s(%d) [%s:%d %s]",
		Os::getProgramName(),
		static_cast<int>(Os::getProcessId()),
		m_remoteAddress.c_str(),
		static_cast<int>(m_remotePort),
		m_description.c_str()));
}
}

//-----------------------------------------------------------------------

void Connection::checkOverflow(const unsigned int bytesPending)
{
	if (udpConnection && m_overflowLimit > 0)
	{
		int totalPendingBytes = udpConnection->TotalPendingBytes() + m_deferredDataSize + bytesPending;
		if (totalPendingBytes > m_overflowLimit)
		{
			onConnectionOverflowing(totalPendingBytes);
		}
	}
}

//-----------------------------------------------------------------------

void Connection::receive(const unsigned char * buffer, int length)
{
	if (length < 1)
		return;

	const unsigned char * readIter = buffer;
	const unsigned char * end = readIter + length;

	do
	{
		if( m_rawTCP )
		{
			m_tcpInput->put( buffer, length );
			NetworkHandler::onReceive(this, m_tcpInput->getBuffer(), m_tcpInput->getSize());
			m_tcpInput->clear();
			return;
		}
		// check current state of the header
		if (m_tcpHeader->getSize() < 4)
		{
			// need to pull bytes off the stream to complete header
			// information
			if (end - readIter >= 4)
			{
				int bytesNeeded = 4 - m_tcpHeader->getSize();
				m_tcpHeader->put(readIter, bytesNeeded);
				readIter += bytesNeeded;
			}
			else
			{
				// there isn't enough data in the stream
				// to complete the header
				m_tcpHeader->put(readIter, end - readIter);
				return;
			}
		}

		// m_tcpHeader is complete by this point
		int expectedBytes = 0;
		static Archive::ReadIterator ri;
		ri = m_tcpHeader->begin();
		Archive::get(ri, expectedBytes);
		expectedBytes = expectedBytes - m_tcpInput->getSize();
		if (expectedBytes == 0)
		{
			// clear header and packet buffers
			m_tcpInput->clear();
			m_tcpHeader->clear();
		}
		else if (end - readIter >= expectedBytes)
		{
			// the packet can be completed with what is currently
			// in the stream, complete it
			m_tcpInput->put(readIter, expectedBytes);
			readIter += expectedBytes;

			// queue for dispatch
			//receive(*m_tcpInput);
			NetworkHandler::onReceive(this, m_tcpInput->getBuffer(), m_tcpInput->getSize());

			// clear header and packet buffers
			m_tcpInput->clear();
			m_tcpHeader->clear();
		}
		else if (end - readIter > 0)
		{
			// only a fragment of the packet is available, build
			// what is there, then bail out waiting for more
			// input on the socket
			m_tcpInput->put(readIter, end - readIter);
			readIter += end - readIter; // should bail out of the loop now.
		}
	} while (readIter < end);
}

//-----------------------------------------------------------------------

void Connection::receive(const Archive::ByteStream & bs)
{
	static const int reportInterval = ConfigSharedNetwork::getReportStatisticsInterval();
	static const bool reportMessages = ConfigSharedNetwork::getReportMessages();

	if (!isNetLogConnection())
	{
		if (reportMessages)
			reportReceive(bs);

		if (reportInterval > 0)
		{
			m_bytesReceived += bs.getSize();
			m_recvBytesReportInterval += bs.getSize();
			// check statistics
			m_lastRecvTime = Clock::getFrameStartTimeMs();
			if (m_lastRecvTime > m_lastRecvReportTime + reportInterval)
			{
				unsigned long lifeTime = (m_lastRecvTime - m_connectionStartTime) / 1000;
				// calculate recv stats
				if (lifeTime > 0)
				{
					unsigned long reportTime = (m_lastRecvTime - m_lastRecvReportTime) / 1000;
					if (reportTime > 0)
					{
						int bytesPerSecond = m_recvBytesReportInterval / reportTime;

						if (bytesPerSecond > m_recvPeakBytesPerSecond)
							m_recvPeakBytesPerSecond = bytesPerSecond;

						m_recvAverageBytesPerSecond = m_bytesReceived / lifeTime;

						std::string logChan = "Network:ConnectionStatsRecv:" + getRemoteAddress() + ":";
						char portBuf[7] = {"\0"};
						snprintf(portBuf, sizeof(portBuf), "%d", getRemotePort());
						logChan += portBuf;
						LOG(logChan, ("Current(%d/sec), Average(%d/sec), Peak(%d/sec) Total Bytes(%d) Total Time(%lu)", bytesPerSecond, m_recvAverageBytesPerSecond, m_recvPeakBytesPerSecond, m_bytesReceived, lifeTime));
						m_lastRecvReportTime = m_lastRecvTime;
						m_recvBytesReportInterval = 0;

						if (m_managerHandler->getRecvCompressedByteCount() > 0)
							LOG(logChan, ("Compression Ratio: %.2f : 1.0 - Recv(%d / %d)", m_managerHandler->getCompressionRatio(), m_managerHandler->getRecvUncompressedByteCount(), m_managerHandler->getRecvCompressedByteCount()));
					}
				}
			}
		}
	}

	onReceive(bs);
}

//-----------------------------------------------------------------------

void Connection::reportReceive(const Archive::ByteStream &)
{
}

//-----------------------------------------------------------------------

void Connection::reportSend(const Archive::ByteStream &)
{
}

//-----------------------------------------------------------------------

void Connection::reportSend(const int sendSize)
{
	NetworkHandler::reportBytesSent(sendSize);

	static const int packetSizeWarnThreshold = ConfigSharedNetwork::getPacketSizeWarnThreshold();
	static const int reportInterval = ConfigSharedNetwork::getReportStatisticsInterval();

	if (reportInterval > 0 && !isNetLogConnection())
	{
		if (packetSizeWarnThreshold > 0)
		{
			if (sendSize > packetSizeWarnThreshold)
				 LOG("Network:ExcessiveMessageSize", ("large packet (%d bytes) send request exceeds warning threshold %d defined as SharedNetwork/packetSizeWarnThreshold", sendSize, packetSizeWarnThreshold));
		}

		m_bytesSent += sendSize;
		m_sendBytesReportInterval += sendSize;
		// check statistics
		m_lastSendTime = Clock::getFrameStartTimeMs();
		if (m_lastSendTime > m_lastSendReportTime + reportInterval)
		{
			// calculate send stats
			unsigned long lifeTime = (m_lastSendTime - m_connectionStartTime) / 1000;
			if (lifeTime > 0)
			{
				unsigned long reportTime = (m_lastSendTime - m_lastSendReportTime) / 1000;
				if (reportTime > 0)
				{
					int bytesPerSecond = m_sendBytesReportInterval / reportTime;

					if (bytesPerSecond > m_sendPeakBytesPerSecond)
						m_sendPeakBytesPerSecond = bytesPerSecond;

					m_sendAverageBytesPerSecond = m_bytesSent / lifeTime;

					std::string logChan = "Network:ConnectionStatsSend:" + getRemoteAddress() + ":";
					char portBuf[7] = {"\0"};
					snprintf(portBuf, sizeof(portBuf), "%d", getRemotePort());
					logChan += portBuf;
					LOG(logChan, ("Current(%d/sec), Average(%d/sec), Peak(%d/sec) Total Bytes(%d) Total Time(%lu)", bytesPerSecond, m_sendAverageBytesPerSecond, m_sendPeakBytesPerSecond, m_bytesSent, lifeTime));
					m_lastSendReportTime = m_lastSendTime;
					m_sendBytesReportInterval = 0;

					if (m_managerHandler->getSendCompressedByteCount() > 0)
						LOG(logChan, ("Compression Ratio: %.2f : 1.0 Send(%d / %d)", m_managerHandler->getCompressionRatio(), m_managerHandler->getSendUncompressedByteCount(), m_managerHandler->getSendCompressedByteCount()));
				}
			}
		}

	}
}

//-----------------------------------------------------------------------

void Connection::send(const Archive::ByteStream & bs, const bool r)
{
	if (NetworkHandler::removing())
		return;

	if (NetworkHandler::getCurrentFrame() != m_currentFrame)
	{
		// clear pending packet vector
		m_pendingPackets.clear();
	}

	static const bool reportMessages = ConfigSharedNetwork::getReportMessages();

	if (!isNetLogConnection())
	{
		if (reportMessages)
			reportSend(bs);

		reportSend(bs.getSize());

		static const bool logAllNetworkTraffic = ConfigSharedNetwork::getLogAllNetworkTraffic();

		if (logAllNetworkTraffic)
		{
			std::string logChan = "Network:Send:" + getRemoteAddress() + ":";
			char portBuf[7] = {"\0"};
			snprintf(portBuf, sizeof(portBuf), "%d", getRemotePort());
			logChan += portBuf;
			std::string output;
			const unsigned char * uc = bs.getBuffer();
			while (uc < bs.getBuffer() + bs.getSize())
			{
				if (isalpha(*uc))
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


	UdpChannel c = cUdpChannelReliable1;
	if (!r)
		c = cUdpChannelUnreliable;
	else
		checkOverflow(bs.getSize());

	flush();

	if (m_tcpClient)
		m_tcpClient->send(bs.getBuffer(), bs.getSize());
	else if (udpConnection && udpConnection->GetStatus() == UdpConnection::cStatusConnected)
		udpConnection->Send(c, bs.getBuffer(), bs.getSize());
	else
	{
		DeferredSendArchive * const d = new DeferredSendArchive(bs);
		m_deferredMessages.push_back(d);
		m_deferredDataSize += bs.getSize();

		if (ConfigSharedNetwork::getLogConnectionDeferredMessagesWarning())
			reportDeferredMessages();
	}
}

//-----------------------------------------------------------------------

void Connection::sendSharedPacket(const LogicalPacket * packet, const bool reliable)
{
	if (NetworkHandler::removing())
		return;

	if (packet)
		reportSend(packet->GetDataLen());

	UdpChannel c = cUdpChannelReliable1;
	if (!reliable)
		c = cUdpChannelUnreliable;

	flush();

	if (udpConnection && udpConnection->GetStatus() == UdpConnection::cStatusConnected)
	{
		udpConnection->Send(c, packet);
	}
	else
	{
		DeferredSendLogicalPacket * const d = new DeferredSendLogicalPacket(packet);
		m_deferredMessages.push_back(d);
		m_deferredDataSize += packet->GetDataLen();

		if (ConfigSharedNetwork::getLogConnectionDeferredMessagesWarning())
		{
			reportDeferredMessages();
	}
	}

	if (reliable)
		checkOverflow(packet->GetDataLen());
}

//-----------------------------------------------------------------------

void Connection::onConnectionStalled(const unsigned long stallTimeMs)
{
	LOG("Network", ("Connection stalled with %d bytes pending for %d milliseconds", udpConnection->TotalPendingBytes(), stallTimeMs));
}

//-----------------------------------------------------------------------

void Connection::setNoDataTimeout(const int timeout)
{
	udpConnection->SetNoDataTimeout(timeout);
}

//-----------------------------------------------------------------------

void Connection::setOverflowLimit(const int newLimit)
{
	m_overflowLimit = newLimit;
}

//-----------------------------------------------------------------------

TcpClient * Connection::getTcpClient()
{
	return m_tcpClient;
}

//-----------------------------------------------------------------------

void Connection::setTcpClientPendingSendAllocatedSizeLimit(const unsigned int limit)
{
	if (m_tcpClient)
		m_tcpClient->setPendingSendAllocatedSizeLimit(limit);
}

//-----------------------------------------------------------------------

Service * Connection::getService()
{
	return m_service;
}

//-----------------------------------------------------------------------

void Connection::setService(Service * s)
{
	m_service = s;
}

//-----------------------------------------------------------------------

int Connection::getPendingBytes() const
{
	int result = 0;
	if (udpConnection)
		result = udpConnection->TotalPendingBytes();
	return result;
}

//-----------------------------------------------------------------------

void Connection::remove()
{
	while (!s_connections.empty())
	{
		Connection *c = *s_connections.begin();

		std::vector<Connection *>::iterator f = std::find(s_clientConnections.begin(), s_clientConnections.end(), c);
		if (f != s_clientConnections.end())
			s_clientConnections.erase(f);

		c->onConnectionClosed();
		delete c;
	}

	TcpClient::remove();
}

//-----------------------------------------------------------------------

int Connection::getOldestUnacknowledgedAge() const
{
	int result = 0;
	if (udpConnection)
	{
		UdpConnection::ChannelStatus stats;
		udpConnection->GetChannelStatus(cUdpChannelReliable1, &stats);
		result = stats.oldestUnacknowledgedAge;
	}
	return result;
}

//-----------------------------------------------------------------------

int Connection::getCongestionWindowSize() const
{
	int result = 0;
	if (udpConnection)
	{
		UdpConnection::ChannelStatus stats;
		udpConnection->GetChannelStatus(cUdpChannelReliable1, &stats);
		result = stats.congestionWindowSize;
	}
	return result;
}

//-----------------------------------------------------------------------

int Connection::getAckAveragePing() const
{
	int result = 0;
	if (udpConnection)
	{
		UdpConnection::ChannelStatus stats;
		udpConnection->GetChannelStatus(cUdpChannelReliable1, &stats);
		result = stats.ackAveragePing;
	}
	return result;
}


//-----------------------------------------------------------------------

int Connection::getLastSend() const
{
	int result = 0;
	if (udpConnection)
	{
		result = udpConnection->LastSend();
	}
	return result;
}

//-----------------------------------------------------------------------

int Connection::getLastReceive() const
{
	int result = 0;
	if (udpConnection)
	{
		result = udpConnection->LastReceive();
	}
	return result;
}

//-----------------------------------------------------------------------

void Connection::describeConnection(const std::string & desc)
{
	m_description = desc;
}

//-----------------------------------------------------------------------

const std::string & Connection::getConnectionDescription() const
{
	return m_description;
}

//-----------------------------------------------------------------------

void Connection::update()
{
	// check keepalives
	{
		for (std::vector<Connection *>::iterator i = s_connections.begin(); i != s_connections.end(); ++i)
		{
			Connection *c = *i;
			if (c->m_tcpClient)
				c->m_tcpClient->checkKeepalive();
		}
	}

	// update client connections
	{
		std::vector<Connection *> activeConnections = s_clientConnections;
		for (std::vector<Connection *>::iterator i = activeConnections.begin(); i != activeConnections.end(); ++i)
		{
			Connection *c = *i;
			if (c->m_tcpClient)
				c->m_tcpClient->update();
		}
	}
}

//-----------------------------------------------------------------------

std::string const &Connection::getDisconnectReason() const
{
	return m_disconnectReason;
}

//-----------------------------------------------------------------------

void Connection::reportDeferredMessages() const
{
	if (   !m_deferredMessages.empty()
	    && (m_deferredMessages.size() % ConfigSharedNetwork::getLogConnectionDeferredMessagesWarningInterval()) == 0)
	{
		LOG("Connection", ("deferred message count(%i) %s(%d) [%s:%d %s]",
		m_deferredMessages.size(),
		Os::getProgramName(),
		static_cast<int>(Os::getProcessId()),
		m_remoteAddress.c_str(),
		static_cast<int>(m_remotePort),
		m_description.c_str()));
	}
}

//-----------------------------------------------------------------------

bool Connection::isNetLogConnection() const
{
	return false;
}

//-----------------------------------------------------------------------

void Connection::setRawTCP( bool bNewValue )
{
	if( m_tcpClient )
	{
		m_rawTCP = bNewValue;
		m_tcpClient->setRawTCP( bNewValue );
	}

}
