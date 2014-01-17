// CustomerServiceConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "CustomerServiceConnection.h"

//-----------------------------------------------------------------------

CustomerServiceConnection::CustomerServiceConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t),
gameServicePort(0)
{
}

//-----------------------------------------------------------------------

CustomerServiceConnection::~CustomerServiceConnection()
{
}

//-----------------------------------------------------------------------

const unsigned short CustomerServiceConnection::getGameServicePort() const
{
	return gameServicePort;
}

//-----------------------------------------------------------------------

void CustomerServiceConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();
	static MessageConnectionCallback m("CustomerServiceConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CustomerServiceConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();
	static MessageConnectionCallback m("CustomerServiceConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CustomerServiceConnection::onReceive(const Archive::ByteStream & message)
{
	ServerConnection::onReceive(message);
}

//-----------------------------------------------------------------------

void CustomerServiceConnection::setGameServicePort(const unsigned short p)
{
	gameServicePort = p;
}

//-----------------------------------------------------------------------

