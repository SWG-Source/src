// ======================================================================
//
// SharedRemoteDebugServerConnection.cpp
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedRemoteDebugServer/FirstSharedRemoteDebugServer.h"
#include "Archive/ByteStream.h"
#include "sharedRemoteDebugServer/SharedRemoteDebugServer.h"
#include "sharedRemoteDebugServer/SharedRemoteDebugServerConnection.h"
#include "sharedDebug/RemoteDebug_inner.h"
#include "sharedNetwork/NetworkSetupData.h"

//-----------------------------------------------------------------------

SharedRemoteDebugServerConnection::SharedRemoteDebugServerConnection(const std::string & a, const unsigned short p) :
Connection(a, p, NetworkSetupData()),
m_remotedebugCommandChannel (nullptr)
{
}

//-----------------------------------------------------------------------

SharedRemoteDebugServerConnection::SharedRemoteDebugServerConnection(UdpConnectionMT * u, TcpClient * t) :
Connection(u, t),
m_remotedebugCommandChannel (nullptr)
{
}

//-----------------------------------------------------------------------

SharedRemoteDebugServerConnection::~SharedRemoteDebugServerConnection()
{
}

//-----------------------------------------------------------------------

void SharedRemoteDebugServerConnection::onConnectionClosed()
{
	RemoteDebugServer::close();
}

//-----------------------------------------------------------------------

void SharedRemoteDebugServerConnection::onConnectionOpened()
{
	RemoteDebugServer::isReady();
}

//-----------------------------------------------------------------------

void SharedRemoteDebugServerConnection::onConnectionOverflowing(const unsigned int bytesPending)
{
	UNREF(bytesPending);
}

//-----------------------------------------------------------------------

void SharedRemoteDebugServerConnection::onReceive(const Archive::ByteStream & message)
{
	RemoteDebugServer::receive(message.getBuffer(), message.getSize());
}

//-----------------------------------------------------------------------
