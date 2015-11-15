// ======================================================================
//
// ConfigCommodityServer.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
// Server Infrastructure by: Justin Randall
//
// This is message handler for incoming gameserver messages.
//
// ======================================================================

#ifndef	_INCLUDED_GameServerConnection_H
#define	_INCLUDED_GameServerConnection_H


#include "serverUtility/ServerConnection.h"

// ======================================================================

class GameServerConnection : public ServerConnection
{
public:
	GameServerConnection(UdpConnectionMT *, TcpClient * t);
	~GameServerConnection();

	virtual void  onConnectionClosed  ();
	virtual void  onConnectionOpened  ();
	virtual void  onReceive           (const Archive::ByteStream & message);

private:
	int               m_gameServerId;
	bool              m_showAllDebugInfo;

	GameServerConnection();
	GameServerConnection & operator = (const GameServerConnection & rhs);
	GameServerConnection(const GameServerConnection & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_GameServerConnection_H
