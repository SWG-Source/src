// CustomerServiceServerConnection.cpp
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 

#include "FirstChatServer.h"
#include "CustomerServiceServerConnection.h"

#include "ChatInterface.h"
#include "ChatServer.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/ChatRequestLog.h"
#include "sharedNetworkMessages/ChatOnRequestLog.h"

//----------------------------------------------------------------------
CustomerServiceServerConnection::CustomerServiceServerConnection(const std::string & a, const unsigned short p)
: ServerConnection(a, p, NetworkSetupData())
{
	ChatServer::fileLog(true, "CSServerConnection", "Connection created...listening on (%s:%d)", a.c_str(), static_cast<int>(p));
}

//-----------------------------------------------------------------------
CustomerServiceServerConnection::~CustomerServiceServerConnection()
{
}

//-----------------------------------------------------------------------
void CustomerServiceServerConnection::onConnectionClosed()
{
	ChatServer::fileLog(true, "CSServerConnection", "onConnectionClosed()");

	ChatServer::clearCustomerServiceServerConnection();
}

//-----------------------------------------------------------------------
void CustomerServiceServerConnection::onConnectionOpened()
{
	ChatServer::fileLog(true, "CSServerConnection", "onConnectionOpened()");
}

//-----------------------------------------------------------------------
void CustomerServiceServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage gameNetworkMessage(ri);

	if (gameNetworkMessage.isType("ChatRequestLog"))
	{
		ri = message.begin();
		ChatRequestLog chatRequestLog(ri);

		std::vector<ChatLogEntry> chatLog;
		ChatServer::getChatLog(chatRequestLog.getPlayer(), chatLog);

		ChatServer::fileLog(true, "CSServerConnection", "onReceive() message(ChatRequestLog) player(%s) sequenceId(%i) chatLogSize(%i)", Unicode::wideToNarrow(chatRequestLog.getPlayer()).c_str(), chatRequestLog.getSequence(), chatLog.size());

		ChatOnRequestLog chatOnRequestLog(chatRequestLog.getSequence(), chatLog);
		send(chatOnRequestLog, true);
	}
}

//-----------------------------------------------------------------------
