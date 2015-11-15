// PingConnection.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_PingConnection_H
#define	_INCLUDED_PingConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class PingConnection : public ServerConnection
{
public:
	PingConnection(UdpConnectionMT *, TcpClient *);
	~PingConnection();
	void                onConnectionOpened();
	void                onReceive(const Archive::ByteStream & message);

private:
	PingConnection & operator = (const PingConnection & rhs);
	PingConnection(const PingConnection & source);

};//lint !e1712 default constructor not defined

//-----------------------------------------------------------------------

#endif	// _INCLUDED_PingConnection_H
