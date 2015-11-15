// GameServerConnection.cpp
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------
#include "FirstCustomerServiceServer.h"
#include "GameServerConnection.h"

#include "CustomerServiceServer.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ChatOnRequestLog.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------
namespace GameServerConnectionNamespace
{
	std::set<GameServerConnection *> s_connections;
};

using namespace GameServerConnectionNamespace;

//-----------------------------------------------------------------------
GameServerConnection::GameServerConnection(UdpConnectionMT * u, TcpClient * t)
 : ServerConnection(u, t)
{
	LOG("GameServerConnection", ("Connection created...listening on (%s:%d)", getRemoteAddress().c_str(), static_cast<int>(getRemotePort())));

	s_connections.insert(this);
}

//-----------------------------------------------------------------------
GameServerConnection::~GameServerConnection()
{
	std::set<GameServerConnection *>::iterator iterConnections = s_connections.find(this);

	if (iterConnections != s_connections.end())
	{
		s_connections.erase(iterConnections);
	}
}

//-----------------------------------------------------------------------
void GameServerConnection::onConnectionClosed()
{
	LOG("GameServerConnection", ("onConnectionClosed()"));
}

//-----------------------------------------------------------------------
void GameServerConnection::onConnectionOpened()
{
	LOG("GameServerConnection", ("onConnectionOpened()"));
}

//-----------------------------------------------------------------------
void GameServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage gameNetworkMessage(ri);
	ri = message.begin();

	if (gameNetworkMessage.isType("ChatOnRequestLog"))
	{
		LOG("GameServerConnection", ("onReceive(ChatRequestLog)"));
		ChatOnRequestLog chatOnRequestLog(ri);
		CustomerServiceServer::getInstance().handleGameServerChatLog(chatOnRequestLog);
	}
}

//-----------------------------------------------------------------------
void GameServerConnection::broadcast(GameNetworkMessage const &message)
{
	LOG("GameServerConnection", ("broadcast() message(%s)", message.getCmdName().c_str()));

	std::set<GameServerConnection *>::const_iterator iterConnections = s_connections.begin();

	for (; iterConnections != s_connections.end(); ++iterConnections)
	{
		GameServerConnection *connection = (*iterConnections);

		connection->send(message, true);
	}
}

//-----------------------------------------------------------------------
