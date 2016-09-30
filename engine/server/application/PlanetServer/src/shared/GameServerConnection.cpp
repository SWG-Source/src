// ======================================================================
//
// GameServerConnection.cpp
//
// Copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstPlanetServer.h"
#include "GameServerConnection.h"

#include "ConfigPlanetServer.h"
#include "PlanetProxyObject.h"
#include "Scene.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

#include "sharedFoundation/CrcConstexpr.hpp"

// ======================================================================

GameServerConnection::GameServerConnection(UdpConnectionMT *u, TcpClient *t) :
	ServerConnection(u, t),
	m_preloadNumber(0),
	m_forwardCounts(),
	m_forwardDestinationServers(),
	m_forwardMessages()
{
}

// ----------------------------------------------------------------------

GameServerConnection::~GameServerConnection()
{
}

// ----------------------------------------------------------------------

void GameServerConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();

	static MessageConnectionCallback m("GameConnectionClosed");
	emitMessage(m);
}

// ----------------------------------------------------------------------

void GameServerConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();

	LOG("PlanetServerConnections", ("%s got a connection from a GameServer", ConfigPlanetServer::getSceneID()));

	static MessageConnectionCallback m("GameConnectionOpened");
	emitMessage(m);
}

// ----------------------------------------------------------------------

void GameServerConnection::onReceive(Archive::ByteStream const &message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage msg(ri);
	ri = message.begin();

	const uint32 messageType = msg.getType();

	if (!m_forwardDestinationServers.empty())
	{
		if (isMessageForwardable(messageType))
		{
			m_forwardMessages.push_back(std::make_pair(message, m_forwardDestinationServers.back()));
			return;
		}
		else
		{
			switch (messageType) {
				case constcrc("EndForward") :
				{
					if (--m_forwardCounts.back() == 0)
					{
						m_forwardCounts.pop_back();
						m_forwardDestinationServers.pop_back();
						if (m_forwardDestinationServers.empty())
							pushAndClearObjectForwarding();
					}
					return;
				}
				case constcrc("BeginForward") :
				{
					GenericValueTypeMessage<std::vector<uint32> > const beginForwardMessage(ri);
					if (beginForwardMessage.getValue() == m_forwardDestinationServers.back())
						++m_forwardCounts.back();
					else
					{
						m_forwardCounts.push_back(1);
						m_forwardDestinationServers.push_back(beginForwardMessage.getValue());
					}
					return;
				}
			}
		}
	}

	if (messageType == constcrc("BeginForward"))
	{
		GenericValueTypeMessage<std::vector<uint32> > const beginForwardMessage(ri);

		m_forwardCounts.push_back(1);
		m_forwardDestinationServers.push_back(beginForwardMessage.getValue());
		return;
	}

	ServerConnection::onReceive(message);
}

// ----------------------------------------------------------------------

void GameServerConnection::pushAndClearObjectForwarding()
{
	GameNetworkMessage const endForwardMessage("EndForward");
	std::vector<std::pair<Archive::ByteStream, std::vector<uint32> > > centralForwardMessages;

	{
		for (std::vector<std::pair<Archive::ByteStream, std::vector<uint32> > >::const_iterator i = m_forwardMessages.begin(); i != m_forwardMessages.end(); ++i)
		{
			Archive::ByteStream const &msg = (*i).first;
			std::vector<uint32> const &destinationServers = (*i).second;
			std::vector<uint32> centralForwardPids;

			for (std::vector<uint32>::const_iterator j = destinationServers.begin(); j != destinationServers.end(); ++j)
			{
				GameServerConnection * const conn = PlanetServer::getInstance().getGameServerConnection(*j);
				if (conn)
					conn->Connection::send(msg, true);
				else
					centralForwardPids.push_back(*j);
			}

			if (!centralForwardPids.empty())
				centralForwardMessages.push_back(std::make_pair(msg, centralForwardPids));
		}
	}

	if (!centralForwardMessages.empty())
	{
		ServerConnection * const conn = PlanetServer::getInstance().getCentralServerConnection();

		if (conn)
		{
			std::vector<uint32> const *lastDestinationServers = &(*centralForwardMessages.begin()).second;
			bool subBlock = false;

			GenericValueTypeMessage<std::vector<uint32> > const beginForwardMessage("BeginForward", *lastDestinationServers);
			conn->send(beginForwardMessage, true);

			for (std::vector<std::pair<Archive::ByteStream, std::vector<uint32> > >::const_iterator i = centralForwardMessages.begin(); i != centralForwardMessages.end(); ++i)
			{
				Archive::ByteStream const &msg = (*i).first;
				std::vector<uint32> const &destinationServers = (*i).second;

				if (destinationServers != *lastDestinationServers)
				{
					if (subBlock)
						conn->send(endForwardMessage, true);
					GenericValueTypeMessage<std::vector<uint32> > const beginForwardMessage("BeginForward", destinationServers);
					conn->send(beginForwardMessage, true);
					subBlock = true;
					lastDestinationServers = &destinationServers;
				}

				conn->Connection::send(msg, true);
			}

			if (subBlock)
				conn->send(endForwardMessage, true);
			conn->send(endForwardMessage, true);
		}
	}

	m_forwardMessages.clear();
}

// ======================================================================

