// ConnectionServer.h
// copyright 2001 Verant Interactive

#ifndef	_ConnectionServer_H
#define	_ConnectionServer_H

//-----------------------------------------------------------------------

#include <unordered_map>
#include <set>
#include <string>

#include "CentralConnection.h"
#include "Client.h"
#include "GameConnection.h"
#include "Singleton/Singleton.h"
#include "serverKeyShare/KeyServer.h"
#include "serverKeyShare/KeyShare.h"
#include "sharedFoundation/StationId.h"
#include "sharedMessageDispatch/Receiver.h"

class ClientConnection;
class CharacterListMessageData;
class ChatServerConnection;
class ConnectionServerId;
class CustomerServiceConnection;
class ConnectionServerMetricsData;
class NetworkBarrier;
class SessionApiClient;
class UdpSock;

//-----------------------------------------------------------------------

class ConnectionServer : public MessageDispatch::Receiver
{
  public:
	static void            install();
	static void            remove();
	void setDone(char const *reasonfmt, ...);

	~ConnectionServer ();
		
	typedef std::unordered_map<uint32, GameConnection *>              GameServerMap;
	typedef std::unordered_map<NetworkId, Client *,NetworkId::Hash>   ClientMap;
	typedef std::unordered_map<uint32, ClientConnection *>            SuidMap;
	typedef std::set<uint32>                                     FreeTrialsSet;
	
	static void                   addNewClient(ClientConnection* cconn, const NetworkId &oid, GameConnection* gconn, const std::string &sceneName, bool sendToStarport );
	static void                   addConnectedClient(uint32 suid, ClientConnection* conn);
	static void                   addGameConnection(unsigned long gameServerId, GameConnection* gc);
	static bool                   decryptToken(const KeyShare::Token & token, uint32 & stationUserId, bool & secure, std::string & accountName);
	static bool                   decryptToken(const KeyShare::Token & token, char* sessionKey, StationId & stationId);
	static CentralConnection *    getCentralConnection();
	static void                   dropClient(ClientConnection * conn, const std::string &description);
	static const Service *        getChatService  ();
	static const Service *        getCustomerService  ();
	static const CustomerServiceConnection *  getCustomerServiceConnection  ();
	static Client*                getClient(const NetworkId & oid);
	static const ClientMap &      getClientMap();
	static Service *              getClientServicePrivate  ();
	static Service *              getClientServicePublic  ();
	static ClientConnection*      getClientConnection(uint32 suid);
	static uint16                 getPingPort         ();
	static int                    getPingTrafficNumBytes();
	static int                    getNumberOfClients();
	static int                    getNumberOfFreeTrials();
	static int                    getNumberOfGameServers();
	static GameConnection*        getGameConnection(uint32 gameServerId);
	static GameConnection*        getGameConnection(const std::string &sceneName);
	static GameConnection*        getAnyGameConnection();
	static const Service *        getGameService ();
	static SessionApiClient*      getSessionApiClient();
	void                          handleConnectionServerIdMessage(const ConnectionServerId& msg);
	static KeyShare::Token        makeToken(const unsigned char * newData, const uint32 dataLen);
	static void                   pushKey(const KeyShare::Key & newKey);
	void                          receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	static void                   removeConnectedCharacter(uint32 suid);
	static void                   run(void);
	static void                   sendToCentralProcess(const GameNetworkMessage& msg);
	static void                   addRecoveringClient(const NetworkId& networkId);		
	
private:
	ConnectionServer  ();
	ConnectionServer (const ConnectionServer & source);
	ConnectionServer & operator=(const ConnectionServer & rhs);
	static ConnectionServer & instance();

	void                    setupConnections();
	void                    unsetupConnections();
	void                    update();
	void                    updateRecoveringClientList(uint elapsedTime);
	void                    installSessionValidation();

	void                    addToClientMap(const NetworkId &oid, ClientConnection* cconn);
	void                    removeFromClientMap(const NetworkId &oid);

	void                    addToConnectedMap(uint32 suid, ClientConnection* conn);
	void                    removeFromConnectedMap(uint32 suid);

	void                    updatePopulationOnCentralServer();
	
private:
	Service *                         chatService;
	Service *                         customerService;
	Service *                         clientServicePrivate;
	Service *                         clientServicePublic;
	Service *                         gameService;
	KeyServer*                        loginServerKeys;
	
	bool                              done;
	int                               m_id;
	ConnectionServerMetricsData*      m_metricsData;
	
	CentralConnection *                     centralConnection;
	std::set<ChatServerConnection *>        chatServers;
	std::set<CustomerServiceConnection *>   customerServiceServers;
	ClientMap                               clientMap;
	SuidMap                                 connectedMap;
	GameServerMap                           gameServerMap;
	FreeTrialsSet                           freeTrials;
	NetworkBarrier *                        networkBarrier;
	UdpSock *                               pingSocket;
	uint                                    m_recoverTime;
	SessionApiClient*                       m_sessionApiClient;
	int                                     m_pingTrafficNumBytes;
	
	typedef std::vector<std::pair<uint, NetworkId> > RecoveringClientListType;
	RecoveringClientListType                m_recoveringClientList;
};

//-----------------------------------------------------------------------

#endif	//_ConnectionServer_H





