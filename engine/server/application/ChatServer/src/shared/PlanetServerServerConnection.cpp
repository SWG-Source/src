// PlanetServerConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstChatServer.h"
#include "PlanetServerConnection.h"

//-----------------------------------------------------------------------

PlanetServerConnection::PlanetServerConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t)
{
}

//-----------------------------------------------------------------------

PlanetServerConnection::~PlanetServerConnection()
{
}

//-----------------------------------------------------------------------

void PlanetServerConnection::onConnectionClosed()
{
}

//-----------------------------------------------------------------------

void PlanetServerConnection::onConnectionOpened()
{
}

//-----------------------------------------------------------------------

void PlanetServerConnection::onReceive(const Archive::ByteStream &)
{
}

//-----------------------------------------------------------------------

