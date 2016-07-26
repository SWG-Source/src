// CentralServer.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "CentralServer.h"

#include "CentralCSHandler.h"
#include "CentralServerMetricsData.h"
#include "CharacterCreationTracker.h"
#include "ChatServerConnection.h"
#include "ConfigCentralServer.h"
#include "ConnectionServerConnection.h"
#include "ConsoleConnection.h"
#include "ConsoleManager.h"
#include "CustomerServiceConnection.h"
#include "fileInterface/StdioFile.h"
#include "GameServerConnection.h"
#include "LoginServerConnection.h"
#include "PlanetManager.h"
#include "PlanetServerConnection.h"
#include "TaskConnection.h"
#include "UnicodeUtils.h"
#include "UniverseManager.h"
#include "serverKeyShare/KeyShare.h"
#include "serverMetrics/MetricsManager.h"
#include "serverNetworkMessages/AccountFeatureIdResponse.h"
#include "serverNetworkMessages/AddCharacterMessage.h"
#include "serverNetworkMessages/AddObjectIdBlockMessage.h"
#include "serverNetworkMessages/AdjustAccountFeatureIdResponse.h"
#include "serverNetworkMessages/CentralConnectionServerMessages.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/CentralPingMessage.h"
#include "serverNetworkMessages/CentralPlanetServerConnect.h"
#include "serverNetworkMessages/CentralTaskMessages.h"
#include "serverNetworkMessages/CharacterListMessage.h"
#include "serverNetworkMessages/ChatServerOnline.h"
#include "serverNetworkMessages/ChunkObjectListMessage.h"
#include "serverNetworkMessages/ClaimRewardsMessage.h"
#include "serverNetworkMessages/ClaimRewardsReplyMessage.h"
#include "serverNetworkMessages/ConnectionServerDown.h"
#include "serverNetworkMessages/CreateNewObjectMessage.h"
#include "serverNetworkMessages/EnumerateServers.h"
#include "serverNetworkMessages/ExchangeListCreditsMessage.h"
#include "serverNetworkMessages/ExcommunicateGameServerMessage.h"
#include "serverNetworkMessages/FailedToLoadObjectMessage.h"
#include "serverNetworkMessages/FeatureIdTransactionResponse.h"
#include "serverNetworkMessages/ForceUnloadObjectMessage.h"
#include "serverNetworkMessages/GameServerForLoginMessage.h"
#include "serverNetworkMessages/GameServerReadyMessage.h"
#include "serverNetworkMessages/GameServerUniverseLoadedMessage.h"
#include "serverNetworkMessages/GameTaskManagerMessages.h"
#include "serverNetworkMessages/LoadObjectMessage.h"
#include "serverNetworkMessages/LocateStructureMessage.h"
#include "serverNetworkMessages/LoginConnectionServerAddress.h"
#include "serverNetworkMessages/LoginCreateCharacterAckMessage.h"
#include "serverNetworkMessages/LoginCreateCharacterMessage.h"
#include "serverNetworkMessages/LoginKeyPush.h"
#include "serverNetworkMessages/LoginRestoreCharacterMessage.h"
#include "serverNetworkMessages/LoginUpgradeAccountMessage.h"
#include "serverNetworkMessages/MessageToAckMessage.h"
#include "serverNetworkMessages/MessageToMessage.h"
#include "serverNetworkMessages/PlanetLoadCharacterMessage.h"
#include "serverNetworkMessages/PopulationListMessage.h"
#include "serverNetworkMessages/PreloadFinishedMessage.h"
#include "serverNetworkMessages/PreloadRequestCompleteMessage.h"
#include "serverNetworkMessages/ProfilerOperationMessage.h"
#include "serverNetworkMessages/RandomName.h"
#include "serverNetworkMessages/ReleaseAuthoritativeMessage.h"
#include "serverNetworkMessages/RenameCharacterMessage.h"
#include "serverNetworkMessages/RequestChunkMessage.h"
#include "serverNetworkMessages/RequestGameServerForLoginMessage.h"
#include "serverNetworkMessages/RequestObjectIdsMessage.h"
#include "serverNetworkMessages/RequestObjectMessage.h"
#include "serverNetworkMessages/RestartServerMessage.h"
#include "serverNetworkMessages/SceneTransferMessages.h"
#include "serverNetworkMessages/ServerDeleteCharacterMessage.h"
#include "serverNetworkMessages/SetConnectionServerPublic.h"
#include "serverNetworkMessages/SetObjectPositionMessage.h"
#include "serverNetworkMessages/TaskProcessDiedMessage.h"
#include "serverNetworkMessages/TaskSpawnProcess.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/UnloadObjectMessage.h"
#include "serverNetworkMessages/UpdateLoginConnectionServerStatus.h"
#include "serverNetworkMessages/UpdatePlayerCountMessage.h"
#include "serverNetworkMessages/ValidateAccountMessage.h"
#include "serverNetworkMessages/ValidateAccountReplyMessage.h"
#include "serverNetworkMessages/ValidateCharacterForLoginMessage.h"
#include "serverNetworkMessages/ValidateCharacterForLoginReplyMessage.h"
#include "serverNetworkMessages/VerifyAndLockName.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "serverUtility/ClusterWideDataManagerList.h"
#include "serverUtility/ServerClock.h"
#include "serverUtility/SetupServerUtility.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Timer.h"
#include "sharedLog/Log.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/NameErrors.h"
#include "sharedNetworkMessages/NetworkStringIds.h"
#include "sharedUtility/SetupSharedUtility.h"
#include "sharedUtility/StartingLocationData.h"
#include "sharedUtility/StartingLocationManager.h"
#include "TransferServerConnection.h"
#include "StationPlayersCollectorConnection.h"
#include "AuctionTransferClient.h"
#include "unicodeArchive/UnicodeArchive.h"
#include <stdio.h>


// Trying todo something here ...
#include "webAPI.h"

namespace CentralServerNamespace
{
	bool gs_connectionServersPublic=false;
	bool gs_clusterIsLocked=false;
	bool gs_clusterIsSecret=false;

	std::map<std::string, std::string> ms_sceneToHostMap;

	// every connection server started on the cluster will
	// have a entry in this vector which contains the node
	// on which to start that particular connection server;
	// every connection server that is started will be assigned
	// a 1-based number, which corresponds to its position
	// in this vector; that is, the connection server assigned
	// number 1 will be at position 0 in this vector
	std::vector<std::string> s_connectionServerHostList;

	std::pair<std::string, unsigned short> s_customerServiceServerChatServerServiceAddress;
	std::pair<std::string, unsigned short> s_customerServiceServerGameServerServiceAddress;
	Service * s_commodityServerService = 0;
	bool s_retryTransferServerConnection = false;
	bool s_retryStationPlayersCollectorConnection = false;

	std::map<StationId, uint32> m_purgeAccountToLoginServerMap;
	std::map<NetworkId, std::pair<time_t, int> > s_pendingRenameCharacter;
}

using namespace CentralServerNamespace;

// ======================================================================

CentralServer::CentralObject::CentralObject() :
m_sceneId(),
m_authoritativeProcess(0),
m_proxyProcessList()
{

}

//-----------------------------------------------------------------------

CentralServer::CentralObject::CentralObject(const CentralServer::SceneId & sceneId, uint32 authProcess) :
m_sceneId(sceneId),
m_authoritativeProcess(authProcess),
m_proxyProcessList()
{

}

//-----------------------------------------------------------------------

CentralServer::CentralServer() :
Singleton<CentralServer>(),
MessageDispatch::Receiver(),
m_connectionServerConnections(),
m_chatServerConnections(),
m_csServerConnections(),
m_gameServers(),
m_accountConnectionMap(),
m_dbProcessServerProcessId(0),
m_done(false),
m_gameService(0),
m_chatService(0),
m_connService(0),
m_csService(0),
m_planetService(0),
m_consoleService(0),
m_transferServerConnection(0),
m_stationPlayersCollectorConnection(0),
m_gameServerConnections(),
m_gameServerConnectionsList(),
m_playerSceneMap(),
m_loginServerConnectionMap(),
m_loginServerKeys(0),
m_loginService(0),
m_pAuctionTransferClient(0),
m_messagesWaitingForPlanetServer(),
m_pendingLoadingObjects(),
m_pendingNewObjects(),
m_nextFreeProcessId(0),
m_taskManager(0),
m_taskService(0),
m_commandLine(),
m_pendingTransfers(),
m_totalPlayerCount(0),
m_totalFreeTrialCount(0),
m_totalEmptySceneCount(0),
m_totalTutorialSceneCount(0),
m_totalFalconSceneCount(0),
m_metricsData(0),
m_pendingPlanetServers(),
m_planetsWaitingForPreload(),
m_planetServers(),
m_nextPlanetWatcherPort(ConfigCentralServer::getFirstPlanetWatcherPort()),
m_databaseBacklogged(false),
m_lastLoadingStateTime(0),
m_timeClusterStarted(time(0)),
m_clusterStartupTime(-1),
m_timeClusterWentIntoLoadingState(time(0)),
m_clusterId(0),
m_serverPings(),
m_shutdownTotalTime(0),
m_shutdownMaxTime(0),
m_shutdownSystemMessage(Unicode::narrowToWide("")),
m_shutdownPhase(0),
m_shutdownHaveDatabaseSaveStart(false),
m_shutdownHaveDatabaseComplete(false),
m_curTime(0),
m_lastTimeSystemTimeMismatchNotification(0),
m_lastTimeSystemTimeMismatchNotificationDescription(),
m_disconnectedTaskManagerList(),
m_populationStatistics(),
m_timePopulationStatisticsRefresh(0),
m_timePopulationStatisticsNextRefresh(0),
m_gcwScoreStatistics(),
m_timeGcwScoreStatisticsRefresh(0),
m_timeGcwScoreStatisticsNextRefresh(0),
m_lastLoginTimeStatistics(),
m_createTimeStatistics(),
m_timeLastLoginTimeStatisticsRefresh(0),
m_timeLastLoginTimeStatisticsNextRefresh(0),
m_numberOfCharacterMatchRequests(0),
m_numberOfCharacterMatchResults(0),
m_timeSpentOnCharacterMatchRequestsMs(0),
m_timeCharacterMatchStatisticsNextRefresh(0)
{
	m_curTime = static_cast<uint32>(time(0));
	m_loginServerKeys = new KeyServer(20);

	connectToMessage("LoginKeyPush");

	//Connection Messages
	connectToMessage("ClientConnectionClosed"); // from connection server
	connectToMessage("ConnectionOpened");
	connectToMessage("RequestClusterShutdown");
	connectToMessage("GameConnectionClosed");
	connectToMessage("CentralGameServerConnect");
	connectToMessage("CentralGameServerDbProcessServerProcessId");
	connectToMessage("LoginConnectionOpened");
	connectToMessage("LoginConnectionClosed");
	connectToMessage("ClusterId");
	connectToMessage("GameServerReadyMessage");
	connectToMessage("GameServerUniverseLoadedMessage");
	connectToMessage("ConnectionServerConnectionClosed");
	connectToMessage("ConnectionServerConnectionOpened");
	connectToMessage("TaskConnectionClosed");
	connectToMessage("TaskConnectionOpened");
	connectToMessage("RandomNameRequest"); // from connection server
	connectToMessage("RandomNameResponse"); // from game server
	connectToMessage("VerifyAndLockNameRequest"); // from connection server	
	connectToMessage("VerifyAndLockNameResponse"); // from game server	
	
	//Object Messages
	connectToMessage("RequestObjectMessage");
	connectToMessage("RequestChunkMessage");
	connectToMessage("ChunkObjectListMessage");
	connectToMessage("CreateNewObjectMessage");
	connectToMessage("SetObjectPositionMessage");
	connectToMessage("FailedToLoadObjectMessage");
	connectToMessage("ReleaseAuthoritativeMessage");
	connectToMessage("ForceUnloadObjectMessage");
	connectToMessage("RequestUnloadObjectMessage");
	connectToMessage("MessageToMessage");
	connectToMessage("MessageToAckMessage");
	connectToMessage("LocateStructureMessage");

	//Create character messages
	connectToMessage("ConnectionCreateCharacter");
	connectToMessage("DatabaseCreateCharacterSuccess");
	connectToMessage("GameCreateCharacterFailed");
	connectToMessage("RequestGameServerForLoginMessage");
	connectToMessage("GameServerForLoginMessage");
	connectToMessage("LoginCreateCharacterAckMessage");
	connectToMessage("LoginUpgradeAccountMessage");
	connectToMessage("LoginRestoreCharacterMessage");
	connectToMessage("NewCharacterCreated");

	connectToMessage("ServerDeleteCharacterMessage");
	connectToMessage("ValidateCharacterForLoginMessage");
	connectToMessage("ValidateCharacterForLoginReplyMessage");
	connectToMessage("EnableCharacterMessage");
	connectToMessage("EnableCharacterReplyMessage");
	connectToMessage("DisableCharacterMessage");
	connectToMessage("DisableCharacterReplyMessage");

	connectToMessage("RequestOIDsMessage");
	connectToMessage("AddOIDBlockMessage");
	connectToMessage("CharacterListMessage");
	connectToMessage("LoggedInMessage");
	connectToMessage("CentralPlanetServerConnect");
	connectToMessage("PlanetCreateProxy");
	connectToMessage("PlanetRemoveProxy");

	connectToMessage("RequestSceneTransfer");
	connectToMessage("SceneTransferMessage");
	connectToMessage("PlanetLoadCharacterMessage");

	connectToMessage("ConnSrvDropDupeConns");
	connectToMessage("CntrlSrvDropDupeConns");

	connectToMessage("PlayerDroppedFromGameServerCrash");

	// Station exchange messages
	connectToMessage("ExchangeListCreditsMessage");

	// Chat Server messages
	connectToMessage("ChatServerConnectionOpened");
	connectToMessage("ChatServerConnectionClosed");
	connectToMessage("ChatServerOnline");

	// Chat Server messages
	connectToMessage("CustomerServiceConnectionOpened");
	connectToMessage("CustomerServiceConnectionClosed");

	connectToMessage("ValidateAccountMessage");
	connectToMessage("ValidateAccountReplyMessage");

	connectToMessage("RequestPreloadListMessage");
	connectToMessage("PreloadListMessage");
	connectToMessage("PreloadRequestCompleteMessage");
	connectToMessage("PreloadFinishedMessage");
	connectToMessage("DatabaseBackloggedMessage");

	connectToMessage("UpdatePlayerCountMessage");
	connectToMessage("RenameCharacterMessageEx");
	connectToMessage("PlayerRenameRequestSubmitted");
	connectToMessage("PlayerRenameRequestCompleted");
	connectToMessage("PopulationListMessage");

	connectToMessage("SetConnectionServerPublic");

	connectToMessage("ProfilerOperationMessage");
	connectToMessage("CentralPingMessage");

	connectToMessage("DatabaseSaveStart");
	connectToMessage("DatabaseSaveComplete");
	connectToMessage("ShutdownCluster");
	connectToMessage("AbortShutdown");
	connectToMessage("SetSceneForPlayer");
	connectToMessage("DatabaseConsoleReplyMessage");
	connectToMessage("TaskProcessDiedMessage");
	connectToMessage("SystemTimeMismatchNotification");
	connectToMessage("DisconnectedTaskManagerMessage");
	connectToMessage("TransferServerConnectionClosed");
	connectToMessage("PlanetRequestSave");
	connectToMessage("TransferReplyLoginLocationData");
	connectToMessage("CustomerServiceServerChatServerServiceAddress");
	connectToMessage("CustomerServiceServerGameServerServiceAddress");
	connectToMessage("ChatClosedConnectionWithGameServer");

	connectToMessage("ReconnectToTransferServer");
	connectToMessage("ClaimRewardsMessage");
	connectToMessage("ClaimRewardsReplyMessage");
	connectToMessage("PurgeStructuresForAccountMessage");
	connectToMessage("WarnStructuresAboutPurgeMessage");
	connectToMessage("PurgeCompleteMessage");
	connectToMessage("RestartServerMessage");
	connectToMessage("RestartServerByRoleMessage");
	connectToMessage("ExcommunicateGameServerMessage");
	connectToMessage("RestartPlanetMessage");
	
	// Cluster state
	connectToMessage("UpdateClusterLockedAndSecretState");

	// Population statistics
	connectToMessage("PopStatRsp");
	connectToMessage("LLTStatRsp");

	// GCW score statistics
	connectToMessage("GcwScoreStatRsp");

	// Character Search statistics
	connectToMessage("LfgStatRsp");

	// Account feature Id management
	connectToMessage("AdjustAccountFeatureIdResponse");
	connectToMessage("AccountFeatureIdResponse");
	connectToMessage("FeatureIdTransactionResponse");

	// Unlocked Slot management
	connectToMessage("OccupyUnlockedSlotRsp");
	connectToMessage("VacateUnlockedSlotRsp");
	connectToMessage("SwapUnlockedSlotRsp");

	// Cluster wide data manager
	ClusterWideDataManagerList::install();
	ClusterWideDataManagerList::registerMessage(*this);
	ClusterWideDataManagerList::setLockTimeoutValue(ConfigCentralServer::getClusterWideDataLockTimeout());

	CentralCSHandler::install();

	gs_connectionServersPublic = ConfigCentralServer::getStartPublic();

	m_metricsData = new CentralServerMetricsData;
	MetricsManager::install(m_metricsData, false, "CentralServer", "", 0);
	ConsoleManager::install();
	CharacterCreationTracker::install();

	IGNORE_RETURN(UniverseManager::getInstance()); //make sure it's constructed
}

//-----------------------------------------------------------------------

CentralServer::~CentralServer()
{
	ConsoleManager::remove();
	
	CentralCSHandler::remove();

	ms_sceneToHostMap.clear();

	// released during CentralServer::remove()
	m_gameService = 0;
	m_chatService = 0;
	m_connService = 0;
	m_csService = 0;
	m_planetService = 0;
	m_consoleService = 0;
	m_transferServerConnection = 0;
	m_stationPlayersCollectorConnection = 0;
	m_loginServerKeys = 0; //lint !e423 : leak
	m_loginService = 0;
	m_taskManager = 0;
	m_taskService = 0;
	m_metricsData = 0; //lint !e423 : leak
}

//-----------------------------------------------------------------------
void CentralServer::addGameServer(GameServerConnection * newGameServer)
{
	uint32	pid = newGameServer->getProcessId();

	LOG("ServerStartup", ("Adding server %lu for scene %s", pid, newGameServer->getSceneId().c_str()));

	IGNORE_RETURN(m_gameServers.insert(SceneGameMap::value_type(
		newGameServer->getSceneId(), newGameServer)));
	m_gameServerConnections[pid] = newGameServer;

	if (newGameServer->getSceneId() != "database")
	{
		m_gameServerConnectionsList.push_back(newGameServer);

		if (ConfigCentralServer::getGameServerConnectionPendingAllocatedSizeLimit() != 0)
			newGameServer->setTcpClientPendingSendAllocatedSizeLimit(ConfigCentralServer::getGameServerConnectionPendingAllocatedSizeLimit());

		PlanetManager::addGameServerForScene(newGameServer->getSceneId(), newGameServer);

		// Have the new game server connect to the db and notify it of what game servers exist
		std::vector<uint32> serverList;

		GameServerConnection const * const databaseProcessConnection = CentralServer::getInstance().getGameServer(CentralServer::getInstance().getDbProcessServerProcessId());

		FATAL(!databaseProcessConnection, ("Tried to add a game server without a database process connection?"));

		for (std::map<uint32, GameServerConnection *>::iterator i = m_gameServerConnections.begin(); i != m_gameServerConnections.end(); ++i)
		{
			GameServerConnection const * const g = (*i).second;
			if (g != databaseProcessConnection && g->getProcessId() != pid)
				serverList.push_back(g->getProcessId());
		}

		GenericValueTypeMessage<std::pair<std::vector<uint32>, std::pair<uint32, std::pair<std::string, uint16> > > > const setupMessage(
			"GameServerSetupMessage",
			std::make_pair(
				serverList,
				std::make_pair(
					databaseProcessConnection->getProcessId(),
					std::make_pair(
						databaseProcessConnection->getGameServiceAddress(),
						databaseProcessConnection->getGameServicePort()))));

		newGameServer->send(setupMessage, true);
	}
}

//-----------------------------------------------------------------------

void CentralServer::broadcastToChatServers(const GameNetworkMessage & message) const
{
	for (std::set<ChatServerConnection *>::const_iterator i = m_chatServerConnections.begin(); i != m_chatServerConnections.end(); ++i)
		(*i)->send(message, true);
}

//-----------------------------------------------------------------------

void CentralServer::broadcastToCustomerServiceServers(const GameNetworkMessage & message) const
{
	for (std::set<CustomerServiceConnection *>::const_iterator i = m_csServerConnections.begin(); i != m_csServerConnections.end(); ++i)
		(*i)->send(message, true);
}

//-----------------------------------------------------------------------

void CentralServer::broadcastToGameServers(const GameNetworkMessage & message) const
{
	for (SceneGameMap::const_iterator i = m_gameServers.begin(); i != m_gameServers.end(); ++i)
		(*i).second->send(message, true);
}

//-----------------------------------------------------------------------
// /**
//  * Clears the object process map for an object.
//  *
//  * @param objectId		id of object we want to clear
//  */
// void CentralServer::clearObjectMap(const NetworkId & objectId)
// {
// 	IGNORE_RETURN(m_centralObjectMap.erase(objectId));
// }

//-----------------------------------------------------------------------

bool CentralServer::getIsClusterPublic() const
{
	return gs_connectionServersPublic;
}

//-----------------------------------------------------------------------

bool CentralServer::getIsClusterLocked() const
{
	return gs_clusterIsLocked;
}

//-----------------------------------------------------------------------

bool CentralServer::getIsClusterSecret() const
{
	return gs_clusterIsSecret;
}

//-----------------------------------------------------------------------

const std::string& CentralServer::getHostForScene(const std::string& scene) const
{
	static std::string anyString = "any";
	std::map<std::string, std::string>::iterator i = ms_sceneToHostMap.find(scene);
	if (i != ms_sceneToHostMap.end())
	{
		return i->second;
	}
	return anyString;
}

// ----------------------------------------------------------------------

/**
 * Get the list of game servers that are ready
 */
void CentralServer::getReadyGameServers(std::vector<uint32> &theList)
{
	std::map<uint32, GameServerConnection *>::const_iterator i;
	for (i = m_gameServerConnections.begin(); i!=m_gameServerConnections.end(); ++i)
	{
		if ((*i).second->getProcessId()!=getDbProcessServerProcessId() && i->second->getReady())
			theList.push_back(i->second->getProcessId());
	}
}

//-------------------------------------------------------------

GameServerConnection * CentralServer::getRandomGameServer(void)
{
	if (m_gameServerConnectionsList.empty())
		return nullptr;

	// m_gameServerConnectionsList ***DOES NOT*** contain the DB server so
	// we don't have to worry about checking for and excluding the DB server
	size_t const listSize = m_gameServerConnectionsList.size();
	size_t const indexChosenGameServer = (::rand() % listSize);
	if (m_gameServerConnectionsList[indexChosenGameServer]->getReady())
		return m_gameServerConnectionsList[indexChosenGameServer];

	// randomly chosen game server is not ready, find next ready game server
	size_t indexNextGameServer = (indexChosenGameServer + 1);
	if (indexNextGameServer == listSize)
		indexNextGameServer = 0;

	while (indexNextGameServer != indexChosenGameServer)
	{
		if (m_gameServerConnectionsList[indexNextGameServer]->getReady())
			return m_gameServerConnectionsList[indexNextGameServer];

		++indexNextGameServer;
		if (indexNextGameServer == listSize)
			indexNextGameServer = 0;
	}

	return nullptr;
}

//-----------------------------------------------------------------------

/**
* Returns a game server that is handling a scene.
*
* @param scene		the scene
*
* @return a game server connection for that scene
*/
GameServerConnection * CentralServer::getGameServer(const SceneId &scene) const
{
	// @todo: we should pick a gameserver based on load

	CentralServer::ServersList range = getGameServers(scene);
	SceneGameMap::const_iterator iter = range.first;
	while (iter != range.second)
	{
		if (iter->second->getReady())
			return iter->second;
		++iter;
	}
	return 0;
}

//-----------------------------------------------------------------------

/**
* Returns the game server for a given process id.
*
* @param processId		the process id
*
* @return the game server connection
*/
GameServerConnection * CentralServer::getGameServer(const uint32 processId) const
{
	GameServerConnection * result = 0;
	std::map<uint32, GameServerConnection *>::const_iterator i = m_gameServerConnections.find(processId);
	if(i != m_gameServerConnections.end())
		result = (*i).second;
	return result;
}

//-----------------------------------------------------------------------

/**
* Returns a list of game servers that are handling a scene.
*
* @param scene		the scene
*
* @return an iterator pair for a range of GameServerConnections
*/
const CentralServer::ServersList CentralServer::getGameServers(const SceneId &scene) const
{
	return m_gameServers.equal_range(scene); //lint !e64 //Type mismatch
}

//-----------------------------------------------------------------------

void CentralServer::pushAllKeys(ConnectionServerConnection * targetConnectionServer) const
{
	DEBUG_FATAL(static_cast<int>(m_loginServerKeys->getKeyCount()) < 0, ("Invalid number of keys (uint overflow) in CentralServer.h"));

	for(int i = static_cast<int>(m_loginServerKeys->getKeyCount()) - 1; i >= 0 ; i --)
	{
		ConnectionKeyPush pk(m_loginServerKeys->getKey(static_cast<uint32>(i)));
		targetConnectionServer->send(pk, true);
	}
}

//-----------------------------------------------------------------------

void CentralServer::launchStartingProcesses() const
{
	static bool startingProcessesLaunched = false;
	if (startingProcessesLaunched)
		return;

	// we can only launch starting processes once we have a connection to the
	// TaskManager *AND* have received our cluster id from the LoginServer
	if (!m_taskManager || !m_taskManager->isConnected() || (m_clusterId == 0))
	{
		if (!m_taskManager)
			REPORT_LOG(true, ("CentralServer not launching starting processes because m_taskManager is nullptr\n"));
		else if (!m_taskManager->isConnected())
			REPORT_LOG(true, ("CentralServer not launching starting processes because m_taskManager->isConnected() is false\n"));

		if (m_clusterId == 0)
			REPORT_LOG(true, ("CentralServer not launching starting processes because m_clusterId is 0, meaning hasn't received the cluster Id from the LoginServer yet\n"));

		return;
	}

	REPORT_LOG(true, ("CentralServer now launching starting processes because has a connection to the TaskManager *AND* has received cluster Id (%u) from the LoginServer\n", m_clusterId));

	startingProcessesLaunched = true;

	// spawn a connection server
	launchStartingConnectionServers();

	std::string options = "-s dbProcess centralServerAddress=";
	if(getGameService())
	{
		options += getGameService()->getBindAddress();
	}
	else
	{
		options += NetworkHandler::getHostName();
	}

	TaskSpawnProcess pd(ConfigCentralServer::getDbServerHost(), "SwgDatabaseServer", options);
	m_taskManager->send(pd, true);

	options = "-s ChatServer centralServerAddress=";
	if(getChatService())
	{
		options += getChatService()->getBindAddress();
	}
	else
	{
		options += NetworkHandler::getHostName();
	}
	options += " clusterName=";
	options += ConfigCentralServer::getClusterName();
	TaskSpawnProcess pc(ConfigCentralServer::getChatServerHost(), "ChatServer", options);
	m_taskManager->send(pc, true);

	options = "-s CustomerServiceServer centralServerAddress=";
	if(getCustomerService())
	{
		options += getCustomerService()->getBindAddress();
	}
	else
	{
		options += NetworkHandler::getHostName();
	}

	options += " clusterName=";
	options += ConfigCentralServer::getClusterName();
	TaskSpawnProcess pcs("any", "CustomerServiceServer", options);
	m_taskManager->send(pcs, true);
}

//-----------------------------------------------------------------------

void CentralServer::launchStartingConnectionServers() const
{
	// if there are several planets listed explicitly in the config file/command line,
	// spawn them

	int i = 0;

	// check for nesteds
	int const numberOfStartingConnectionServers = ConfigCentralServer::getNumberOfStartingConnectionServers();
	for (i = 0; i < numberOfStartingConnectionServers; ++i)
	{
		char const * const host = ConfigCentralServer::getStartingConnectionServer(i);
		if (host)
		{
			char const * listenAddress = strchr(host,':');
			DEBUG_FATAL(!listenAddress, ("Could not start up connection server because string %s has no listen address", host));
			std::string hostString(host, static_cast<unsigned long>(listenAddress - host));
			++listenAddress;

			char const * publicPort = strchr(listenAddress,':');
			DEBUG_FATAL(!publicPort, ("Could not start up connection server because string %s has no public port", host));
			std::string listenString(listenAddress, static_cast<unsigned long>(publicPort - listenAddress));
			++publicPort;

			char const * privatePort = strchr(publicPort,':');
			DEBUG_FATAL(!privatePort, ("Could not start up connection server because string %s has no private port", host));
			std::string publicPortString(publicPort, static_cast<unsigned long>(privatePort - publicPort));
			++privatePort;
			std::string privatePortString(privatePort);

			DEBUG_REPORT_LOG(true, ("Spawning Connection Server on %s listen:%s public:%s priv:%s\n", hostString.c_str(), listenString.c_str(), publicPortString.c_str(), privatePortString.c_str()));

			// must update s_connectionServerHostList before calling startConnectionServer()
			s_connectionServerHostList.push_back(hostString);
			startConnectionServer(s_connectionServerHostList.size(), listenString, &publicPortString, &privatePortString, 0);
		}
	}

	// if there were none specified, use defaults
	if(i == 0)
	{
		// must update s_connectionServerHostList before calling startConnectionServer()
		s_connectionServerHostList.push_back("node0");
		startConnectionServer(s_connectionServerHostList.size(), NetworkHandler::getHostName(), 0, 0, 0);
	}

}

//-----------------------------------------------------------------------

void CentralServer::launchStartingPlanetServers()
{
	// if there are several planets listed explicitly in the config file/command line,
	// spawn them
	int i = 0;
	int const numberOfStartPlanets = ConfigCentralServer::getNumberOfStartPlanets();

	for (i = 0; i < numberOfStartPlanets ; ++i)
	{
		char const * const p = ConfigCentralServer::getStartPlanet(i);
		if (p)
		{
			FATAL(!*p, ("CentralServer::launchStartingPlanetServers: ConfigCentralServer::getStartPlanet(%d) specified a non-nullptr but empty planet name", i));

			std::string planetName;
			std::string hostName;
			char const * planet = p;
			NOT_NULL(planet);
			char const * host = strchr(planet,':');

			if (host)
			{
				planetName = std::string(planet, static_cast<unsigned long>(host - planet) );
				++host;
				hostName = host;
			}
			else
			{
				planetName = planet;
			}

			startPlanetServer(hostName, planetName, 0);
			ms_sceneToHostMap[planetName] = hostName;
		}
	}

	// if there were no planets specified, get the configfile default and
	// start that planet
	if(i == 0)
	{
		startPlanetServer("any", ConfigCentralServer::getStartPlanet(), 0);
	}
}

//-----------------------------------------------------------------------

void CentralServer::launchCommoditiesServer()
{
	std::string options;
	TaskSpawnProcess csp("any", "CommoditiesServer", options);
	CentralServer::getInstance().sendTaskMessage(csp);
}

//-----------------------------------------------------------------------

void CentralServer::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	if(message.isType("LoginKeyPush"))
	{
		// receiving another key
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const LoginKeyPush k(ri);
		m_loginServerKeys->pushKey(k.getKey());
		DEBUG_REPORT_LOG(true, ("Received session key.\n"));
		ConnectionServerConnectionList::iterator i = m_connectionServerConnections.begin();
		for (;i != m_connectionServerConnections.end(); ++i)
		{
			(*i)->send(k, true);
		}
	}
	else if (message.isType("RequestClusterShutdown"))
	{
		REPORT_LOG(true, ("An admin requested we shutdown the entire cluster\n"));
		m_done = true;
	}
	else if (message.isType("ClientConnectionClosed"))
	{
		Archive::ReadIterator ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
		GenericValueTypeMessage<StationId> const msg(ri);

		ConnectionServerSUIDMap::iterator i = m_accountConnectionMap.find(msg.getValue());
		if ((i != m_accountConnectionMap.end()) && (i->second == &source))
		{
			// Erase the entry
			m_accountConnectionMap.erase(i);
		}
	}
	else if(message.isType("ConnectionOpened"))
	{
		if (!dynamic_cast<const LoginServerConnection *>(&source))
		{
			ServerConnection *newServer = const_cast<ServerConnection *>(safe_cast<ServerConnection const *>(&source));
			uint32	pid = newServer->getProcessId();

			CentralGameServerSetProcessId	pidMessage(pid, ServerClock::getInstance().getSubtractInterval(), ConfigCentralServer::getClusterName());
			newServer->send(pidMessage, true);
		}
	}
	else if(message.isType("ConnectionServerConnectionClosed"))
	{
		DEBUG_REPORT_LOG(true,("Handling connection server crash.\n"));
		ConnectionServerConnection const *c = safe_cast<ConnectionServerConnection const *>(&source);
		removeFromAccountConnectionMap(c->getId());
	}
	else if(message.isType("GameConnectionClosed"))
	{
		DEBUG_REPORT_LOG(true, ("Game server closed connection\n"));
		GameServerConnection const *g = safe_cast<GameServerConnection const *>(&source);
		removeGameServer(g);
	}

	else if (message.isType("LoginConnectionOpened"))
	{
		LoginServerConnection *l = const_cast<LoginServerConnection*>(safe_cast<LoginServerConnection const *>(&source));
		m_loginServerConnectionMap[l->getProcessId()] = l;
		//Send connection server data
		ConnectionServerConnectionList::iterator i = m_connectionServerConnections.begin();
		for(; i != m_connectionServerConnections.end(); ++i)
		{
			const ConnectionServerConnection * const csc = *i;
			if ( (csc->getClientServicePortPrivate() != 0) || (csc->getClientServicePortPublic() != 0) )
			{
				const LoginConnectionServerAddress csa(csc->getId(), csc->getClientServiceAddress(), csc->getClientServicePortPrivate(), csc->getClientServicePortPublic(), csc->getPlayerCount(), csc->getPingPort ());
				l->send(csa, true);
			}
		}

		// send preload is finished, if it is
		PreloadFinishedMessage msg(isPreloadFinished());
		l->send(msg, true);

		CharacterCreationTracker::getInstance().retryLoginServerCreates();
	}

	else if(message.isType("LoginConnectionClosed"))
	{
		LoginServerConnection const *l = safe_cast<const LoginServerConnection *>(&source);
		LoginServerConnectionMapType::iterator i=m_loginServerConnectionMap.find(l->getProcessId());
		if (i!=m_loginServerConnectionMap.end())
			m_loginServerConnectionMap.erase(i);

		// In development mode, try to reconnect to the login server if we aren't shutting down
		if (ConfigCentralServer::getDevelopmentMode() && !m_done)
			connectToLoginServer();

		CharacterCreationTracker::getInstance().onLoginServerDisconnect(l->getProcessId());
	}

	else if(message.isType("ClusterId"))
	{
		Archive::ReadIterator ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
		GenericValueTypeMessage<uint32> const msg(ri);

		if (m_clusterId == 0)
		{
			FATAL(((msg.getValue() < 1) || (msg.getValue() > 255)),("Cluster Id (%lu) must be between 1 and 255 inclusive", msg.getValue()));

			m_clusterId = static_cast<uint8>(msg.getValue());

			// start launching processes
			launchStartingProcesses();
		}
	}

	else if(message.isType("CentralGameServerConnect"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		CentralGameServerConnect c(ri);

		GameServerConnection *g = const_cast<GameServerConnection *>(safe_cast<GameServerConnection const *>(&source));

		if (g != nullptr) {

			FATAL(ConfigCentralServer::getValidateBuildVersionNumber() && strcmp(ApplicationVersion::getInternalVersion(), c.getBuildVersionNumber().c_str()),
				("Build version number mismatch: central server (%s), remote server %s (%s)",
				ApplicationVersion::getInternalVersion(), g->getRemoteAddress().c_str(), c.getBuildVersionNumber().c_str()));

			// a game server (or db process) has connected...
			addGameServer(g);

			//Send connection server data
			ConnectionServerConnectionList::iterator i = m_connectionServerConnections.begin();
			for (; i != m_connectionServerConnections.end(); ++i)
			{
				if ((*i)->getGameServicePort() != 0)
				{
					ConnectionServerAddress csa((*i)->getGameServiceAddress(), (*i)->getGameServicePort());
					g->send(csa, true);
				}
			}

			std::set<ChatServerConnection *>::const_iterator chatIter;
			for (chatIter = m_chatServerConnections.begin(); chatIter != m_chatServerConnections.end(); ++chatIter)
			{
				if ((*chatIter)->getGameServicePort())
				{
					ChatServerOnline cso((*chatIter)->getRemoteAddress(), (*chatIter)->getGameServicePort());
					g->send(cso, true);
				}
			}

			const GenericValueTypeMessage<std::pair<std::string, unsigned short> > address("CustomerServiceServerGameServerServiceAddress", std::make_pair(s_customerServiceServerGameServerServiceAddress.first, s_customerServiceServerGameServerServiceAddress.second));
			g->send(address, true);
		}
	}
	else if (message.isType("ChatClosedConnectionWithGameServer"))
	{
		GameServerConnection *g = const_cast<GameServerConnection *>(safe_cast<GameServerConnection const *>(&source));
		std::set<ChatServerConnection *>::const_iterator chatIter;
		for(chatIter = m_chatServerConnections.begin(); chatIter != m_chatServerConnections.end(); ++chatIter)
		{
			if((*chatIter)->getGameServicePort())
			{
				ChatServerOnline cso((*chatIter)->getRemoteAddress(), (*chatIter)->getGameServicePort());
				g->send(cso, true);
			}
		}
	}
	else if (message.isType("CustomerServiceServerGameServerServiceAddress"))
	{
		//DEBUG_REPORT_LOG(true, ("CentralServer::receiveMessage(CustomerServiceServerGameServerServiceAddress)\n"));

		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const GenericValueTypeMessage<std::pair<std::string, unsigned short> > address(ri);
		s_customerServiceServerGameServerServiceAddress.first = address.getValue().first;
		s_customerServiceServerGameServerServiceAddress.second = address.getValue().second;

		broadcastToGameServers(address);
	}
	else if(message.isType("TaskConnectionClosed"))
	{
		//		Net::getInstance().connect(Network::Address("127.0.0.1", ConfigCentralServer::getTaskManagerPort()), TaskConnection());
	}
	else if(message.isType("TaskConnectionOpened"))
	{
		DEBUG_REPORT_LOG(true, ("Task manager connection opened\n"));
	}

	else if(message.isType("CentralGameServerDbProcessServerProcessId"))
	{
		DEBUG_REPORT_LOG(true, ("dbProcess connected\n"));
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		CentralGameServerDbProcessServerProcessId d(ri);
		m_dbProcessServerProcessId = d.getServerProcessId();
		ServerClock::getInstance().setSubtractInterval(d.getSubtractInterval());

		launchCommoditiesServer();
		launchStartingPlanetServers();
	}

	else if(message.isType("RequestChunkMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		RequestChunkMessage t(ri);

		// Forward this message to the dbProcess
		sendToGameServer(m_dbProcessServerProcessId, t, true);
	}
	else if(message.isType("LocateStructureMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		LocateStructureMessage t(ri);
		sendToPlanetServer(t.getSceneId(), t, true);
	}
	else if(message.isType("ForceUnloadObjectMessage"))
	{
		//N.B.  This message can come from a game server or from the planet server.
		DEBUG_WARNING(true, ("Received ForceUnloadObject.  Need to implement this\n"));

		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ForceUnloadObjectMessage t(ri);

		//@todo: figure out some way to handle this (such as forwarding to PlanetServers), or remove every case where it's sent
		//forceUnload(t.getId(),t.getPermaDelete());
	}
	//Character Creation Messages
	else if(message.isType("ConnectionCreateCharacter"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ConnectionCreateCharacter c(ri);

		LOG("TraceCharacterCreation", ("%d received ConnectionCreateCharacter", c.getStationId()));
		CharacterCreationTracker::getInstance().handleCreateNewCharacter(c);
	}
	else if(message.isType("GameCreateCharacterFailed"))
	{
		DEBUG_REPORT_LOG(true, ("Game server advises central that character creation failed\n"));
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GameCreateCharacterFailed f(ri);
		CharacterCreationTracker::getInstance().handleGameCreateCharacterFailed(f.getStationId(), f.getName(), f.getErrorMessage(), f.getOptionalDetailedErrorMessage());
	}
	else if(message.isType("DatabaseCreateCharacterSuccess"))
	{
		DEBUG_REPORT_LOG(true, ("Database Process advises central that character creation succeeded\n"));
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		DatabaseCreateCharacterSuccess s(ri);
		CharacterCreationTracker::getInstance().handleDatabaseCreateCharacterSuccess(s.getStationId(), s.getCharacterName(), s.getObjectId(), s.getTemplateId(), s.getJedi());
	}
	else if(message.isType("LoginCreateCharacterAckMessage"))
	{
		DEBUG_REPORT_LOG(true, ("Login Server advises central that character creation succeeded\n"));
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		LoginCreateCharacterAckMessage s(ri);
		CharacterCreationTracker::getInstance().handleLoginCreateCharacterAck(s.getStationId());
	}
	else if(message.isType("LoginRestoreCharacterMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		LoginRestoreCharacterMessage msg(ri);

		IGNORE_RETURN(sendToArbitraryLoginServer(msg));
	}
	else if(message.isType("NewCharacterCreated"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<StationId> const ncc(ri);

		CentralServer::getInstance().sendToAllConnectionServers(ncc, true);
	}
	else if(message.isType("DatabaseConsoleReplyMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::string, std::string> > msg(ri);

		IGNORE_RETURN(sendToRandomGameServer(msg));
	}
	else if(message.isType("LoginUpgradeAccountMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		LoginUpgradeAccountMessage msg(ri);

		if (msg.getIsAck())
		{
			MessageToMessage const reply(
				MessageToPayload(msg.getReplyToObject(), NetworkId::cms_invalid, msg.getReplyMessage(), msg.getPackedMessageData(), 0, false, MessageToPayload::DT_c,NetworkId::cms_invalid,std::string(), 0),
				0);
			sendToAllGameServers (reply,true);
		}
		else
		{
			IGNORE_RETURN(sendToArbitraryLoginServer(msg));
		}
	}
	else if (message.isType("RandomNameRequest"))
	{
		GameServerConnection * gameServer = getRandomGameServer();
		if(gameServer)
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			RandomNameRequest crnr(ri);
			gameServer->send(crnr, true);
		}
	}
	else if (message.isType("RandomNameResponse"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		RandomNameResponse crnr(ri);
		ConnectionServerConnection * conn = getConnectionServerForAccount(crnr.getStationId());
		if (conn)
		{
			conn->send(crnr, true);
		}
		else
		{
			DEBUG_REPORT_LOG(true,("Could not send name to client because unable to determine which connection server to use.\n"));
		}
	}
	else if (message.isType("VerifyAndLockNameRequest"))
	{
		GameServerConnection * gameServer = getRandomGameServer();
		if(gameServer)
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			VerifyAndLockNameRequest valnr(ri);
			gameServer->send(valnr, true);
		}
	}
	else if (message.isType("VerifyAndLockNameResponse"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		VerifyAndLockNameResponse valnr(ri);
		ConnectionServerConnection * conn = getConnectionServerForAccount(valnr.getStationId());
		if (conn)
		{
			conn->send(valnr, true);
		}
		else
		{
			DEBUG_REPORT_LOG(true,("Could not send name lock response to client because unable to determine which connection server to use.\n"));
		}
	}	
	else if (message.isType("RequestOIDsMessage"))
	{
		DEBUG_REPORT_LOG(true,("Got RequestOIDsMessage.\n"));
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		RequestOIDsMessage m(ri);

		if (m.getLogRequest())
			LOG("ObjectIdManager", ("Requesting %lu more object ids for pid %lu", m.getHowMany(), m.getServerId()));

		sendToGameServer(m_dbProcessServerProcessId, m, true);
	}
	else if (message.isType("AddOIDBlockMessage"))
	{
		DEBUG_REPORT_LOG(true,("Got AddOIDBlockMessage.\n"));
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		AddOIDBlockMessage m(ri);

		if (m.getLogRequest())
			LOG("ObjectIdManager", ("Returning object ids (%s - %s) for pid %lu", m.getStart().getValueString().c_str(), m.getEnd().getValueString().c_str(), m.getServerId()));

		sendToGameServer(m.getServerId(), m, true);
	}
	else if (message.isType("LoggedInMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		LoggedInMessage m (ri);
		DEBUG_REPORT_LOG(true, ("Pending character %lu is logging in or dropping\n", m.getAccountNumber()));

		// Once they're logged in, Central doesn't need to know about them anymore:
		removeFromAccountConnectionMap(m.getAccountNumber());
	}

	else if (message.isType("CharacterListMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		CharacterListMessage m(ri);

		// Find the client connection and send the character to it.
		DEBUG_REPORT_LOG(true,("Got CharacterListMessage for %lu.\n",m.getAccountNumber()));
		ConnectionServerConnection *conn = getConnectionServerForAccount(m.getAccountNumber());

		if (conn)
		{
			conn->send(m, true);
		}
		else
		{
			DEBUG_REPORT_LOG(true,("Warning:  received CharacterListMessage for client that is not connected."));
		}
	}
	else if (message.isType("ValidateCharacterForLoginMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ValidateCharacterForLoginMessage m(ri);

		// ask DBProcess to check whether the character is valid
		//pendingCharactersLogin[m.getSuid()] = safe_cast<ConnectionServerConnection const *>(&source);
		if (hasDBConnection())
			sendToDBProcess(m, true);
	}
	else if (message.isType("ValidateCharacterForLoginReplyMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ValidateCharacterForLoginReplyMessage msg(ri);

		ConnectionServerConnection *conn = getConnectionServerForAccount(msg.getSuid());
		if(conn)
			conn->send(msg,true);
		else
			DEBUG_REPORT_LOG(true,("Trying to handle ValidateCharacterForLoginReplyMessage for account %lu, but could not determine which connection server to use.\n",msg.getSuid()));
	}
	else if (message.isType("EnableCharacterMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::pair<StationId, NetworkId>, std::string> > msg(ri);

		LOG("CustomerService", ("EnableCharacter %d, %s request from %s\n", msg.getValue().first.first, msg.getValue().first.second.getValueString().c_str(), msg.getValue().second.c_str()));
		// forward this request to LoginServer
		IGNORE_RETURN(sendToArbitraryLoginServer(msg));
	}
	else if (message.isType("EnableCharacterReplyMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::string, std::string> > msg(ri);
		GenericValueTypeMessage<std::pair<std::string, std::string> > reply("DatabaseConsoleReplyMessage", std::make_pair(msg.getValue().first, msg.getValue().second));
		getRandomGameServer()->send(reply, true);
	}
	else if (message.isType("DisableCharacterMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::pair<StationId, NetworkId>, std::string> > msg(ri);

		LOG("CustomerService", ("DisableCharacter %d, %s request from %s\n", msg.getValue().first.first, msg.getValue().first.second.getValueString().c_str(), msg.getValue().second.c_str()));
		// forward this request to LoginServer
		IGNORE_RETURN(sendToArbitraryLoginServer(msg));
	}
	else if (message.isType("DisableCharacterReplyMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::string, std::string> > msg(ri);
		GenericValueTypeMessage<std::pair<std::string, std::string> > reply("DatabaseConsoleReplyMessage", std::make_pair(msg.getValue().first, msg.getValue().second));
		getRandomGameServer()->send(reply, true);
	}
	else if(message.isType("TransferReplyLoginLocationData"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<TransferCharacterData> reply(ri);
		// If this request has a CS Tool Id associated with it, it is an admin request for the CSTool,
		// and so we should send it directly to the connection server, and not depend on a 
		// transfer server existing.
		
		if(reply.getValue().getCSToolId() > 0)
		{
			GenericValueTypeMessage<TransferCharacterData> loginMessage("TransferLoginCharacterToSourceServer", reply.getValue());
			ConnectionServerConnection * conn = getAnyConnectionServer();
			if(conn)
			{
				conn->send(loginMessage, true);
			}
			return;
		}

		LOG("CustomerService", ("CharacterTransfer: Received TransferReplyLoginLocationData from database for character %s\n", reply.getValue().getSourceCharacterName().c_str()));

		if(reply.getValue().getTransferRequestSource() == TransferRequestMoveValidation::TRS_transfer_server)
		{
			CentralServer::getInstance().sendToTransferServer(reply);
		}
		else
		{
			// send character to ConnectionServer for login to a game server
			ConnectionServerConnection * connectionServer = CentralServer::getInstance().getAnyConnectionServer();
			if(connectionServer)
			{
				const GenericValueTypeMessage<TransferCharacterData> login("TransferLoginCharacterToSourceServer", reply.getValue());
				connectionServer->send(login, true);

				LOG("CustomerService", ("CharacterTransfer: Sending TransferLoginCharacterToSourceServer to ConnectionServer : %s", login.getValue().toString().c_str()));
			}		
		}
	}
	else if (message.isType("CentralPlanetServerConnect"))
	{
		PlanetServerConnection *g = const_cast<PlanetServerConnection*>(safe_cast<PlanetServerConnection const *>(&source));
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		CentralPlanetServerConnect msg(ri);
		g->setGameServerConnectionData(msg.getConnectionAddress(), msg.getGameServerPort());
		PlanetManager::addServer(msg.getSceneId(), g);

		std::map<std::string, std::pair<std::pair<std::string, std::string>, time_t> >::iterator f = m_pendingPlanetServers.find(msg.getSceneId());
		if(f != m_pendingPlanetServers.end())
			m_pendingPlanetServers.erase(f);

		IGNORE_RETURN(m_planetServers.insert(std::make_pair(msg.getSceneId(), g)));

		if (isPreloadFinished())
			m_timeClusterWentIntoLoadingState = 0;
		else if (m_timeClusterWentIntoLoadingState <= 0)
			m_timeClusterWentIntoLoadingState = time(0);

		//  handle planet transfers and logins for planet server that aren't up yet
		std::vector<Archive::ByteStream>::iterator t;
		for(t = m_messagesWaitingForPlanetServer.begin(); t != m_messagesWaitingForPlanetServer.end();)
		{
			Archive::ReadIterator tri = t->begin();
			const GameNetworkMessage gnm(tri);
			tri = t->begin();
			if(gnm.isType("RequestGameServerForLoginMessage"))
			{
				const RequestGameServerForLoginMessage loginMessage(tri);
				if (loginMessage.getScene() == msg.getSceneId())
				{
					t = m_messagesWaitingForPlanetServer.erase(t);
					handleRequestGameServerForLoginMessage(loginMessage);
				}
			}
			else if(gnm.isType("RequestSceneTransfer"))
			{
				const RequestSceneTransfer sceneMessage(tri);
				if (sceneMessage.getSceneName() == msg.getSceneId())
				{
					t = m_messagesWaitingForPlanetServer.erase(t);
					handleRequestSceneTransfer(sceneMessage);
				}
			}
			else
			{
				t = m_messagesWaitingForPlanetServer.erase(t);
				WARNING_STRICT_FATAL(true, ("Unknown message type waiting for planet server"));
			}
		}
	}
	else if (message.isType("RequestSceneTransfer"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const RequestSceneTransfer msg (ri);

		handleRequestSceneTransfer(msg);
	}
	else if (message.isType("SceneTransferMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		SceneTransferMessage msg (ri);

		sendToGameServer(msg.getSourceGameServer(), msg, true);
	}
	else if (message.isType("GameServerReadyMessage"))
	{
		GameServerConnection const *g = safe_cast<GameServerConnection const *>(&source);
		UniverseManager::getInstance().onGameServerReady(*g);
	}
	else if (message.isType("GameServerUniverseLoadedMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const GameServerUniverseLoadedMessage msg(ri);

		// forward to the universe game server as ack that
		// the game server who sent this message has received
		// its UniverseCompleteMessage from the universe game server
		if (msg.getSourceOfUniverseDataProcessId() != getDbProcessServerProcessId())
			sendToGameServer(msg.getSourceOfUniverseDataProcessId(), msg, true);

		CharacterCreationTracker::getInstance().retryGameServerCreates();
	}
	else if (message.isType("MessageToMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		MessageToMessage msg(ri);

		WARNING_STRICT_FATAL(true,("CentralServer receieved a messageTo.  These should not go to Central anymore.  Sender was server %u\n", safe_cast<GameServerConnection const *>(&source)->getProcessId()));
	}
	else if (message.isType("MessageToAckMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		MessageToAckMessage msg(ri);

		WARNING_STRICT_FATAL(true,("CentralServer receieved a MessageToAckMessage.  These should not go to Central anymore.  Sender was server %u\n",safe_cast<GameServerConnection const *>(&source)->getProcessId()));
	}
	else if(message.isType("ChatServerConnectionOpened"))
	{
		// enumerate servers
		ChatServerConnection *chatServer = const_cast<ChatServerConnection *>(safe_cast<ChatServerConnection const *>(&source));
		IGNORE_RETURN(m_chatServerConnections.insert(chatServer));
		const int ct = static_cast<int>(EnumerateServers::CONNECTION_SERVER);
		for (ConnectionServerConnectionList::const_iterator ci = m_connectionServerConnections.begin(); ci != m_connectionServerConnections.end(); ++ci)
		{
			ConnectionServerConnection * c = (*ci);

			if (   (c != nullptr)
			    && !c->getChatServiceAddress().empty()
			    && (c->getChatServicePort() != 0))
			{
				EnumerateServers e(true, c->getChatServiceAddress(), c->getChatServicePort(), ct);
				chatServer->send(e, true);
			}
			else
			{
				LOG("ChatServer", ("receiveMessage() ChatServerConnectionOpened - Invalid connection address in m_connectionServerConnections, skipping entry"));
			}
		}

		// Tell the single chat server the address to communicate with the customer service server

		if (!s_customerServiceServerChatServerServiceAddress.first.empty())
		{
			//DEBUG_REPORT_LOG(true, ("CentralServer::receiveMessage(ChatServerConnectionOpened)\n"));

			const GenericValueTypeMessage<std::pair<std::string, unsigned short> > msg("CustomerServiceServerChatServerServiceAddress", std::make_pair(s_customerServiceServerChatServerServiceAddress.first, s_customerServiceServerChatServerServiceAddress.second));

			chatServer->send(msg, true);
		}
	}
	else if (message.isType("CustomerServiceServerChatServerServiceAddress"))
	{
		//DEBUG_REPORT_LOG(true, ("CentralServer::receiveMessage(CustomerServiceServerChatServerServiceAddress)\n"));

		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const GenericValueTypeMessage<std::pair<std::string, unsigned short> > address(ri);

		s_customerServiceServerChatServerServiceAddress.first = address.getValue().first;
		s_customerServiceServerChatServerServiceAddress.second = address.getValue().second;

		// Tell the chat servers the address to communicate with the customer service server

		std::set<ChatServerConnection *>::iterator iterChatServerConnections = m_chatServerConnections.begin();

		for (; iterChatServerConnections != m_chatServerConnections.end(); ++iterChatServerConnections)
		{
			ChatServerConnection *connection = (*iterChatServerConnections);

			if (connection != nullptr)
			{
				connection->send(address, true);
			}
			else
			{
				REPORT_LOG(true, ("Trying to send the customer service server: chat server service address to a nullptr chat server\n"));
			}
		}
	}
	else if (message.isType("CustomerServiceConnectionOpened"))
	{
		CustomerServiceConnection *csServer = const_cast<CustomerServiceConnection *>(safe_cast<CustomerServiceConnection const *>(&source));
		IGNORE_RETURN(m_csServerConnections.insert(csServer));
		const int ct = static_cast<int>(EnumerateServers::CONNECTION_SERVER);
		for (ConnectionServerConnectionList::const_iterator ci = m_connectionServerConnections.begin(); ci != m_connectionServerConnections.end(); ++ci)
		{
			ConnectionServerConnection * c = (*ci);

			if (   (c != nullptr)
			    && !c->getCustomerServiceAddress().empty()
			    && (c->getCustomerServicePort() != 0))
			{
				EnumerateServers e(true, c->getCustomerServiceAddress(), c->getCustomerServicePort(), ct);
				csServer->send(e, true);
			}
			else
			{
				LOG("CustServ", ("receiveMessage() CustomerServiceConnectionOpened - Invalid connection address in m_connectionServerConnections, skipping entry"));
			}
		}
	}
	else if(message.isType("ChatServerConnectionClosed"))
	{
		ChatServerConnection *chatServer = const_cast<ChatServerConnection *>(safe_cast<ChatServerConnection const *>(&source));
		IGNORE_RETURN(m_chatServerConnections.erase(chatServer));

		// spawn a new chat server!
		std::string options = "-s ChatServer centralServerAddress=";
		if(CentralServer::getInstance().getChatService())
		{
			options += CentralServer::getInstance().getChatService()->getBindAddress();
		}
		else
		{
			options += NetworkHandler::getHostName();
		}

		options += " clusterName=";
		options += ConfigCentralServer::getClusterName();
		TaskSpawnProcess pc(ConfigCentralServer::getChatServerHost(), "ChatServer", options, ConfigCentralServer::getChatServerRestartDelayTimeSeconds());
		CentralServer::getInstance().sendTaskMessage(pc);

	}
	else if (message.isType("CustomerServiceConnectionClosed"))
	{
		CustomerServiceConnection *csServer = const_cast<CustomerServiceConnection *>(safe_cast<CustomerServiceConnection const *>(&source));
		IGNORE_RETURN(m_csServerConnections.erase(csServer));

		// spawn a new cs server!
		std::string options = "-s CustomerServiceServer centralServerAddress=";
		if(CentralServer::getInstance().getCustomerService())
		{
			options += CentralServer::getInstance().getCustomerService()->getBindAddress();
		}
		else
		{
			options += NetworkHandler::getHostName();
		}

		options += " clusterName=";
		options += ConfigCentralServer::getClusterName();
		TaskSpawnProcess pc("any", "CustomerServiceServer", options);
		CentralServer::getInstance().sendTaskMessage(pc);
	}
	else if(message.isType("ChatServerOnline"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ChatServerOnline cso (ri);
		ChatServerConnection *csc = const_cast<ChatServerConnection *>(safe_cast<ChatServerConnection const *>(&source));
		csc->setGameServicePort(cso.getPort());
		SceneGameMap::const_iterator iter;
		for(iter = m_gameServers.begin(); iter != m_gameServers.end(); ++iter)
		{
			GameServerConnection * conn = (*iter).second;
			conn->send(cso, true);
		}

	}
	else if (message.isType("RequestGameServerForLoginMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const RequestGameServerForLoginMessage msg(ri);

		time_t const timeNow = ::time(nullptr);
		PlayerSceneMapType::const_iterator i = m_playerSceneMap.find(msg.getCharacterId());
		if ((i != m_playerSceneMap.end()) && (i->second.second > timeNow))
		{
			ServerConnection * c = const_cast<ServerConnection *>(safe_cast<ServerConnection const *>(&source));
			GenericValueTypeMessage<std::pair<NetworkId, uint32> > const loginDeniedRecentCTS("LoginDeniedRecentCTS", std::make_pair(msg.getCharacterId(), msg.getStationId()));
			c->send(loginDeniedRecentCTS, true);
		}
		else
		{
			std::map<NetworkId, std::pair<time_t, int> >::const_iterator const iterFind = s_pendingRenameCharacter.find(msg.getCharacterId());
			if ((iterFind != s_pendingRenameCharacter.end()) && (iterFind->second.first > timeNow))
			{
				ServerConnection * c = const_cast<ServerConnection *>(safe_cast<ServerConnection const *>(&source));
				GenericValueTypeMessage<std::pair<NetworkId, uint32> > const loginDeniedPendingPlayerRenameRequest("LoginDeniedPendingPlayerRenameRequest", std::make_pair(msg.getCharacterId(), msg.getStationId()));
				c->send(loginDeniedPendingPlayerRenameRequest, true);
			}
			else
			{
				handleRequestGameServerForLoginMessage(msg);
			}
		}
	}
	else if (message.isType("GameServerForLoginMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GameServerForLoginMessage msg(ri);

		handleGameServerForLoginMessage(msg);
	}



	else if (message.isType("ExchangeListCreditsMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ExchangeListCreditsMessage msg(ri);

		handleExchangeListCreditsMessage(msg);
	}



	else if (message.isType("PlanetLoadCharacterMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		PlanetLoadCharacterMessage msg(ri);

		// let all the game servers know we are about to load a character from the DB
		GenericValueTypeMessage<std::pair<NetworkId, uint32> > const aboutToLoadCharacterFromDB("AboutToLoadCharacterFromDB", std::make_pair(msg.getCharacterId(), msg.getGameServerId()));
		sendToAllGameServersExceptDBProcess(aboutToLoadCharacterFromDB, true);

		sendToDBProcess(msg,true);
	}
	else if (message.isType("ConnSrvDropDupeConns"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<uint32, std::string> > const msg(ri);

		// A ConnectionServer has received and validated a new connection from the client

		// request that all other ConnectionServers drop any existing connection(s) for the account
		std::string galaxyConnectionInfo = "(";
		galaxyConnectionInfo += ConfigCentralServer::getClusterName();

		if (!msg.getValue().second.empty())
		{
			galaxyConnectionInfo += ", ";
			galaxyConnectionInfo += msg.getValue().second;
		}

		galaxyConnectionInfo += ")";

		GenericValueTypeMessage<std::pair<uint32, std::string> > const dropDuplicateConnections("CntrlSrvDropDupeConns", std::make_pair(msg.getValue().first, galaxyConnectionInfo));
		sendToAllConnectionServers(dropDuplicateConnections, true, dynamic_cast<Connection const *>(&source));

		// request (via LoginServer) that all ConnectionServers on all other galaxies drop any existing connection(s) for the account
		if (ConfigCentralServer::getDisconnectDuplicateConnectionsOnOtherGalaxies())
			IGNORE_RETURN(sendToArbitraryLoginServer(dropDuplicateConnections));
	}
	else if (message.isType("CntrlSrvDropDupeConns"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<uint32, std::string> > const msg(ri);

		// request that all ConnectionServers drop any existing connection(s) for the account
		sendToAllConnectionServers(msg, true);
	}
	else if (message.isType("PlayerDroppedFromGameServerCrash"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::set<NetworkId> > const m(ri);

		std::set<NetworkId>::const_iterator const end = m.getValue().end();
		for (std::set<NetworkId>::const_iterator iter = m.getValue().begin(); iter != end; ++iter)
		{
			IGNORE_RETURN(m_playerSceneMap.erase(*iter));
			LOG("TRACE_LOGIN", ("Forgetting sceneId (because of game server crash) for character (%s) -- will query the database on next login", iter->getValueString().c_str()));
		}
	}
	else if (message.isType("ServerDeleteCharacterMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		LoginServerConnection const *l = safe_cast<LoginServerConnection const *>(&source);
		ServerDeleteCharacterMessage msg(ri);
		msg.setLoginServerId(l->getProcessId());

		if (hasDBConnection())
		{
			LOG("CustomerService", ("Player:deleted character %s for stationId %u", msg.getCharacterId().getValueString().c_str(), msg.getStationId()));
			sendToDBProcess(msg,true);

			// let the game servers know that the character is being deleted
			GenericValueTypeMessage<NetworkId> const msg2("DeleteCharacterNotificationMessage", msg.getCharacterId());
			IGNORE_RETURN(sendToRandomGameServer(msg2));
		}
	}
	else if (message.isType("UpdatePlayerCountMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		UpdatePlayerCountMessage msg(ri);

		ConnectionServerConnection * csc = const_cast<ConnectionServerConnection*>(dynamic_cast<const ConnectionServerConnection *>(&source));
		if (csc)
		{
			// Update our stored values for the connection server
			csc->setPlayerCount(msg.getCount());
			csc->setFreeTrialCount(msg.getFreeTrialCount());
			csc->setEmptySceneCount(msg.getEmptySceneCount());
			csc->setTutorialSceneCount(msg.getTutorialSceneCount());
			csc->setFalconSceneCount(msg.getFalconSceneCount());

			// Update the login server with the new values
			UpdateLoginConnectionServerStatus ulc(csc->getId(), csc->getClientServicePortPublic(), csc->getClientServicePortPrivate(), msg.getCount());
			sendToAllLoginServers(ulc);

			// Update the login servers with new population values
			sendPopulationUpdateToLoginServer();
		}
		else
		{
			WARNING_STRICT_FATAL(true,("Got UpdatePlayerCountMessage from something that wasn't a ConnectionServer.\n"));
		}
	}
	else if (message.isType("ValidateAccountMessage"))
	{
		DEBUG_REPORT_LOG(true,("ValidateAccountMessage\n"));
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ValidateAccountMessage msg(ri);

		// Store the connection information for the account
		addToAccountConnectionMap(msg.getStationId(), const_cast<ConnectionServerConnection *>(dynamic_cast<const ConnectionServerConnection*>(&source)), msg.getSubscriptionBits());

		// Pass the validation to a LoginServer
		IGNORE_RETURN(sendToArbitraryLoginServer(msg));
	}
	else if (message.isType("ValidateAccountReplyMessage"))
	{
		DEBUG_REPORT_LOG(true,("ValidateAccountReplyMessage\n"));
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ValidateAccountReplyMessage msg(ri);

		ConnectionServerConnection *conn=getConnectionServerForAccount(msg.getStationId());
		if (conn)
		{
			conn->send(msg,true);
		}
	}
	else if (message.isType("PreloadRequestCompleteMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		PreloadRequestCompleteMessage msg(ri);

		sendToDBProcess(msg,true);
	}
	else if (message.isType("ReconnectToTransferServer"))
	{
		if(ConfigCentralServer::getTransferServerPort())
		{
			if(! getInstance().m_transferServerConnection)
			{
				getInstance().m_transferServerConnection = new TransferServerConnection(ConfigCentralServer::getTransferServerAddress(), ConfigCentralServer::getTransferServerPort());
				s_retryTransferServerConnection = true;
			}
		}
	}
	else if (message.isType("ReconnectToStationPlayersCollector"))
	{
		if(ConfigCentralServer::getStationPlayersCollectorPort())
		{
			if(! getInstance().m_stationPlayersCollectorConnection)
			{
				getInstance().m_stationPlayersCollectorConnection = new StationPlayersCollectorConnection(ConfigCentralServer::getStationPlayersCollectorAddress(), ConfigCentralServer::getStationPlayersCollectorPort());
				s_retryStationPlayersCollectorConnection = true;
			}
		}
	}
	else if (message.isType("PreloadFinishedMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		PreloadFinishedMessage msg(ri);

		const PlanetServerConnection *conn=dynamic_cast<const PlanetServerConnection*>(&source);
		WARNING_STRICT_FATAL(!conn,("Programmer bug:  got PreloadFinishedMessaage from something that wasn't a PlanetServer.\n"));
		if (conn)
		{
			if (msg.getFinished())
			{
				IGNORE_RETURN(m_planetsWaitingForPreload.erase(conn->getSceneId()));
				if (isPreloadFinished())
				{
					m_timeClusterWentIntoLoadingState = 0;

					DEBUG_REPORT_LOG(true,("Preload finished on all planets.\n"));

					// record how long it took the cluster to come up
					if (m_clusterStartupTime == -1)
					{
						m_clusterStartupTime = static_cast<int>(time(0) - m_timeClusterStarted) / 60;

						// let all the game servers know that the cluster has completed its initial startup
						GenericValueTypeMessage<bool> clusterStartComplete("ClusterStartComplete", true);
						sendToAllGameServersExceptDBProcess(clusterStartComplete, true);
					}
					else
					{
						// let all the game servers know that the cluster has recovered from a crash
						GenericValueTypeMessage<bool> clusterStartComplete("ClusterStartComplete", false);
						sendToAllGameServersExceptDBProcess(clusterStartComplete, true);
					}

					sendToAllLoginServers(msg);
					sendTaskMessage(msg);
					m_lastLoadingStateTime=time(0);
					// connect to the character transfer server
					if(ConfigCentralServer::getTransferServerPort())
					{
						if(! getInstance().m_transferServerConnection)
						{
							getInstance().m_transferServerConnection = new TransferServerConnection(ConfigCentralServer::getTransferServerAddress(), ConfigCentralServer::getTransferServerPort());
							s_retryTransferServerConnection = true;
						}
					}
					
					// connect to the station players collector
					if(ConfigCentralServer::getStationPlayersCollectorPort())
					{
						if(! getInstance().m_stationPlayersCollectorConnection)
						{
							getInstance().m_stationPlayersCollectorConnection = new StationPlayersCollectorConnection(ConfigCentralServer::getStationPlayersCollectorAddress(), ConfigCentralServer::getStationPlayersCollectorPort());
							s_retryStationPlayersCollectorConnection = true;
						}
					}
				}
				else if (m_timeClusterWentIntoLoadingState <= 0)
				{
					m_timeClusterWentIntoLoadingState = time(0);
				}
			}
			else
			{
				if(getInstance().m_transferServerConnection != nullptr)
				{
					getInstance().m_transferServerConnection->disconnect();
					getInstance().m_transferServerConnection = 0;
					s_retryTransferServerConnection = false;
				}
				
				if(getInstance().m_stationPlayersCollectorConnection != nullptr)
				{
					getInstance().m_stationPlayersCollectorConnection->disconnect();
					getInstance().m_stationPlayersCollectorConnection = 0;
					s_retryStationPlayersCollectorConnection = false;
				}


				IGNORE_RETURN(m_planetsWaitingForPreload.insert(conn->getSceneId()));

				if (isPreloadFinished())
					m_timeClusterWentIntoLoadingState = 0;
				else if (m_timeClusterWentIntoLoadingState <= 0)
					m_timeClusterWentIntoLoadingState = time(0);


				sendToAllLoginServers(msg);
				sendTaskMessage(msg);
			}
		}
	}
	else if (message.isType("RenameCharacterMessageEx"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		RenameCharacterMessageEx msg(ri);

		IGNORE_RETURN(sendToArbitraryLoginServer(RenameCharacterMessage(msg.getCharacterId(), msg.getNewName(), msg.getRequestedBy())));

		// if the player requested the rename, also rename the chat avatar, so mail, friends list, and ignore list will migrate
		if ((msg.getRenameCharacterMessageSource() == RenameCharacterMessageEx::RCMS_player_request) && !msg.getLastNameChangeOnly() && !ConfigFile::getKeyBool("CharacterRename", "disableRenameChatAvatar", false))
		{
			broadcastToChatServers(msg);
		}
	}
	else if (message.isType("PlayerRenameRequestSubmitted"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<unsigned int, std::pair<NetworkId, std::pair<std::string, bool> > > > const msg(ri);

		GenericValueTypeMessage<unsigned int> kick("TransferKickConnectedClients", msg.getValue().first);
		CentralServer::getInstance().sendToAllLoginServers(kick);
		CentralServer::getInstance().sendToAllConnectionServers(kick, true);

		// update the list of pending rename requests, to prevent a character with a pending rename request from logging in
		std::map<NetworkId, std::pair<time_t, int> >::iterator const iterFind = s_pendingRenameCharacter.find(msg.getValue().second.first);
		if (iterFind != s_pendingRenameCharacter.end())
		{
			++(iterFind->second.second);
			iterFind->second.first = ::time(nullptr) + 3600; // 1 hour timeout
		}
		else
		{
			s_pendingRenameCharacter.insert(std::make_pair(msg.getValue().second.first, std::make_pair((::time(nullptr) + 3600), 1))); // 1 hour timeout
		}

		// tell the chat server to destroy any avatar with the new name, but only if the first name changed
		if (!msg.getValue().second.second.second)
		{
			GenericValueTypeMessage<std::string> const chatDestroyAvatar("ChatDestroyAvatar", msg.getValue().second.second.first);
			broadcastToChatServers(chatDestroyAvatar);
		}
	}
	else if (message.isType("PlayerRenameRequestCompleted"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<unsigned int, NetworkId> > const msg(ri);

		// update the list of pending rename requests, to prevent a character with a pending rename request from logging in
		std::map<NetworkId, std::pair<time_t, int> >::iterator const iterFind = s_pendingRenameCharacter.find(msg.getValue().second);
		if (iterFind != s_pendingRenameCharacter.end())
		{
			if (iterFind->second.second <= 1)
				s_pendingRenameCharacter.erase(iterFind);
			else
				--(iterFind->second.second);
		}
	}
	else if(message.isType("SetConnectionServerPublic"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		SetConnectionServerPublic msg(ri);
		if(msg.getIsPublic())
			gs_connectionServersPublic = true;
		else
			gs_connectionServersPublic = false;

		sendToAllConnectionServers(msg, true);
	}
	else if(message.isType("ProfilerOperationMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ProfilerOperationMessage msg(ri);
		sendToAllGameServers(msg, true);
		sendToAllPlanetServers(msg, true);
		sendToAllConnectionServers(msg, true);
	}
	else if (message.isType("PopulationListMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		PopulationListMessage msg(ri);
		sendToAllGameServers(msg, true);
	}
	else if (message.isType("CentralPingMessage"))
	{
		GameServerConnection const *g = safe_cast<GameServerConnection const *>(&source);
		NOT_NULL(g);
		LOG("CentralServerPings",("Got reply from %lu",g->getProcessId()));
		IGNORE_RETURN(m_serverPings.erase(g->getProcessId()));
	}
	else if (message.isType("DatabaseBackloggedMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<bool> dbbacklog(ri);

		m_databaseBacklogged = dbbacklog.getValue();
		bool const preloadFinished = isPreloadFinished();

		if (preloadFinished)
			m_timeClusterWentIntoLoadingState = 0;
		else if (m_timeClusterWentIntoLoadingState <= 0)
			m_timeClusterWentIntoLoadingState = time(0);

		PreloadFinishedMessage msg(preloadFinished);
		sendToAllLoginServers(msg);
		sendTaskMessage(msg);
	}

	else if (message.isType("DatabaseSaveStart"))
	{
		LOG("CentralServer",("Received DatabaseSaveStart network message."));
		if( m_shutdownPhase == 4 )
		{
			LOG("CentralServerShutdown",("Shutdown Phase %d: Setting indicator for receipt of DatabaseSaveStart.", m_shutdownPhase));
			m_shutdownHaveDatabaseSaveStart = true;
			checkShutdownProcess();
		}
		else if( m_shutdownPhase == 5 && m_shutdownHaveDatabaseSaveStart)
		{
			LOG("CentralServerShutdown",("Shutdown Phase %d: Setting indicator that final database save cycle is complete.", m_shutdownPhase));
			m_shutdownHaveDatabaseComplete = true;
			checkShutdownProcess();
		}
	}
	else if (message.isType("DatabaseSaveComplete"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<int> msg(ri);
		LOG("CentralServer",("Received DatabaseSaveComplete network message."));

		// tell all the Planet Servers that the save finished
		sendToAllPlanetServers(msg,true);

		// don't want to indicate this yet until we are at the beginning of a full cycle
		if( m_shutdownPhase == 5 && m_shutdownHaveDatabaseSaveStart)
		{
			LOG("CentralServerShutdown",("Shutdown Phase %d: Setting indicator that final database save cycle is complete.", m_shutdownPhase));
			m_shutdownHaveDatabaseComplete = true;
			checkShutdownProcess();
		}
	}
	else if (message.isType("PlanetRequestSave"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GameNetworkMessage msg(ri);

		sendToDBProcess(msg,true);
	}
	else if (message.isType("ShutdownCluster"))
	{
		LOG("CentralServerShutdown",("Received ShutdownCluster network message."));
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ShutdownCluster m(ri);
		startShutdownProcess(m.getTimeToShutdown(), m.getMaxTime(), m.getSystemMessage());
	}
	else if (message.isType("AbortShutdown"))
	{
		LOG("CentralServerShutdown",("Received AbortShutdown network message."));
		abortShutdownProcess();
	}


	else if (message.isType("SetSceneForPlayer"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<NetworkId, std::pair<std::string, bool> > > ssfp(ri);

		if (ssfp.getValue().second.first.empty())
		{
			static const std::string loginTrace("TRACE_LOGIN");

			IGNORE_RETURN(m_playerSceneMap.erase(ssfp.getValue().first));
			LOG(loginTrace, ("Forgetting sceneId for character (%s) -- will query the database on next login", ssfp.getValue().first.getValueString().c_str()));
		}
		else
		{
			PlayerSceneMapType::iterator i = m_playerSceneMap.find(ssfp.getValue().first);
			if (i != m_playerSceneMap.end())
			{
				i->second.first = ssfp.getValue().second.first;
				if (ssfp.getValue().second.second)
					i->second.second = ::time(nullptr) + static_cast<time_t>(ConfigCentralServer::getCtsDenyLoginThresholdSeconds());
			}
			else
			{
				m_playerSceneMap[ssfp.getValue().first]=std::make_pair(ssfp.getValue().second.first, (ssfp.getValue().second.second ? (::time(nullptr) + static_cast<time_t>(ConfigCentralServer::getCtsDenyLoginThresholdSeconds())) : 0));
			}
		}
	}

	else if (message.isType("TaskProcessDiedMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		TaskProcessDiedMessage died(ri);
		LOG("TaskProcessDied", ("received TaskProcessDied for %s:%i", died.getProcessName().c_str(), died.getProcessId()));;
		if(died.getProcessName().find("SwgGameServer") != std::string::npos)
		{
			LOG("TaskProcessDied", ("Dead process %i is a game server", died.getProcessId()));
			// extract sceneId
			size_t pos = died.getProcessName().find("sceneID=");
			if(pos != std::string::npos)
			{
				pos += std::string("sceneID=").length();
				size_t end = died.getProcessName().find_first_of(' ', pos);
				if(end != std::string::npos)
				{
					std::string scene = died.getProcessName().substr(pos, end - pos);
					LOG("TaskProcessDied", ("Dead game server process %i was running sceneID %s, advising planet server", died.getProcessId(), scene.c_str()));
					std::map<std::string, PlanetServerConnection *>::iterator f = m_planetServers.find(scene);
					if(f != m_planetServers.end())
					{
						f->second->send(died, true);
					}
				}
			}
		}
	}
	else if (message.isType("SystemTimeMismatchNotification"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::string> msg(ri);

		m_lastTimeSystemTimeMismatchNotification = time(0);
		m_lastTimeSystemTimeMismatchNotificationDescription = msg.getValue();
	}
	else if (message.isType("DisconnectedTaskManagerMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::string> msg(ri);

		m_disconnectedTaskManagerList = msg.getValue();
	}
	else if(message.isType("TransferServerConnectionClosed"))
	{
		// connect to the character transfer server
		if(ConfigCentralServer::getTransferServerPort())
		{
			if(s_retryTransferServerConnection)
			{
				getInstance().m_transferServerConnection = new TransferServerConnection(ConfigCentralServer::getTransferServerAddress(), ConfigCentralServer::getTransferServerPort());
			}
		}
	}
	else if(message.isType("StationPlayersCollectorConnectionClosed"))
	{
		// connect to the station players collector
		if(ConfigCentralServer::getStationPlayersCollectorPort())
		{
			if(s_retryStationPlayersCollectorConnection)
			{
				getInstance().m_stationPlayersCollectorConnection = new StationPlayersCollectorConnection(ConfigCentralServer::getStationPlayersCollectorAddress(), ConfigCentralServer::getStationPlayersCollectorPort());
			}
		}
	}
	else if(message.isType("ClaimRewardsMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ClaimRewardsMessage msg(ri);
		sendToArbitraryLoginServer(msg);
	}
	else if(message.isType("ClaimRewardsReplyMessage"))
	{
		DEBUG_REPORT_LOG(true,("Central got ClaimRewardsReplyMessage\n"));
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ClaimRewardsReplyMessage msg(ri);
		sendToGameServer(msg.getGameServer(), msg, true);
	}
	else if (message.isType("PurgeStructuresForAccountMessage") || message.isType("WarnStructuresAboutPurgeMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<StationId> msg(ri);
		LoginServerConnection const * l = dynamic_cast<LoginServerConnection const *>(&source);
		if (l)
		{
			m_purgeAccountToLoginServerMap[msg.getValue()]= l->getProcessId(); // remember which login server is handling this purge
			sendToDBProcess(msg, true);
		}
	}
	else if (message.isType("PurgeCompleteMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<StationId> msg(ri);

		std::map<StationId, uint32>::iterator i=m_purgeAccountToLoginServerMap.find(msg.getValue());
		if (i!=m_purgeAccountToLoginServerMap.end())
		{
			sendToLoginServer(i->second,msg);
			m_purgeAccountToLoginServerMap.erase(i);
		}
	}
	else if (message.isType("RestartServerMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		RestartServerMessage msg(ri);

		sendToPlanetServer(msg.getScene(), msg, true);
	}
	else if (message.isType("RestartServerByRoleMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::string, uint32> > msg(ri);

		sendToPlanetServer(msg.getValue().first, msg, true);
	}
	else if (message.isType("ExcommunicateGameServerMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ExcommunicateGameServerMessage msg(ri);

		excommunicateServer(msg);
	}
	else if (message.isType("RestartPlanetMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::string> msg(ri);

		GenericValueTypeMessage<int> const shutdownMsg("ShutdownMessage", 0); 
		sendToPlanetServer(msg.getValue(),shutdownMsg, true);
	}
	else if (message.isType("UpdateClusterLockedAndSecretState"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<bool, bool> > const msg(ri);

		gs_clusterIsLocked = msg.getValue().first;
		gs_clusterIsSecret = msg.getValue().second;
	}
	else if (message.isType("PopStatRsp"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::map<std::string, int> > const msg(ri);

		m_timePopulationStatisticsRefresh = ::time(nullptr);
		m_populationStatistics = msg.getValue();
	}
	else if (message.isType("GcwScoreStatRsp"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::map<std::string, int>, std::pair<std::map<std::string, std::pair<int64, int64> >, std::map<std::string, std::pair<int64, int64> > > > > const msg(ri);

		m_timeGcwScoreStatisticsRefresh = ::time(nullptr);
		std::string const timeGcwScoreStatisticsRefreshStr = CalendarTime::convertEpochToTimeStringLocal(m_timeGcwScoreStatisticsRefresh);

		std::map<std::string, int> const & gcwImperialScorePercentile = msg.getValue().first;
		std::map<std::string, std::pair<int64, int64> > const & gcwImperialScore = msg.getValue().second.first;
		std::map<std::string, std::pair<int64, int64> > const & gcwRebelScore = msg.getValue().second.second;
		std::map<std::string, std::pair<int64, int64> >::const_iterator iterFind;
		std::map<std::string, int>::const_iterator iterFindImperialScorePercentile;
		std::pair<std::map<std::string, std::pair<int, std::pair<std::string, std::string> > >::iterator, bool> iterInsertion;
		int imperialScorePercentile;
		std::string scoreText, scoreTextDesc;

		for (std::map<std::string, std::pair<int64, int64> >::const_iterator iterImp = gcwImperialScore.begin(); iterImp != gcwImperialScore.end(); ++iterImp)
		{
			iterFindImperialScorePercentile = gcwImperialScorePercentile.find(iterImp->first);
			if (iterFindImperialScorePercentile != gcwImperialScorePercentile.end())
				imperialScorePercentile = iterFindImperialScorePercentile->second;
			else
				imperialScorePercentile = 0;

			iterFind = gcwRebelScore.find(iterImp->first);
			if (iterFind != gcwRebelScore.end())
				scoreText = FormattedString<1024>().sprintf("Imp (%s, %s) Reb (%s, %s) - ", NetworkId(iterImp->second.first).getValueString().c_str(), NetworkId(iterImp->second.second).getValueString().c_str(), NetworkId(iterFind->second.first).getValueString().c_str(), NetworkId(iterFind->second.second).getValueString().c_str());
			else
				scoreText = FormattedString<1024>().sprintf("Imp (%s, %s) Reb (0, 0) - ", NetworkId(iterImp->second.first).getValueString().c_str(), NetworkId(iterImp->second.second).getValueString().c_str());

			scoreTextDesc = scoreText + timeGcwScoreStatisticsRefreshStr;

			iterInsertion = m_gcwScoreStatistics.insert(std::make_pair("gcwScore." + iterImp->first, std::make_pair(imperialScorePercentile, std::make_pair(scoreText, scoreTextDesc))));
			if (!iterInsertion.second)
			{
				if (iterInsertion.first->second.second.first != scoreText)
				{
					iterInsertion.first->second.first = imperialScorePercentile;
					iterInsertion.first->second.second.first = scoreText;
					iterInsertion.first->second.second.second = scoreTextDesc;
				}
			}
		}

		for (std::map<std::string, std::pair<int64, int64> >::const_iterator iterReb = gcwRebelScore.begin(); iterReb != gcwRebelScore.end(); ++iterReb)
		{
			iterFind = gcwImperialScore.find(iterReb->first);
			if (iterFind == gcwImperialScore.end())
			{
				iterFindImperialScorePercentile = gcwImperialScorePercentile.find(iterReb->first);
				if (iterFindImperialScorePercentile != gcwImperialScorePercentile.end())
					imperialScorePercentile = iterFindImperialScorePercentile->second;
				else
					imperialScorePercentile = 0;

				scoreText = FormattedString<1024>().sprintf("Imp (0, 0) Reb (%s, %s) - ", NetworkId(iterReb->second.first).getValueString().c_str(), NetworkId(iterReb->second.second).getValueString().c_str());
				scoreTextDesc = scoreText + timeGcwScoreStatisticsRefreshStr;

				iterInsertion = m_gcwScoreStatistics.insert(std::make_pair("gcwScore." + iterReb->first, std::make_pair(imperialScorePercentile, std::make_pair(scoreText, scoreTextDesc))));
				if (!iterInsertion.second)
				{
					if (iterInsertion.first->second.second.first != scoreText)
					{
						iterInsertion.first->second.first = imperialScorePercentile;
						iterInsertion.first->second.second.first = scoreText;
						iterInsertion.first->second.second.second = scoreTextDesc;
					}
				}
			}
		}
	}
	else if (message.isType("LLTStatRsp"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::map<int, std::pair<std::string, int> >, std::map<int, std::pair<std::string, int> > > > const msg(ri);

		m_timeLastLoginTimeStatisticsRefresh = ::time(nullptr);
		m_lastLoginTimeStatistics = msg.getValue().first;
		m_createTimeStatistics = msg.getValue().second;
	}
	else if (message.isType("LfgStatRsp"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<unsigned long, std::pair<unsigned long, unsigned long> > > const msg(ri);

		m_numberOfCharacterMatchRequests += msg.getValue().first;
		m_numberOfCharacterMatchResults += msg.getValue().second.first;
		m_timeSpentOnCharacterMatchRequestsMs += msg.getValue().second.second;
	}
	else if (message.isType("OccupyUnlockedSlotRsp"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::pair<int, NetworkId>, uint32> > const occupyUnlockedSlotRsp(ri);

		// CS log the response
		LoginUpgradeAccountMessage::OccupyUnlockedSlotResponse const response = static_cast<LoginUpgradeAccountMessage::OccupyUnlockedSlotResponse>(occupyUnlockedSlotRsp.getValue().first.first);
		if (response == LoginUpgradeAccountMessage::OUSR_success)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s OccupyUnlockedSlot request SUCCESS", occupyUnlockedSlotRsp.getValue().first.second.getValueString().c_str()));
		}
		else if (response == LoginUpgradeAccountMessage::OUSR_db_error)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s OccupyUnlockedSlot request FAILED - internal db error", occupyUnlockedSlotRsp.getValue().first.second.getValueString().c_str()));
		}
		else if (response == LoginUpgradeAccountMessage::OUSR_account_has_no_unlocked_slot)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s OccupyUnlockedSlot request FAILED - account doesn't have an unlocked slot", occupyUnlockedSlotRsp.getValue().first.second.getValueString().c_str()));
		}
		else if (response == LoginUpgradeAccountMessage::OUSR_account_has_no_unoccupied_unlocked_slot)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s OccupyUnlockedSlot request FAILED - account has no unoccupied unlocked slot", occupyUnlockedSlotRsp.getValue().first.second.getValueString().c_str()));
		}
		else if (response == LoginUpgradeAccountMessage::OUSR_cluster_already_has_unlocked_slot_character)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s OccupyUnlockedSlot request FAILED - cluster already has an unlocked slot character", occupyUnlockedSlotRsp.getValue().first.second.getValueString().c_str()));
		}
		else
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s OccupyUnlockedSlot request FAILED - unknown result code (%d)", occupyUnlockedSlotRsp.getValue().first.second.getValueString().c_str(), occupyUnlockedSlotRsp.getValue().first.first));
		}

		GameServerConnection * gs = getGameServer(occupyUnlockedSlotRsp.getValue().second);
		if (!gs)
			gs = getRandomGameServer();

		if (gs)
			gs->send(occupyUnlockedSlotRsp, true);
	}
	else if (message.isType("VacateUnlockedSlotRsp"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::pair<int, NetworkId>, std::pair<uint32, uint32> > > const vacateUnlockedSlotRsp(ri);

		// CS log the response
		LoginUpgradeAccountMessage::VacateUnlockedSlotResponse const response = static_cast<LoginUpgradeAccountMessage::VacateUnlockedSlotResponse>(vacateUnlockedSlotRsp.getValue().first.first);
		if (response == LoginUpgradeAccountMessage::VUSR_success)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s VacateUnlockedSlot request SUCCESS", vacateUnlockedSlotRsp.getValue().first.second.getValueString().c_str()));

			// to safeguard against any sort of timing exploit to create another normal
			// slot character while this one is being converted to normal, thus allowing
			// the account to have more normal slot characters on this cluster than is
			// allowed, make it look like a normal character has been created on this
			// account on this cluster, and that mechanism will prevent such an exploit

			// let all connection servers know that a new character has been created for the station account
			GenericValueTypeMessage<StationId> const ncc("NewCharacterCreated", static_cast<StationId>(vacateUnlockedSlotRsp.getValue().second.first));
			CentralServer::getInstance().sendToAllConnectionServers(ncc, true);
		}
		else if (response == LoginUpgradeAccountMessage::VUSR_db_error)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s VacateUnlockedSlot request FAILED - internal db error", vacateUnlockedSlotRsp.getValue().first.second.getValueString().c_str()));
		}
		else if (response == LoginUpgradeAccountMessage::VUSR_account_has_no_unlocked_slot)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s VacateUnlockedSlot request FAILED - account doesn't have an unlocked slot", vacateUnlockedSlotRsp.getValue().first.second.getValueString().c_str()));
		}
		else if (response == LoginUpgradeAccountMessage::VUSR_not_unlocked_slot_character)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s VacateUnlockedSlot request FAILED - character is not an unlocked slot character", vacateUnlockedSlotRsp.getValue().first.second.getValueString().c_str()));
		}
		else if (response == LoginUpgradeAccountMessage::VUSR_no_available_normal_character_slot)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s VacateUnlockedSlot request FAILED - no available normal character slot for the account on this galaxy", vacateUnlockedSlotRsp.getValue().first.second.getValueString().c_str()));
		}
		else
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s VacateUnlockedSlot request FAILED - unknown result code (%d)", vacateUnlockedSlotRsp.getValue().first.second.getValueString().c_str(), vacateUnlockedSlotRsp.getValue().first.first));
		}

		GameServerConnection * gs = getGameServer(vacateUnlockedSlotRsp.getValue().second.second);
		if (!gs)
			gs = getRandomGameServer();

		if (gs)
			gs->send(vacateUnlockedSlotRsp, true);
	}
	else if (message.isType("SwapUnlockedSlotRsp"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<std::pair<int, NetworkId>, std::pair<uint32, std::pair<NetworkId, std::string> > > > const swapUnlockedSlotRsp(ri);

		// CS log the response
		LoginUpgradeAccountMessage::SwapUnlockedSlotResponse const response = static_cast<LoginUpgradeAccountMessage::SwapUnlockedSlotResponse>(swapUnlockedSlotRsp.getValue().first.first);
		if (response == LoginUpgradeAccountMessage::SUSR_success)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s SwapUnlockedSlot with %s (%s) request SUCCESS", swapUnlockedSlotRsp.getValue().first.second.getValueString().c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str(), swapUnlockedSlotRsp.getValue().second.second.second.c_str()));
		}
		else if (response == LoginUpgradeAccountMessage::SUSR_db_error)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s SwapUnlockedSlot with %s request FAILED - internal db error", swapUnlockedSlotRsp.getValue().first.second.getValueString().c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str()));
		}
		else if (response == LoginUpgradeAccountMessage::SUSR_account_has_no_unlocked_slot)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s SwapUnlockedSlot with %s request FAILED - account doesn't have an unlocked slot", swapUnlockedSlotRsp.getValue().first.second.getValueString().c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str()));
		}
		else if (response == LoginUpgradeAccountMessage::SUSR_not_unlocked_slot_character)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s SwapUnlockedSlot with %s request FAILED - source character is not an unlocked slot character", swapUnlockedSlotRsp.getValue().first.second.getValueString().c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str()));
		}
		else if (response == LoginUpgradeAccountMessage::SUSR_invalid_target_character)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s SwapUnlockedSlot with %s request FAILED - target character is either not valid, not on the same account, or not on this galaxy", swapUnlockedSlotRsp.getValue().first.second.getValueString().c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str()));
		}
		else if (response == LoginUpgradeAccountMessage::SUSR_target_character_already_unlocked_slot_character)
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s SwapUnlockedSlot with %s request FAILED - target character is already an unlocked slot character", swapUnlockedSlotRsp.getValue().first.second.getValueString().c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str()));
		}
		else
		{
			LOG("CustomerService",("JediUnlockedSlot:Player %s SwapUnlockedSlot with %s request FAILED - unknown result code (%d)", swapUnlockedSlotRsp.getValue().first.second.getValueString().c_str(), swapUnlockedSlotRsp.getValue().second.second.first.getValueString().c_str(), swapUnlockedSlotRsp.getValue().first.first));
		}

		GameServerConnection * gs = getGameServer(swapUnlockedSlotRsp.getValue().second.first);
		if (!gs)
			gs = getRandomGameServer();

		if (gs)
			gs->send(swapUnlockedSlotRsp, true);
	}
	else if (message.isType("AdjustAccountFeatureIdResponse"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		AdjustAccountFeatureIdResponse const msg(ri);

		GameServerConnection * gs = getGameServer(msg.getGameServer());
		if (!gs)
			gs = getRandomGameServer();

		if (gs)
			gs->send(msg, true);
	}
	else if (message.isType("AccountFeatureIdResponse"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		AccountFeatureIdResponse const msg(ri);

		GameServerConnection * gs = getGameServer(msg.getGameServer());
		if (!gs)
			gs = getRandomGameServer();

		if (gs)
			gs->send(msg, true);
	}
	else if (message.isType("FeatureIdTransactionResponse"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		FeatureIdTransactionResponse const msg(ri);

		GameServerConnection * gs = getGameServer(msg.getGameServer());
		if (!gs)
			gs = getRandomGameServer();

		if (gs)
			gs->send(msg, true);
	}
	else if(ClusterWideDataManagerList::handleMessage(source, message))
	{
		// nothing else to do with the message since it was
		// handled by the Cluster wide data manager
	}
}

//-----------------------------------------------------------------------
void CentralServer::removeConnectionServerConnection(const ConnectionServerConnection * conn)
{
	DEBUG_REPORT_LOG(true, ("Removing connection server connection\n"));
	if (conn)
	{
		startConnectionServer(conn->getConnectionServerNumber(),
							  conn->getGameServiceAddress(),
							  conn->getClientServicePortPublic(),
							  conn->getClientServicePortPrivate(), true,
							  ConfigCentralServer::getConnectionServerRestartDelayTimeSeconds());

		ConnectionServerConnectionList::iterator i = m_connectionServerConnections.begin();
		for(;i != m_connectionServerConnections.end();++i)
		{
			if (conn->getId() == (*i)->getId())
			{
				ConnectionServerDown c((*i)->getId());
				sendToAllLoginServers(c);
				IGNORE_RETURN(m_connectionServerConnections.erase(i));
				return;
			}

		}

		// Clean up all the account info for the connection
		removeFromAccountConnectionMap(conn->getId());
	}
}
//-----------------------------------------------------------------------
/**
* Removes all references to a game server from the server.
*
* @param gameServer		the game server connection to remove
*/
void CentralServer::removeGameServer(GameServerConnection const *gameServer)
{
	if (gameServer != nullptr)
	{
		uint32 const pid = gameServer->getProcessId();

		LOG("ServerStartup", ("Game Server %lu went down", pid));

		//@todo this whole function needs a re-write.
		NOT_NULL(gameServer);
		std::map<uint32, GameServerConnection *>::iterator const i = m_gameServerConnections.find(pid);
		if (i == m_gameServerConnections.end())
			return;

		for (std::vector<GameServerConnection *>::iterator ii = m_gameServerConnectionsList.begin(); ii != m_gameServerConnectionsList.end();)
		{
			if ((*ii) == i->second)
				ii = m_gameServerConnectionsList.erase(ii);
			else
				++ii;
		}

		m_gameServerConnections.erase(i);

		/** @todo: this is slow, find a better way (probably by creating reverse
		* lookup table(s))
		*/
		if (pid == m_dbProcessServerProcessId)
		{
			DEBUG_REPORT_LOG(true, ("Database process died -- Central will exit and let the cluster restart\n"));
			m_done = true;
			return; //lint !e527 Unreachable
		}

		DEBUG_WARNING(true, ("Game server %lu crashed", pid));

		for (SceneGameMap::iterator j = m_gameServers.begin(); j != m_gameServers.end();)
		{
			if ((*j).second == gameServer)
				m_gameServers.erase(j++);
			else
				++j;
		}

		UniverseManager::getInstance().onGameServerDisconnect(*gameServer);
		PlanetManager::onGameServerDisconnect(gameServer);
		CharacterCreationTracker::getInstance().onGameServerDisconnect(pid);
		ClusterWideDataManagerList::onGameServerDisconnect(pid);

		ExcommunicateGameServerMessage const excommunicateMessage(pid, 0, "");
		sendToAllGameServersExceptDBProcess(excommunicateMessage, true);
	}
	else 
	{
		DEBUG_WARNING(true, ("A game server crashed but our process ID ptr is nullptr."));
	}
}

//-----------------------------------------------------------------------

void CentralServer::done()
{
	getInstance().m_done = true;
}

//-----------------------------------------------------------------------

void CentralServer::run(void)
{
	SetupSharedLog::install("CentralServer");
	SetupSharedUtility::Data utilityData;
	SetupSharedUtility::install (utilityData);
	SetupServerUtility::install();

	gs_connectionServersPublic = ConfigCentralServer::getStartPublic();
	// listen for game server connect requests
	static CentralServer & cserver = getInstance();

	NetworkSetupData setup;
	setup.maxConnections = 100;
	setup.bindInterface = ConfigCentralServer::getGameServiceBindInterface();
	setup.port = ConfigCentralServer::getGameServicePort();
	Service * gs = new Service(ConnectionAllocator<GameServerConnection>(), setup);
	cserver.m_gameService = gs;

	// listen for planet server connection requests
	setup.bindInterface = ConfigCentralServer::getPlanetServiceBindInterface();
	setup.port = ConfigCentralServer::getPlanetServicePort();
	Service * ps = new Service(ConnectionAllocator<PlanetServerConnection>(), setup);
	cserver.m_planetService = ps;

	setup.port = ConfigCentralServer::getChatServicePort();
	setup.maxConnections = 32;
	setup.bindInterface = ConfigCentralServer::getChatServiceBindInterface();
	Service * cs = new Service(ConnectionAllocator<ChatServerConnection>(), setup);
	cserver.m_chatService = cs;

	setup.port = ConfigCentralServer::getCustomerServicePort();
	setup.bindInterface = ConfigCentralServer::getCustomerServiceBindInterface();
	Service * css = new Service(ConnectionAllocator<CustomerServiceConnection>(), setup);
	cserver.m_csService = css;

	setup.port = ConfigCentralServer::getConnectionServicePort();
	setup.bindInterface = ConfigCentralServer::getConnectionServiceBindInterface();
	Service * cons = new Service(ConnectionAllocator<ConnectionServerConnection>(), setup);
	NOT_NULL(cons);
	cserver.m_connService = cons;

	setup.port = ConfigCentralServer::getConsoleServicePort();
	setup.bindInterface = ConfigCentralServer::getConsoleServiceBindInterface();
	cserver.m_consoleService = new Service(ConnectionAllocator<ConsoleConnection>(), setup);

	setup.port = ConfigCentralServer::getCommodityServerServicePort();
	setup.bindInterface = ConfigCentralServer::getCommodityServerServiceBindInterface();
	s_commodityServerService = new Service(ConnectionAllocator<ServerConnection>(), setup);
	
	setup.port = ConfigCentralServer::getLoginServicePort();
	if (ConfigCentralServer::getDevelopmentMode())
		cserver.connectToLoginServer();
	else
	{
		setup.bindInterface = ConfigCentralServer::getLoginServiceBindInterface();
		cserver.m_loginService = new Service(ConnectionAllocator<LoginServerConnection>(), setup);
	}

	// connect to the task manager
	cserver.m_taskManager = new TaskConnection("127.0.0.1", ConfigCentralServer::getTaskManagerPort());
	
	unsigned long startTime = Clock::timeMs();
	unsigned long nextLoadingLogTime=0;
	unsigned long nextPingTime=0;
	unsigned long nextPopulationLogTime = 0;

	LOG("ServerStartup",("CentralServer starting"));
#ifndef WIN32
	if( FileExists( ".shutdown") )
	{
		LOG("CentralServer", ("Removing stale .shutdown file."));
		IGNORE_RETURN(::remove( ".shutdown" ));
	}
	if( FileExists( ".abortshutdown" ) )
	{
		LOG("CentralServer", ("Removing stale .abortshutdown file."));
		IGNORE_RETURN(::remove( ".abortshutdown" ));
	}
	if( FileExists( ".startanymissingplanet") )
	{
		LOG("CentralServer", ("Removing stale .startanymissingplanet file."));
		IGNORE_RETURN(::remove( ".startanymissingplanet" ));
	}
	if( FileExists( ".startanymissinggameserver") )
	{
		LOG("CentralServer", ("Removing stale .startanymissinggameserver file."));
		IGNORE_RETURN(::remove( ".startanymissinggameserver" ));
	}
#endif

	while (!cserver.m_done)
	{
		unsigned long lastFrameTime = 0;
		unsigned long frameStartTime = Clock::timeMs();
		
		PROFILER_AUTO_BLOCK_DEFINE("main loop");

		bool barrierReached = true;

		do
		{
			PROFILER_AUTO_BLOCK_DEFINE("BarrierWait");

			if (!Os::update())
				cserver.m_done = true;

			{
				PROFILER_AUTO_BLOCK_DEFINE("NetworkHandler::update");
				NetworkHandler::update();
			}

			{
				PROFILER_AUTO_BLOCK_DEFINE("MetricsManager::update");
				unsigned long curTime = Clock::timeMs();
				MetricsManager::update(static_cast<float>(curTime - startTime));
				startTime = curTime;
			}

			if (ConfigCentralServer::getShouldSleep())
			{
				PROFILER_AUTO_BLOCK_DEFINE("Os::sleep");
				Os::sleep(1);
			}

		} while (!barrierReached && !cserver.m_done);

		//@todo Central needs to run a clock so we can schedule re-tries with the login server.
		{
			PROFILER_AUTO_BLOCK_DEFINE("NetworkHandler::dispatch");
			NetworkHandler::dispatch();
		}

		{
			PROFILER_AUTO_BLOCK_DEFINE("update");
			cserver.update();
		}

		NetworkHandler::clearBytesThisFrame();

		{
			//Peridically log loading status
			unsigned long curTime = Clock::timeMs();
			if (curTime > nextLoadingLogTime)
			{
				nextLoadingLogTime = curTime + 10000;
				for (std::set<std::string>::const_iterator i=getInstance().m_planetsWaitingForPreload.begin(); i!=getInstance().m_planetsWaitingForPreload.end(); ++i)
					LOG("Preload",("Waiting for planet %s",i->c_str()));
			}

			//Perodically ping all the servers
			if (ConfigCentralServer::getServerPingTimeout() != 0 && static_cast<int>(curTime-nextPingTime) > 0)
			{
				nextPingTime = curTime + static_cast<uint32>(ConfigCentralServer::getServerPingTimeout() * 1000);
				getInstance().doServerPings();
			}

			//Periodically write out population to a log every 5 minutes
			if (curTime > nextPopulationLogTime)
			{
				nextPopulationLogTime = curTime + 300000;
				LOG("PopulationLog", ("Current Population: %d", getInstance().getPlayerCount()));
			}
		}




		ServerClock::getInstance().incrementServerFrame();
		if(ConfigCentralServer::getShutdown())
		{
			cserver.done();
		}

		unsigned long currentTime = Clock::timeMs();
		lastFrameTime = currentTime - frameStartTime;
		if(lastFrameTime > 1000)
		{
			LOG("profile", ("Long loop (%u ms):\n%s", lastFrameTime, PROFILER_GET_LAST_FRAME_DATA()));
		}
	}

	LOG("ServerStartup",("CentralServer exiting"));

	SetupSharedLog::remove();
	CentralServer::remove();
}

// ----------------------------------------------------------------------
/**
 * Called every frame to do any required updates.
 */
void CentralServer::update()
{
	static int loopCount = 0;
	static int apiLoopCount = 0;
	static int shutdownCheckLoopCount = 0;

	m_curTime = static_cast<uint32>(time(0));
	
	// Tell the LoginServers if necessary
	if ((++loopCount > ConfigCentralServer::getUpdatePlayerCountFrequency()))
	{
		loopCount = 0;

		// Update the population on the server
		sendPopulationUpdateToLoginServer();
	}


	// update the webAPI if specified
	int webUpdateIntervalSeconds = ConfigCentralServer::getWebUpdateIntervalSeconds();
        std::string updateURL = std::string(ConfigCentralServer::getMetricsDataURL());

	// assuming that every 5th frame is ~1 second, we can multiply and then check
	if ( !(updateURL.empty()) && webUpdateIntervalSeconds && (++apiLoopCount > (webUpdateIntervalSeconds*1000)) )
	{
		apiLoopCount = 0;

		// update the web api
		sendMetricsToWebAPI(updateURL);
	}

	if ( ConfigCentralServer::getAuctionEnabled() ) // allow auctions?
	{
		if ( m_pAuctionTransferClient == nullptr )
		{
               		const char* hostName[1] = { ConfigCentralServer::getAuctionServer() };
               		const short port[1] = { (short)ConfigCentralServer::getAuctionPort() };

               		std::string s_id = ConfigCentralServer::getAuctionIDPrefix();
			s_id += ConfigCentralServer::getClusterName(); 

       			const char *identifier[1];
               		identifier[ 0 ] = s_id.c_str();

               		m_pAuctionTransferClient = new AuctionTransferClient( hostName, port, 1, identifier, 1 );
		}
		m_pAuctionTransferClient->process();
  	}
	else if ( m_pAuctionTransferClient )
	{
		delete( m_pAuctionTransferClient );
		m_pAuctionTransferClient = 0;
	}

	// check every 5th frame (one second roughly?)
	if ( ++shutdownCheckLoopCount > 5 )
	{
		shutdownCheckLoopCount = 0;

		checkShutdownProcess();
	}

	// update cluster wide data manager
	ClusterWideDataManagerList::update(Clock::frameTime());
}

// ----------------------------------------------------------------------
void CentralServer::sendPopulationUpdateToLoginServer()
{
	// Add up all the population totals (these are referenced by the metrics data)
	m_totalPlayerCount        = 0;
	m_totalFreeTrialCount     = 0;
	m_totalEmptySceneCount    = 0;
	m_totalTutorialSceneCount = 0;
	m_totalFalconSceneCount   = 0;

	
	ConnectionServerConnectionList::const_iterator i;
	for (i=m_connectionServerConnections.begin(); i!=m_connectionServerConnections.end(); ++i)
	{
		if (*i)
		{
			m_totalPlayerCount        += (**i).getPlayerCount();
			m_totalFreeTrialCount     += (**i).getFreeTrialCount();
			m_totalEmptySceneCount    += (**i).getEmptySceneCount();
			m_totalTutorialSceneCount += (**i).getTutorialSceneCount();
			m_totalFalconSceneCount   += (**i).getFalconSceneCount();
		}
	}

	bool loadedRecently=false;
	if (!isPreloadFinished() || (time(0)-m_lastLoadingStateTime < static_cast<time_t>(ConfigCentralServer::getRecentLoadingStateSeconds())))
		loadedRecently=true;

	UpdatePlayerCountMessage upm(loadedRecently, m_totalPlayerCount, m_totalFreeTrialCount, m_totalEmptySceneCount, m_totalTutorialSceneCount, m_totalFalconSceneCount);
	sendToAllLoginServers(upm);
}

void CentralServer::sendMetricsToWebAPI(std::string updateURL)
{
	std::ostringstream postBuf;

	postBuf << "totalPlayerCount=" << m_totalPlayerCount << "&totalGameServers=" << m_gameServers.size() - 1 << "&totalPlanetServers=" << m_planetServers.size() << "&isPublic=" << getIsClusterPublic() << "&isLocked=" << getIsClusterLocked() << "&isSecret=" << getIsClusterSecret() << "&preloadFinished=" << getClusterStartupTime() << "&databasebacklogged=" << isDatabaseBacklogged() << "&totalTutorialSceneCount=" << m_totalTutorialSceneCount << "&totalFalconSceneCount=" << m_totalFalconSceneCount;

	std::string response = webAPI::simplePost(updateURL, std::string(postBuf.str()), "");
	WARNING((response != "success"), ("Error sending stats: %s", response.c_str()));
}

//-----------------------------------------------------------------------

void CentralServer::sendTaskMessage(const GameNetworkMessage & source)
{
	if(m_taskManager)
		m_taskManager->send(source, true);
	DEBUG_REPORT_LOG(!m_taskManager, ("There is no task manager connection, but app is attempting to send to one\n"));
}

//-----------------------------------------------------------------------

void CentralServer::sendToGameServer(const uint32 gameServerProcessId, const GameNetworkMessage & message, const bool reliable) const
{
	GameServerConnection * g = getGameServer(gameServerProcessId);
	if(g)
	{
		g->send(message, reliable);
	}
	else
	{
		DEBUG_WARNING(true,("Attempted to send to game server %i, without connection.\n",gameServerProcessId));
	}
}

// ----------------------------------------------------------------------

void CentralServer::sendToLoginServer(uint32 loginServerId, const GameNetworkMessage &message)
{
	LoginServerConnectionMapType::iterator i=m_loginServerConnectionMap.find(loginServerId);
	if (i!=m_loginServerConnectionMap.end() && i->second)
		i->second->send(message,true);
}

// ----------------------------------------------------------------------

void CentralServer::sendToAllGameServers(const GameNetworkMessage & message, const bool reliable)
{
	for (std::map<uint32, GameServerConnection *>::const_iterator i = m_gameServerConnections.begin(); i!=m_gameServerConnections.end(); ++i)
	{
		(*i).second->send(message, reliable);
	}
}

// ----------------------------------------------------------------------

void CentralServer::sendToAllGameServersExceptDBProcess(const GameNetworkMessage & message, const bool reliable)
{
	for (std::map<uint32, GameServerConnection *>::const_iterator i = m_gameServerConnections.begin(); i!=m_gameServerConnections.end(); ++i)
	{
		if ((*i).first != getDbProcessServerProcessId())
			(*i).second->send(message, reliable);
	}
}

// ----------------------------------------------------------------------

void CentralServer::sendToAllPlanetServers(const GameNetworkMessage & message, const bool reliable)
{
	for (std::map<std::string, PlanetServerConnection *>::const_iterator i = m_planetServers.begin(); i != m_planetServers.end(); ++i)
		(*i).second->send(message, reliable);
}

// ----------------------------------------------------------------------

bool CentralServer::hasPlanetServer(const std::string & sceneName) const
{
	for (std::map<std::string, PlanetServerConnection *>::const_iterator i = m_planetServers.begin(); i != m_planetServers.end(); ++i)
		if (i->second->getSceneId() == sceneName)
			return true;
	return false;
}

// ----------------------------------------------------------------------

void CentralServer::sendToPlanetServer(const std::string &sceneId, const GameNetworkMessage & message, const bool reliable)
{
	for (std::map<std::string, PlanetServerConnection *>::const_iterator i = m_planetServers.begin(); i != m_planetServers.end(); ++i)
		if (i->second->getSceneId() == sceneId)
			(*i).second->send(message, reliable);
}

// ----------------------------------------------------------------------

void CentralServer::sendToAllConnectionServers(const GameNetworkMessage & message, const bool reliable,  Connection const * exclude /*= nullptr*/)
{
	// send to all connection servers
	for (ConnectionServerConnectionList::const_iterator i = m_connectionServerConnections.begin(); i != m_connectionServerConnections.end(); ++i)
	{
		if ((*i) && (*i != exclude))
			(*i)->send(message, reliable);
	}
}

//-----------------------------------------------------------------------

void CentralServer::sendToDBProcess(const GameNetworkMessage & message, const bool reliable) const
{
	GameServerConnection * g = getGameServer(getDbProcessServerProcessId());
	if(g)
	{
		g->send(message, reliable);
	}
	else
	{
		DEBUG_FATAL(!m_done,("Attempted to send message to DBProcess without connection (there should always be a connection to DBProcess)."));
	}
}

// ----------------------------------------------------------------------

bool CentralServer::hasDBConnection() const
{
	const GameServerConnection * g = getGameServer(getDbProcessServerProcessId());
	return (g != nullptr);
}

//-----------------------------------------------------------------------

void CentralServer::setTaskManager(TaskConnection * newTaskManager)
{
	if (m_taskManager && newTaskManager != m_taskManager)
	{
		m_taskManager->setDisconnectReason("CentralServer::setTaskManager called");
		m_taskManager->disconnect();
	}
	m_taskManager = newTaskManager;
}

// ----------------------------------------------------------------------

void CentralServer::removePlanetServer(const PlanetServerConnection * p)
{
	for (std::map<std::string, PlanetServerConnection *>::iterator i = m_planetServers.begin(); i != m_planetServers.end();)
	{
		if ((*i).second == p)
		{
			DEBUG_REPORT_LOG(true,("Central lost connection to Planet Server %s\n",p->getSceneId().c_str()));
			i = m_planetServers.erase(i);

			if (isPreloadFinished())
				m_timeClusterWentIntoLoadingState = 0;
			else if (m_timeClusterWentIntoLoadingState <= 0)
				m_timeClusterWentIntoLoadingState = time(0);

			return;
		}
		else
		{
			++i;
		}
	}
}

// ----------------------------------------------------------------------

const std::string & CentralServer::getCommandLine() const
{
	return m_commandLine;
}

//-----------------------------------------------------------------------

void CentralServer::setCommandLine(const std::string & c)
{
	m_commandLine = c;
}

//-----------------------------------------------------------------------

const bool getStartLocation(const std::string & name, std::string & planetName, Vector & coordinates, NetworkId & cellId)
{
	const StartingLocationData * const sld = StartingLocationManager::findLocationByName (name);
	if (sld)
	{
		planetName    = sld->planet;
		coordinates.x = sld->x;
		coordinates.y = sld->y;
		coordinates.z = sld->z;
		cellId        = NetworkId (sld->cellId);
		return true;
	}

	REPORT_LOG(true, ("The start location \"%s\" could not be found in StartingLocationManager", name.c_str()));
	return false;
}

//-----------------------------------------------------------------------

void CentralServer::startPlanetServer(const std::string & host, const std::string & sceneId, const SpawnDelaySeconds spawnDelay)
{
	FATAL(sceneId.empty(), ("CentralServer::startPlanetServer: empty sceneId, host='%s'", host.c_str()));
	std::map<std::string, std::pair<std::pair<std::string, std::string>, time_t> >::const_iterator f = m_pendingPlanetServers.find(sceneId);
	if(f == m_pendingPlanetServers.end())
	{
		std::map<std::string, PlanetServerConnection *>::const_iterator pf = m_planetServers.find(sceneId);
		if(pf == m_planetServers.end())
		{
			std::string options = "-s PlanetServer centralServerAddress=";
			if(m_planetService)
			{
				options += m_planetService->getBindAddress();
			}
			else
			{
				options += NetworkHandler::getHostName();
			}
			options += " sceneID=";
			options += sceneId;

			char buffer[20];
			IGNORE_RETURN(_itoa(m_nextPlanetWatcherPort++,buffer,10));
			options += " watcherServicePort=";
			options += buffer;

			static unsigned int portBase = 0;
			IGNORE_RETURN(_itoa(portBase,buffer,10));
			portBase += 100;
			options += " gameServerDebuggingPortBase=";
			options += buffer;
			
			TaskSpawnProcess spawn(host.empty() ? std::string("any") : host, "PlanetServer", options, spawnDelay);
			CentralServer::getInstance().sendTaskMessage(spawn);
			m_pendingPlanetServers[sceneId] = std::make_pair(std::make_pair(host, options), ::time(nullptr));
			IGNORE_RETURN(m_planetsWaitingForPreload.insert(sceneId));

			bool const preloadFinished = isPreloadFinished();
			if (preloadFinished)
				m_timeClusterWentIntoLoadingState = 0;
			else if (m_timeClusterWentIntoLoadingState <= 0)
				m_timeClusterWentIntoLoadingState = time(0);

			if(getInstance().m_transferServerConnection != nullptr && !preloadFinished)
			{
				getInstance().m_transferServerConnection->disconnect();
				getInstance().m_transferServerConnection = 0;
				s_retryTransferServerConnection = false;
			}
			
			if(getInstance().m_stationPlayersCollectorConnection != nullptr && ! isPreloadFinished())
			{
				getInstance().m_stationPlayersCollectorConnection->disconnect();
				getInstance().m_stationPlayersCollectorConnection = 0;
				s_retryStationPlayersCollectorConnection = false;
			}

			PreloadFinishedMessage msg(preloadFinished);
			sendToAllLoginServers(msg);
		}
	}
}

//-----------------------------------------------------------------------
/**
* Forward the RequestGameServerForLoginMessage to the appropriate PlanetServer
*/
void CentralServer::handleRequestGameServerForLoginMessage(const RequestGameServerForLoginMessage & msg)
{
	static const std::string loginTrace("TRACE_LOGIN");

	std::string effectiveScene;
	PlayerSceneMapType::iterator i=m_playerSceneMap.find(msg.getCharacterId());
	if (i!=m_playerSceneMap.end())
	{
		effectiveScene=i->second.first;
		LOG(loginTrace, ("using sceneId (%s) from memory for character (%s)", effectiveScene.c_str(), msg.getCharacterId().getValueString().c_str()));
	}
	else
		effectiveScene=msg.getScene(); // only use the DB scene if we don't have more recent information

	// for CTS, the source character must currently be on one of the 10 original ground planets
	if (msg.getForCtsSourceCharacter())
	{
		static std::set<std::string> s_tenOriginalGroundPlanets;
		if (s_tenOriginalGroundPlanets.empty())
		{
			IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("tatooine"));
			IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("naboo"));
			IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("corellia"));
			IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("rori"));
			IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("talus"));
			IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("yavin4"));
			IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("endor"));
			IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("lok"));
			IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("dantooine"));
			IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("dathomir"));
		}

		if (s_tenOriginalGroundPlanets.count(effectiveScene) <= 0)
		{
			// fail CTS because source character is not on one of the 10 original ground planets
			LOG("CustomerService", ("CharacterTransfer: RequestGameServerForLoginMessage failed for source character (%s) stationId (%lu) because effective scene for the character (%s) is not one of the 10 original ground planets", msg.getCharacterId().getValueString().c_str(), msg.getStationId(), effectiveScene.c_str()));

			GenericValueTypeMessage<std::pair<NetworkId, unsigned int> > const failureMsg("CtsSrcCharWrongPlanet", std::make_pair(msg.getCharacterId(), msg.getStationId()));
			IGNORE_RETURN(ConnectionServerConnection::sendToPseudoClientConnection(msg.getStationId(), failureMsg));

			return;
		}
	}

	PlanetServerConnection *conn = PlanetManager::getPlanetServerForScene(effectiveScene);
	if (conn)
	{
		LOG(loginTrace, ("handling RequestGameServerForLoginMessage(%s)", msg.getCharacterId().getValueString().c_str()));
		conn->send(msg,true);
	}
	else
	{
		bool isPlanetValid = false;
		bool allowDynamicStart = ConfigFile::getKeyBool("CentralServer", "startPlanetsDynamically", false);

		std::map<std::string, std::string>::const_iterator f = ms_sceneToHostMap.find(msg.getScene());
		if(f != ms_sceneToHostMap.end())
		{
			isPlanetValid = true;
		}

		if(isPlanetValid || allowDynamicStart)
		{
			LOG(loginTrace, ("deferring RequestGameServerForLoginMessage(%s)", msg.getCharacterId().getValueString().c_str()));
			DEBUG_REPORT_LOG(true, ("Starting planet server for login"));
			startPlanetServer(getHostForScene(msg.getScene()), msg.getScene(), 0);
			m_messagesWaitingForPlanetServer.push_back(Archive::ByteStream());
			msg.pack(m_messagesWaitingForPlanetServer.back());
		}
		else
		{
			WARNING_STRICT_FATAL(true, ("Central received a request to start a planet (name=\"%s\") that is not in the startPlanet= list in the CentralServer config file.", msg.getScene().c_str()));
		}
	}
}

// ----------------------------------------------------------------------

void CentralServer::handleRequestSceneTransfer(const RequestSceneTransfer &msg)
{
	PlanetServerConnection *conn = PlanetManager::getPlanetServerForScene(msg.getSceneName());
	if (conn)
	{
		conn->send(msg,true);
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("Starting planet server for login"));
		startPlanetServer(getHostForScene(msg.getSceneName()), msg.getSceneName(), 0);
		m_messagesWaitingForPlanetServer.push_back(Archive::ByteStream());
		msg.pack(m_messagesWaitingForPlanetServer.back());
	}
}

// ----------------------------------------------------------------------

/**
* Forward the GameServerForLoginMessage to the appropriate ConnectionServer
*/
void CentralServer::handleGameServerForLoginMessage(const GameServerForLoginMessage &msg)
{
	ConnectionServerConnection *conn = getConnectionServerForAccount(msg.getStationId());
	if (conn)
	{
		conn->send(msg,true);
	}
	else
	{
		if(! ConnectionServerConnection::sendToPseudoClientConnection(msg.getStationId(), msg))
		{
			LOG("TRACE_LOGIN", ("Trying to log account %i in, but could not determine which connection server to use.",msg.getStationId()));
		}
	}
}

// ----------------------------------------------------------------------

void CentralServer::handleExchangeListCreditsMessage(const ExchangeListCreditsMessage &msg)
{
	LOG("Exchange", ("Central Server got exchange list credits %d.",msg.getCredits()));
	if ( m_pAuctionTransferClient == nullptr )
	{
		// send failure packet
	}
	//////////////////////m_pAuctionTransferClient->addCoinToAuction( msg );
}





ConnectionServerConnection * CentralServer::getAnyConnectionServer()
{
	ConnectionServerConnection * result = 0;
	if(! m_connectionServerConnections.empty())
	{
		result = *(m_connectionServerConnections.begin());
		if(result)
		{
			int leastPlayers = result->getPlayerCount();

			std::vector<ConnectionServerConnection *>::iterator i;
			for(i = m_connectionServerConnections.begin(); i != m_connectionServerConnections.end();  ++i)
			{
				if((*i)->getPlayerCount() < leastPlayers)
					result = *i;
			}
		}
	}
	return result;
}

// ----------------------------------------------------------------------

ConnectionServerConnection * CentralServer::getConnectionServerForAccount(StationId suid)
{
	ConnectionServerConnection * result = nullptr;
	ConnectionServerSUIDMap::iterator i=m_accountConnectionMap.find(suid);
	if (i!=m_accountConnectionMap.end())
	{
		result = (*i).second;
	}
	else
	{
		result = ConnectionServerConnection::getConnectionForAccount(suid);
	}

	return result;
}

// ----------------------------------------------------------------------

void CentralServer::addToAccountConnectionMap(StationId suid, ConnectionServerConnection * cconn, uint32 subscriptionBits)
{
	m_accountConnectionMap[suid] = cconn;

	// remove corresponding "pseudo client connection"
	ConnectionServerConnection::removeFromAccountConnectionMap(suid);
}

// ----------------------------------------------------------------------

void CentralServer::removeFromAccountConnectionMap(StationId suid)
{
	ConnectionServerSUIDMap::iterator i=m_accountConnectionMap.find(suid);
	if (i!=m_accountConnectionMap.end())
	{
		// Erase the entry
		m_accountConnectionMap.erase(i++);
	}
}

// ----------------------------------------------------------------------

void CentralServer::removeFromAccountConnectionMap(int connectionServerConnectionId)
{
	ConnectionServerSUIDMap::iterator i;
	for(i = m_accountConnectionMap.begin(); i != m_accountConnectionMap.end();)
	{
		const ConnectionServerConnection* cconn = (*i).second;

		if(cconn && (cconn->getId() == connectionServerConnectionId))
		{
			// Erase the entry
			m_accountConnectionMap.erase(i++);
		}
		else
		{
			++i;
		}
	}

	// We could update the LoginServer population but people
	// leaving the server are not as important as people connecting
	// and so we will wait for update() to handle things
}

//--------------------------------------------------------------------------------

const Service * CentralServer::getChatService() const
{
	return m_chatService;
}

//--------------------------------------------------------------------------------

const Service * CentralServer::getCustomerService() const
{
	return m_csService;
}

//--------------------------------------------------------------------------------

const Service * CentralServer::getGameService() const
{
	return m_gameService;
}

//--------------------------------------------------------------------------------

void CentralServer::startConnectionServer(int connectionServerNumber, const std::string& listenAddress, uint16 publicPort, uint16 internalPort, bool, const SpawnDelaySeconds spawnDelay) const
{
	char tmp1[32];
	char tmp2[32];
	std::string publicString = _itoa(publicPort, tmp1, 10);
	std::string privateString = _itoa(internalPort, tmp2, 10);
	startConnectionServer(connectionServerNumber, listenAddress, &publicString, &privateString, spawnDelay);
}

//--------------------------------------------------------------------------------

void CentralServer::startConnectionServer(int connectionServerNumber, const std::string& listenAddress, const std::string * publicPort, const std::string * internalPort, const SpawnDelaySeconds spawnDelay) const
{
	UNREF(listenAddress);  //once we add dual nic support, we can use this
	if(!m_connService)
	{
		DEBUG_WARNING(true, ("Could not start connection server because there was no service for it"));
		return;
	}

	if (!ConfigCentralServer::getAllowZeroConnectionServerPort())
	{
		if (!publicPort)
		{
			WARNING(true, ("Could not start connection server because there were not client ports specified"));
			return;
		}
		if (*publicPort == "0")
		{
			WARNING(true, ("Could not start connection server because one or more client ports were 0."));
			return;
		}
	}

	// there should already be an entry in s_connectionServerHostList that
	// contains the node on which to start this particular connection server
	if ((connectionServerNumber <= 0) || (connectionServerNumber > static_cast<int>(s_connectionServerHostList.size())))
	{
		WARNING(true, ("Could not start connection server because connectionServerNumber=%d and s_connectionServerHostList.size()=%d", connectionServerNumber, static_cast<int>(s_connectionServerHostList.size())));
		return;
	}

	// spawn a connection server
	std::string options = "-s ConnectionServer clusterName=";
	options += ConfigCentralServer::getClusterName();

	if(gs_connectionServersPublic)
		options += " startPublicServer=true";

	options += " centralServerAddress=";
	options += m_connService->getBindAddress();

	char servicePort[32];

	options += " centralServerPort=";
	options += _itoa(m_connService->getBindPort(), servicePort, 10);

	if (publicPort != 0)
	{
		options += " clientServicePortPublic=";
		options += *publicPort;
	}
	if (internalPort != 0)
	{
		options += " clientServicePortPrivate=";
		options += *internalPort;
	}

	options += " connectionServerNumber=";
	options += _itoa(connectionServerNumber, servicePort, 10);

	DEBUG_REPORT_LOG(true, ("Spawning Connection server with options %s\n", options.c_str()));
	TaskSpawnProcess p(s_connectionServerHostList[connectionServerNumber-1], "ConnectionServer", options, spawnDelay);
	CentralServer::getInstance().sendTaskMessage(p);
}

// ----------------------------------------------------------------------

void CentralServer::sendToAllLoginServers(const GameNetworkMessage &message)
{
	for (LoginServerConnectionMapType::iterator i=m_loginServerConnectionMap.begin(); i!=m_loginServerConnectionMap.end(); ++i)
		i->second->send(message,true);
}

// ----------------------------------------------------------------------

uint32 CentralServer::sendToArbitraryLoginServer(const GameNetworkMessage &message, bool roundRobin /*= true*/)
{
	static unsigned int nextServer = 0;

	if (m_loginServerConnectionMap.empty())
		return 0;

	LoginServerConnectionMapType::const_iterator i = m_loginServerConnectionMap.begin();

	if (!roundRobin)
	{
		i->second->send(message,true);
		return i->second->getProcessId();
	}

	++nextServer;
	if (nextServer >= m_loginServerConnectionMap.size())
		nextServer = 0;

	std::advance(i, nextServer);

	i->second->send(message,true);
	return i->second->getProcessId();
}

// ----------------------------------------------------------------------

/**
 * In development mode, connect to a login server.
 */
void CentralServer::connectToLoginServer() const
{
	IGNORE_RETURN(new LoginServerConnection(ConfigCentralServer::getLoginServerAddress(), ConfigCentralServer::getLoginServerPort()));
}

//-----------------------------------------------------------------------

void CentralServer::remove()
{
	CentralServer & cs = getInstance();
	MetricsManager::remove();
	delete cs.m_metricsData;
	cs.m_metricsData = 0;

	delete cs.m_loginServerKeys;

	cs.m_messagesWaitingForPlanetServer.clear();
	cs.m_gameServerConnections.clear();
	cs.m_gameServerConnectionsList.clear();
	cs.m_connectionServerConnections.clear();

	delete cs.m_gameService;
	delete cs.m_chatService;
	delete cs.m_csService;
	delete cs.m_connService;
	delete cs.m_planetService;
	delete cs.m_loginService;
	if(cs.m_taskManager)
	{
		cs.m_taskManager->setDisconnectReason("CentralServer::remove");
		cs.m_taskManager->disconnect();
	}
	delete cs.m_taskService;
	delete cs.m_consoleService;
}

//-----------------------------------------------------------------------

std::vector<const GameServerConnection *> CentralServer::getGameServers() const
{
	std::vector<const GameServerConnection *> result;
	std::map<uint32, GameServerConnection *>::const_iterator i;
	for(i = m_gameServerConnections.begin(); i != m_gameServerConnections.end(); ++i)
	{
		result.push_back((*i).second);
	}
	return result;
}

// ----------------------------------------------------------------------

void CentralServer::sendToConnectionServerForAccount(StationId account, const GameNetworkMessage & message, const bool reliable)
{
	ConnectionServerConnection *conn = getConnectionServerForAccount(account);
	if (conn)
		conn->send(message,reliable);
}

// ----------------------------------------------------------------------

uint32 CentralServer::sendToRandomGameServer(const GameNetworkMessage &message)
{
	GameServerConnection *conn = getRandomGameServer();
	if (conn)
	{
		conn->send(message,true);
		return conn->getProcessId();
	}
	else
		return 0;
}

// ----------------------------------------------------------------------

void CentralServer::doServerPings()
{
	for (std::set<uint32>::iterator i=m_serverPings.begin(); i!=m_serverPings.end(); ++i)
	{
		LOG("CentralServerPings",("Dropping server %lu because it hasn't responded to CentralPingMessage",*i));
		GameServerConnection *conn=getGameServer(*i);
		if (conn)
		{
			// tell task manager to kill the offending process (in case it's hung)
			ExcommunicateGameServerMessage msg(*i, conn->getOsProcessId(), conn->getRemoteAddress());
			// disconnect, if it's not hung, it should die
			conn->setDisconnectReason("CentralServer::doServerPings, failure to respond to CentralPingMessage");
			conn->disconnect();
			sendTaskMessage(msg);
			excommunicateServer(msg);
		}
		else
		{
			LOG("CentralServerPings",("Didn't have connection to %lu to drop",*i));
			ExcommunicateGameServerMessage const excommunicateMessage(*i, 0, "");
			excommunicateServer(excommunicateMessage); // haven't received a reply (see CentralPingMessage, above)
		}
	}
	m_serverPings.clear();

	CentralPingMessage ping;
	for (std::map<uint32, GameServerConnection *>::iterator j=m_gameServerConnections.begin(); j!=m_gameServerConnections.end(); ++j)
	{
		if (j->first != getDbProcessServerProcessId())
		{
			LOG("CentralServerPings",("Pinging  %lu",j->first));
			IGNORE_RETURN(m_serverPings.insert(j->first));
			j->second->send(ping,true);
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Declare a game server anathema.  All other servers will shun this server.
 */
void CentralServer::excommunicateServer(const ExcommunicateGameServerMessage & msg)
{
	sendToAllGameServers(msg,true);
	sendToAllPlanetServers(msg,true);
	sendToAllConnectionServers(msg,true);
}

//-----------------------------------------------------------------------
void CentralServer::startShutdownProcess(const uint32 timeToShutdown, const uint32 maxTime, const Unicode::String &systemMessage)
{
	// sanity checking
	if ( m_shutdownPhase > 6)
	{
		WARNING("CentralServerShutdown", ("startShutdownProcess: invalid value in CentralServer::m_shutdownPhase.  Current value is %d. Resetting.", m_shutdownPhase));
		m_shutdownPhase = 0;
	}

	// server already in the process of shutting down
	if ( m_shutdownPhase > 0 )
	{
		LOG("CentralServerShutdown", ("startShutdownProcess: Server already in the process of shutting down.  Current shutdown phase is %d.", m_shutdownPhase));
		return;
	}

	if( timeToShutdown>maxTime )
	{
		LOG("CentralServerShutdown", ("startShutdownProcess: time to shutdown is greater than max time."));
		return;
	}
	m_shutdownPhase = 1;
	LOG("CentralServerShutdown",("Shutdown Phase %d: Shutdown sequence starting now.", m_shutdownPhase));
	LOG("CentralServerShutdown",("Shutdown Phase %d: time to shutdown=%dsec, max time to wait=%dsec, broadcast warning message=\"%s\".", m_shutdownPhase, timeToShutdown, maxTime, Unicode::wideToNarrow(systemMessage).c_str()));
	// this is the time in the future that we need to start the shutdown process.
	m_shutdownTotalTime = m_curTime + timeToShutdown;
	m_shutdownMaxTime = m_curTime + maxTime;
	m_shutdownSystemMessage = systemMessage;
	m_shutdownHaveDatabaseSaveStart = false;
	m_shutdownHaveDatabaseComplete = false;
	checkShutdownProcess();
}
//------------------------------------------------------------------------
void CentralServer::checkShutdownProcess()
{
	static bool warn30 = false;
	static bool warn10 = false;
	static bool warn0 = false;
	static uint32 lastWarn = m_curTime;
	static uint32 nextAbortCheck = m_curTime+1;
	static uint32 nextShutdownCheck = m_curTime+30;

	if( !m_shutdownPhase )
	{
#ifndef WIN32
		// check if a .shutdown file exists
		if( m_curTime > nextShutdownCheck )
		{
			nextShutdownCheck = m_curTime + 30;
			if( FileExists(".shutdown") )
			{
				StdioFile shutdownFile(".shutdown", "r");
				if(shutdownFile.isOpen())
				{
					char destBuffer[64] = {"\0"};
					if(shutdownFile.read(destBuffer, sizeof(destBuffer)) > 0)
					{
						destBuffer[sizeof(destBuffer) - 1] = 0;
						int shutdownTime = atoi(destBuffer);
						if(shutdownTime > 0)
						{
							LOG("CentralServerShutdown", ("Detected a .shutdown file.  Initiating shutdown sequence."));
							startShutdownProcess(shutdownTime, 7200, Unicode::narrowToWide("The server will be shutting down soon.  Please find a safe place to logout."));
						}
					}
					shutdownFile.close();
				}
				IGNORE_RETURN(::remove(".shutdown"));
				return;
			}

			if( FileExists( ".startanymissingplanet") )
			{
				LOG("CentralServer", ("Detected a .startanymissingplanet file.  Checking for any planets that are not started, and starting them."));

				for (std::map<std::string, std::string>::const_iterator iter = ms_sceneToHostMap.begin(); iter != ms_sceneToHostMap.end(); ++iter)
				{
					if (!hasPlanetServer(iter->first))
					{
						std::map<std::string, std::pair<std::pair<std::string, std::string>, time_t> >::iterator iterPendingPlanetServer = m_pendingPlanetServers.find(iter->first);
						if (iterPendingPlanetServer != m_pendingPlanetServers.end())
						{
							// if it's been "awhile" since we requested to restart the PlanetServer,
							// then assume that something has gone wrong, and try the restart again
							time_t const timeNow = ::time(nullptr);

							if ((iterPendingPlanetServer->second.second + static_cast<time_t>(ConfigCentralServer::getMaxTimeToWaitForPlanetServerStartSeconds())) < timeNow)
							{
								LOG("CentralServer", ("startanymissingplanet:Starting missing planet %s because it has been %s since we requested it to be started, which has exceeded the %s limit.", iter->first.c_str(), CalendarTime::convertSecondsToMS(static_cast<unsigned int>(timeNow - iterPendingPlanetServer->second.second)).c_str(), CalendarTime::convertSecondsToMS(static_cast<unsigned int>(ConfigCentralServer::getMaxTimeToWaitForPlanetServerStartSeconds())).c_str()));
								iterPendingPlanetServer->second.second = timeNow;
								TaskSpawnProcess spawn(iterPendingPlanetServer->second.first.first.empty() ? std::string("any") : iterPendingPlanetServer->second.first.first, "PlanetServer", iterPendingPlanetServer->second.first.second);
								CentralServer::getInstance().sendTaskMessage(spawn);
							}
							else
							{
								LOG("CentralServer", ("startanymissingplanet:It's only been %s since we requested for planet %s to be started, and we must wait %s before we can start it again.", CalendarTime::convertSecondsToMS(static_cast<unsigned int>(timeNow - iterPendingPlanetServer->second.second)).c_str(), iter->first.c_str(), CalendarTime::convertSecondsToMS(static_cast<unsigned int>(ConfigCentralServer::getMaxTimeToWaitForPlanetServerStartSeconds())).c_str()));
							}
						}
						else
						{
							LOG("CentralServer", ("startanymissingplanet:Hmmm... planet %s is missing, but also is not in the list of planets we have attempted to start, so we're ***NOT*** going to start it.", iter->first.c_str()));
						}
					}
				}

				IGNORE_RETURN(::remove( ".startanymissingplanet" ));
				return;
			}

			if( FileExists( ".startanymissinggameserver") )
			{
				LOG("CentralServer", ("Detected a .startanymissinggameserver file.  Checking for any game servers that are not started, and starting them."));

				const GenericValueTypeMessage<uint8> startAnyMissingGameServer("SAMGS", 0);
				sendToAllPlanetServers(startAnyMissingGameServer, true);

				IGNORE_RETURN(::remove( ".startanymissinggameserver" ));
				return;
			}
		}
#endif
		return;
	}

	// should we abort?
	if( m_curTime > nextAbortCheck )
	{
#ifndef WIN32
		nextAbortCheck = m_curTime + 1;
		if( FileExists(".abortshutdown") )
		{

			IGNORE_RETURN(::remove(".abortshutdown"));
			LOG("CentralServerShutdown", ("Shutdown Phase %d: Detected a .abortshutdown file.  Aborting shutdown sequence.", m_shutdownPhase));
			abortShutdownProcess();
			return;
		}
#endif
	}

	// Phase 1 is an immediate broadcast to players for the first shutdown warning
	if( m_shutdownPhase == 1 )
	{
		char strTimeLeft[1024];
		uint32 timeLeft = m_shutdownTotalTime - m_curTime;
		if( timeLeft < 1 )
		{
			strTimeLeft[0] = '\0';
		}
		else if( timeLeft < 60 )
		{
			sprintf(strTimeLeft, " ( %lusec left )", timeLeft);
		}
		else
		{
			sprintf(strTimeLeft, " ( %lumin left )", timeLeft/60);
		}

		LOG("CentralServerShutdown",("Shutdown Phase %d: Broadcasting first shutdown message to players: \"%s %s\"", m_shutdownPhase, Unicode::wideToNarrow(m_shutdownSystemMessage).c_str(), strTimeLeft));
		ConGenericMessage const msg("game any systemMessage " + Unicode::wideToNarrow(m_shutdownSystemMessage)+strTimeLeft, 0);
		IGNORE_RETURN(sendToRandomGameServer(msg));
		m_shutdownPhase = 2;
		lastWarn = m_curTime;
	}
	// Phase 2 is broadcasting a system message every 60sec while we wait for time to expire
	else if( m_shutdownPhase == 2 )
	{
		if(  m_curTime >= m_shutdownTotalTime)
		{
			LOG("CentralServerShutdown",("Shutdown Phase %d: Done broadcasting shutdown warning message to players . Advancing shutdown phase.", m_shutdownPhase));
			m_shutdownPhase = 3;
			warn30 = false;
			warn10 = false;
			warn0 = false;


		}
		// broadcast warning message to players every 60sec.
		else if( m_curTime >= lastWarn+60 )
		{
			char strTimeLeft[1024];
			uint32 timeLeft = m_shutdownTotalTime - m_curTime;
			if( timeLeft < 1 )
			{
				strTimeLeft[0] = '\0';
			}
			else if( timeLeft < 60 )
			{
				sprintf(strTimeLeft, " ( %lusec left)", timeLeft);
			}
			else
			{
				sprintf(strTimeLeft, " ( %lumin left)", timeLeft/60);
			}

			LOG("CentralServerShutdown",("Shutdown Phase %d: Broadcasting shutdown message to players: \"%s %s\"", m_shutdownPhase, Unicode::wideToNarrow(m_shutdownSystemMessage).c_str(), strTimeLeft));
			ConGenericMessage const msg("game any systemMessage " + Unicode::wideToNarrow(m_shutdownSystemMessage)+strTimeLeft, 0);
			IGNORE_RETURN(sendToRandomGameServer(msg));
			lastWarn = m_curTime;
		}
	}
	// warn the players they are about to be disconnected
	else if( m_shutdownPhase == 3 )
	{
		if( m_curTime>=m_shutdownTotalTime && !warn30 )
		{
			LOG("CentralServerShutdown",("Shutdown Phase %d: Broadcasting 30sec disconnect warning message to players.", m_shutdownPhase));
			ConGenericMessage const msg("game any systemMessage You will be disconnected in 30sec so the server can perform a final save before shutting down.  Please find a safe place to logout now.", 0);
			IGNORE_RETURN(sendToRandomGameServer(msg));
			warn30 = true;

		}
		else if( m_curTime >= (m_shutdownTotalTime+20) && !warn10 )
		{
			LOG("CentralServerShutdown",("Shutdown Phase %d: Broadcasting 10sec disconnect warning message to players.", m_shutdownPhase));
			ConGenericMessage const msg("game any systemMessage You will be disconnected in 10sec so the server can perform a final save before shutting down.  Please find a safe place to logout now.", 0);
			IGNORE_RETURN(sendToRandomGameServer(msg));
			warn10 = true;
		}
		else if( m_curTime >= (m_shutdownTotalTime+30) && !warn0 )
		{
			LOG("CentralServerShutdown",("Shutdown Phase %d: Broadcasting final disconnect warning message to players.", m_shutdownPhase));
			ConGenericMessage const msg("game any systemMessage You will now be disconnected so the server can perform a final save before shutting down.", 0);
			IGNORE_RETURN(sendToRandomGameServer(msg));
			warn0 = true;
		}
		else if( m_curTime >= (m_shutdownTotalTime+35) )
		{
			LOG("CentralServerShutdown",("Shutdown Phase %d: Setting server to private, disconnecting the players and now waiting for next database save cycle to begin.", m_shutdownPhase));
			gs_connectionServersPublic = false;
			SetConnectionServerPublic const msg(false);
			sendToAllConnectionServers(msg, true);
			m_shutdownPhase = 4;

			// send a message to the DB telling it we're shutting down
			GenericValueTypeMessage<bool> const msg2("ClusterShutdownMessage", true);
			sendToDBProcess(msg2, true);

			// send a save start to the DB; must be sent after the above
			// "ClusterShutdownMessage" message so the DB will know this
			// is the final save before cluster shutdown
			GenericValueTypeMessage<NetworkId> const msg3("StartSaveMessage", NetworkId::cms_invalid);
			sendToDBProcess(msg3, true);
		}
	}
	else if( m_shutdownPhase == 4 )
	{
		if( m_shutdownHaveDatabaseSaveStart )
		{
			LOG("CentralServerShutdown",("Shutdown Phase %d: Starting final database save cycle. Advancing shutdown phase.", m_shutdownPhase));
			m_shutdownPhase = 5;
		}
	}
	else if( m_shutdownPhase == 5 )
	{
		// we are done.  Shut it down.  Shut it down now!
		if( m_shutdownHaveDatabaseComplete)
		{
			LOG("CentralServerShutdown",("Shutdown Phase %d: Completed final database save cycle. Advancing shutdown phase.", m_shutdownPhase));
			m_shutdownPhase = 6;
			LOG("CentralServerShutdown",("Shutdown Phase %d: Instructing TaskManager to shutdown the cluster without restarting.", m_shutdownPhase));
			ConGenericMessage const msg("stop", 0);
			sendTaskMessage(msg);
		}
	}
	// maximum amount of time to wait has expired.  Shut it down now without waiting for anything else.
	else if( m_curTime >= m_shutdownMaxTime )
	{
		LOG("CentralServerShutdown",("Shutdown Phase %d: Maximum time has gone by.  Forcing a shutdown now.", m_shutdownPhase));
		m_shutdownPhase = 6;
		LOG("CentralServerShutdown",("Shutdown Phase %d: Instructing TaskManager to shutdown the cluster without restarting.", m_shutdownPhase));
		ConGenericMessage const msg("stop", 0);
		sendTaskMessage(msg);
	}
}

void CentralServer::abortShutdownProcess()
{
	LOG("CentralServerShutdown", ("Shutdown Phase %d: Aborting shutdown sequence.", m_shutdownPhase));
	if( m_shutdownPhase > 0 )
	{
		ConGenericMessage const msg("game any systemMessage The server shutdown has been aborted.  The server is no longer shutting down.", 0);
		IGNORE_RETURN(sendToRandomGameServer(msg));
	}

	// send a message to the DB telling it we're no longer shutting down
	GenericValueTypeMessage<bool> const msg("ClusterShutdownMessage", false);
	sendToDBProcess(msg, true);

	m_shutdownPhase = 0;
	m_shutdownTotalTime = 0;
	m_shutdownMaxTime = 0;
	m_shutdownSystemMessage = Unicode::narrowToWide("");
	m_shutdownHaveDatabaseSaveStart = false;
	m_shutdownHaveDatabaseComplete = false;
}

//-----------------------------------------------------------------------

void CentralServer::sendToTransferServer(const GameNetworkMessage & msg) const
{
	if(getInstance().m_transferServerConnection)
	{
		getInstance().m_transferServerConnection->send(msg, true);
	}
}

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

void CentralServer::sendToStationPlayersCollector(const GameNetworkMessage & msg) const
{
	if(getInstance().m_stationPlayersCollectorConnection)
	{
		getInstance().m_stationPlayersCollectorConnection->send(msg, true);
	}
}

//-----------------------------------------------------------------------

int CentralServer::getPlanetServersCount() const
{
	return m_planetServers.size();
}

//-----------------------------------------------------------------------

const Service * CentralServer::getCommoditiesService()
{
	return s_commodityServerService;
}

//-----------------------------------------------------------------------

int CentralServer::getClusterStartupTime() const
{
	if (m_clusterStartupTime != -1)
		return m_clusterStartupTime;
	else
		// return how long the cluster has been taking to come up
		return int((time(0) - m_timeClusterStarted) / 60);
}

//-----------------------------------------------------------------------

int CentralServer::getSecondsClusterHasBeenInLoadingState() const
{
	if (m_timeClusterWentIntoLoadingState > 0)
		return (time(0) - m_timeClusterWentIntoLoadingState);
	else
		return 0;
}

// ----------------------------------------------------------------------

const std::map<std::string, int> & CentralServer::getPopulationStatistics(time_t & refreshTime)
{
	// periodically request updated statistics from the game server
	time_t const timeNow = ::time(nullptr);
	if (m_timePopulationStatisticsNextRefresh <= timeNow)
	{
		GameServerConnection * universeGameServerConnection = getGameServer(UniverseManager::getInstance().getUniverseProcess());
		if (universeGameServerConnection)
		{
			const GenericValueTypeMessage<uint8> populationStatisticsRequest("PopStatReq", 0);
			universeGameServerConnection->send(populationStatisticsRequest, true);

			m_timePopulationStatisticsNextRefresh = timeNow + 60;
		}
	}

	refreshTime = m_timePopulationStatisticsRefresh;
	return m_populationStatistics;
}

// ----------------------------------------------------------------------

const std::map<std::string, std::pair<int, std::pair<std::string, std::string> > > & CentralServer::getGcwScoreStatistics(time_t & refreshTime)
{
	// periodically request updated statistics from the game server
	time_t const timeNow = ::time(nullptr);
	if (m_timeGcwScoreStatisticsNextRefresh <= timeNow)
	{
		GameServerConnection * universeGameServerConnection = getGameServer(UniverseManager::getInstance().getUniverseProcess());
		if (universeGameServerConnection)
		{
			const GenericValueTypeMessage<uint8> gcwScoreStatisticsRequest("GcwScoreStatReq", 0);
			universeGameServerConnection->send(gcwScoreStatisticsRequest, true);

			m_timeGcwScoreStatisticsNextRefresh = timeNow + 60;
		}
	}

	refreshTime = m_timeGcwScoreStatisticsRefresh;
	return m_gcwScoreStatistics;
}

// ----------------------------------------------------------------------

std::pair<std::map<int, std::pair<std::string, int> > const *, std::map<int, std::pair<std::string, int> > const *> CentralServer::getLastLoginTimeStatistics(time_t & refreshTime)
{
	// periodically request updated statistics from the game server
	time_t const timeNow = ::time(nullptr);
	if (m_timeLastLoginTimeStatisticsNextRefresh <= timeNow)
	{
		GameServerConnection * universeGameServerConnection = getGameServer(UniverseManager::getInstance().getUniverseProcess());
		if (universeGameServerConnection)
		{
			const GenericValueTypeMessage<uint8> lastLoginTimeStatisticsRequest("LLTStatReq", 0);
			universeGameServerConnection->send(lastLoginTimeStatisticsRequest, true);

			m_timeLastLoginTimeStatisticsNextRefresh = timeNow + (15 * 60);
		}
	}

	refreshTime = m_timeLastLoginTimeStatisticsRefresh;
	
	return std::make_pair(&m_lastLoginTimeStatistics, &m_createTimeStatistics);
}

// ----------------------------------------------------------------------

void CentralServer::getCharacterMatchStatistics(int & numberOfCharacterMatchRequests, int & numberOfCharacterMatchResultsPerRequest, int & timeSpentPerCharacterMatchRequestMs)
{
	// periodically request updated statistics from the game server
	time_t const timeNow = ::time(nullptr);
	if (m_timeCharacterMatchStatisticsNextRefresh <= timeNow)
	{
		const GenericValueTypeMessage<uint8> characterMatchStatisticsRequest("LfgStatReq", 0);
		sendToAllGameServersExceptDBProcess(characterMatchStatisticsRequest, true);

		m_timeCharacterMatchStatisticsNextRefresh = timeNow + 60;
	}

	// wraparound, reset statistics
	if (m_numberOfCharacterMatchRequests < 0)
	{
		m_numberOfCharacterMatchRequests = 0;
		m_numberOfCharacterMatchResults = 0;
		m_timeSpentOnCharacterMatchRequestsMs = 0;
	}

	numberOfCharacterMatchRequests = m_numberOfCharacterMatchRequests;
	numberOfCharacterMatchResultsPerRequest = (m_numberOfCharacterMatchRequests ? static_cast<int>(m_numberOfCharacterMatchResults / m_numberOfCharacterMatchRequests) : 0);
	timeSpentPerCharacterMatchRequestMs = (m_numberOfCharacterMatchRequests ? static_cast<int>(m_timeSpentOnCharacterMatchRequestsMs / m_numberOfCharacterMatchRequests) : 0);
}

// ======================================================================
//lint -e766 // todo : TRF Fix this
