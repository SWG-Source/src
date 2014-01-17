// ======================================================================
//
// PlanetServerConnection.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstCentralServer.h"
#include "PlanetServerConnection.h"

#include "ConsoleConnection.h"
#include "GameServerConnection.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "serverNetworkMessages/CentralPlanetServerConnect.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

PlanetServerConnection::PlanetServerConnection(UdpConnectionMT *u, TcpClient *t) :
	ServerConnection(u, t),
	m_gameServerConnectionAddress(""),
	m_gameServerConnectionPort(0),
	sceneId(),
	m_forwardCounts(),
	m_forwardDestinationServers(),
	m_forwardMessages()
{
}

// ----------------------------------------------------------------------

PlanetServerConnection::~PlanetServerConnection()
{
	onConnectionClosed();
}

// ----------------------------------------------------------------------

std::string const &PlanetServerConnection::getSceneId() const
{
	return sceneId;
}

// ----------------------------------------------------------------------

void PlanetServerConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();

	static MessageConnectionCallback m("PlanetServerConnectionOpened");
	emitMessage(m);
}

// ----------------------------------------------------------------------

void PlanetServerConnection::onConnectionClosed()
{
	static MessageConnectionCallback m("PlanetServerConnectionClosed");
	CentralServer::getInstance().removePlanetServer(this);
	emitMessage(m);
}

// ----------------------------------------------------------------------

void PlanetServerConnection::onReceive(Archive::ByteStream const &message)
{
	Archive::ReadIterator ri(message);
	GameNetworkMessage const msg(ri);
	ri = message.begin();

	if (!m_forwardDestinationServers.empty())
	{
		if (isMessageForwardable(msg.getType()))
		{
			m_forwardMessages.push_back(std::make_pair(message, m_forwardDestinationServers.back()));
			return;
		}
		else if (msg.isType("EndForward"))
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
		else if (msg.isType("BeginForward"))
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

	if (msg.isType("BeginForward"))
	{
		GenericValueTypeMessage<std::vector<uint32> > const beginForwardMessage(ri);

		m_forwardCounts.push_back(1);
		m_forwardDestinationServers.push_back(beginForwardMessage.getValue());
		return;
	}

	ServerConnection::onReceive(message);

	if (msg.isType("CentralPlanetServerConnect"))
	{
		CentralPlanetServerConnect const id(ri);
		sceneId = id.getSceneId();
	}
	else if (msg.isType("TaskSpawnProcess"))
	{
		TaskSpawnProcess const spawn(ri);
		CentralServer::getInstance().sendTaskMessage(spawn);
	}
	else if (msg.isType("ConGenericMessage"))
	{
		ConGenericMessage const con(ri);
		ConsoleConnection::onCommandComplete(con.getMsg(), static_cast<int>(con.getMsgId()));
	}
}

// ----------------------------------------------------------------------

void PlanetServerConnection::setGameServerConnectionData(std::string const &address, uint16 port)
{
	m_gameServerConnectionAddress = address;
	m_gameServerConnectionPort = port;
}

// ----------------------------------------------------------------------

std::string const &PlanetServerConnection::getGameServerConnectionAddress() const
{
	return m_gameServerConnectionAddress;
}

// ----------------------------------------------------------------------

uint16 PlanetServerConnection::getGameServerConnectionPort() const
{
	return m_gameServerConnectionPort;
}

// ----------------------------------------------------------------------

void PlanetServerConnection::pushAndClearObjectForwarding()
{
	for (std::vector<std::pair<Archive::ByteStream, std::vector<uint32> > >::const_iterator i = m_forwardMessages.begin(); i != m_forwardMessages.end(); ++i)
	{
		Archive::ByteStream const &msg = (*i).first;
		std::vector<uint32> const &destinationServers = (*i).second;

		for (std::vector<uint32>::const_iterator j = destinationServers.begin(); j != destinationServers.end(); ++j)
		{
			GameServerConnection * const conn = CentralServer::getInstance().getGameServer(*j);
			if (conn)
				conn->Connection::send(msg, true);
		}
	}

	m_forwardMessages.clear();
}

// ======================================================================

