// PingConnection.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "FirstLoginServer.h"
#include "CentralServerConnection.h"
#include "LoginServer.h"
#include "PingConnection.h"
#include "Archive/ByteStream.h"

//-----------------------------------------------------------------------

PingConnection::PingConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t)
{
}

//-----------------------------------------------------------------------

PingConnection::~PingConnection()
{
}

//-----------------------------------------------------------------------

void PingConnection::onConnectionOpened()
{
}

//-----------------------------------------------------------------------

void PingConnection::onReceive(const Archive::ByteStream &message )
{
	Archive::ReadIterator r(message);
	GameNetworkMessage g(r);

	if (g.isType("LoginPingMessage"))
	{
		send(g, true);
	}
}

//-----------------------------------------------------------------------

