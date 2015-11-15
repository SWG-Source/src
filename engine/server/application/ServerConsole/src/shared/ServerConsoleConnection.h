// ServerConsoleConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ServerConsoleConnection_H
#define	_INCLUDED_ServerConsoleConnection_H

//-----------------------------------------------------------------------

#include "sharedNetwork/Connection.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ServerConsoleConnection : public Connection
{
public:
	ServerConsoleConnection(const std::string & address, const unsigned short port);
	~ServerConsoleConnection();

	void  onConnectionClosed  ();
	void  onConnectionOpened  ();
	void  onReceive           (const Archive::ByteStream & bs);
	void  send                (const GameNetworkMessage &);

private:
	ServerConsoleConnection & operator = (const ServerConsoleConnection & rhs);
	ServerConsoleConnection(const ServerConsoleConnection & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ServerConsoleConnection_H
