// ======================================================================
//
// UdpConnectionMT.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _UdpConnectionMT_H_
#define _UdpConnectionMT_H_

// ======================================================================

#include "UdpLibrary.h"

// ======================================================================

class UdpConnectionHandlerMT;
class UdpConnectionHandlerInternal;

// ======================================================================

class UdpConnectionMT
{
public:
	void AddRef();
	void Release();

	void processReceive(unsigned char const *data, int dataLen);
	void processTerminated();
	void processConnectComplete();

	bool Send(UdpChannel channel, unsigned char const *data, int dataLen);
	bool Send(UdpChannel channel, LogicalPacket const *packet);
	void Disconnect(int flushTimeout = 0);

	UdpConnection::DisconnectReason GetDisconnectReason() const;
	void *GetPassThroughData() const;
	UdpConnection::Status GetStatus() const;
	int TotalPendingBytes() const;
	unsigned short ServerSyncStampShort() const;
	unsigned long ServerSyncStampLong() const;
	UdpIpAddress GetDestinationIp() const;
	int GetDestinationPort() const;
	void GetChannelStatus(UdpChannel channel, UdpConnection::ChannelStatus *channelStatus) const;
	int LastReceive() const;
	int LastSend() const;

	void SetHandler(UdpConnectionHandlerMT *handler);
	void SetPassThroughData(void *passThroughData);
	void SetNoDataTimeout(int noDataTimeout);

private:
	UdpConnectionMT(UdpConnectionMT const &);
	UdpConnectionMT &operator=(UdpConnectionMT const &);

private:
	friend class UdpManagerMT;
	friend class UdpManagerHandlerMT;
	friend class EventConnectRequest;
	UdpConnectionMT(UdpConnection *udpConnection);
	~UdpConnectionMT();

private:
	int m_refCount;
	UdpConnection *m_udpConnection;
	void *m_passThroughData;
	UdpConnectionHandlerInternal *m_connectionHandlerInternal;
};

// ======================================================================

#endif // _UdpConnectionMT_H_

