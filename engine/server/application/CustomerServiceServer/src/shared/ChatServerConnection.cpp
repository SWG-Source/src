// ChatServerConnection.cpp
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------
#include "FirstCustomerServiceServer.h"
#include "ChatServerConnection.h"

#include "CustomerServiceServer.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------
namespace ChatServerConnectionNamespace
{
	ChatServerConnection *s_connection = 0;
};

using namespace ChatServerConnectionNamespace;

//-----------------------------------------------------------------------
ChatServerConnection::ChatServerConnection(UdpConnectionMT * u, TcpClient * t)
 : ServerConnection(u, t)
{
	LOG("ChatServerConnection", ("Connection created...listening on (%s:%d)", getRemoteAddress().c_str(), static_cast<int>(getRemotePort())));

	s_connection = this;
}

//-----------------------------------------------------------------------
ChatServerConnection::~ChatServerConnection()
{
	s_connection = 0;
}

//-----------------------------------------------------------------------
void ChatServerConnection::onConnectionClosed()
{
	LOG("ChatServerConnection", ("onConnectionClosed()"));
}

//-----------------------------------------------------------------------
void ChatServerConnection::onConnectionOpened()
{
	LOG("ChatServerConnection", ("onConnectionOpened()"));
}

//-----------------------------------------------------------------------
void ChatServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage gameNetworkMessage(ri);

	ri = message.begin();

	if (gameNetworkMessage.isType("ChatOnRequestLog"))
	{
		ChatOnRequestLog chatOnRequestLog(ri);
		CustomerServiceServer::getInstance().handleChatServerChatLog(chatOnRequestLog);
	}
}

//-----------------------------------------------------------------------
void ChatServerConnection::sendTo(GameNetworkMessage const &message)
{
	LOG("ChatServerConnection", ("sendTo() message(%s)", message.getCmdName().c_str()));

	if (s_connection != nullptr)
	{
		s_connection->send(message, true);
	}
	else
	{
		LOG("ChatServerConnection", ("sendTo() Unable to send, nullptr ChatServerConnection"));
	}
}

//-----------------------------------------------------------------------
