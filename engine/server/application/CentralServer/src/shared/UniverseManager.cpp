// ======================================================================
//
// UniverseManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstCentralServer.h"
#include "UniverseManager.h"

#include "GameServerConnection.h"
#include "serverNetworkMessages/ChangeUniverseProcessMessage.h"
#include "serverNetworkMessages/CreateDynamicRegionCircleMessage.h"
#include "serverNetworkMessages/CreateDynamicRegionRectangleMessage.h"
#include "serverNetworkMessages/CreateGroupMessage.h"
#include "serverNetworkMessages/GameServerUniverseLoadedMessage.h"
#include "serverNetworkMessages/LoadUniverseMessage.h"
#include "serverNetworkMessages/SetUniverseAuthoritativeMessage.h"
#include "sharedLog/Log.h"

#include "sharedFoundation/CrcConstexpr.hpp"

// ======================================================================

UniverseManager::UniverseManager() :
		Singleton<UniverseManager>(),
		MessageDispatch::Receiver(),
		m_universeProcess(0),
		m_databaseAuthoritative(true),
		m_serversLoadingUniverse(new ServersLoadingUniverseType)
{
	connectToMessage("CentralGameServerDbProcessServerProcessId");
	connectToMessage("ChangeUniverseProcessMessage");
	connectToMessage("CreateDynamicRegionCircleMessage");
	connectToMessage("CreateDynamicRegionRectangleMessage");
	connectToMessage("CreateGroupMessage");
	connectToMessage("GameServerUniverseLoadedMessage");
}

// ----------------------------------------------------------------------

UniverseManager::~UniverseManager()
{
	delete m_serversLoadingUniverse;
	m_serversLoadingUniverse=0;
}

// ----------------------------------------------------------------------

void UniverseManager::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	const uint32 messageType = message.getType();
	
	switch(messageType) {
		case constcrc("CentralGameServerDbProcessServerProcessId") :
		{
			DEBUG_REPORT_LOG(true, ("dbProcess connected\n"));
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CentralGameServerDbProcessServerProcessId d(ri);

			const GameServerConnection * g = static_cast<const GameServerConnection *>(&source); //lint !e826 Suspicious pointer-to-pointer conversion
			NOT_NULL(g);
			onDatabaseProcessConnect(*g);
			break;
		}
		case constcrc("CreateDynamicRegionCircleMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CreateDynamicRegionCircleMessage msg(ri);
			if (!m_databaseAuthoritative)
				CentralServer::getInstance().sendToGameServer(m_universeProcess, msg, true);
			break;
		}
		case constcrc("CreateDynamicRegionRectangleMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CreateDynamicRegionRectangleMessage msg(ri);
			if (!m_databaseAuthoritative)
				CentralServer::getInstance().sendToGameServer(m_universeProcess, msg, true);
			break;
		}
		case constcrc("CreateGroupMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CreateGroupMessage msg(ri);
			if (!m_databaseAuthoritative)
				CentralServer::getInstance().sendToGameServer(m_universeProcess, msg, true);
			break;
		}
		case constcrc("ChangeUniverseProcessMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			ChangeUniverseProcessMessage msg(ri);
			uint32 processId = static_cast<uint32>(msg.getId());
			if (!m_databaseAuthoritative && CentralServer::getInstance().getGameServer(processId))
			{
				m_universeProcess = processId;
				DEBUG_REPORT_LOG(true, ("Changing our Universe process to %lu\n",m_universeProcess));
				LOG("Universe", ("Changing our Universe process to %i\n",m_universeProcess));

				SetUniverseAuthoritativeMessage authMsg(m_universeProcess);
				CentralServer::getInstance().sendToAllGameServers(authMsg,true);
			}
			break;
		}
		case constcrc("GameServerUniverseLoadedMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GameServerUniverseLoadedMessage d(ri);
			UNREF(d);

			const GameServerConnection * g = static_cast<const GameServerConnection *>(&source); //lint !e826 Suspicious pointer-to-pointer conversion
			NOT_NULL(g);
			// DEBUG_REPORT_LOG(true,("m_serversLoadingUniverse->erase(%lu);\n",g->getProcessId()));
			IGNORE_RETURN(m_serversLoadingUniverse->erase(g->getProcessId()));
			break;
		}
	}
}

// ----------------------------------------------------------------------

void UniverseManager::onGameServerReady(const GameServerConnection &gameServer)
{
	sendUniverseToServer(gameServer.getProcessId());
}

// ----------------------------------------------------------------------

void UniverseManager::sendUniverseToServer(uint32 gameServerId)
{
	DEBUG_FATAL(m_universeProcess==0,("A game server connected to central before the database.  This probably indicates the servers were started out of order, or a game server from a previous run was not killed.\n"));

	LoadUniverseMessage loadMsg(gameServerId);
	CentralServer::getInstance().sendToGameServer(m_universeProcess, loadMsg, true);

	if (m_databaseAuthoritative)
	{
		m_databaseAuthoritative=false;
		m_universeProcess=gameServerId;
	}

//	DEBUG_REPORT_LOG(true,("m_serversLoadingUniverse->insert(%lu);\n",gameServerId));
	IGNORE_RETURN(m_serversLoadingUniverse->insert(gameServerId));
}

// ----------------------------------------------------------------------

void UniverseManager::onGameServerDisconnect(const GameServerConnection &gameServer)
{
	if (m_universeProcess == gameServer.getProcessId())
	{
		std::vector<uint32> readyGameServers;
		CentralServer::getInstance().getReadyGameServers(readyGameServers);
		uint32 replacement=0;
		for (std::vector<uint32>::iterator i=readyGameServers.begin(); i!=readyGameServers.end(); ++i)
		{
			if (m_serversLoadingUniverse->find(*i)==m_serversLoadingUniverse->end())
			{
				replacement = *i;
				break;
			}
		}

		if (replacement != 0)
		{
			DEBUG_FATAL(replacement==m_universeProcess,("Picked the server we just lost connection to as the universe process.\n"));
			m_universeProcess=replacement;
			DEBUG_REPORT_LOG(true, ("Lost our Universe process.  Picking process %lu to be the new one\n",m_universeProcess));
			LOG("Universe", ("Lost our Universe process.  Picking process %i to be the new one\n",m_universeProcess));

			SetUniverseAuthoritativeMessage msg(m_universeProcess);
			CentralServer::getInstance().sendToAllGameServers(msg,true);
		}
		else
		{
			DEBUG_REPORT_LOG(true,("Setting universe authority back to the database.\n"));
			LOG("Universe",("Setting universe authority back to the database.\n"));
			m_universeProcess=CentralServer::getInstance().getDbProcessServerProcessId();
			m_databaseAuthoritative=true;
		}

		for (ServersLoadingUniverseType::const_iterator resendServer=m_serversLoadingUniverse->begin(); resendServer != m_serversLoadingUniverse->end(); ++resendServer)
		{
			if (CentralServer::getInstance().getGameServer(*resendServer))
			{
				DEBUG_REPORT_LOG(true,("Resending universe objects to server %lu because the original universe process crashed.\n",*resendServer));
				LOG("Universe",("Resending universe objects to server %lu because the original universe process crashed.\n",*resendServer));
				sendUniverseToServer(*resendServer);
			}
		}
	}
}

// ----------------------------------------------------------------------

void UniverseManager::onDatabaseProcessConnect(const GameServerConnection &databaseProcess)
{
	DEBUG_FATAL(m_universeProcess!=0,("A second database process was started when one was already running.\n"));
	m_universeProcess = databaseProcess.getProcessId();
}

// ======================================================================
