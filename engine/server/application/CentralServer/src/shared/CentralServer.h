// CentralServer.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_CentralServer_H
#define	_CentralServer_H

//-----------------------------------------------------------------------
#pragma warning(disable : 4100)
#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <set>
#include "Singleton/Singleton.h"
#include "serverKeyShare/KeyServer.h"
#include "sharedFoundation/StationId.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Receiver.h"

class CentralServerMetricsData;
class ChatServerConnection;
class Connection;
class ConnectionServerConnection;
class ConnectionCreateCharacter;
class CustomerServiceConnection;
class ExcommunicateGameServerMessage;
class GameNetworkMessage;
class GameServerConnection;
class GameServerConnectionHandler;
class GameServerForLoginMessage;
class LoginServerConnection;
class PlanetServerConnection;
class RequestGameServerForLoginMessage;
class RequestSceneTransfer;
class Service;
class TaskConnection;
class TransferServerConnection;
class StationPlayersCollectorConnection;
class AuctionTransferClient;
class ExchangeListCreditsMessage;

//-----------------------------------------------------------------------

class CentralServer : public Singleton<CentralServer>, public MessageDispatch::Receiver
{
public:
	typedef std::string         SceneId;
	typedef std::vector<uint32> ProcessList;
	typedef unsigned int        SpawnDelaySeconds;
	struct CentralObject
	{
		CentralObject(); 
		CentralObject(const SceneId & sceneId, uint32 authProcess);

		SceneId             m_sceneId;
		uint32              m_authoritativeProcess;
		ProcessList         m_proxyProcessList;
	};
	~CentralServer();
private:
	typedef std::vector<ConnectionServerConnection *>                 ConnectionServerConnectionList;
	typedef std::map<NetworkId, CentralObject>                       CentralObjectMap;
	typedef std::unordered_multimap<SceneId, GameServerConnection *> SceneGameMap;

	typedef std::pair<SceneGameMap::const_iterator, SceneGameMap::const_iterator> ServersList;
	typedef std::unordered_map<uint32, ConnectionServerConnection *>                   ConnectionServerSUIDMap;

	typedef std::map<NetworkId, std::pair<SceneId, time_t> >                      PlayerSceneMapType;

public:
	void     addConnectionServerConnection(ConnectionServerConnection* conn);
	void     addGameServer(GameServerConnection * newGameServer);
	void     broadcastToChatServers(const GameNetworkMessage & message) const;
	void     broadcastToCustomerServiceServers(const GameNetworkMessage & message) const;
	void     broadcastToGameServers(const GameNetworkMessage & message) const;
	bool     getIsClusterPublic() const;
	bool     getIsClusterLocked() const;
	bool     getIsClusterSecret() const;
	bool     isPreloadFinished() const;
	bool     isInClusterInitialStartup() const;
	uint8    getClusterId() const;
	int      getClusterStartupTime() const;
	int      getSecondsClusterHasBeenInLoadingState() const;
	int      getPlanetsWaitingForPreloadCount() const;
	int      getPlanetServersCount() const;
	bool     isDatabaseBacklogged() const;
	bool     hasDBConnection() const;
	const Service * getGameService() const;
	const Service * getChatService() const;
	const Service * getCustomerService() const;
	static const Service * getCommoditiesService();
	const Service * getPlanetService() const;
	const std::string& getHostForScene(const std::string& scene) const;
	int      getNumChatServers() const;
	int      getNumConnectionServers() const;
	int      getNumDatabaseServers() const;
	int      getNumGameServers() const;
	int      getNumPlanetServers() const;
	int      getPlayerCount() const;
	int      getFreeTrialCount() const;
	int      getEmptySceneCount() const;
	int      getTutorialSceneCount() const;
	int      getFalconSceneCount() const;

	bool     hasPlanetServer(const std::string & sceneName) const;

	void     launchStartingProcesses() const;
	void     launchStartingConnectionServers() const;
	void     launchStartingPlanetServers();
	void     launchCommoditiesServer();

	void     receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	void     removeConnectionServerConnection(const ConnectionServerConnection* conn);
	void     removeGameServer(const GameServerConnection *gameServer);
	void     removePlanetServer(const PlanetServerConnection *planetServer);
	void     pushAllKeys(ConnectionServerConnection * targetConnServer) const;
	void     sendTaskMessage(const GameNetworkMessage & source);
	void     sendToGameServer(const uint32 gameServerProcessId, const GameNetworkMessage & message, const bool reliable) const;
	void     sendToAllGameServers(const GameNetworkMessage & message, const bool reliable);
	void     sendToAllGameServersExceptDBProcess(const GameNetworkMessage & message, const bool reliable);
	void     sendToAllPlanetServers(const GameNetworkMessage & message, const bool reliable);
	void     sendToPlanetServer(const std::string &sceneId, const GameNetworkMessage & message, const bool reliable);
	void     sendToAllConnectionServers(const GameNetworkMessage & message, const bool reliable, Connection const * exclude = nullptr);
	void     sendToConnectionServerForAccount(StationId account, const GameNetworkMessage & message, const bool reliable);
	void     sendToDBProcess(const GameNetworkMessage & message, const bool reliable) const;
	void     sendToLoginServer(uint32 loginServerId, const GameNetworkMessage &message);
	void     sendToAllLoginServers(const GameNetworkMessage &message);
	uint32   sendToArbitraryLoginServer(const GameNetworkMessage &message, bool roundRobin = true);
	uint32   sendToRandomGameServer(const GameNetworkMessage &message);
	void     sendToTransferServer(const GameNetworkMessage & message) const;
	void     sendToStationPlayersCollector(const GameNetworkMessage & msg) const;
	void     setTaskManager(TaskConnection * newTaskmanager);
	void     setDbProcessServerProcessId(const uint32 newDbServerProcessid);
	void     startConnectionServer(int connectionServerNumber, const std::string& listenAddress, const std::string * publicPort, const std::string * internalPort, SpawnDelaySeconds spawnDelay) const;
	void     startConnectionServer(int connectionServerNumber, const std::string& listenAddress, uint16 publicPort, uint16 internalPort, bool, SpawnDelaySeconds spawnDelay) const;
	void     startPlanetServer(const std::string& host, const std::string& sceneId, SpawnDelaySeconds spawnDelay);
	void     startShutdownProcess(const uint32 timeToShutdown, const uint32 maxTime, const Unicode::String &systemMessage);
	void     checkShutdownProcess();
	void     abortShutdownProcess();

	GameServerConnection *       getRandomGameServer(void);
	void                         getReadyGameServers(std::vector<uint32> &theList);
	const uint32                 getDbProcessServerProcessId(void) const;

	const std::string &          getCommandLine() const;
	void                         setCommandLine(const std::string & cmdLine);
	static void                  run(void);
	static void                  done();
	static void                  remove();
	
	ConnectionServerConnection * getAnyConnectionServer();

	GameServerConnection *       getGameServer       	(const uint32 processId) const;
	GameServerConnection *       getGameServer        (const SceneId &scene) const;
	const ServersList            getGameServers       (const SceneId &scene) const;
	std::vector<const GameServerConnection *>  getGameServers() const;

	time_t                       getLastTimeSystemTimeMismatchNotification() const;
	const std::string &          getLastTimeSystemTimeMismatchNotificationDescription() const;

	const std::string &          getDisconnectedTaskManagerList() const;

	const std::map<std::string, int> & getPopulationStatistics(time_t & refreshTime);

	const std::map<std::string, std::pair<int, std::pair<std::string, std::string> > > & getGcwScoreStatistics(time_t & refreshTime);

	std::pair<std::map<int, std::pair<std::string, int> > const *, std::map<int, std::pair<std::string, int> > const *> getLastLoginTimeStatistics(time_t & refreshTime);

	void                         getCharacterMatchStatistics(int & numberOfCharacterMatchRequests, int & numberOfCharacterMatchResultsPerRequest, int & timeSpentPerCharacterMatchRequestMs);

	void                         removeFromAccountConnectionMap(StationId suid);

private:
	void     handleRequestGameServerForLoginMessage (const RequestGameServerForLoginMessage & msg);
	void     handleRequestSceneTransfer             (const RequestSceneTransfer & msg);
	void     handleGameServerForLoginMessage        (const GameServerForLoginMessage & msg);


	void	handleExchangeListCreditsMessage	(const ExchangeListCreditsMessage& msg);

	size_t                       getGameServerCount		(void) const;
	void                         update();
	void                         sendPopulationUpdateToLoginServer();
	void			     sendMetricsToWebAPI(std::string updateURL);
	ConnectionServerConnection * getConnectionServerForAccount(StationId suid);
	void                         addToAccountConnectionMap(StationId suid, ConnectionServerConnection * cconn, uint32 subscriptionBits);
	void                         removeFromAccountConnectionMap(int connectionServerConnectionId);
	void                         doServerPings();
	void                         excommunicateServer(const ExcommunicateGameServerMessage &);

protected: 
	friend class Singleton<CentralServer>;
	CentralServer();

private:
	void connectToLoginServer() const;

	CentralServer(const CentralServer & source);
	CentralServer&	operator= (const CentralServer & rhs);

	ConnectionServerConnectionList                 m_connectionServerConnections;
	std::set<ChatServerConnection *>               m_chatServerConnections;
	std::set<CustomerServiceConnection *>          m_csServerConnections;
	SceneGameMap                                   m_gameServers;
	ConnectionServerSUIDMap                        m_accountConnectionMap;

//	Network::Address   clientService;
	/**
		The dbProcessServerProcessId is set by the DBProcess. The central server
		refers to this process if there are no matching objects in any of its maps.
	*/
	uint32                        m_dbProcessServerProcessId;
	bool                          m_done;
	Service *                     m_gameService;
	Service *                     m_chatService;
	Service *                     m_connService;
	Service *                     m_csService;
	Service *                     m_planetService;
	Service *                     m_consoleService;
	TransferServerConnection *    m_transferServerConnection;
	StationPlayersCollectorConnection * m_stationPlayersCollectorConnection;

	// these two lists must be kept in sync, ***EXCEPT*** that
	// m_gameServerConnectionsList doesn't contain the DB server
	std::map<uint32, GameServerConnection *> m_gameServerConnections;
	std::vector<GameServerConnection *> m_gameServerConnectionsList;

	PlayerSceneMapType            m_playerSceneMap;

	typedef std::map<uint32, LoginServerConnection *> LoginServerConnectionMapType;
	LoginServerConnectionMapType  m_loginServerConnectionMap;

	KeyServer *                   m_loginServerKeys;
	Service *                     m_loginService;

	AuctionTransferClient *       m_pAuctionTransferClient;

	std::vector<Archive::ByteStream>        m_messagesWaitingForPlanetServer;

	std::map<NetworkId, uint32>   m_pendingLoadingObjects;
	std::map<NetworkId, uint32>   m_pendingNewObjects;
	uint32                        m_nextFreeProcessId;
	TaskConnection *              m_taskManager;
	Service *                     m_taskService;
	std::string                   m_commandLine;
	std::map<std::string, NetworkId>  m_pendingTransfers;
	int                           m_totalPlayerCount;
	int                           m_totalFreeTrialCount;
	int                           m_totalEmptySceneCount;
	int                           m_totalTutorialSceneCount;
	int                           m_totalFalconSceneCount;
	CentralServerMetricsData*     m_metricsData;
	std::map<std::string, std::pair<std::pair<std::string, std::string>, time_t> > m_pendingPlanetServers;
	std::set<std::string>         m_planetsWaitingForPreload;
	std::map<std::string, PlanetServerConnection *> m_planetServers;
	uint16                        m_nextPlanetWatcherPort;
	bool                          m_databaseBacklogged;
	time_t                        m_lastLoadingStateTime;
	time_t                        m_timeClusterStarted;
	int                           m_clusterStartupTime;
	time_t                        m_timeClusterWentIntoLoadingState;
	uint8                         m_clusterId;

	std::set<uint32>              m_serverPings;

	uint32                        m_shutdownTotalTime;
	uint32                        m_shutdownMaxTime;
	Unicode::String               m_shutdownSystemMessage;
	// This tracks various phases of the shutdown sequence.
	// 0 = not in the process of shutting down
	// 1 = first broadcast message immediately after receiving a shutdown command
	// 2 = beginning process of shutting down.  Sending system message broadcasts each minute warning players.
	// 3 = give players 30sec to get to a safe place before being disconnected.
	// 4 = done broadcasting messages; set the cluster to private, kick the players and wait for the next DatabaseSaveStart message
	// 5 = received DatabaseStartSave, now we are waiting for either another DatabaseSaveStart or DatabaseSaveComplete
	// 	(it's possible to receive another DatabaseSaveStart without a matching DatabaseSaveComplete in cases where
	// 	there happens to be nothing to save for that cycle)
	// 6 = received a DatabaseSaveComplete (or another DatabaseSaveStart) and am in the process of shutting down.
	uint32                        m_shutdownPhase;
	bool                          m_shutdownHaveDatabaseSaveStart;
	bool                          m_shutdownHaveDatabaseComplete;
	uint32                        m_curTime;

	time_t                        m_lastTimeSystemTimeMismatchNotification;
	std::string                   m_lastTimeSystemTimeMismatchNotificationDescription;

	std::string                   m_disconnectedTaskManagerList;

	std::map<std::string, int>    m_populationStatistics;
	time_t                        m_timePopulationStatisticsRefresh;
	time_t                        m_timePopulationStatisticsNextRefresh;

	std::map<std::string, std::pair<int, std::pair<std::string, std::string> > > m_gcwScoreStatistics;
	time_t                        m_timeGcwScoreStatisticsRefresh;
	time_t                        m_timeGcwScoreStatisticsNextRefresh;

	std::map<int, std::pair<std::string, int> > m_lastLoginTimeStatistics;
	std::map<int, std::pair<std::string, int> > m_createTimeStatistics;
	time_t                        m_timeLastLoginTimeStatisticsRefresh;
	time_t                        m_timeLastLoginTimeStatisticsNextRefresh;

	int                           m_numberOfCharacterMatchRequests;
	uint64                        m_numberOfCharacterMatchResults;
	uint64                        m_timeSpentOnCharacterMatchRequestsMs;
	time_t                        m_timeCharacterMatchStatisticsNextRefresh;
};

//-----------------------------------------------------------------------
inline void CentralServer::addConnectionServerConnection(ConnectionServerConnection* conn)
{
	m_connectionServerConnections.push_back(conn);
}
//-----------------------------------------------------------------------

inline int CentralServer::getNumChatServers() const
{
	return m_chatServerConnections.size(); //lint !e713
}

//-----------------------------------------------------------------------

inline int CentralServer::getNumConnectionServers() const
{
	return m_connectionServerConnections.size(); //lint !e713
}

//-----------------------------------------------------------------------

inline int CentralServer::getNumDatabaseServers() const
{
	GameServerConnection * g = getGameServer(getDbProcessServerProcessId());
	if(g)
	{
		return 1;
	}
	return 0;
}

//-----------------------------------------------------------------------

inline int CentralServer::getNumGameServers() const
{
	return m_gameServers.size() - 1; //lint !e713
}

//-----------------------------------------------------------------------

inline int CentralServer::getNumPlanetServers() const
{
	return m_planetServers.size(); //lint !e713
}

//-----------------------------------------------------------------------

inline const uint32 CentralServer::getDbProcessServerProcessId(void) const
{
	return m_dbProcessServerProcessId;
}

//-----------------------------------------------------------------------

inline size_t CentralServer::getGameServerCount(void) const
{
	return m_gameServers.size();
}

//-----------------------------------------------------------------------

inline int CentralServer::getPlayerCount() const
{
	return m_totalPlayerCount;
}

//-----------------------------------------------------------

inline int CentralServer::getFreeTrialCount() const
{
	return m_totalFreeTrialCount;
}

//-----------------------------------------------------------

inline int CentralServer::getEmptySceneCount() const
{
	return m_totalEmptySceneCount;
}

//-----------------------------------------------------------

inline int CentralServer::getTutorialSceneCount() const
{
	return m_totalTutorialSceneCount;
}

//-----------------------------------------------------------

inline int CentralServer::getFalconSceneCount() const
{
	return m_totalFalconSceneCount;
}

//-----------------------------------------------------------------------

inline void CentralServer::setDbProcessServerProcessId(const uint32 newDbServerProcessId)
{
	m_dbProcessServerProcessId = newDbServerProcessId;
}

//-----------------------------------------------------------------------

inline bool CentralServer::isPreloadFinished() const
{
	return (!m_databaseBacklogged && m_planetsWaitingForPreload.empty() && !m_planetServers.empty());
}

//-----------------------------------------------------------------------

inline bool CentralServer::isInClusterInitialStartup() const
{
	return (m_clusterStartupTime == -1);
}

// ----------------------------------------------------------------------

inline int CentralServer::getPlanetsWaitingForPreloadCount() const
{
	return static_cast<int>(m_planetsWaitingForPreload.size());
}

// ----------------------------------------------------------------------

inline bool CentralServer::isDatabaseBacklogged() const
{
	return m_databaseBacklogged;
}

// ----------------------------------------------------------------------

inline time_t CentralServer::getLastTimeSystemTimeMismatchNotification() const
{
	return m_lastTimeSystemTimeMismatchNotification;
}

// ----------------------------------------------------------------------

inline const std::string & CentralServer::getLastTimeSystemTimeMismatchNotificationDescription() const
{
	return m_lastTimeSystemTimeMismatchNotificationDescription;
}

// ----------------------------------------------------------------------

inline const std::string & CentralServer::getDisconnectedTaskManagerList() const
{
	return m_disconnectedTaskManagerList;
}

// ----------------------------------------------------------------------

inline uint8 CentralServer::getClusterId() const
{
	return m_clusterId;
}

// ======================================================================

#endif	//_CentralServer_H
