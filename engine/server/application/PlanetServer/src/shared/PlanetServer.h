// ======================================================================
//
// PlanetServer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PlanetServer_H
#define INCLUDED_PlanetServer_H

// ======================================================================

#include <map>
#include <set>
#include <vector>

#include "sharedMessageDispatch/Receiver.h"
#include "Singleton/Singleton.h"

#include "PreloadManager.h"
#include "TaskConnection.h"

class CentralServerConnection;
class ChunkCompleteMessage;
class GameNetworkMessage;
class GameServerConnection;
class GameServerData;
class PlanetProxyObject;
class PlanetServerMetricsData;
class PreloadListData;
class RequestGameServerForLoginMessage;
class RequestSceneTransfer;
class Service;
class Vector;
class WatcherConnection;
struct CharacterFindInfo;

//-----------------------------------------------------------------------

class PlanetServer : public Singleton<PlanetServer>, public MessageDispatch::Receiver
{
public:
	typedef std::vector<WatcherConnection*> WatcherList;
	typedef unsigned int                    GameServerSpawnDelaySeconds;

public:
	PlanetServer();
	~PlanetServer();

	static void          run(void);
	void                 mainLoop(void);
	void                 setDone(char const *reasonfmt, ...);

	const unsigned short getGameServicePort() const;
	const Service *      getGameService() const;
	int                  getNumberOfGameServers() const;
	int                  getObjectCountForServer(uint32 serverId) const;
	int                  getInterestObjectCountForServer(uint32 serverId) const;
	int                  getInterestCreatureObjectCountForServer(uint32 serverId) const;
	void                 getObjectCountsForAllServers(stdmap<uint32, GameServerData>::fwd &counts) const;
	virtual void         receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	void                 sendToCentral(const GameNetworkMessage & message, const bool reliable);
	void                 sendToGameServer(const uint32 processId, const GameNetworkMessage & message);
	void                 setTaskManager(TaskConnection*);
	void                 startGameServer(const std::set<PreloadServerId> & preloadServerId, GameServerSpawnDelaySeconds spawnDelay);
	GameServerData *     getGameServerData(uint32 serverId) const;
	void                 preloadCompleteOnAllServers();
	int                  getCurrentServerInterestObjectLimit() const;
	void                 queueMessageForObject(const NetworkId &networkId, GameNetworkMessage *theMessage);
	void                 sendQueuedMessagesForObject(const PlanetProxyObject &theObject);
	void                 unloadObject (const NetworkId &object, uint32 authServer);
	void                 sendChunkRequest(uint32 server, int x, int z);
	void                 sendPreloadRequestCompleteMessage(uint32 realServerId, uint32 preloadServerId);
	void                 requestAndWaitForSave();
	void                 onCentralConnected(CentralServerConnection *connection);

	const WatcherList &  getWatchers() const;
	bool                 isWatcherPresent() const;
	bool                 isInTutorialMode() const;
	bool                 getEnablePreload() const;
	bool                 isInSpaceMode() const;

	GameServerConnection *getGameServerConnection(uint32 serverId);
	ServerConnection *getCentralServerConnection();

private:
	typedef std::map<uint32, GameServerData*>       GameServerMapType;

private:
	void                        handleRequestGameServerForLoginMessage  (const RequestGameServerForLoginMessage *&msg);
	void                        loadCharacterForLogin(NetworkId const &characterId, Vector const &coords, unsigned long gameServerId);
	void                        handleRequestSceneTransfer(const RequestSceneTransfer *& msg);
	void                        handleChunkComplete(const ChunkCompleteMessage &msg, const GameServerConnection *conn);
	void                        flushWaitingMessages();
	void                        findOrLoadCharacter(NetworkId const &characterId, NetworkId const &containerId, Vector const &coords, uint32 stationId, bool forCtsSourceCharacter);
	void                        handleFindAuthObjectResponse(NetworkId const &characterId, unsigned int sequence, bool found, uint32 gameServerId);
	void                        forceLoadCharacter(NetworkId const &characterId, NetworkId const &containerId, Vector const &coords, uint32 stationId, bool forCtsSourceCharacter);
	void                        handleDatabaseSaveComplete(int newSaveCounter);
	
private:
	CentralServerConnection *  m_pendingCentralServerConnection;
	CentralServerConnection *  m_centralServerConnection;
	Service *                  m_gameService;
	Service *                  m_watcherService;
	GameServerMapType          m_gameServers;
	TaskConnection *           m_taskConnection;
	bool                       m_done;
	int                        m_roundRobinGameServer;
	stdmap<PreloadServerId, GameServerSpawnDelaySeconds>::fwd * m_pendingServerStarts; // number of game servers to start when central & task manager are ready
	stdmap<int, std::pair<std::string, time_t> >::fwd * m_startingGameServers; // number of game servers we've started that haven't connected to us yet
	uint32                     m_firstGameServer;
	bool                       m_tutorialMode;
	bool                       m_spaceMode;
	std::set<GameServerConnection *> m_pendingGameServerDisconnects;

	std::vector<const GameNetworkMessage *> m_messagesWaitingForGameServer;
	PlanetServerMetricsData*                m_metricsData;
	TaskConnection*                         m_taskManagerConnection;
	stdlist<const RequestSceneTransfer *>::fwd *m_sceneTransferChunkLoads;
	stdmap<NetworkId, uint32>::fwd          *m_pendingCharacterSaves;

	WatcherList m_watchers;
	bool m_watcherIsPresent;

	typedef std::map<NetworkId, std::vector<GameNetworkMessage*> > QueuedMessagesType;
	QueuedMessagesType m_queuedMessages;
	stdmap<NetworkId, CharacterFindInfo>::fwd *m_characterFindMap;
	unsigned int m_characterFindSequence;
	int          m_waitForSaveCounter;
	int          m_lastSaveCounter;

private:
	//disable:
	PlanetServer(const PlanetServer&);
	PlanetServer &operator=(const PlanetServer&);
};

// ----------------------------------------------------------------------

inline int PlanetServer::getNumberOfGameServers() const
{
	return static_cast<int>(m_gameServers.size());
}

// ----------------------------------------------------------------------

inline void PlanetServer::onCentralConnected(CentralServerConnection *connection)
{
	m_pendingCentralServerConnection=nullptr;
	m_centralServerConnection=connection;
}

// ----------------------------------------------------------------------

inline void PlanetServer::setTaskManager(TaskConnection* con)
{
	m_taskManagerConnection = con;
}

// ----------------------------------------------------------------------

inline bool PlanetServer::isWatcherPresent() const
{
	return m_watcherIsPresent;
}

// ----------------------------------------------------------------------

inline const PlanetServer::WatcherList & PlanetServer::getWatchers() const
{
	return m_watchers;
}

// ----------------------------------------------------------------------

inline bool PlanetServer::isInTutorialMode() const
{
	return m_tutorialMode;
}

// ----------------------------------------------------------------------

inline bool PlanetServer::isInSpaceMode() const
{
	return m_spaceMode;
}

// ======================================================================

#endif	//_CentralServer_H
