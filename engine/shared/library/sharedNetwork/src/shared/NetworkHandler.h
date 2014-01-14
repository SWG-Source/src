// NetworkHandler.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#ifndef	INCLUDED_NetworkHandler_H
#define	INCLUDED_NetworkHandler_H

//-----------------------------------------------------------------------

#include <string>
#include <vector>

class Connection;
class LogicalPacket;
class ManagerHandler;
class Service;
class UdpConnectionMT;
class UdpManagerMT;

//-----------------------------------------------------------------------

class NetworkHandler
{
public:
	NetworkHandler();
	virtual ~NetworkHandler() = 0;

	static void                 install             ();
	static void                 remove              ();
	static void                 dispatch            ();
	static void                 flushAndConfirmAll  ();
	static const int            getBytesReceivedThisFrame();
	static const int            getBytesSentThisFrame();
	static const int            getPacketsReceivedThisFrame();
	static const int            getPacketsSentThisFrame();
	
	static int                  getRecvTotalCompressedByteCount    ();
	static int                  getRecvTotalUncompressedByteCount  ();
	static int                  getSendTotalCompressedByteCount    ();
	static int                  getSendTotalUncompressedByteCount  ();
	static float                getTotalCompressionRatio           ();

	static const std::string &  getHostName         ();
	static const std::string &  getHumanReadableHostName();
	static const std::vector<std::pair<std::string, std::string> > &  getInterfaceAddresses ();
	static bool                 isAddressLocal      (const std::string & address);
	static void                 update              ();
	static void                 clearBytesThisFrame ();
	static void                 reportMessage       (const std::string & message, const int size);

	virtual void                onConnectionOpened  (UdpConnectionMT *) = 0;
	const std::string &         getBindAddress      () const;
	const unsigned short        getBindPort         () const;
	virtual void                onConnectionClosed  (Connection *) = 0;
	void                        setBindAddress      (const std::string & address);
	void                        setBindPort         (const unsigned short port);
	static bool                 removing            ();
	LogicalPacket const *       createPacket        (unsigned char const *data, int size);
	void                        releasePacket       (LogicalPacket const *p);
	static bool                 isPortReserved      (unsigned short port);
	static void                 onTerminate         (Connection * c);

protected:
	friend class ConnectionHandler;
	friend class ManagerHandler;

	static void reportBytesSent(const int bytes);
	static void reportBytesReceived(const int bytes);
	static void onConnect(void * callback, UdpConnectionMT * connection);
	static void onReceive(Connection *, const unsigned char *, int);
	static void onReceive(void *, UdpConnectionMT * u, const unsigned char * d, int s);
	static void onTerminate(void * m, UdpConnectionMT * u);
	static uint64 getCurrentFrame();

	void newManager(UdpManagerMT *);
	void newService(Service *);
	void removeService(Service *);
	virtual int flushAndConfirmAllData () = 0;
	static void disconnectConnection(Connection *);

private:
	NetworkHandler & operator = (const NetworkHandler & rhs);
	NetworkHandler(const NetworkHandler & source);

private:
	unsigned short       m_bindPort;
	std::string          m_bindAddress;
	
protected:
	UdpManagerMT *       m_udpManager;
	ManagerHandler *     m_managerHandler;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_NetworkHandler_H
