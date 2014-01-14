// ======================================================================
//
// ConnectionHandler.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _ConnectionHandler_H_
#define _ConnectionHandler_H_

// ======================================================================

#include "sharedNetwork/UdpHandlerMT.h"

// ======================================================================

class Connection;

// ======================================================================

class ConnectionHandler: public UdpConnectionHandlerMT
{
public:
	ConnectionHandler(Connection *owner);

	virtual void OnRoutePacket(UdpConnectionMT *con, unsigned char const *data, int dataLen);
	virtual void OnConnectComplete(UdpConnectionMT *con);
	virtual void OnTerminated(UdpConnectionMT *con);
	virtual void OnPacketCorrupt(UdpConnectionMT *con, const uchar *data, int dataLen, UdpCorruptionReason reason);
	void setOwner(Connection *owner);
	
private:
	ConnectionHandler(ConnectionHandler const &);
	ConnectionHandler &operator=(ConnectionHandler const &);

	virtual ~ConnectionHandler();

private:
	Connection *m_owner;
};

// ======================================================================

#endif // _ConnectionHandler_H_

