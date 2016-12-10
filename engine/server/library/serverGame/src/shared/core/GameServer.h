// GameServer.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_GAME_SERVER_H
#define	_GAME_SERVER_H

//-----------------------------------------------------------------------

#include "Archive/ByteStream.h"
#include "serverGame/ProxyList.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedFoundation/NetworkId.h"

class CachedNetworkId;
class CentralCreateCharacter;
class CentralServerConnection;
class ChatServerConnection;
class ConnectionServerConnection;
class Client;
class CreatureObject;
class CustomerServiceServerConnection;
class DatabaseProcessConnection;
class DynamicVariable;
class EnableNewJediTrackingMessage;
class GameNetworkMessage;
class GameServerCSRequestMessage;
class GameServerMetricsData;
class MessageToPayload;
class NameGenerator;
class NetworkId;
class ObjectList;
class PlanetServerConnection;
class PopulationList;
class RandomNameRequest;
class ReloadAdminTableMessage;
class ReloadCommandTableMessage;
class ReloadDatatableMessage;
class ReloadScriptMessage;
class ReloadTemplateMessage;
class RetrievedItemLoadMessage;
class ServerCreatureObjectTemplate;
class ServerObject;
class ServerWorld;
class Service;
class SetMessageToBacklogThreshold;
class SetMessageToTimeLimitMessage;
class StringId;
class TaskManagerConnection;
class TeleportMessage;
class Vector;
class VerifyNameResponse;
class VerifyAndLockNameRequest;

namespace MessageDispatch {
	class Callback;
}

//-----------------------------------------------------------------------

class GameServer : 
public MessageDispatch::Receiver
{
public:
	typedef std::unordered_map<NetworkId, Client *, NetworkId::Hash> ClientMap;
	typedef std::vector<ConnectionServerConnection *>           ConnectionServerVector;

	virtual ~GameServer ();

	static GameServer &          getInstance ();
	static void                  install     ();
	static void                  remove      ();

	bool                         getDone                       ();
	void                         connectToDatabaseProcess      (std::string const &address, uint16 port, uint32 processId);
	void                         createRemoteProxy             (uint32 remoteProcessId, ServerObject *object);
	void                         createProxyOnAllServers       (ServerObject* object);
	void                         deliverMessageToClientObject  (const NetworkId &oid, const Archive::ByteStream & message);
	void                         spamAllClients                (const GameNetworkMessage & message, const bool reliable) const;
	void                         dropClient(const NetworkId &oid);
	void                         dropClient(const NetworkId &oid, bool const immediate);
	const Service *	             getClientService              () const;
	uint32                       getDatabaseProcessId          () const;
	Client *                     getClient                     (const NetworkId& networkId);
	const std::string &          getClusterName                () const;
	uint8                        getClusterId                  () const;
	bool                         isGameServerConnected         (uint32 processId) const;
	ConnectionServerConnection * getConnectionServerConnection (const std::string & connectionServerIp, const uint16 connectionServerPort);
	uint32                       getFirstGameServerForPlanet   ();
	unsigned long                getFrameTime                  () const;
	int                          getNumClients() const;
	uint32                       getProcessId                  () const;
	uint32                       getPreloadAreaId              () const;
	virtual void                 receiveMessage                (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	bool                         requestSceneWarp              (const CachedNetworkId &objectId, const std::string &sceneName, const Vector &newPosition_w, const NetworkId &newContainer, const Vector &newPosition_p, const char * scriptCallback = nullptr, bool forceLoadScreen = false);
	bool                         requestSceneWarp              (const CachedNetworkId &objectId, const std::string &sceneName, const Vector &newPosition_w, const NetworkId &newBuilding, const std::string &newCellName, const Vector &newPosition_p, const char * scriptCallback = nullptr, bool forceLoadScreen = false);
	bool                         requestSceneWarpDelayed       (const CachedNetworkId &objectId, const std::string &sceneName, const Vector &newPosition_w, const NetworkId &newContainer, const Vector &newPosition_p, float delayTime, const char * scriptCallback = nullptr, bool forceLoadScreen = false);
	bool                         requestSceneWarpDelayed       (const CachedNetworkId &objectId, const std::string &sceneName, const Vector &newPosition_w, const NetworkId &newBuilding, const std::string &newCellName, const Vector &newPosition_p, float delayTime, const char * scriptCallback = nullptr, bool forceLoadScreen = false);
	
	static void                  run                           ();
	void                         sendToCentralServer           (GameNetworkMessage const &message);
	void                         sendToDatabaseServer          (GameNetworkMessage const &message);
	void                         sendToPlanetServer            (GameNetworkMessage const &message);
	void                         sendToConnectionServers       (GameNetworkMessage const &message);
	void                         sendToCustomerServiceServer   (GameNetworkMessage const &message);
	void                         clearCustomerServiceServerConnection();
	void                         setCentralServerConnection    (CentralServerConnection *newCentralServerConnection);
	void                         setDatabaseProcessConnection  (DatabaseProcessConnection *newDatabaseProcessConnection);
	void                         setDone                       (char const *reasonfmt, ...);
	void                         setProcessId                  (uint32 newProcessId);
	void                         onDatabaseProcessConnectionEstablished();
	void                         onPlanetServerConnectionEstablished ();
	void                         onReceivedFirstNetworkIdBlock       ();
	
	const std::string &          getCommandLine() const;
	void                         setCommandLine(const std::string &);
	
	void                         loadTerrain ();

	std::vector<uint32> const &getAllGameServerPids() const;
	bool                         hasConnectionsToOtherGameServers() const;

	void                         getObjectsWithClients(std::vector<ServerObject *> &objects) const;

	static uint64                getTotalObjectCreatesReceived();
	static uint64                getTotalObjectCreatesSent();
	static void                  incrementTotalObjectCreatesSent();

	static void                  reportClusterStartupResidenceStructure(int cityId, const NetworkId & structureId, const NetworkId & ownerId);
	static void                  reportBuildingObjectDestroyed(const NetworkId & structureId);

	bool                         isPlanetEnabledForCluster(std::string const &sceneName) const;
	
	virtual void                 handleCSRequest( GameServerCSRequestMessage & request );

	static bool                  addPendingLoadRequest(NetworkId const & id);
	static void                  removePendingLoadRequest(NetworkId const & id);
	static bool                  isAtPendingLoadRequestLimit();
	static int                   getPendingLoadRequestLimit();
	static int                   getNumberOfPendingLoadRequests();
	static unsigned long         getOldestPendingLoadRequestTime(NetworkId & id);

	static std::string           getRetroactiveCtsHistory(std::string const & clusterName, NetworkId const & characterId);
	static std::vector<std::vector<std::pair<std::string, DynamicVariable> > const *> const & getRetroactiveCtsHistoryObjvars(NetworkId const & characterId);
	static void                  setRetroactiveCtsHistory(CreatureObject & player);

	static time_t                getRetroactivePlayerCityCreationTime(std::string const & clusterName, int cityId);

	static int                   getServerSpawnLimit();

#ifdef _DEBUG
	static void                  setExtraDelayPerFrameMs(int ms); // to emulate long loop time
#endif

	const StringId               handleVerifyAndLockNameRequest(const VerifyAndLockNameRequest &request, bool sendFailedResponseToCentralServer, bool verifyAgainstDb);

protected:
	             GameServer ();
	virtual void initialize ();
	virtual void shutdown   ();

protected:
	static GameServer * ms_instance;
	
private:
	GameServer                    (const GameServer & source);
	GameServer &  operator =      (const GameServer & rhs);
	void          debugIO         ();
	
private:
	void                         gameServerReady               ();
	void                         checkWhetherGameServerIsReady ();
	void                         handleTeleportMessage         (TeleportMessage const &message);
	void                         handleRetrievedItemLoadMessage(RetrievedItemLoadMessage const &message);
	void                         handleReloadAdminTableMessage (ReloadAdminTableMessage const &message);
	void                         handleReloadCommandTableMessage(ReloadCommandTableMessage const &message);
	void                         handleReloadDatatableMessage  (ReloadDatatableMessage const &message);
	void                         handleReloadScriptMessage     (ReloadScriptMessage const &message);
	void                         handleReloadTemplateMessage   (ReloadTemplateMessage const &message);
	void                         handleEnableNewJediTrackingMessage(EnableNewJediTrackingMessage const &message);
	void                         checkIdleClients              (float time);
	void                         receiveMessage2               (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	
private:
	//-----------------------------------------------------------------------
	//	maps
	
	typedef std::map<NetworkId, std::vector<NetworkId> >     PendingContainedObjectsMap;
	typedef std::map<uint32, const CentralCreateCharacter *>    CharactersPendingCreationMap;

	bool                            m_done;
	uint32                          m_processId;
	uint32                          m_preloadAreaId;
	uint32                          m_databaseProcessId;
	std::vector<uint32>             m_gameServerPids;
	CentralServerConnection *       m_centralService;
	CentralServerConnection *       m_centralServerConnection;
	CustomerServiceServerConnection *m_customerServiceServerConnection;
	ConnectionServerVector *        m_connectionServerVector;
	PlanetServerConnection *        m_planetServerConnection;
	DatabaseProcessConnection *     m_databaseProcessConnection;
	DatabaseProcessConnection *     m_pendingDatabaseProcessConnection;
	TaskManagerConnection *         m_taskManagerConnection;    
	ClientMap *                     m_clients;
	Service *                       m_clientService;
	std::string                     m_clusterName;
	uint8                           m_clusterId;
	uint32                          m_firstGameServerForPlanet;
	

	PendingContainedObjectsMap *                       m_pendingContainedObjects;
	CharactersPendingCreationMap *                     m_charactersPendingCreation;

	GameServerMetricsData *         m_metricsData;
	std::string                     m_commandLine;
	
	// name generation and validation ----------------------------------
	const StringId                  verifyCharacterName(const ServerCreatureObjectTemplate &serverTemplate, const Unicode::String &name, bool forPlayer) const; 
	const StringId                  verifyCharacterName(const std::string &serverTemplate, const Unicode::String &name, bool forPlayer) const; 
	const ServerCreatureObjectTemplate * getServerCreatureObjectTemplate(const std::string &name) const;
	void                            handleCreateCharacter(const CentralCreateCharacter * createMessage);
	void                            handleNameRequest(const RandomNameRequest &request);
	void                            handleNameFailure(const RandomNameRequest &request, const StringId &errorMessage);
	void                            handleVerifyAndLockNameVerification(const VerifyNameResponse &response);
	void                            handleCharacterCreateNameVerification(const VerifyNameResponse &response);
	const StringId &                canCreateAvatar(const std::string &templateName);
	bool                            isCreatePending(uint32 stationId) const;
	// ----------------------------------------------------------------------
	//  GameServer startup flags
	bool                            m_gameServerReadyObjectIds;
	bool                            m_gameServerReadyDatabaseConnected;
	bool                            m_gameServerReadyPlanetConnected;
	uint32                          m_connectionTimeout;
	
	ChatServerConnection *          m_chatServerConnection;
};

//-----------------------------------------------------------------------

inline bool GameServer::getDone(void)
{
	return m_done;
}

//-----------------------------------------------------------------------

inline const std::string & GameServer::getClusterName(void) const
{
	return m_clusterName;
}

// ----------------------------------------------------------------------

inline uint8 GameServer::getClusterId() const
{
	return m_clusterId;
}

//-----------------------------------------------------------------------

inline uint32 GameServer::getProcessId() const
{
	return m_processId;
}

//-----------------------------------------------------------------------

inline uint32 GameServer::getPreloadAreaId() const
{
	return m_preloadAreaId;
}

//-----------------------------------------------------------------------

inline void GameServer::setProcessId(uint32 newProcessId)
{
	m_processId = newProcessId;
}

//-----------------------------------------------------------------------

inline void GameServer::setCentralServerConnection(CentralServerConnection * newCentralServerConnection)
{
	m_centralServerConnection = newCentralServerConnection;
}

//-----------------------------------------------------------------------

inline const std::string & GameServer::getCommandLine() const
{
	return m_commandLine;
}

//-----------------------------------------------------------------------

inline void GameServer::setCommandLine(const std::string & c)
{
	m_commandLine = c;
}

//----------------------------------------------------------------------

inline GameServer & GameServer::getInstance ()
{
	FATAL(!ms_instance, ("GameServer not installed!"));

	return *NON_NULL (ms_instance);
}

//-----------------------------------------------------------------------

inline uint32 GameServer::getFirstGameServerForPlanet()
{
	return m_firstGameServerForPlanet;
}

// ----------------------------------------------------------------------

#endif	// _GAME_SERVER_H
