// ChatServerConnection.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	INCLUDED_ChatServerConnection_H
#define	INCLUDED_ChatServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class GameNetworkMessage;

class ChatServerConnection : public ServerConnection
{
public:

	ChatServerConnection(UdpConnectionMT * u, TcpClient * t);
	~ChatServerConnection();

	void onConnectionClosed();
	void onConnectionOpened();
	void onReceive(const Archive::ByteStream & bs);

	static void sendTo(GameNetworkMessage const &message);
	
private:

	ChatServerConnection & operator = (const ChatServerConnection & rhs);
	ChatServerConnection(const ChatServerConnection & source);
}; //lint !e1712 default constructor not defined for class

//-----------------------------------------------------------------------

#endif	// INCLUDED_ChatServerConnection_H
