// ======================================================================
//
// ConnectionHandler.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetwork/FirstSharedNetwork.h"
#include "sharedCompression/ZlibCompressor.h"
#include "sharedNetwork/ConfigSharedNetwork.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetwork/ConnectionHandler.h"
#include "sharedNetwork/UdpConnectionMT.h"

// ======================================================================

ConnectionHandler::ConnectionHandler(Connection *owner) :
	UdpConnectionHandlerMT(),
	m_owner(owner)
{
}

// ----------------------------------------------------------------------

ConnectionHandler::~ConnectionHandler()
{
}

// ----------------------------------------------------------------------

void ConnectionHandler::setOwner(Connection *owner)
{
	m_owner = owner;
}

// ----------------------------------------------------------------------

void ConnectionHandler::OnRoutePacket(UdpConnectionMT *con, const uchar *data, int dataLen)
{ 
	NetworkHandler::onReceive(m_owner, con, data, dataLen); 
}

// ----------------------------------------------------------------------

void ConnectionHandler::OnConnectComplete(UdpConnectionMT *con)
{ 
	NetworkHandler::onConnect(m_owner, con); 
}

// ----------------------------------------------------------------------

void ConnectionHandler::OnTerminated(UdpConnectionMT *con)
{ 
	NetworkHandler::onTerminate(m_owner, con); 
}

//-----------------------------------------------------------------------

void ConnectionHandler::OnPacketCorrupt(UdpConnectionMT *con, const uchar *data, int dataLen, UdpCorruptionReason reason)
{
	UNREF(con);
	UNREF(data);
	UNREF(dataLen);
	UNREF(reason);
	con->Disconnect();
}

// ======================================================================

