// GameServerConnection.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	INCLUDED_GameServerConnection_H
#define	INCLUDED_GameServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class GameNetworkMessage;

class GameServerConnection : public ServerConnection
{
public:

	GameServerConnection(UdpConnectionMT * u, TcpClient * t);
	~GameServerConnection();

	void onConnectionClosed();
	void onConnectionOpened();
	void onReceive(const Archive::ByteStream & bs);
	
	static void broadcast(GameNetworkMessage const &message);

private:

	GameServerConnection & operator = (const GameServerConnection & rhs);
	GameServerConnection(const GameServerConnection & source);
}; //lint !e1712 default constructor not defined for class

//-----------------------------------------------------------------------

#endif	// INCLUDED_GameServerConnection_H
