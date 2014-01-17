// ConsoleConnection.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ConsoleConnection_H
#define	_INCLUDED_ConsoleConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class ConsoleConnection : public ServerConnection
{
public:
	ConsoleConnection(UdpConnectionMT * udpConnection, TcpClient *);
	~ConsoleConnection();

	virtual void  onReceive  (const Archive::ByteStream & message);

private:
	ConsoleConnection & operator = (const ConsoleConnection & rhs);
	ConsoleConnection(const ConsoleConnection & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConsoleConnection_H
