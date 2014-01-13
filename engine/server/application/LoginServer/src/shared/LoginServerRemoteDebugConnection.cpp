// ======================================================================
//
// LoginServerRemoteDebugConnection.cpp
// copyright 2001, Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "LoginServerRemoteDebugConnection.h"
#include "sharedDebug/RemoteDebug_inner.h"

//-----------------------------------------------------------------------

LoginServerRemoteDebugConnection::LoginServerRemoteDebugConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t)
{
}

//-----------------------------------------------------------------------

LoginServerRemoteDebugConnection::~LoginServerRemoteDebugConnection(void)
{
}

//-----------------------------------------------------------------------

void LoginServerRemoteDebugConnection::onConnectionClosed()
{
	RemoteDebugServer::close();
}

//-----------------------------------------------------------------------

void LoginServerRemoteDebugConnection::onConnectionOpened()
{
	RemoteDebugServer::isReady();
}

//-----------------------------------------------------------------------

void LoginServerRemoteDebugConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage m(ri);
	emitMessage(m);
}

//-----------------------------------------------------------------------
