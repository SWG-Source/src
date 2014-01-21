// ConnectionServerConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_ConnectionServerConnection_H
#define	_INCLUDED_ConnectionServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class GameNetworkMessage;

class ConnectionServerConnection : public ServerConnection
{
public:
	ConnectionServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	~ConnectionServerConnection();

	void          onConnectionClosed       ();
	void          onConnectionOpened       ();
	void          onReceive                (const Archive::ByteStream & bs);
	void          sendToClient             (const NetworkId & clientId, const GameNetworkMessage & message);
	
private:
	ConnectionServerConnection & operator = (const ConnectionServerConnection & rhs);
	ConnectionServerConnection(const ConnectionServerConnection & source);
}; //lint !e1712 default constructor not defined for class

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConnectionServerConnection_H
