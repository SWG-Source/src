// Service.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#ifndef	INCLUDED_Service_H
#define	INCLUDED_Service_H

//-----------------------------------------------------------------------

#include "sharedNetwork/NetworkHandler.h"

#include <set>

class Connection;
class UdpConnectionMT;
class NetworkSetupData;
class TcpServer;
class TcpClient;

namespace Archive
{
	class ByteStream;
}

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------------

struct ConnectionAllocatorBase
{
	virtual ~ConnectionAllocatorBase() = 0;
	virtual ConnectionAllocatorBase * clone() const = 0;
	virtual Connection * allocateConnection(UdpConnectionMT *, TcpClient * t = 0) const = 0;
};

//-----------------------------------------------------------------------

template<typename ConnectionType>
struct ConnectionAllocator : public ConnectionAllocatorBase
{
	ConnectionAllocatorBase * clone() const
	{
		return new ConnectionAllocator<ConnectionType>;
	}

	Connection * allocateConnection(UdpConnectionMT * u, TcpClient * t = 0) const
	{
		return new ConnectionType(u, t);
	}
};

//-----------------------------------------------------------------------

class Service : public NetworkHandler
{
public:
	//Service(const ConnectionAllocatorBase & connectionAllocator, const unsigned short listenPort, const int maxConnections, const int keepAliveDelay = 1000, const std::string & interfaceAddress = std::string(""), const bool compress=false);
	Service(const ConnectionAllocatorBase & connectionAllocator, const NetworkSetupData & setupData);
	virtual ~Service();

	void         onConnectionOpened(TcpClient * t);
	void         onConnectionOpened(Connection * c);
	virtual void onConnectionOpened(UdpConnectionMT * udpConnection);
	virtual void onConnectionClosed(Connection * connection);
	void logBackloggedPackets();

	template<typename ConnectionType>
		void setConnectionAllocator(const ConnectionType * const)
	{
		connectionAllocator = new ConnectionAllocator<ConnectionType>();
	}

	int flushAndConfirmAllData();
	void removeConnection(Connection * c);
	void updateTcp();
	void broadcast(Archive::ByteStream const &bs, bool reliable);

private:
	Service & operator = (const Service & rhs);
	Service(const Service & source);

	const ConnectionAllocatorBase *  connectionAllocator;
	std::set<Connection *>           connections;
	int                              m_maxConnections;
	MessageDispatch::Callback *      m_callback;
	TcpServer *                      m_tcpServer;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_Service_H
