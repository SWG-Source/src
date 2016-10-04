// ServerConsoleConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstServerConsole.h"
#include "ServerConsole.h"
#include "ServerConsoleConnection.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"

#include "sharedFoundation/CrcConstexpr.hpp"

//-----------------------------------------------------------------------

ServerConsoleConnection::ServerConsoleConnection(const std::string & address, const unsigned short port) :
Connection(address, port, NetworkSetupData())
{
}

//-----------------------------------------------------------------------

ServerConsoleConnection::~ServerConsoleConnection()
{
}

//-----------------------------------------------------------------------

void ServerConsoleConnection::onConnectionClosed()
{
	ServerConsole::done();
}

//-----------------------------------------------------------------------

void ServerConsoleConnection::onConnectionOpened()
{
}


//-----------------------------------------------------------------------

void ServerConsoleConnection::onReceive(const Archive::ByteStream & bs)
{
	Archive::ReadIterator ri = bs.begin();
	GameNetworkMessage m(ri);
	ri = bs.begin();
	
	const uint32 messageType = m.getType();

	switch(messageType) {
		case constcrc("ConGenericMessage") :
		{
			ConGenericMessage msg(ri);
			fprintf(stdout, "%s", msg.getMsg().c_str());
			break;
		}
		case constcrc("RequestDisconnect") :
		{
			disconnect();
			break;
		}
	}
}

//-----------------------------------------------------------------------

void ServerConsoleConnection::send(const GameNetworkMessage & msg)
{
	Archive::ByteStream bs;
	msg.pack(bs);
	Connection::send(bs, true);
}

//-----------------------------------------------------------------------

