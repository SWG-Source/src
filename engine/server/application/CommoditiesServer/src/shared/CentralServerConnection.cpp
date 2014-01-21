// CentralServerConnection.cpp
// copyright 2001 Verant Interactive

//-----------------------------------------------------------------------

#include "sharedFoundation/FirstSharedFoundation.h"
#include "CentralServerConnection.h"
#include "sharedNetwork/NetworkSetupData.h"

//-----------------------------------------------------------------------
CentralServerConnection::CentralServerConnection(const std::string & address, const unsigned short port) :
ServerConnection(address, port, NetworkSetupData())
{
}


//-----------------------------------------------------------------------
CentralServerConnection::CentralServerConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t)
{
}

//-----------------------------------------------------------------------

CentralServerConnection::~CentralServerConnection()
{
}

//-----------------------------------------------------------------------


void CentralServerConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();
	static MessageConnectionCallback m("CentralServerConnectionClosed");
	emitMessage(m);
	exit(0);
}

//-----------------------------------------------------------------------

void CentralServerConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();
	static const MessageConnectionCallback m("CentralServerConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CentralServerConnection::onReceive(const Archive::ByteStream & message)
{
	ServerConnection::onReceive(message);
}
//-----------------------------------------------------------------------
