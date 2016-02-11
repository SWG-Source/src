// ======================================================================
//
// PlanetManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstCentralServer.h"
#include "PlanetManager.h"

#include "ConfigCentralServer.h"
#include "GameServerConnection.h"
#include "PlanetServerConnection.h"
#include "serverNetworkMessages/PlanetObjectIdMessage.h"
#include "serverNetworkMessages/RequestPlanetObjectIdMessage.h"
#include "serverNetworkMessages/SetPlanetServerMessage.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "sharedNetwork/NetworkHandler.h"

// ======================================================================

PlanetManager::PlanetRec::PlanetRec() :
m_connection(0),
m_planetObjectId(NetworkId::cms_invalid)
{
}

// ----------------------------------------------------------------------

PlanetManager::PlanetManager() :
MessageDispatch::Receiver(),
m_pendingGameServers(),
m_servers()
{
	connectToMessage("PlanetObjectIdMessage");
	connectToMessage("PlanetServerConnectionClosed");
}

// ----------------------------------------------------------------------

void PlanetManager::addServer(const std::string &sceneId, PlanetServerConnection *connection)
{
	instance().m_servers[sceneId].m_connection=connection;
	DEBUG_REPORT_LOG(true,("Added Planet Server for scene %s\n",sceneId.c_str()));

	std::vector<std::pair<std::string, GameServerConnection *> >::iterator i;
	for(i = instance().m_pendingGameServers.begin(); i != instance().m_pendingGameServers.end();)
	{
		if((*i).first == sceneId)
		{
			addGameServerForScene(sceneId, (*i).second);
			i = instance().m_pendingGameServers.erase(i);
		}
		else
			++i;
	}
}

// ----------------------------------------------------------------------

/**
 * Called when a new game server connects.
 * Locates the PlanetServer for the game server's scene and tells the game server about it.
 */

void PlanetManager::addGameServerForScene(const std::string &sceneId, GameServerConnection *gameServer)
{
	ServerListType::const_iterator i=instance().m_servers.find(sceneId);
	if (i==instance().m_servers.end())
	{
		CentralServer::getInstance().startPlanetServer("any", sceneId, 0);
		instance().m_pendingGameServers.push_back(std::make_pair(sceneId, gameServer));

		return;
	}
	else
	{
		const PlanetServerConnection *planetServer=(*i).second.m_connection;
		SetPlanetServerMessage m(planetServer->getGameServerConnectionAddress(), planetServer->getGameServerConnectionPort());
		gameServer->send(m,true);
	}
}

// ----------------------------------------------------------------------

void PlanetManager::receiveMessage(const MessageDispatch::Emitter &source , const MessageDispatch::MessageBase & message)
{
	if(message.isType("PlanetObjectIdMessage"))  //@todo:  Is this message actually used anymore?
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		PlanetObjectIdMessage msg(ri);

		// Message identifies the PlanetObject that belongs to a particular planet
		ServerListType::iterator i=m_servers.find(msg.getSceneId());
		if (i==m_servers.end())
		{
			WARNING_STRICT_FATAL(true,("Got unexpected PlanetObjectIdMessage for %s.\n",msg.getSceneId().c_str()));
			return;
		}

		(*i).second.m_planetObjectId=msg.getPlanetObject();

		// Not needed because all UniverseObjects are proxied.
//		CentralServer::getInstance().proxyOnAllServersForScene(msg.getPlanetObject(),msg.getSceneId());
	}
	else if(message.isType("PlanetServerConnectionClosed"))
	{
		const PlanetServerConnection * c = static_cast<const PlanetServerConnection *>(&source); //lint !e826 Suspiscious pointer-to-pointer conversion (area too small)
		ServerListType::iterator i = m_servers.begin();
		for (; i != m_servers.end(); ++i)
		{
			if (i->second.m_connection == c)
			{
				m_servers.erase(i);
				break;
			}
		}

		if (ConfigCentralServer::getRequestDbSaveOnPlanetServerCrash())
		{
			GameNetworkMessage const msg("CentralRequestSave");
			CentralServer::getInstance().sendToDBProcess(msg, true);
		}

		CentralServer::getInstance().startPlanetServer(CentralServer::getInstance().getHostForScene(c->getSceneId()), c->getSceneId(), ConfigCentralServer::getPlanetServerRestartDelayTimeSeconds());
	}
	else
	{
		WARNING_STRICT_FATAL(true,("Planet Manager got unexpected message.\n"));
	}
}

// ----------------------------------------------------------------------

PlanetServerConnection *PlanetManager::getPlanetServerForScene(const std::string &sceneId)
{
	ServerListType::const_iterator i=instance().m_servers.find(sceneId);
	if (i!=instance().m_servers.end())
		return (*i).second.m_connection;
	else
		return nullptr;
}

// ----------------------------------------------------------------------

/**
 * Remove the game server from the pending list, if it is in there.
 */
void PlanetManager::onGameServerDisconnect(const GameServerConnection *gameServer)
{
	for (PendingGameServersType::iterator i=instance().m_pendingGameServers.begin(); i!=instance().m_pendingGameServers.end(); )
	{
		if ((*i).second == gameServer)
			i=instance().m_pendingGameServers.erase(i);
		else
			++i;
	}
}

//-----------------------------------------------------------------------

PlanetManager & PlanetManager::instance()
{
	static PlanetManager m;
	return m;
}

// ======================================================================
