// ======================================================================
//
// PlanetServer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstPlanetServer.h"
#include "PlanetServer.h"

#include "CentralServerConnection.h"
#include "ConfigPlanetServer.h"
#include "ConsoleManager.h"
#include "GameServerConnection.h"
#include "GameServerData.h"
#include "PlanetProxyObject.h"
#include "PlanetServerMetricsData.h"
#include "QuadtreeNode.h"
#include "Scene.h"
#include "TaskConnection.h"
#include "WatcherConnection.h"
#include "serverMetrics/MetricsManager.h"
#include "serverNetworkMessages/ChunkCompleteMessage.h"
#include "serverNetworkMessages/ExcommunicateGameServerMessage.h"
#include "serverNetworkMessages/FactionalSystemMessage.h"
#include "serverNetworkMessages/FirstPlanetGameServerIdMessage.h"
#include "serverNetworkMessages/GameGameServerMessages.h" // we use the GameGameServerConnect message
#include "serverNetworkMessages/GameServerForLoginMessage.h"
#include "serverNetworkMessages/GameServerForceChangeAuthorityMessage.h"
#include "serverNetworkMessages/GameServerReadyMessage.h"
#include "serverNetworkMessages/GameServerUniverseLoadedMessage.h"
#include "serverNetworkMessages/LoadStructureMessage.h"
#include "serverNetworkMessages/LoadStructureMessage.h"
#include "serverNetworkMessages/LocateStructureMessage.h"
#include "serverNetworkMessages/LocationRequest.h"
#include "serverNetworkMessages/LocationResponse.h"
#include "serverNetworkMessages/PersistedPlayerMessage.h"
#include "serverNetworkMessages/PlanetLoadCharacterMessage.h"
#include "serverNetworkMessages/PreloadFinishedMessage.h"
#include "serverNetworkMessages/PreloadRequestCompleteMessage.h"
#include "serverNetworkMessages/ProfilerOperationMessage.h"
#include "serverNetworkMessages/RequestChunkMessage.h"
#include "serverNetworkMessages/RequestGameServerForLoginMessage.h"
#include "serverNetworkMessages/RestartServerMessage.h"
#include "serverNetworkMessages/SceneTransferMessages.h"
#include "serverNetworkMessages/TaskProcessDiedMessage.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "serverNetworkMessages/UnloadObjectMessage.h"
#include "serverNetworkMessages/UnloadProxyMessage.h"
#include "serverNetworkMessages/UnloadedPlayerMessage.h"
#include "serverNetworkMessages/UpdateObjectOnPlanetMessage.h"
#include "serverUtility/PopulationList.h"
#include "serverUtility/SetupServerUtility.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Timer.h"
#include "sharedLog/Log.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetworkMessages/FrameEndMessage.h"
#include "sharedNetworkMessages/GalaxyLoopTimesResponse.h"
#include "sharedNetworkMessages/GameServerStatus.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/ServerInfo.h"
#include "sharedUtility/LocationManager.h"
#include "unicodeArchive/UnicodeArchive.h"

#include <cstdio>
#include <list>
#include <set>

// ======================================================================

static const std::string loginTrace("TRACE_LOGIN");

// ======================================================================

struct CharacterFindInfo
{
	unsigned int sequence;
	NetworkId containerId;
	Vector coords;
	uint32 stationId;
	int pendingResponseCount;
	bool forCtsSourceCharacter;
};

namespace PlanetServerNamespace
{
	std::vector<RequestChunkMessage::Chunk> m_pendingChunkRequests;
	std::vector<std::pair<uint32, uint32> > m_pendingPreloadRequestCompletes;
}
using namespace PlanetServerNamespace;

// ======================================================================

PlanetServer::PlanetServer() :
		Singleton<PlanetServer>(),
		MessageDispatch::Receiver(),
		m_pendingCentralServerConnection(nullptr),
		m_centralServerConnection(nullptr),
		m_gameService(nullptr),
		m_watcherService(nullptr),
		m_gameServers(),
		m_taskConnection(nullptr),
		m_done(false),
		m_roundRobinGameServer(0),
		m_pendingServerStarts(new std::map<PreloadServerId, GameServerSpawnDelaySeconds>()),
		m_startingGameServers(new std::map<int, std::pair<std::string, time_t> >()),
		m_firstGameServer(0),
		m_tutorialMode(false),
		m_spaceMode(false),
		m_messagesWaitingForGameServer(),
		m_metricsData(0),
		m_taskManagerConnection(nullptr),
		m_sceneTransferChunkLoads(new std::list<const RequestSceneTransfer*>),
		m_pendingCharacterSaves(new std::map<NetworkId, uint32>),
		m_watchers(),
		m_watcherIsPresent(false),
		m_queuedMessages(),
		m_characterFindMap(new std::map<NetworkId, CharacterFindInfo>),
		m_characterFindSequence(0),
		m_waitForSaveCounter(0),
		m_lastSaveCounter(0)
{
	SetupServerUtility::install();
	ConsoleManager::install();
	// set up central server connection
	m_pendingCentralServerConnection = new CentralServerConnection(ConfigPlanetServer::getCentralServerAddress(), ConfigPlanetServer::getCentralServerPort());

	// connect to the task manager
	// jrandall, set on connection
	m_taskConnection = new TaskConnection("127.0.0.1", ConfigPlanetServer::getTaskManagerPort());

	// set up game server connection
	NetworkSetupData setup;
	setup.port = ConfigPlanetServer::getGameServicePort();
	setup.bindInterface = ConfigPlanetServer::getGameServiceBindInterface();
	setup.maxConnections = 1000;
	m_gameService = new Service(ConnectionAllocator<GameServerConnection>(), setup);

	// set up watcher connection
	if (ConfigPlanetServer::getMaxWatcherConnections() > 0)
	{
		setup.port = ConfigPlanetServer::getWatcherServicePort();
		setup.maxConnections = 1;
		setup.bindInterface = ConfigPlanetServer::getWatcherServiceBindInterface();
		m_watcherService = new Service(ConnectionAllocator<WatcherConnection>(), setup);
	}

	connectToMessage("CentralConnectionClosed");
	connectToMessage("CentralConnectionOpened");
	connectToMessage("ChunkCompleteMessage");
	connectToMessage("DatabaseSaveComplete");
	connectToMessage("DenySetAuthoritativeMessage");
	connectToMessage("ExcommunicateGameServerMessage");
	connectToMessage("FactionalSystemMessage");
	connectToMessage("MessageToPlayersOnPlanet");
	connectToMessage("FindAuthObjectResponse");
	connectToMessage("FrameEndMessage");
	connectToMessage("GameConnectionClosed");
	connectToMessage("GameGameServerConnect");
	connectToMessage("GameServerForceChangeAuthorityMessage");
	connectToMessage("GameServerReadyMessage");
	connectToMessage("GameServerUniverseLoadedMessage");
	connectToMessage("LocateStructureMessage");
	connectToMessage("LocationRequest");
	connectToMessage("PersistedPlayerMessage");
	connectToMessage("PreloadListMessage");
	connectToMessage("PreloadRequestCompleteMessage");
	connectToMessage("ProfilerOperationMessage");
	connectToMessage("RequestAuthTransfer");
	connectToMessage("RequestGameServerForLoginMessage");
	connectToMessage("RequestSameServer");
	connectToMessage("RequestSceneTransfer");
	connectToMessage("RestartServerByRoleMessage");
	connectToMessage("RestartServerMessage");
	connectToMessage("SAMGS");
	connectToMessage("ShutdownMessage");
	connectToMessage("TaskConnectionOpened");
	connectToMessage("TaskProcessDiedMessage");
	connectToMessage("UnloadedPlayerMessage");
	connectToMessage("WatcherConnectionClosed");
	connectToMessage("WatcherConnectionOpened");

	m_metricsData = new PlanetServerMetricsData;
	std::string sceneName = ConfigPlanetServer::getSceneID() ;
	MetricsManager::install(m_metricsData, false, "PlanetServer" , sceneName, 0);
	if (sceneName == "tutorial")
	{
		DEBUG_REPORT_LOG(true,("Running in tutorial mode\n"));
		m_tutorialMode=true; // tutorial mode can't be set in the config file because all planet servers have the same config file
	}
	if (strncmp(sceneName.c_str(),"space_",6)==0)
	{
		DEBUG_REPORT_LOG(true,("Running in space mode\n"));
		m_spaceMode=true; // space mode can't be set in the config file because all planet servers have the same config file
	}

	PlanetProxyObject::install();
	PreloadManager::install(static_cast<PreloadManager*>(0));
	Node::install();
	Scene::install(static_cast<Scene*>(0)); // this syntax is required by MSVC to identify the template function
}

//-----------------------------------------------------------------------

PlanetServer::~PlanetServer()
{
	ConsoleManager::remove();
	MetricsManager::remove();

	m_centralServerConnection = 0;
	m_gameService = 0;
	m_metricsData = 0;
	m_pendingCentralServerConnection = 0;
	m_taskConnection = 0;
	m_taskManagerConnection = 0;
	m_watcherService = 0;

	delete m_pendingServerStarts;
	m_pendingServerStarts = 0;
	delete m_startingGameServers;
	m_startingGameServers = 0;

	std::vector<const GameNetworkMessage *>::iterator t;
	for(t = m_messagesWaitingForGameServer.begin(); t != m_messagesWaitingForGameServer.end();++t)
	{
		delete *t; //lint !e605 // deleting const
		*t = 0;
	}

	m_messagesWaitingForGameServer.clear();

	while (m_sceneTransferChunkLoads->begin() != m_sceneTransferChunkLoads->end())
	{
		delete *m_sceneTransferChunkLoads->begin();  //lint !e605 // deleting const
		IGNORE_RETURN(m_sceneTransferChunkLoads->erase(m_sceneTransferChunkLoads->begin()));
	}
	delete m_sceneTransferChunkLoads;
	m_sceneTransferChunkLoads = 0;
	delete m_pendingCharacterSaves;
	m_pendingCharacterSaves=0;

	{
		GameServerMapType::iterator i=m_gameServers.begin();
		for (; i!=m_gameServers.end(); ++i)
		{
			delete i->second;
		}
	}

	delete m_characterFindMap;
	m_characterFindMap = 0;

}

//-----------------------------------------------------------------------

const Service * PlanetServer::getGameService() const
{
	return m_gameService;
}

//-----------------------------------------------------------------------

const unsigned short PlanetServer::getGameServicePort() const
{
	unsigned short result = 0;
	if(m_gameService)
		result = m_gameService->getBindPort();
	return result;
}

// ----------------------------------------------------------------------

/**
 * This function exists to provide a static function for the engine's
 * callback.
 * It converts between an engine-style singleton (all functions static)
 * and the patterns-style singleton (one instance).
 */

void PlanetServer::run(void)
{
	NetworkHandler::install();
	LOG("ServerStartup",("PlanetServer starting on %s", NetworkHandler::getHostName().c_str()));

	PlanetServer::getInstance().mainLoop();
	NetworkHandler::remove();
}

// ----------------------------------------------------------------------

void PlanetServer::mainLoop(void)
{
	Scene::getInstance().setSceneId(ConfigPlanetServer::getSceneID()); // also insures Scene is constructed

	Timer loadingLogTimer(10);
	Timer preloadBailoutTimer(static_cast<float>(ConfigPlanetServer::getPreloadBailoutTime()));

	bool hasLoggedPreloadCompletion = false;

	while (!m_done)
	{
		PROFILER_AUTO_BLOCK_DEFINE("main loop");

		if (!Os::update())
			setDone("Os condition (Parent pid change)");

		float updateTime = Clock::frameTime();

		{
			PROFILER_AUTO_BLOCK_DEFINE("NetworkHandler::update");
			NetworkHandler::update();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("NetworkHandler::dispatch");
			NetworkHandler::dispatch();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("MetricsManager::update");
			MetricsManager::update(updateTime * 1000);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("Os::sleep");
			Os::sleep(1);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("Scene::update");
			Scene::getInstance().update(updateTime);
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("Send chunk requests");
			if (m_waitForSaveCounter==0) // Don't send load requests during crash recovery
			{
				if (!m_pendingChunkRequests.empty())
				{
					RequestChunkMessage const msg(m_pendingChunkRequests, Scene::getInstance().getSceneId());
					PlanetServer::getInstance().sendToCentral(msg, true);
					m_pendingChunkRequests.clear();
				}

				if (!m_pendingPreloadRequestCompletes.empty())
				{
					for(std::vector<std::pair<uint32, uint32> >::const_iterator i=m_pendingPreloadRequestCompletes.begin(); i!=m_pendingPreloadRequestCompletes.end(); ++i)
					{
						PreloadRequestCompleteMessage const msg(i->first, i->second);
						PlanetServer::getInstance().sendToCentral(msg, true);
					}
					m_pendingPreloadRequestCompletes.clear();
				}
			}
		}

		for (WatcherList::const_iterator i = m_watchers.begin(); i != m_watchers.end(); ++i)
		{
			PROFILER_AUTO_BLOCK_DEFINE("WatcherConnection::flushQueuedData");
			WatcherConnection * w = (*i);
			w->flushQueuedData();
		}

		//--Log preloading status, bail out if preloading hangs
		{
			if (loadingLogTimer.updateSubtract(updateTime))
			{
				if (hasLoggedPreloadCompletion && !PreloadManager::getInstance().isPreloadComplete())
					hasLoggedPreloadCompletion = false;
				if (!hasLoggedPreloadCompletion)
				{
					std::string message;
					PreloadManager::getInstance().getDebugString(message);
					LOG("Preload",("%s %s",Scene::getInstance().getSceneId().c_str(),message.c_str()));
					if (PreloadManager::getInstance().isPreloadComplete())
						hasLoggedPreloadCompletion = true;
				}
			}
			if (ConfigPlanetServer::getPreloadBailoutTime()!=0)
			{
				if (PreloadManager::getInstance().isPreloadComplete())
					preloadBailoutTimer.reset();
				else
				{
					if (preloadBailoutTimer.updateZero(updateTime))
						FATAL(TRUE,("Preload did not finish on Planet Server %s within a reasonable amount of time",Scene::getInstance().getSceneId().c_str()));
				}
			}
		}

		NetworkHandler::clearBytesThisFrame();
	}

	m_watcherIsPresent = false; // so that we don't try to send to the watcher while shutting down
	m_watchers.clear(); // UDP library will delete the objects
}

// ----------------------------------------------------------------------

void PlanetServer::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	if(message.isType("CentralConnectionOpened"))
	{
		DEBUG_REPORT_LOG(true,("Connected to Central.\n"));

		//handle preloading cities -- the preload list will determine how many servers we spawn
		PreloadManager::getInstance().handlePreloadList();
	}
	else if (message.isType("ShutdownMessage"))
	{
		REPORT_LOG(true, ("Instructed to shutdown the server\n"));
		setDone("Received ShutdownMessage");
	}
	else if(message.isType("CentralConnectionClosed"))
	{
		DEBUG_REPORT_LOG(true, ("Central server closed connection.  Exiting.\n"));
		ServerConnection const *serverConnection = dynamic_cast<ServerConnection const *>(&source);
		setDone("CentralConnectionClosed : %s", serverConnection ? serverConnection->getDisconnectReason().c_str() : "");
		m_centralServerConnection = 0; //lint !e423 // object is deleted elsewhere
		m_pendingCentralServerConnection = 0; //lint !e423 // object is deleted elsewhere
	}
	else if (message.isType("GameConnectionClosed"))
	{
		GameServerConnection *gameServer=const_cast<GameServerConnection *>(dynamic_cast<const GameServerConnection*>(&source));
		std::set<GameServerConnection *>::iterator f = m_pendingGameServerDisconnects.find(gameServer);

		WARNING_DEBUG_FATAL(gameServer==0,("Source was nullptr or source was not a GameServerConnection."));
		DEBUG_REPORT_LOG(true, ("removing crashed gameserver\n"));
		uint32 id = gameServer->getProcessId();
		GameServerMapType::iterator i=m_gameServers.find(id);
		if (i!=m_gameServers.end())
		{
			delete i->second;
			m_gameServers.erase(i);
		}

		// must call this before removeGameServer() to get the preload number
		// for the dead game server; we'll need that in order to start a replacement
		PreloadServerId preloadServerId = PreloadManager::getInstance().getPreloadServerId(id);

		// if the game server hadn't gotten far enough to be assigned a preload
		// number, use the preload number that was assigned to it when it was started
		if (preloadServerId == 0)
			preloadServerId = gameServer->getPreloadNumber();

		PreloadManager::getInstance().removeGameServer(id);
		if(f == m_pendingGameServerDisconnects.end())
		{
			std::set<PreloadServerId> startGameServerList;
			IGNORE_RETURN(startGameServerList.insert(preloadServerId));

			startGameServer(startGameServerList, ConfigPlanetServer::getGameServerRestartDelayTimeSeconds());
		}
		else
		{
			m_pendingGameServerDisconnects.erase(f);
		}
		// advise watchers that the game server has closed its connection
		ServerInfo info;
		info.ipAddress = gameServer->getRemoteAddress();
		info.serverId = id;
		info.systemPid = gameServer->getOsProcessId();
		GameServerStatus statusMsg(false, info);
		std::vector<WatcherConnection *>::iterator w;
		std::vector<WatcherConnection *>::iterator begin = m_watchers.begin();
		std::vector<WatcherConnection *>::iterator end = m_watchers.end();

		for(w = begin; w != end; ++w)
		{
			(*w)->send(statusMsg, true);
		}

		if (m_firstGameServer == id)
		{
			if (m_gameServers.size() == 0)
				m_firstGameServer = 0;
			else
			{
				GameServerMapType::const_iterator j=m_gameServers.begin();
				m_firstGameServer = j->first;
				FirstPlanetGameServerIdMessage const msg(m_firstGameServer);
				for (; j!=m_gameServers.end(); ++j)
					j->second->getConnection()->send(msg, true);
			}
		}
		PreloadFinishedMessage const preloadFinishedMessage(PreloadManager::getInstance().isPreloadComplete());
		sendToCentral(preloadFinishedMessage, true);
	}

	else if(message.isType("GameGameServerConnect"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GameGameServerConnect msg(ri);
		GameServerConnection * conn = const_cast<GameServerConnection*>(safe_cast<const GameServerConnection *>(&source));

		std::map<int, std::pair<std::string, time_t> >::iterator f = m_startingGameServers->find(msg.getSpawnCookie());
		if(f != m_startingGameServers->end())
			m_startingGameServers->erase(f);

		uint32 id = msg.getProcessId();
		m_gameServers[id]=new GameServerData(conn);
		conn->setProcessId(id);
		conn->setPreloadNumber(msg.getPreloadNumber());
		LOG("PlanetServerConnections",("Game Server %lu (preload %d) connected on scene %s.  m_startingGameServers is now %i", id, msg.getPreloadNumber(), ConfigPlanetServer::getSceneID(),m_startingGameServers->size()));

		DEBUG_REPORT_LOG(true,("Game Server %lu (preload %d) connected on scene %s.\n", id, msg.getPreloadNumber(), ConfigPlanetServer::getSceneID()));

		ServerInfo info;
		info.ipAddress = conn->getRemoteAddress();
		info.serverId = id;
		info.systemPid = conn->getOsProcessId();
		GameServerStatus statusMsg(true, info);
		std::vector<WatcherConnection *>::iterator i;
		std::vector<WatcherConnection *>::iterator begin = m_watchers.begin();
		std::vector<WatcherConnection *>::iterator end = m_watchers.end();

		for(i = begin; i != end; ++i)
		{
			(*i)->send(statusMsg, true);
		}

		if (m_firstGameServer == 0)
		{
			WARNING_STRICT_FATAL(m_gameServers.size() > 1, ("m_firstGameServer was not set but there were Game Server connections in the list."));
			m_firstGameServer = id;
		}

		FirstPlanetGameServerIdMessage const fpgsim(m_firstGameServer);
		conn->send(fpgsim, true);
	}
	else if(message.isType("GameServerReadyMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const GameServerReadyMessage msg(ri);

		GameServerConnection * conn = const_cast<GameServerConnection*>(dynamic_cast<const GameServerConnection *>(&source));
		if (conn)
		{
			uint32 id = conn->getProcessId();
			GameServerMapType::iterator mapIter = m_gameServers.find(id);
			if (mapIter==m_gameServers.end())
			{
				WARNING_STRICT_FATAL(true,("GameServer %lu wasn't in the map, but we got GameServerReadyMessage from it.",id));
				return;
			}
			Scene::getInstance().setMapSize(msg.getMapWidth());
			mapIter->second->ready();
		}
	}
	else if(message.isType("GameServerUniverseLoadedMessage"))
	{
		GameServerConnection * conn = const_cast<GameServerConnection*>(dynamic_cast<const GameServerConnection *>(&source));
		if (conn)
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			const GameServerUniverseLoadedMessage msg(ri);

			uint32 id = conn->getProcessId();
			LOG("PlanetServerConnections", ("Game Server %lu (preload %d) has loaded the universe objects.", conn->getProcessId(), conn->getPreloadNumber()));

			GameServerMapType::iterator mapIter = m_gameServers.find(id);
			WARNING_STRICT_FATAL(mapIter==m_gameServers.end(),("GameServer %s wasn't in the map, but we got UniverseLoadedMessage from it.",id));
			if (mapIter==m_gameServers.end())
				return;
			mapIter->second->universeLoaded();
			if(! PreloadManager::getInstance().onGameServerReady(id, static_cast<PreloadServerId>(conn->getPreloadNumber())))
			{
				m_pendingGameServerDisconnects.insert(conn);
				conn->disconnect();
			}
		}
		flushWaitingMessages();
	}
	else if (message.isType("PreloadRequestCompleteMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const PreloadRequestCompleteMessage msg(ri);

		PreloadManager::getInstance().preloadCompleteOnServer(msg.getGameServerId());
		GameServerData *data = getGameServerData(msg.getGameServerId());
		if (data)
			data->preloadComplete();
	}
	else if (message.isType("RequestGameServerForLoginMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const RequestGameServerForLoginMessage *msg = new RequestGameServerForLoginMessage(ri);

		handleRequestGameServerForLoginMessage(msg); //this function handles memory management for msg. msg is INVALID after this call.
		msg = 0;
	}
	else if (message.isType("RequestSceneTransfer"))
	{
		//This message comes from Central in response to a GameServer sending central a request scene transfer message.
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		RequestSceneTransfer const *msg = new RequestSceneTransfer(ri);

		handleRequestSceneTransfer(msg); //this function handles memory management for msg.  msg is INVALID after this call.
		msg = 0;
	}
	else if (message.isType("ChunkCompleteMessage"))
	{
		GameServerConnection * conn = const_cast<GameServerConnection*>(dynamic_cast<const GameServerConnection *>(&source));
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ChunkCompleteMessage msg(ri);
		handleChunkComplete(msg, conn);
	}
	else if (message.isType("WatcherConnectionOpened"))
	{
		WatcherConnection *newConn = const_cast<WatcherConnection*>(dynamic_cast<const WatcherConnection*>(&source));
		if (newConn)
		{
			// advertise status of all game servers to new watcher
			GameServerMapType::iterator i;
			GameServerMapType::iterator begin = m_gameServers.begin();
			GameServerMapType::iterator end = m_gameServers.end();
			for(i = begin; i != end; ++i)
			{
				ServerInfo info;
				info.ipAddress = (*i).second->getConnection()->getRemoteAddress();
				info.serverId = (*i).second->getConnection()->getProcessId();
				info.systemPid = (*i).second->getConnection()->getOsProcessId();
				info.sceneId = ConfigPlanetServer::getSceneID();
				GameServerStatus const status(true, info);
				newConn->send(status, true);
			}

			if (static_cast<int>(m_watchers.size() + 1) > ConfigPlanetServer::getMaxWatcherConnections())
			{
				DEBUG_REPORT_LOG(true,("Dropping previous watcher connection.\n"));
				WARNING_DEBUG_FATAL(m_watchers.size()==0,("MaxWatcherConnections must be at least 1."));
				m_watchers[0]->disconnect();
			}
			m_watchers.push_back(newConn);
			m_watcherIsPresent=true;
			Scene::getInstance().outputStatus(*newConn);
		}
		else
			WARNING_STRICT_FATAL(true,("Got WatcherConnectionOpened from something that wasn't a WatcherConnection."));
	}
	else if (message.isType("WatcherConnectionClosed"))
	{
		for (WatcherList::iterator i=m_watchers.begin(); i!= m_watchers.end(); )
		{
			if ((*i)==&source)
				i=m_watchers.erase(i);
			else
				++i;
		}
		if (m_watchers.empty())
		{
			m_watcherIsPresent=false;
			DEBUG_REPORT_LOG(true,("No more watchers connected.\n"));
		}
	}
	else if (message.isType("TaskConnectionOpened"))
	{
		std::set<PreloadServerId> startGameServerList;
		startGameServer(startGameServerList, 0); // flush any pending game server start requests
	}
	else if(message.isType("ProfilerOperationMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ProfilerOperationMessage msg(ri);
		unsigned int processId = msg.getProcessId();
		if (!processId)
			Profiler::handleOperation(msg.getOperation().c_str());
	}
	else if(message.isType("FrameEndMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		FrameEndMessage const msg(ri);
		for (WatcherList::const_iterator i = m_watchers.begin(); i != m_watchers.end(); ++i)
			(*i)->send(msg, true);
	}
	else if (message.isType("UnloadedPlayerMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		UnloadedPlayerMessage msg(ri);

		GameServerConnection *gameServer=const_cast<GameServerConnection *>(dynamic_cast<const GameServerConnection*>(&source));
		WARNING_DEBUG_FATAL(gameServer==0,("Source was nullptr or source was not a GameServerConnection."));
		uint32 gameServerId = gameServer->getProcessId();

		(*m_pendingCharacterSaves)[msg.getPlayerId()]=gameServerId;
	}
	else if (message.isType("PersistedPlayerMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		PersistedPlayerMessage msg(ri);

		m_pendingCharacterSaves->erase(msg.getPlayerId());
	}
	else if (message.isType("ExcommunicateGameServerMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ExcommunicateGameServerMessage msg(ri);

		LOG("GameGameConnect",("Planet Server %s was told to drop connection to %lu by Central",Scene::getInstance().getSceneId().c_str(),msg.getServerId()));

		GameServerConnection *conn =getGameServerConnection(msg.getServerId());
		if (conn)
			conn->disconnect();
	}
	else if (message.isType("FactionalSystemMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		FactionalSystemMessage msg(ri);

		// forward message to all game servers on this planet for processing
		for (GameServerMapType::iterator i = m_gameServers.begin(); i != m_gameServers.end(); ++i)
			(*i).second->getConnection()->send(msg, true);
	}
	else if (message.isType("MessageToPlayersOnPlanet"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::pair<std::pair<std::string, std::vector<int8> >, std::pair<float, bool> >, std::pair<Vector, float> > > const msg(ri);

		// forward message to all game servers on this planet for processing
		for (GameServerMapType::iterator i = m_gameServers.begin(); i != m_gameServers.end(); ++i)
			(*i).second->getConnection()->send(msg, true);
	}
	else if (message.isType("RequestSameServer"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > msg(ri);

		bool result=Scene::getInstance().requestSameServer(msg.getValue().first, msg.getValue().second);

		GameServerConnection * const gameServer = const_cast<GameServerConnection *>(dynamic_cast<const GameServerConnection*>(&source));

		if (gameServer)
		{
			GenericValueTypeMessage<bool> const rssReply("RequestSameServerReply", result);
			gameServer->send(rssReply, true);
		}
	}
	else if (message.isType("RequestAuthTransfer"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<NetworkId, uint32> > msg(ri);

		bool result=Scene::getInstance().requestAuthTransfer(msg.getValue().first, msg.getValue().second);

		GameServerConnection * const gameServer = const_cast<GameServerConnection *>(dynamic_cast<const GameServerConnection*>(&source));

		if (gameServer)
		{
			GenericValueTypeMessage<bool> const ratReply("RequestAuthTransferReply", result);
			gameServer->send(ratReply, true);
		}
	}
	else if (message.isType("GameServerForceChangeAuthorityMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GameServerForceChangeAuthorityMessage msg(ri);
		GameServerData * fromServer = PlanetServer::getGameServerData(msg.getFromProcess());
		GameServerData * toServer = PlanetServer::getGameServerData(msg.getToProcess());
		PlanetProxyObject * object = Scene::getInstance().findObjectByID(msg.getId());
		if (fromServer == nullptr || toServer == nullptr || object == nullptr)
		{
			if (fromServer == nullptr)
			{
				WARNING(true, ("Message GameServerForceChangeAuthorityMessage: no "
					"from server %lu", msg.getFromProcess()));
			}
			if (toServer == nullptr)
			{
				WARNING(true, ("Message GameServerForceChangeAuthorityMessage: no "
					"to server %lu", msg.getToProcess()));
			}
			if (object == nullptr)
			{
				WARNING(true, ("Message GameServerForceChangeAuthorityMessage: no "
					"object for id %s", msg.getId().getValueString().c_str()));
			}
		}
		else if (Scene::getInstance().getGameServerForObject(msg.getId()) != msg.getFromProcess())
		{
			WARNING(true, ("Message GameServerForceChangeAuthorityMessage: object "
				"%s was supposed to be on process %lu, but we think it is on %lu",
				msg.getId().getValueString().c_str(), msg.getFromProcess(),
				Scene::getInstance().getGameServerForObject(msg.getId())));
		}
		else
		{
			object->changeAuthority(msg.getToProcess(), false, true);
		}
	}
	else if (message.isType("FindAuthObjectResponse"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::pair<NetworkId, unsigned int>, bool> > msg(ri);
		GameServerConnection const *gameServer = dynamic_cast<const GameServerConnection*>(&source);
		NOT_NULL(gameServer);
		handleFindAuthObjectResponse(msg.getValue().first.first, msg.getValue().first.second, msg.getValue().second, gameServer->getProcessId());
	}
	else if (message.isType("TaskProcessDiedMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		TaskProcessDiedMessage died(ri);
		// verify it was a game server
		const std::string & proc = died.getProcessName();
		if(proc.find("SwgGameServer") != proc.npos)
		{
			// pull cookie and preload server id from the command line of the process that died
			size_t start = proc.find("spawnCookie");
			size_t preloadServerIdIndex = proc.find("preloadNumber");
			if ((start != proc.npos) && (preloadServerIdIndex != proc.npos))
			{
				start += std::string("spawnCookie=").length();
				size_t end = proc.find(' ', start);
				std::string cookieName = proc.substr(start, end);
				int i = atoi(cookieName.c_str());
				std::map<int, std::pair<std::string, time_t> >::iterator f = m_startingGameServers->find(i);
				if(f != m_startingGameServers->end())
				{
					m_startingGameServers->erase(f);

					preloadServerIdIndex += std::string("preloadNumber=").length();
					end = proc.find(' ', preloadServerIdIndex);
					std::string preloadServerIdStr = proc.substr(preloadServerIdIndex, end);
					PreloadServerId preloadServerId = static_cast<PreloadServerId>(atoi(preloadServerIdStr.c_str()));

					std::set<PreloadServerId> startGameServerList;
					IGNORE_RETURN(startGameServerList.insert(preloadServerId));

					startGameServer(startGameServerList, ConfigPlanetServer::getGameServerRestartDelayTimeSeconds());

					LOG("TaskProcessDied", ("TaskManager advises the PlanetServer that a GameServer %u for scene %s started with cookie %i is no longer running", preloadServerId, ConfigPlanetServer::getSceneID(), i));
				}
			}
		}
	}
	else if (message.isType ("LocationRequest"))
	{
		Archive::ReadIterator readIterator = static_cast<GameNetworkMessage const &> (message).getByteStream ().begin ();

		//-- received a location request message
		LocationRequest const locationRequest (readIterator);

		//-- ask the location manager if the location is valid
		float x = locationRequest.getSearchX ();
		float z = locationRequest.getSearchZ ();
		bool const valid = LocationManager::requestLocation (x, z, locationRequest.getSearchRadius (), locationRequest.getLocationReservationRadius (), locationRequest.getCheckWater (), locationRequest.getCheckSlope (), x, z);

		//-- sent result back to gameserver that requested it
		LocationResponse const locationResponse (locationRequest.getNetworkId (), valid, locationRequest.getLocationId (), x, z, locationRequest.getLocationReservationRadius ());
		PlanetServer::getInstance ().sendToGameServer (locationRequest.getProcessId (), locationResponse);
	}
	else if (message.isType ("DatabaseSaveComplete"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<int> msg(ri);
		handleDatabaseSaveComplete(msg.getValue());
	}
	else if (message.isType("LocateStructureMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		LocateStructureMessage msg(ri);
		LoadStructureMessage lsm(msg.getStructureId(), msg.getWhoRequested());

		uint32 gameServerId = Scene::getInstance().findNodeByPosition(msg.getX(), msg.getZ())->getPreferredServer();
		PlanetServer::getInstance().sendToGameServer(gameServerId, lsm);
	}
	else if (message.isType("DenySetAuthoritativeMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<NetworkId, uint32> > const msg(ri);

		// restore the authority of the object
		PlanetProxyObject *object = Scene::getInstance().findObjectByID(msg.getValue().first);
		if (object)
		{
			if (ConfigPlanetServer::getLogObjectLoading())
				LOG("ObjectLoading",
					("Set authoritative denied by game server:  restoring authority for object %s to server %lu",
					 msg.getValue().first.getValueString().c_str(),msg.getValue().second));

			object->restoreAuthority(msg.getValue().second);
		}
	}
	else if (message.isType("RestartServerMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		RestartServerMessage const msg(ri);

		Node const * const node = Scene::getInstance().findNodeByPositionConst(msg.getX(), msg.getZ());
		uint32 const serverId = NON_NULL(node)->getPreferredServer();

		if (serverId != 0)
		{
			ExcommunicateGameServerMessage exmsg(serverId, 0, "");
			sendToCentral(exmsg, true);
		}
	}
	else if (message.isType("SAMGS"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<uint8> const startAnyMissingGameServer(ri);
		UNREF(startAnyMissingGameServer);

		LOG("PlanetServer", ("startanymissinggameserver:Planet %s checking for any game servers that are not started, and starting them.", Scene::getInstance().getSceneId().c_str()));
		if (m_centralServerConnection)
		{
			// if it's been "awhile" since we requested to restart the GameServer,
			// then assume that something has gone wrong, and try the restart again
			time_t const timeNow = ::time(nullptr);

			for (std::map<int, std::pair<std::string, time_t> >::iterator iter = m_startingGameServers->begin(); iter != m_startingGameServers->end(); ++iter)
			{
				if ((iter->second.second + static_cast<time_t>(ConfigPlanetServer::getMaxTimeToWaitForGameServerStartSeconds())) < timeNow)
				{
					LOG("CentralServer", ("startanymissinggameserver:Starting missing game server (%s) because it has been %s since we requested it to be started, which has exceeded the %s limit.", iter->second.first.c_str(), CalendarTime::convertSecondsToMS(static_cast<unsigned int>(timeNow - iter->second.second)).c_str(), CalendarTime::convertSecondsToMS(static_cast<unsigned int>(ConfigPlanetServer::getMaxTimeToWaitForGameServerStartSeconds())).c_str()));
					iter->second.second = timeNow;
					TaskSpawnProcess spawn("any", "SwgGameServer", iter->second.first);
					m_centralServerConnection->send(spawn,true);
				}
				else
				{
					LOG("PlanetServer", ("startanymissinggameserver:It's only been %s since we requested for game server (%s) to be started, and we must wait %s before we can start it again.", CalendarTime::convertSecondsToMS(static_cast<unsigned int>(timeNow - iter->second.second)).c_str(), iter->second.first.c_str(), CalendarTime::convertSecondsToMS(static_cast<unsigned int>(ConfigPlanetServer::getMaxTimeToWaitForGameServerStartSeconds())).c_str()));
				}
			}
		}
		else
		{
			LOG("PlanetServer", ("startanymissinggameserver:Planet %s ignoring request because there is no central server connection.", Scene::getInstance().getSceneId().c_str()));
		}
	}
	else if (message.isType("RestartServerByRoleMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::string, uint32> > msg(ri);

		uint32 serverId = PreloadManager::getInstance().getRealServerId(msg.getValue().second);
		if (serverId != 0)
		{
			ExcommunicateGameServerMessage exmsg(serverId, 0, "");
			sendToCentral(exmsg, true);
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Send a message to the Central server.
 */
void PlanetServer::sendToCentral(const GameNetworkMessage & message, const bool reliable)
{
	if (m_centralServerConnection)
	{
		m_centralServerConnection->send(message,reliable);
	}
	else
	{
		DEBUG_REPORT_LOG(true,("Central is not connected.\n"));
	}
}

//-----------------------------------------------------------------------

void PlanetServer::sendToGameServer(const uint32 id, const GameNetworkMessage & msg)
{
	GameServerConnection *conn=getGameServerConnection(id);
	if (conn)
		conn->send(msg, true);
	else
		DEBUG_REPORT_LOG(true,("Not sending message to server %lu because we are not connected.\n",id));
}

// ----------------------------------------------------------------------

/**
 * Send a message to a game server to force it to unload an object.
 * @param object The id of the object
 * @param authServer The id of the server that is authoritative for the object
 */
void PlanetServer::unloadObject(const NetworkId &object, uint32 authServer)
{
	UnloadObjectMessage msg(object);
	GameServerConnection *conn=getGameServerConnection(authServer);
	if (conn)
		conn->send(msg,true);
	else
	{
		// We should either have a connection or have handled the crash and removed all of that server's objects
		WARNING(true,("Couldn't send UnloadObjectMessage for object %s, because we did not have a connection to game server %i.",object.getValueString().c_str(),authServer));
	}
}

// ----------------------------------------------------------------------

GameServerConnection *PlanetServer::getGameServerConnection(uint32 serverId)
{
	GameServerMapType::iterator i=m_gameServers.find(serverId);
	if (i!=m_gameServers.end())
		return (*i).second->getConnection();
	else
		return nullptr;
}

// ----------------------------------------------------------------------

ServerConnection *PlanetServer::getCentralServerConnection()
{
	return m_centralServerConnection;
}

// ----------------------------------------------------------------------

/**
 * Get a game server for a character that wants to log in, and load the
 * character on that game server.
 * 1)  If the building the player is in is already loaded, pick the
 *     authoritative server for that building.
 * 2)  Otherwise, pick a good game server for that location
 * 3)  If the chunk is not loaded, load it
 * 4)  Put a lock on the chunk to prevent it from being unloaded
 *     while waiting for the player to load
 * 5)  Tell the database to load the character onto the selected gameserver.
 * 6)  Tell the connection server which game server to use
 */

void PlanetServer::handleRequestGameServerForLoginMessage(const RequestGameServerForLoginMessage*& msg)
{
	NOT_NULL(msg);

	if (!PreloadManager::getInstance().isPreloadComplete())
	{
		DEBUG_REPORT_LOG(true, ("Waiting for game server for login\n"));
		m_messagesWaitingForGameServer.push_back(msg);
		return;
	}

	std::map<NetworkId, uint32>::iterator pendingSave=m_pendingCharacterSaves->find(msg->getCharacterId());
	if (pendingSave!=m_pendingCharacterSaves->end())
	{
		uint32 gameServerId = pendingSave->second;
		if (getGameServerConnection(pendingSave->second)) //make sure it's still up
		{
			LOG(loginTrace,("Reconnecting character %s who logged out but has not yet been saved.  Game server is %lu",msg->getCharacterId().getValueString().c_str(),gameServerId));

			GameServerForLoginMessage const reply(msg->getStationId(), gameServerId, msg->getCharacterId());
			sendToCentral(reply, true);
			delete msg; //lint !e605  Deleting const pointer
			msg = 0;
			m_pendingCharacterSaves->erase(pendingSave);
			return;
		}
	}

	findOrLoadCharacter(msg->getCharacterId(), msg->getContainerId(), msg->getCoordinates(), msg->getStationId(), msg->getForCtsSourceCharacter());
}

// ----------------------------------------------------------------------

void PlanetServer::findOrLoadCharacter(NetworkId const &characterId, NetworkId const &containerId, Vector const &coords, uint32 stationId, bool forCtsSourceCharacter)
{
	// put in map along with expected response count
	CharacterFindInfo &cfi = (*m_characterFindMap)[characterId];
	cfi.sequence = ++m_characterFindSequence;
	cfi.containerId = containerId;
	cfi.coords = coords;
	cfi.stationId = stationId;
	cfi.pendingResponseCount = static_cast<int>(m_gameServers.size());
	cfi.forCtsSourceCharacter = forCtsSourceCharacter;

	// message all game servers for this planet asking if they have this auth character
	GenericValueTypeMessage<std::pair<NetworkId, unsigned int> > const msg("FindAuthObject", std::make_pair(characterId, cfi.sequence));
	for (GameServerMapType::iterator i = m_gameServers.begin(); i != m_gameServers.end(); ++i)
		(*i).second->getConnection()->send(msg, true);
}

// ----------------------------------------------------------------------

void PlanetServer::handleFindAuthObjectResponse(NetworkId const &characterId, unsigned int sequence, bool found, uint32 gameServerId)
{
	std::map<NetworkId, CharacterFindInfo>::iterator i = m_characterFindMap->find(characterId);
	if (i != m_characterFindMap->end() && (*i).second.sequence == sequence)
	{
		if (found)
		{
			LOG(loginTrace, ("Send GameServerForLogin(%s) to CentralServer for object found on game server", characterId.getValueString().c_str()));
			GameServerForLoginMessage const msg((*i).second.stationId, gameServerId, characterId);
			sendToCentral(msg, true);
			m_characterFindMap->erase(i);
		}
		else if (--((*i).second.pendingResponseCount) <= 0)
		{
			forceLoadCharacter(characterId, (*i).second.containerId, (*i).second.coords, (*i).second.stationId, (*i).second.forCtsSourceCharacter);
			m_characterFindMap->erase(i);
		}
	}
}

// ----------------------------------------------------------------------

void PlanetServer::forceLoadCharacter(NetworkId const &characterId, NetworkId const &containerId, Vector const &coords, uint32 stationId, bool forCtsSourceCharacter)
{
	Node *node = Scene::getInstance().findNodeByPosition(static_cast<int>(coords.x), static_cast<int>(coords.z));
	NOT_NULL(node);

	uint32 gameServerId=Scene::getInstance().getGameServerForObject(containerId);
	if (gameServerId==0)
	{
		if (!node->isLoaded())
			node->load();
		gameServerId=node->getPreferredServer();
		WARNING_DEBUG_FATAL(gameServerId==0,("Programmer bug:  getPreferredGameServer returned 0 even after we forced the node to be loaded."));
	}

	if (gameServerId == 0) //lint !e774 // boolean is always false (only in debug mode)
	{
		DEBUG_REPORT_LOG(true, ("Waiting for game server for login\n"));
		m_messagesWaitingForGameServer.push_back(
			new RequestGameServerForLoginMessage(
				stationId,
				characterId,
				containerId,
				ConfigPlanetServer::getSceneID(),
				coords,
				forCtsSourceCharacter));
	}
	else
	{
		if (!node->isLoaded())
			node->load();

		loadCharacterForLogin(characterId, coords, gameServerId);

		LOG(loginTrace, ("Send GameServerForLogin(%s) to CentralServer for object being loaded", characterId.getValueString().c_str()));
		GameServerForLoginMessage const msg(stationId, gameServerId, characterId);
		sendToCentral(msg, true);
	}
}

// -------------------------------------------------------------------------------

void PlanetServer::loadCharacterForLogin(NetworkId const &characterId, Vector const &coords, unsigned long gameServerId)
{
	LOG(loginTrace, ("Character not loaded, sending PlanetLoadCharacter(%s, gs %d) to CentralServer", characterId.getValueString().c_str(), gameServerId));

	// unload if we think it is loaded
	PlanetProxyObject *object = Scene::getInstance().findObjectByID(characterId);
	if (object)
		object->unload(true);

	Node *node = Scene::getInstance().findNodeByPosition(static_cast<int>(coords.x), static_cast<int>(coords.z));
	NOT_NULL(node);

	PlanetLoadCharacterMessage const msg(characterId, gameServerId);
	sendToCentral(msg, true);
}

// -------------------------------------------------------------------------------

void PlanetServer::handleRequestSceneTransfer(const RequestSceneTransfer *& msg)
{
	NOT_NULL(msg);

	if (!PreloadManager::getInstance().isPreloadComplete())
	{
		DEBUG_REPORT_LOG(true, ("Waiting for game server for login\n"));
		m_messagesWaitingForGameServer.push_back(msg);
		return;
	}

	const Vector &coords = msg->getPosition_w();
	Node *node = Scene::getInstance().findNodeByPosition(static_cast<int>(coords.x), static_cast<int>(coords.z));
	NOT_NULL(node);

	uint32 gameServerId=Scene::getInstance().getGameServerForObject(msg->getContainerId());

	if (!node->isLoaded())
	{
		node->load();
		m_sceneTransferChunkLoads->push_back(msg);
	}
	else
	{
		if (gameServerId == 0)
			gameServerId=node->getPreferredServer();
		WARNING_DEBUG_FATAL(gameServerId==0,("Programmer bug:  getPreferredGameServer returned 0 even after we forced the node to be loaded."));

		SceneTransferMessage const reply(*msg, gameServerId);
		sendToCentral(reply, true);
		delete msg; //lint !e605  Deleting const pointer
		msg = 0;
	}
}

// -------------------------------------------------------------------------------

void PlanetServer::handleChunkComplete(const ChunkCompleteMessage &msg, const GameServerConnection *conn)
{
	NOT_NULL(conn);

	std::vector<std::pair<int, int> > const & chunks = msg.getChunks();
	for (std::vector<std::pair<int, int> >::const_iterator chunk=chunks.begin(); chunk!=chunks.end(); ++chunk)
	{
		int nodeX = chunk->first;
		int nodeZ = chunk->second;

		std::list<const RequestSceneTransfer*>::iterator i = m_sceneTransferChunkLoads->begin();
		while (i != m_sceneTransferChunkLoads->end())
		{
			const RequestSceneTransfer *transferMsg = *i;
			const Vector &coords = transferMsg->getPosition_w();
			if (   Node::roundToNode(static_cast<int>(coords.x)) == nodeX
				   && Node::roundToNode(static_cast<int>(coords.z)) == nodeZ)
			{
				SceneTransferMessage const reply(*transferMsg, conn->getProcessId());
				sendToCentral(reply, true);
				delete transferMsg; //lint !e605  Deleting const pointer
				IGNORE_RETURN(m_sceneTransferChunkLoads->erase(i++));
			}
			else
				++i;
		}
	}
}

// -------------------------------------------------------------------------------

/**
 * Start game servers.  If there are previous requests to start game servers that
 * haven't been handled yet, try to start those, too.  If the game servers can't
 * be started right now, queue them for later.
 */
void PlanetServer::startGameServer(const std::set<PreloadServerId> & preloadServerId, const GameServerSpawnDelaySeconds spawnDelay)
{
	if(m_taskManagerConnection && m_centralServerConnection)
	{
		for (std::set<PreloadServerId>::const_iterator it = preloadServerId.begin(); it != preloadServerId.end(); ++it)
			(*m_pendingServerStarts)[*it] = spawnDelay;

		for (std::map<PreloadServerId, GameServerSpawnDelaySeconds>::const_iterator i=m_pendingServerStarts->begin(); i!=m_pendingServerStarts->end(); ++i)
		{
			static int cookie = 0;
			static char numberBuf[32];
			// build the spawn command for the new game server
			std::string options = "-s GameServer centralServerAddress=";
			options += m_centralServerConnection->getRemoteAddress();

			static int portBase = ConfigPlanetServer::getGameServerDebuggingPortBase();
			portBase++;
			// activate remote debugging in the spawn request if the planetserver
			// is configured to start game servers with java debugging enabled
			if(ConfigPlanetServer::getGameServerDebugging())
			{
				snprintf(numberBuf, sizeof(numberBuf), "%d", 58000 + portBase);
				options += " javaDebugPort=";
				options += numberBuf;
				options += " useRemoteDebugJava=true";
			}
			// only profiling or debugging is allowed, not both
			else if(ConfigPlanetServer::getGameServerProfiling())
			{
				snprintf(numberBuf, sizeof(numberBuf), "%d", portBase);
				options += " javaOptions=-Xrunhprof:cpu=times,thread=y,cutoff=0,depth=10,format=a,file=scriptprofile_";
				options += numberBuf;
				options += ".txt";
				options += " javaOptions=-Djava.compiler=NONE";
			}

			options += " preloadNumber=";
			snprintf(numberBuf, sizeof(numberBuf), "%lu", i->first);
			options += numberBuf;
			
			options += " sceneID=";
			options += Scene::getInstance().getSceneId();
			
			options += " groundScene=terrain/";
			options += Scene::getInstance().getSceneId();
			options += ".trn";
			
			options += " -s ServerUtility spawnCookie=";
			snprintf(numberBuf, sizeof(numberBuf), "%i", cookie);
			options += numberBuf;

			TaskSpawnProcess spawn("any", "SwgGameServer", options, i->second);
			if (m_centralServerConnection)
			{
				(*m_startingGameServers)[cookie] = std::make_pair(options, ::time(nullptr));
				++cookie;
				m_centralServerConnection->send(spawn,true);
				DEBUG_REPORT_LOG(true, ("Sent start gameserver request for scene %s\n", ConfigPlanetServer::getSceneID()));
				LOG("PlanetServerConnections", ("Sent start gameserver request for scene %s", ConfigPlanetServer::getSceneID()));
			}
			else
			{
				FATAL(true, ("Cannot start up without central server connection"));
			}
		}

		m_pendingServerStarts->clear();
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("Adding pending server on scene %s\n", ConfigPlanetServer::getSceneID()));

		for (std::set<PreloadServerId>::const_iterator it = preloadServerId.begin(); it != preloadServerId.end(); ++it)
			(*m_pendingServerStarts)[*it] = spawnDelay;
	}
}

// ----------------------------------------------------------------------

GameServerData *PlanetServer::getGameServerData(uint32 serverId) const
{
	GameServerMapType::const_iterator i = m_gameServers.find(serverId);
	if (i!=m_gameServers.end())
		return i->second;
	else
		return nullptr;
}

// ----------------------------------------------------------------------

void PlanetServer::preloadCompleteOnAllServers()
{
	flushWaitingMessages();
	Scene::getInstance().checkServerAssignments();
	PreloadFinishedMessage const msg(true);
	sendToCentral(msg, true);
}

// ----------------------------------------------------------------------

/**
 * Send all messages that were queued waiting for game servers to be ready.
 */
void PlanetServer::flushWaitingMessages()
{
	if (!PreloadManager::getInstance().isPreloadComplete())
		return; // don't handle any logins until preloading is finished

	DEBUG_REPORT_LOG(true,("Handling pending logins and transfers.\n"));
	std::vector<const GameNetworkMessage *>::iterator t;
	for(t = m_messagesWaitingForGameServer.begin(); t != m_messagesWaitingForGameServer.end();)
	{
		const GameNetworkMessage * msg = *t;
		t = m_messagesWaitingForGameServer.erase(t);
		const RequestSceneTransfer * sceneTransfer = dynamic_cast<const RequestSceneTransfer *>(msg);
		if (sceneTransfer)
			handleRequestSceneTransfer(sceneTransfer);
		else
		{
			const RequestGameServerForLoginMessage * loginMessage = dynamic_cast<const RequestGameServerForLoginMessage *>(msg);
			if (loginMessage)
				handleRequestGameServerForLoginMessage(loginMessage);
			else
			{
				WARNING_STRICT_FATAL(true, ("Invalid message type in pending message queue for Game Server"));
				delete msg;  //lint !e605  Deleting const pointer
			}
		}
	}
}

// ----------------------------------------------------------------------

int PlanetServer::getObjectCountForServer(uint32 serverId) const
{
	GameServerMapType::const_iterator i=m_gameServers.find(serverId);
	if (i!=m_gameServers.end())
		return i->second->getObjectCount();
	else
		return 0;
}

// ----------------------------------------------------------------------

int PlanetServer::getInterestObjectCountForServer(uint32 serverId) const
{
	GameServerMapType::const_iterator i=m_gameServers.find(serverId);
	if (i!=m_gameServers.end())
		return i->second->getInterestObjectCount();
	else
		return 0;
}

// ----------------------------------------------------------------------

int PlanetServer::getInterestCreatureObjectCountForServer(uint32 serverId) const
{
	GameServerMapType::const_iterator i=m_gameServers.find(serverId);
	if (i!=m_gameServers.end())
		return i->second->getInterestCreatureObjectCount();
	else
		return 0;
}

// ----------------------------------------------------------------------

void PlanetServer::getObjectCountsForAllServers(std::map<uint32, GameServerData> &counts) const
{
	for (GameServerMapType::const_iterator i=m_gameServers.begin(); i!=m_gameServers.end(); ++i)
		counts[i->first]=*(i->second);
}

// ----------------------------------------------------------------------

void PlanetServer::queueMessageForObject(const NetworkId &networkId, GameNetworkMessage *theMessage)
{
	if (ConfigPlanetServer::getLogObjectLoading())
		LOG("ObjectLoading",("Queueing message for object %s because authority change is in progress.",networkId.getValueString().c_str()));
	std::vector<GameNetworkMessage *> & v = m_queuedMessages[networkId];
	v.push_back(theMessage);
}

// ----------------------------------------------------------------------

void PlanetServer::sendQueuedMessagesForObject(const PlanetProxyObject &theObject)
{
	uint32 server = theObject.getAuthoritativeServer();
	QueuedMessagesType::iterator i=m_queuedMessages.find(theObject.getObjectId());
	if (i!=m_queuedMessages.end())
	{
		if (ConfigPlanetServer::getLogObjectLoading())
			LOG("ObjectLoading",("Sending queued message for object %s.",theObject.getObjectId().getValueString().c_str()));
		std::vector<GameNetworkMessage *> & v = i->second;
		for (std::vector<GameNetworkMessage *>::iterator j=v.begin(); j!=v.end(); ++j)
		{
			sendToGameServer(server,**j);
			delete *j;
		}

		m_queuedMessages.erase(i);
	}
}

// ----------------------------------------------------------------------

void PlanetServer::setDone(char const *reasonfmt, ...)
{
	if (!m_done)
	{
		char reason[1024];
		va_list ap;
		va_start(ap, reasonfmt);
		_vsnprintf(reason, sizeof(reason), reasonfmt, ap);
		reason[sizeof(reason)-1] = '\0';

		LOG(
			"ServerShutdown",
			(
				"PlanetServer (pid %d) shutdown, reason: %s",
				static_cast<int>(Os::getProcessId()),
				reason));

		REPORT_LOG(
			true,
			(
				"PlanetServer (pid %d) shutdown, reason: %s\n",
				static_cast<int>(Os::getProcessId()),
				reason));
		
		va_end(ap);
		m_done = true;
	}
}

// ----------------------------------------------------------------------

void PlanetServer::handleDatabaseSaveComplete(int newSaveCounter)
{
	WARNING_DEBUG_FATAL(m_lastSaveCounter>newSaveCounter,("The database sent a save complete message with a counter that was lower than the previous value."));
	m_lastSaveCounter = newSaveCounter;
	if ((m_waitForSaveCounter!=0) && (m_lastSaveCounter >= m_waitForSaveCounter))
		m_waitForSaveCounter = 0;
}

// ----------------------------------------------------------------------

void PlanetServer::sendChunkRequest(uint32 server, int x, int z)
{
	m_pendingChunkRequests.push_back(RequestChunkMessage::Chunk(server,x,z));
}

// ----------------------------------------------------------------------

void PlanetServer::sendPreloadRequestCompleteMessage(uint32 realServerId, uint32 preloadServerId)
{
	m_pendingPreloadRequestCompletes.push_back(std::make_pair(realServerId, preloadServerId));
}

// ----------------------------------------------------------------------

/**
 * Request a database save and delay all new loads until it finishes.
 */
void PlanetServer::requestAndWaitForSave()
{
	m_waitForSaveCounter = m_lastSaveCounter+1;
	GameNetworkMessage const msg("PlanetRequestSave");
	sendToCentral(msg, true);
}

// ----------------------------------------------------------------------

bool PlanetServer::getEnablePreload() const
{
	return (ConfigPlanetServer::getLoadWholePlanetMultiserver() || ConfigPlanetServer::getLoadWholePlanet() || isInSpaceMode());
}

// ======================================================================
