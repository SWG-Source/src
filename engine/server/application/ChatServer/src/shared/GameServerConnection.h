// GameServerConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_GameServerConnection_H
#define	_INCLUDED_GameServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class GameServerConnection : public ServerConnection
{
public:
	GameServerConnection(UdpConnectionMT *, TcpClient *);
	virtual ~GameServerConnection();

	void          onConnectionClosed       ();
	void          onConnectionOpened       ();
	void          onReceive                (const Archive::ByteStream & bs);
	
private:
	GameServerConnection();
	GameServerConnection & operator = (const GameServerConnection & rhs);
	GameServerConnection(const GameServerConnection & source);

	std::string    m_gameCode;
	unsigned m_connectionId;

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_GameServerConnection_H
