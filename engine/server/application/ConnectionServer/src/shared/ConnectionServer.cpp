
// ConnectionServer.cpp
// copyright 2001 Verant Interactive


//-----------------------------------------------------------------------

#include "FirstConnectionServer.h"
#include "ConnectionServer.h"

#include "Archive/ByteStream.h"
#include "CentralConnection.h"
#include "ChatServerConnection.h"
#include "ClientConnection.h"
#include "ConfigConnectionServer.h"
#include "ConnectionServerMetricsData.h"
#include "CustomerServiceConnection.h"
#include "GameConnection.h"
#include "PseudoClientConnection.h"
#include "SessionApiClient.h"
#include "UdpLibrary.h"
#include "UnicodeUtils.h"
#include "serverKeyShare/KeyShare.h"
#include "serverMetrics/MetricsManager.h"
#include "serverNetworkMessages/CentralConnectionServerMessages.h"
#include "serverNetworkMessages/CharacterListMessage.h"
#include "serverNetworkMessages/ExcommunicateGameServerMessage.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverNetworkMessages/GameServerForLoginMessage.h"
#include "serverNetworkMessages/LoginKeyPush.h"
#include "serverNetworkMessages/NewClient.h"
#include "serverNetworkMessages/ProfilerOperationMessage.h"
#include "serverNetworkMessages/RandomName.h"
#include "serverNetworkMessages/SetConnectionServerPublic.h"
#include "serverNetworkMessages/UpdateConnectionServerStatus.h"
#include "serverNetworkMessages/UpdatePlayerCountMessage.h"
#include "serverNetworkMessages/ValidateAccountReplyMessage.h"
#include "serverNetworkMessages/ValidateCharacterForLoginReplyMessage.h"
#include "serverNetworkMessages/VerifyAndLockName.h"
#include "serverUtility/AdminAccountManager.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedLog/Log.h"
#include "sharedLog/LogManager.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedMemoryManager/MemoryManager.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetwork/UdpSock.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedUtility/DataTableManager.h"
#include <vector>

// ======================================================================
namespace ConnectionServerNamespace
{
	ConnectionServer * s_connectionServer   = 0;
	NetworkSetupData * s_clientServiceSetup = 0;

	const std::string SCENE_NAME_TUTORIAL      = "tutorial";
	const std::string SCENE_NAME_FALCON_PREFIX = "space_npe_falcon";
};

using namespace ConnectionServerNamespace;

// ======================================================================

ConnectionServer & ConnectionServer::instance()
{
	return *s_connectionServer;
}

//-----------------------------------------------------------------------

ConnectionServer::ConnectionServer() :
MessageDispatch::Receiver(),
chatService(0),
customerService(0),
clientServicePrivate(0),
clientServicePublic(0),
gameService(0),
loginServerKeys(0),
done(false),
m_id(0),
m_metricsData(0),
centralConnection(0),
chatServers(),
customerServiceServers(),
clientMap(),
connectedMap(),
gameServerMap(),
freeTrials(),
networkBarrier(0),
pingSocket (new UdpSock),
m_recoverTime(0),
m_sessionApiClient(0),
m_pingTrafficNumBytes(0),
m_recoveringClientList()
{
	if(s_clientServiceSetup == 0)
		s_clientServiceSetup = new NetworkSetupData;
	
	s_clientServiceSetup->maxOutstandingPackets = ConfigConnectionServer::getClientMaxOutstandingPackets();
	s_clientServiceSetup->maxRawPacketSize = ConfigConnectionServer::getClientMaxRawPacketSize();
	s_clientServiceSetup->maxConnections = ConfigConnectionServer::getClientMaxConnections();
	s_clientServiceSetup->fragmentSize = ConfigConnectionServer::getClientFragmentSize();
	s_clientServiceSetup->maxDataHoldTime = ConfigConnectionServer::getClientMaxDataHoldTime();
	s_clientServiceSetup->hashTableSize=ConfigConnectionServer::getClientHashTableSize();
	s_clientServiceSetup->port = ConfigConnectionServer::getClientServicePortPublic();
	s_clientServiceSetup->compress = ConfigConnectionServer::getCompressClientNetworkTraffic();
	s_clientServiceSetup->useTcp = false;

	loginServerKeys = new KeyServer(20);

	Address a("", ConfigConnectionServer::getPingPort());
	IGNORE_RETURN(pingSocket->bind (a));

	if (ConfigConnectionServer::getValidateStationKey())
	{
		installSessionValidation();
	}

}

//-----------------------------------------------------------------------

ConnectionServer::~ConnectionServer()
{
	delete pingSocket;
	pingSocket = 0;

	delete loginServerKeys;
	loginServerKeys = 0;

	centralConnection=0;

	chatServers.clear();

	customerServiceServers.clear();

	connectedMap.clear();

	clientServicePublic = 0;
	clientServicePrivate = 0;
	chatService = 0;
	customerService = 0;
	gameService = 0;
	gameServerMap.clear();
	freeTrials.clear();
	delete s_clientServiceSetup;
}

//-----------------------------------------------------------------------

const CustomerServiceConnection * ConnectionServer::getCustomerServiceConnection  ()
{
	if(! instance().customerServiceServers.empty())
	{
		return (*(instance().customerServiceServers.begin()));
	}
	return NULL;
}

//-----------------------------------------------------------------------

//TODO: This assumes that all characters that aren't on a game server
// are waiting for this gameserver.  This assumption is bad:  they could
// be waiting on a different gameserver, or they could be in the process
// of being validated

void ConnectionServer::addGameConnection(unsigned long gameServerId, GameConnection* gc)
{
	static ConnectionServer & cs = instance();

	cs.gameServerMap[gameServerId] = gc;//@todo check for dupe
	// find characters pending for THIS gameserver
	SuidMap::iterator i;
	for(i = cs.connectedMap.begin(); i != cs.connectedMap.end(); ++i)
	{
		ClientConnection * c = (*i).second;
		if(!c->getHasBeenSentToGameServer())
			IGNORE_RETURN(c->sendToGameServer());
	}
}

//-----------------------------------------------------------------------

bool ConnectionServer::decryptToken(const KeyShare::Token & token, uint32 & stationUserId, bool & secure, std::string & accountName)
{
	static ConnectionServer & cs = instance();

	//Also the sizeof(int) is likewise magic from the session api
	uint32 len = sizeof(uint32) + sizeof(bool) + MAX_ACCOUNT_NAME_LENGTH + 1;
	unsigned char * keyBuffer = new unsigned char[len];
	unsigned char * keyBufferPointer = keyBuffer;
	NOT_NULL(keyBuffer);
	memset(keyBuffer, 0, len);
	
	
	bool retval =  cs.loginServerKeys->decipherToken(token,  keyBuffer, len);

	if (! retval)
		return retval;

	char *tmpBuffer = new char[MAX_ACCOUNT_NAME_LENGTH + 1];
	memset(tmpBuffer, 0, MAX_ACCOUNT_NAME_LENGTH + 1);
	
	memcpy(&stationUserId, keyBufferPointer, sizeof(uint32));
	keyBufferPointer += sizeof(uint32);
	memcpy(&secure, keyBufferPointer, sizeof(bool));
	keyBufferPointer += sizeof(bool);
	memcpy(tmpBuffer, keyBufferPointer, MAX_ACCOUNT_NAME_LENGTH);
	accountName = tmpBuffer;
	delete[] tmpBuffer;
	delete [] keyBuffer;
	return retval;
}	

bool ConnectionServer::decryptToken(const KeyShare::Token & token, char* sessionKey, StationId & stationId)
{
	static ConnectionServer & cs = instance();
	
	uint32 len = apiSessionIdWidth + sizeof(StationId);
	unsigned char * keyBuffer = new unsigned char[len + 1];
	unsigned char * keyBufferPointer = keyBuffer;
	NOT_NULL(keyBuffer);
	memset(keyBuffer, 0, len);
	
	
	bool retval =  cs.loginServerKeys->decipherToken(token,  keyBuffer, len);

	if (! retval)
		return retval;

	memcpy(sessionKey, keyBufferPointer, apiSessionIdWidth);
	keyBufferPointer += apiSessionIdWidth;
	memcpy(&stationId, keyBufferPointer, sizeof(StationId));
	delete [] keyBuffer;
	return retval;
}


//-----------------------------------------------------------------------

const Service * ConnectionServer::getChatService()
{
	static ConnectionServer & cs = instance();
	return cs.chatService;
}

//-----------------------------------------------------------------------

const Service * ConnectionServer::getCustomerService()
{
	static ConnectionServer & cs = instance();
	return cs.customerService;
}

//-----------------------------------------------------------------------

Service * ConnectionServer::getClientServicePrivate()
{
	static ConnectionServer & cs = instance();
	return cs.clientServicePrivate;
}

//-----------------------------------------------------------------------

Service * ConnectionServer::getClientServicePublic()
{
	static ConnectionServer & cs = instance();
	return cs.clientServicePublic;
}

//-----------------------------------------------------------------------

KeyShare::Token ConnectionServer::makeToken(const unsigned char * newData, const uint32 dataLen)
{
	static ConnectionServer & cs = instance();
	return cs.loginServerKeys->makeToken(newData, dataLen);
}

//-----------------------------------------------------------------------

void ConnectionServer::pushKey(const KeyShare::Key & newKey)
{
	static ConnectionServer & cs = instance();
	cs.loginServerKeys->pushKey(newKey);
}

//-----------------------------------------------------------------------

void ConnectionServer::addNewClient(ClientConnection* cconn, const NetworkId &oid, GameConnection* gconn, const std::string &, bool sendToStarport)
{
	static ConnectionServer & cs = instance();
	ClientMap::iterator i = cs.clientMap.find(oid);
	if(i != cs.clientMap.end())
	{
			if(cconn->getClient())
			{
				WARNING_STRICT_FATAL(true, ("Client already connected, attempting to drop old one\n"));
				dropClient(cconn, "Duplicate Login");
				return;
			}
			else
			{
				// stale connection in map
				cs.removeFromClientMap(oid);
			}
	}
	
	// Create a new entry in the client map
	cs.addToClientMap(oid, cconn);

	// Get the client that was just created
	Client * newClient = cs.getClient(oid);
	NOT_NULL(newClient);

	// select a chat server connection for the client
	// if non exists, then the client will be notified when
	// one starts
	if(! cs.chatServers.empty())
	{
		//ChatServerConnection * c = (*chatServers.begin());
		// find chat server with least load
		size_t max = 0xFFFFFFFF;
		ChatServerConnection * candidate = 0;
		std::set<ChatServerConnection *>::const_iterator iter;
		for(iter = cs.chatServers.begin(); iter != cs.chatServers.end(); ++iter)
		{
			if((*iter)->getClients().size() <= max)
			{
				candidate = (*iter);
				max = (*iter)->getClients().size();
			}
		}
		if(candidate)
			newClient->setChatConnection(candidate);
	}

	// select a cs server connection for the client
	// if non exists, then the client will be notified when
	// one starts
	if(! cs.customerServiceServers.empty())
	{
		//ChatServerConnection * c = (*chatServers.begin());
		// find chat server with least load
		size_t max = 0xFFFFFFFF;
		CustomerServiceConnection * candidate = 0;
		std::set<CustomerServiceConnection *>::const_iterator iter;
		for(iter = cs.customerServiceServers.begin(); iter != cs.customerServiceServers.end(); ++iter)
		{
			if((*iter)->getClients().size() <= max)
			{
				candidate = (*iter);
				max = (*iter)->getClients().size();
			}
		}
		if(candidate)
			newClient->setCustomerServiceConnection(candidate);
	}
	
	//send the game server a message about this client.
	static const std::string loginTrace("TRACE_LOGIN");
	LOG(loginTrace, ("NewClient(%d, %s, %s)", cconn->getSUID(), oid.getValueString().c_str(), cconn->getAccountName().c_str()));
	NewClient m(oid, cconn->getAccountName(), cconn->getRemoteAddress(), cconn->getIsSecure(), false, cconn->getSUID(), NULL, cconn->getGameFeatures(), cconn->getSubscriptionFeatures(), cconn->getEntitlementTotalTime(), cconn->getEntitlementEntitledTime(), cconn->getEntitlementTotalTimeSinceLastLogin(), cconn->getEntitlementEntitledTimeSinceLastLogin(), cconn->getBuddyPoints(), cconn->getConsumedRewardEvents(), cconn->getClaimedRewardItems(), cconn->isUsingAdminLogin(), cconn->getCanSkipTutorial(), sendToStarport );
	gconn->send(m, true);
	//@todo move this to ClientConnection.cpp
}

//-----------------------------------------------------------------------

void ConnectionServer::dropClient(ClientConnection * conn, const std::string& description)
{
	DEBUG_FATAL(!conn, ("Cannot call dropClient with NULL connection"));
	if (!conn) //lint !e774 // boolean within 'if' always evaluates to False //suppresed because this is only relevant in DEBUG builds
		return;
	
	static ConnectionServer & cs = instance();
	//Client dropped.  Tell game server if they've logged in.
	LOG("ClientDisconnect", ("Dropping client for SUID %d\n", conn->getSUID()));
	LOG("CustomerService", ("Login:%s Dropped Reason: %s. Character: %s (%s). Play time: %s. Active play time: %s", ClientConnection::describeAccount(conn).c_str(), description.c_str(), conn->getCharacterName().c_str(), conn->getCharacterId().getValueString().c_str(), conn->getPlayTimeDuration().c_str(), conn->getActivePlayTimeDuration().c_str()));

	Client *client = conn->getClient();
	if (client)
	{
		DropClient msg(client->getNetworkId());
		GameConnection* gconn = client->getGameConnection();
		//Don't worry about sending a message to a non-existant game server
		if (gconn)
			gconn->send(msg, true);
		else
			DEBUG_REPORT_LOG(true, ("Could not find game server to drop this client\n"));

		// Remove the entry from the client map
		cs.removeFromClientMap(client->getNetworkId());
	}
	else
	{
		//If they aren't connected to the game yet, they're probably on the pending list.
		//@todo ensure we don't need to send a cleanup message to central
//		removePendingCharacter(conn->getSUID());
	}

	// Remove entry from the connected map
	cs.removeFromConnectedMap(conn->getSUID());

	conn->disconnect();
}

//-----------------------------------------------------------------------
// void ConnectionServer::addPendingCharacter(uint32 suid, ClientConnection* conn)
// {
//     if (pendingMap.find(suid) == pendingMap.end())
//         pendingMap[suid] = conn; 
//     else
//     {
//         WARNING_STRICT_FATAL(true, ("Attepting to add duplicate pending chatacter"));
//         pendingMap[suid] = conn;
//     }
// }

//-----------------------------------------------------------------------

void ConnectionServer::addConnectedClient(uint32 suid, ClientConnection* conn)
{
	static ConnectionServer & cs = instance();
	cs.addToConnectedMap(suid, conn);
}

//-----------------------------------------------------------------------

ClientConnection* ConnectionServer::getClientConnection(const uint32 suid)
{
	static ConnectionServer & cs = instance();
	ClientConnection * result = 0;
	SuidMap::const_iterator i = cs.connectedMap.find(suid);
	if(i != cs.connectedMap.end())
	{
		result = (*i).second;
	}
	return result;
}

// ----------------------------------------------------------------------

const Service * ConnectionServer::getGameService()
{
	static ConnectionServer & cs = instance();
	return cs.gameService;
}

// ----------------------------------------------------------------------

GameConnection* ConnectionServer::getGameConnection(const std::string &sceneName)
{
	static ConnectionServer & cs = instance();
	GameServerMap::iterator i = cs.gameServerMap.begin();
	for(; i != cs.gameServerMap.end(); ++i)
	{
		if (sceneName == (*i).second->getSceneName())
		{
			return (*i).second;
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------
void ConnectionServer::handleConnectionServerIdMessage(const ConnectionServerId & m)
{
	// Connection established with central server.  Set everything up.
	m_id = m.getId();

	const Service * const servicePrivate = getClientServicePrivate();
	const Service * const servicePublic = getClientServicePublic();
	FATAL(servicePrivate == NULL && servicePublic == NULL, ("No client service is active!"));

	const Service * const g = getGameService();
	FATAL(g == NULL, ("No game service is active!"));
	const Service * const c = getChatService();
	const Service * const cs = getCustomerService();
	const uint16   pingPort = getPingPort ();

	if((servicePrivate != NULL || servicePublic != NULL) && g != NULL) //lint !e774 // always evaluates to false // suppresed because the FATAL macro triggers this lint warning
	{
		uint16 chatPort = 0;
		uint16 csPort = 0;
		if(c)
			chatPort = c->getBindPort();

		if (cs)
		{
			csPort = cs->getBindPort();
		}

		uint16 publicPort = 0;
		if(servicePublic)
			publicPort = servicePublic->getBindPort();
		uint16 privatePort = 0;
		if(servicePrivate)
			privatePort = servicePrivate->getBindPort();

		std::string clientServicePublicBindAddress = NetworkHandler::getHostName();
		
		NOT_NULL(gameService);
		NOT_NULL(chatService);
		NOT_NULL(customerService);
		
		const NewCentralConnectionServer ncs(gameService->getBindAddress(), clientServicePublicBindAddress, chatService->getBindAddress(), customerService->getBindAddress(), privatePort, publicPort, g->getBindPort(), chatPort, csPort, pingPort, ConfigConnectionServer::getConnectionServerNumber());
		sendToCentralProcess(ncs);
	}
	else
		FATAL(true, ("Error in connection server startup"));
}

// ----------------------------------------------------------------------

void ConnectionServer::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	// it's reasonably safe to cast, message type verified
	// determine message type

	if(message.isType("GameConnectionOpened"))
	{
		DEBUG_REPORT_LOG(true,("Game Connection opened\n"));
	}
	else if (message.isType("GameConnectionClosed"))
	{
		//@todo handle case where game server drops and we have users connected to it.
		//Drop all connected clients.
		const GameConnection & downConnection = static_cast<const GameConnection &>(source);
		DEBUG_REPORT_LOG(true, ("Game Server connection went down.  Dropping clients.\n"));
		//remove Game Conection from list
		
		PseudoClientConnection::gameConnectionClosed(&downConnection);
		
		GameServerMap::iterator j = gameServerMap.find(downConnection.getGameServerId());
		if (j != gameServerMap.end())
		{
			gameServerMap.erase(j);
		}
	}

	else if(message.isType("CentralConnectionOpened"))
	{
		DEBUG_REPORT_LOG(true,("Opened connection with central\n"));
		centralConnection = const_cast<CentralConnection *>(static_cast<const CentralConnection *>(&source));//lint !e826 // info: Suspiscious pointer-to-pointer conversion (area too small) 
		if(s_clientServiceSetup == 0)
			s_clientServiceSetup = new NetworkSetupData;

		s_clientServiceSetup->useTcp = false;
		if(ConfigConnectionServer::getStartPublicServer())
			clientServicePublic = new Service(ConnectionAllocator<ClientConnection>(), *s_clientServiceSetup);
		s_clientServiceSetup->port = ConfigConnectionServer::getClientServicePortPrivate();
		clientServicePrivate = new Service(ConnectionAllocator<ClientConnection>(), *s_clientServiceSetup);
		s_clientServiceSetup->port = ConfigConnectionServer::getClientServicePortPublic();
		
		connectToMessage("ClientConnectionOpened");
		connectToMessage("ClientConnectionClosed");

	}
	else if (message.isType("CentralConnectionClosed"))
	{
		centralConnection = const_cast<CentralConnection *>(static_cast<const CentralConnection *>(&source));//lint !e826 // info: Suspiscious pointer-to-pointer conversion (area too small) 
		setDone("CentralConnectionClosed: %s", centralConnection ? centralConnection->getDisconnectReason().c_str() : "");
		centralConnection = 0;
		DEBUG_REPORT_LOG(true, ("CentralDied.  So we will too\n"));
		//@todo Drop all pending clients.
	}
	else if(message.isType("ClientConnectionOpened"))
	{
		DEBUG_REPORT_LOG(true, ("Opened connection with client\n"));
	}
	else if (message.isType("ClientConnectionClosed"))
	{
		DEBUG_REPORT_LOG(true, ("Client is Dropping connection\n"));
		ClientConnection * cconn = const_cast<ClientConnection *>(static_cast<const ClientConnection*>(&source));//lint !e826 // info: Suspiscious pointer-to-pointer conversion (area too small) 

		//tell CentralServer
		if (centralConnection)
		{
			GenericValueTypeMessage<StationId> const msg("ClientConnectionClosed", cconn->getSUID());
			centralConnection->send(msg,true);
		}

		//Client dropped.  Tell game server if they've logged in.
		Client *client = cconn->getClient();
		if (client)
		{
			DropClient msg(client->getNetworkId());
			GameConnection* gconn = client->getGameConnection();
			//Don't worry about sending a message to a non-existant game server
			if (gconn)
				gconn->send(msg, true);

			// Remove the entry from the client map
			removeFromClientMap(client->getNetworkId());
		}
		else
		{
			//If they aren't connected to the game yet, they're probably on the pending list.
//            removePendingCharacter(cconn->getSUID());
		}
		removeFromConnectedMap(cconn->getSUID());
	}
	else if (message.isType("ConnectionServerId"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ConnectionServerId m(ri);
		handleConnectionServerIdMessage(m);
	}
	
	else if (message.isType("ConnectionKeyPush"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ConnectionKeyPush pk(ri);
		loginServerKeys->pushKey(pk.getKey());
	}
	
	else if (message.isType("LoginKeyPush"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const LoginKeyPush k(ri);
		loginServerKeys->pushKey(k.getKey());
	}
	else if (message.isType("CharacterListMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const CharacterListMessage msg(ri);
		WARNING_STRICT_FATAL(true,("CharacterListMessage is deprecated on the ConnectionServer -- fix whoever is sending it.\n"));
	}
	else if (message.isType("ConnectionCreateCharacterSuccess"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const ConnectionCreateCharacterSuccess msg(ri);
		LOG("TraceCharacterCreation", ("Received ConnectionCreateCharacterSuccess for %d", msg.getStationId()));
		ClientConnection* const client = getClientConnection(msg.getStationId());
		if (client)
		{
			const ClientCreateCharacterSuccess m (msg.getNetworkId ());
			client->send(m, true);
		}
		else
		{
			LOG("CustomerService", ("CharacterTransfer: Trying to deliver ConnectionCreateCharacterSuccess to PsuedoClientConnection(%d)", msg.getStationId()));
			PseudoClientConnection::tryToDeliverMessageTo(static_cast<unsigned int>(msg.getStationId()), static_cast<const GameNetworkMessage &>(message).getByteStream());
		}
	}
	else if (message.isType("ConnectionCreateCharacterFailed"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const ConnectionCreateCharacterFailed msg(ri);
		ClientConnection* client = getClientConnection(msg.getStationId());
		if (client)
		{
			client->setHasRequestedCharacterCreate(false);

			ClientCreateCharacterFailed m(msg.getName(), msg.getErrorMessage()); //lint !e1013 !e1055 !e746 (Symbol 'getErrorMessage' not a member of class 'const ConnectionCreateCharacterFailed') // supressed because it IS a member of that class. //lint !e1055 //lint !e746
			client->send(m, true);
		}
		else
		{
			PseudoClientConnection::tryToDeliverMessageTo(static_cast<unsigned int>(msg.getStationId()), static_cast<const GameNetworkMessage &>(message).getByteStream());
		}
	}
	else if (message.isType("NewCharacterCreated"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<StationId> const msg(ri);
		ClientConnection* client = getClientConnection(msg.getValue());
		if (client)
		{
			// don't allow this client to request another character create;
			// this will forced the client to disconnect and reconnect at which time
			// a check will be done (taking the newly created character into account)
			// to see if the client is allowed to create another character on this account
			client->setHasCreatedCharacter(true);
		}
	}
	else if (message.isType("RandomNameResponse"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		RandomNameResponse connMsg(ri);

		ClientConnection* cconn = getClientConnection(connMsg.getStationId());
		ClientRandomNameResponse cnr(connMsg.getCreatureTemplate(), connMsg.getName(), connMsg.getErrorMessage());//lint !e1013 (Symbol 'getErrorMessage' not a member of class 'RandomNameResponse') // supressed because it IS a member of that class.
		if (cconn)
			cconn->send(cnr, true);
	}
	else if (message.isType("VerifyAndLockNameResponse"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		VerifyAndLockNameResponse connMsg(ri);

		ClientConnection* cconn = getClientConnection(connMsg.getStationId());
 		ClientVerifyAndLockNameResponse cvalnr(connMsg.getCharacterName(), connMsg.getErrorMessage());
		if (cconn)
			cconn->send(cvalnr, true);
	}
	else if (message.isType("ChatServerConnectionOpened"))
	{
		ChatServerConnection * c = const_cast<ChatServerConnection *>(static_cast<const ChatServerConnection *>(&source));//lint !e826 suspiscious pointer-to-pointer conversion // suppressed, you bet it is
		IGNORE_RETURN(chatServers.insert(c));
	}
	else if (message.isType("ChatServerConnectionClosed"))
	{
		ChatServerConnection * c = const_cast<ChatServerConnection *>(static_cast<const ChatServerConnection *>(&source));//lint !e826 suspiscious pointer-to-pointer conversion // suppressed, you bet it is
		std::set<ChatServerConnection *>::iterator f = chatServers.find(c);
		if(f != chatServers.end())
		{
			// migrate players that were on this chat server
			// to another chat server

			// now remove the server from the set
			chatServers.erase(f);
			if(!chatServers.empty())
			{
				std::set<ChatServerConnection *>::iterator ic = chatServers.begin();

				const std::set<Client *> & clients = c->getClients();
				std::set<Client *>::const_iterator i;
				for(i = clients.begin(); i != clients.end(); ++ i)
				{
					ChatServerConnection * newConn = (*ic);
					Client * cl = (*i);
					cl->setChatConnection(newConn);
					++ic;
					if(ic == chatServers.end())
						ic = chatServers.begin();
				}
			}
			else
			{
				const std::set<Client *> & clients = c->getClients();
				std::set<Client *>::const_iterator i;
				for(i = clients.begin(); i != clients.end(); ++ i)
				{
					(*i)->setChatConnection(NULL);
				}
			}
		}
	}
	else if (message.isType("CustomerServiceConnectionOpened"))
	{
		CustomerServiceConnection * c = const_cast<CustomerServiceConnection *>(static_cast<const CustomerServiceConnection *>(&source));//lint !e826 suspiscious pointer-to-pointer conversion // suppressed, you bet it is
		IGNORE_RETURN(customerServiceServers.insert(c));
	}
	else if (message.isType("CustomerServiceConnectionClosed"))
	{
		CustomerServiceConnection * c = const_cast<CustomerServiceConnection *>(static_cast<const CustomerServiceConnection *>(&source));//lint !e826 suspiscious pointer-to-pointer conversion // suppressed, you bet it is
		std::set<CustomerServiceConnection *>::iterator f = customerServiceServers.find(c);
		if(f != customerServiceServers.end())
		{
			// migrate players that were on this chat server
			// to another chat server

			// now remove the server from the set
			customerServiceServers.erase(f);
			if(!customerServiceServers.empty())
			{
				std::set<CustomerServiceConnection *>::iterator ic = customerServiceServers.begin();

				const std::set<Client *> & clients = c->getClients();
				std::set<Client *>::const_iterator i;
				for(i = clients.begin(); i != clients.end(); ++ i)
				{
					CustomerServiceConnection * newConn = (*ic);
					Client * cl = (*i);
					cl->setCustomerServiceConnection(newConn);
					++ic;
					if(ic == customerServiceServers.end())
						ic = customerServiceServers.begin();
				}
			}
			else
			{
				const std::set<Client *> & clients = c->getClients();
				std::set<Client *>::const_iterator i;
				for(i = clients.begin(); i != clients.end(); ++ i)
				{
					(*i)->setCustomerServiceConnection(NULL);
				}
			}

		}
	}
	else if (message.isType("GameServerForLoginMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const GameServerForLoginMessage msg(ri);

		ClientConnection* client = getClientConnection(msg.getStationId());
		
		// see if the client is for the same character as the login message.
		// this is required to prevent admin login via the CS Tool from
		// disconnecting other characters logged in from the same account
		// (but not the same character) as the character we're administratively
		// logging in.
		bool clientIsForSameCharacterId = false;
		if(client)
		{
			clientIsForSameCharacterId = client->getCharacterId() == msg.getCharacterId();
		}
		
		bool handledByPseudoClient = false;
		// if the character id in the message doesn't match the character id for
		// the existing client, it may be for a pseudoclient.  Check, and if so,
		// handle there.
		if (!clientIsForSameCharacterId)
		{
			PseudoClientConnection * pcc = PseudoClientConnection::getPseudoClientConnection(msg.getCharacterId());
			// hand off to the PCC only if we do have a pseudoclient, and either we don't have a client, or
			// the pseudoclient has a tool id, telling us that it's for cs tool login.
			if(pcc && ((!client) || (pcc->getTransferCharacterData().getCSToolId() > 0)))
			{
				handledByPseudoClient = true;
				bool result;
				result = PseudoClientConnection::tryToDeliverMessageTo(msg.getStationId(), static_cast<const GameNetworkMessage &>(message).getByteStream());
				UNREF(result);
				DEBUG_REPORT_LOG(! result,("Received GameServerForLoginMessage for %lu, who was not connected.\n",msg.getStationId()));
			}
		}

		if (client && (!handledByPseudoClient))
		{
			static const std::string loginTrace("TRACE_LOGIN");
			LOG(loginTrace, ("GameServerForLoginMessage(%d, %s)", client->getSUID(), client->getCharacterId().getValueString().c_str()));
			client->handleGameServerForLoginMessage(msg.getServer());
		}
	}
	else if (message.isType("ValidateCharacterForLoginReplyMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const ValidateCharacterForLoginReplyMessage msg(ri);

		ClientConnection* cconn = getClientConnection(msg.getSuid());
		if (!cconn)
			DEBUG_REPORT_LOG(true, ("Received ValidateCharacterForLoginReplyMessage for account %lu, which is no longer connected.\n",msg.getSuid()));
		else
		{
			cconn->onCharacterValidated(msg.getApproved(),msg.getCharacterId(), Unicode::wideToNarrow(msg.getCharacterName()), msg.getContainerId(), msg.getScene(), msg.getCoordinates());
		}
	}
	else if (message.isType("ValidateAccountReplyMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const ValidateAccountReplyMessage msg(ri);

		ClientConnection* cconn = getClientConnection(msg.getStationId());
		if (!cconn)
			DEBUG_REPORT_LOG(true, ("Received ValidateAccountReplyMessage for account %lu, which is no longer connected.\n",msg.getStationId()));
		else
		{
			cconn->onIdValidated(msg.getCanLogin(),msg.getCanCreateRegular(),msg.getCanCreateJedi(),msg.getCanSkipTutorial(),msg.getConsumedRewardEvents(),msg.getClaimedRewardItems());
		}
	}
	else if(message.isType("SetConnectionServerPublic"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		SetConnectionServerPublic p(ri);//lint !e40 !e522 !e10 // Undeclared identifier 'SetConnectionServerPublic' // suppressed because it IS declared
		bool statusChanged = false;
		DEBUG_REPORT_LOG(true, ("Conn Server: attempting to chang status\n"));
		if(p.getIsPublic())//lint !e40 !e1013 !e10 // Undeclared identifier 'p' // suppressed because it IS declared
		{
			if(! clientServicePublic)
			{
				statusChanged = true;
			}
		}
		else
		{
			delete clientServicePublic;
			clientServicePublic = 0;
			statusChanged = true;
		}

		if(statusChanged && centralConnection)
		{
			const Service * publicService = getClientServicePublic();
			const Service * privateService = getClientServicePrivate();
			if(publicService || privateService)
			{
				uint16 publicServicePort = 0;
				uint16 privateServicePort = 0;
				if(publicService)
				{
					publicServicePort = publicService->getBindPort();
				}
				if(privateService)
				{
					privateServicePort = privateService->getBindPort();
				}
				const UpdateConnectionServerStatus ucs(publicServicePort, privateServicePort);
				centralConnection->send(ucs, true);
			}
		}
	}//lint !e529 // Symbol 'ri' not subsequently referenced // suppressed because it IS referenced. I think lint is very confused for some reason.
	else if(message.isType("ProfilerOperationMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ProfilerOperationMessage msg(ri);
		unsigned int processId = msg.getProcessId();
		if (!processId)
			Profiler::handleOperation(msg.getOperation().c_str());
	}
	else if (message.isType("ExcommunicateGameServerMessage"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ExcommunicateGameServerMessage msg(ri);

		LOG("GameGameConnect",("Told to drop connection to %lu by Central",msg.getServerId()));
		
		GameConnection *conn =getGameConnection(msg.getServerId());
		if (conn)
			conn->disconnect();
	}		
	else if (message.isType("CntrlSrvDropDupeConns"))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		GenericValueTypeMessage<std::pair<uint32, std::string> > const msg(ri);
		
		ClientConnection* client = getClientConnection(msg.getValue().first);
		if (client && !client->isUsingAdminLogin() && !client->getIsSecure())
		{
			std::string s = "New Connection on galaxy ";
			s += msg.getValue().second;

			dropClient(client, s);
		}
	}
}

//-----------------------------------------------------------------------

void ConnectionServer::install()
{
	s_connectionServer = new ConnectionServer;

	char tmp[128] = {"\0"};
	IGNORE_RETURN(snprintf(tmp, sizeof(tmp), "ConnectionServer:%d", Os::getProcessId())); 
	SetupSharedLog::install(tmp);
	s_connectionServer->setupConnections();
	s_connectionServer->m_metricsData = new ConnectionServerMetricsData;
	MetricsManager::install(s_connectionServer->m_metricsData, true, "ConnectionServer" , "", ConfigConnectionServer::getConnectionServerNumber());
	DataTableManager::install();
	AdminAccountManager::install(ConfigConnectionServer::getAdminAccountDataTable());
}

//-----------------------------------------------------------------------

void ConnectionServer::remove()
{
	MetricsManager::remove();
	delete s_connectionServer->m_metricsData;
	s_connectionServer->m_metricsData = 0;

	// explicitly delete all connections that were setup from setupConnections rather than letting 
	// Connection::remove do it.  There are connections that require a valid s_connectionServer which is deleted
	// and set to NULL.
	s_connectionServer->unsetupConnections();
	
	SetupSharedLog::remove();
	
	delete s_connectionServer;
	s_connectionServer = 0;
}

//-----------------------------------------------------------------------


void ConnectionServer::run(void)
{
	static const bool shouldSleep = ConfigConnectionServer::getShouldSleep();
	static ConnectionServer & cserver = instance();
	DEBUG_FATAL (!cserver.m_metricsData, ("Connection server not installed properly"));

	unsigned long startTime = Clock::timeMs();
	Clock::setFrameRateLimit(50.0f);

	LOG("ServerStartup",("ConnectionServer starting on %s", NetworkHandler::getHostName().c_str()));  
	while (!cserver.done)
	{
		PROFILER_AUTO_BLOCK_DEFINE("main loop");

		bool barrierReached = true;

		do
		{
			{
				PROFILER_AUTO_BLOCK_DEFINE("Os::update");
				if (!Os::update())
					cserver.setDone("Os condition (Parent pid change)");
			}

			{
				PROFILER_AUTO_BLOCK_DEFINE("NetworkHandler::update1");
				NetworkHandler::update();
			}

			{
				GenericValueTypeMessage<std::pair<uint16, uint32> > const syncStampMessage("SetSyncStamp", std::make_pair(static_cast<uint16>(UdpMisc::LocalSyncStampShort()), static_cast<uint32>(UdpMisc::LocalSyncStampLong())));
				GameServerMap::iterator end = cserver.gameServerMap.end();
				for(GameServerMap::iterator iter = cserver.gameServerMap.begin(); iter != end; ++iter)
					iter->second->send(syncStampMessage, true);
			}
			
			{
				PROFILER_AUTO_BLOCK_DEFINE("NetworkHandler::dispatch");
				NetworkHandler::dispatch();
			}

			{
				PROFILER_AUTO_BLOCK_DEFINE("update");
				cserver.update();
			}

			{
				PROFILER_AUTO_BLOCK_DEFINE("NetworkHandler::update2");
				NetworkHandler::update();
			}

			{
				PROFILER_AUTO_BLOCK_DEFINE("MetricsManager::update");
				unsigned long curTime = Clock::timeMs();
				MetricsManager::update(static_cast<float>(curTime - startTime));
				startTime = curTime;
			}


			if (shouldSleep)
			{
				PROFILER_AUTO_BLOCK_DEFINE("Os::sleep");
				Os::sleep(1);
			}

		} while (!barrierReached && !cserver.done);

		NetworkHandler::clearBytesThisFrame();
			
		cserver.updateRecoveringClientList(static_cast<uint>(Clock::frameTime()*1000.0f));
	}
}

// ----------------------------------------------------------------------
/** 
 * Invoked every frame to do whatever updates are needed
 */
void ConnectionServer::update()
{
	if (centralConnection)
	{
		static Timer t(5.0f);
		if(t.updateZero(Clock::frameTime()))
		{
			// Update the population on the central server
			updatePopulationOnCentralServer();

			if (getNumberOfClients() >= ConfigConnectionServer::getMaxClients() - 1)
			{
				//@todo need an Alert here
				WARNING(true, ("We've reached maximum client capacity on a connection server with %d clients", getNumberOfClients()));
			}
		}
	}

	if (m_sessionApiClient)
	{
		m_sessionApiClient->update();
	}

	static const int ping_throttle_max = 1024;

	static char buffer [4];

	for (int throttle = 0; pingSocket->canRecv () && throttle < ping_throttle_max; ++throttle)
	{
		Address addr;
		const uint32 count = pingSocket->recvFrom  (addr, buffer, 4);
		m_pingTrafficNumBytes += static_cast<int>(count);
		if (m_pingTrafficNumBytes < 0)
			m_pingTrafficNumBytes = 0;
		IGNORE_RETURN(pingSocket->sendTo (addr, buffer, count));
	}
}

// ----------------------------------------------------------------------

/**
 * The recovering client list holds players who are trying to recover from a server
 * crash.  If another game server takes authority within a few frames, the player
 * is OK and we don't do anything.  If no game server takes authority for them,
 * we drop them and force them to log in again.
 */
void ConnectionServer::updateRecoveringClientList(uint elapsedTime)
{
	if (!m_recoveringClientList.empty())
	{
		m_recoverTime += elapsedTime;

		static std::set<NetworkId> listPlayersDropped;
		for (RecoveringClientListType::iterator i=m_recoveringClientList.begin(); i!=m_recoveringClientList.end();)
		{
			if (m_recoverTime > i->first)
			{
				Client *client = getClient(i->second);
				if (client)
				{
					if (client->getGameConnection())
						DEBUG_REPORT_LOG(true,("Player %s recovered from a server crash and will not be dropped.\n",i->second.getValueString().c_str()));
					else
					{
						LOG("Network", ("Dropping player %s because game server crashed and no other server took authority.\n",i->second.getValueString().c_str()));
						dropClient(client->getClientConnection(), "Game Server Crash");

						IGNORE_RETURN(listPlayersDropped.insert(i->second));
					}
				}
				i=m_recoveringClientList.erase(i);
			}
			else
				++i;
		}

		if (!listPlayersDropped.empty())
		{
			GenericValueTypeMessage<std::set<NetworkId> > const m("PlayerDroppedFromGameServerCrash", listPlayersDropped);

			// let one of the game server know that we dropped the player(s) due to a game server crash
			GameConnection * const anyGameConnection = getAnyGameConnection();
			if (anyGameConnection)
				anyGameConnection->send(m, true);

			// let CentralServer know that we dropped the player(s) due to a game server crash
			sendToCentralProcess(m);

			listPlayersDropped.clear();
		}

		if (m_recoveringClientList.empty())
			m_recoverTime = 0; // prevent rollover and other problems I don't want to worry about 
	}
}

// ----------------------------------------------------------------------

void ConnectionServer::addRecoveringClient(const NetworkId& networkId)
{
	instance().m_recoveringClientList.push_back(std::pair<int, NetworkId>(static_cast<int>(instance().m_recoverTime) + ConfigConnectionServer::getCrashRecoveryTimeout(), networkId));//lint !e737 !e713 !e1703 !e1025 // going nuts over unsigned/signed conversion and type mismatch on template
}

// ----------------------------------------------------------------------

void ConnectionServer::unsetupConnections()
{
	// remove all ClientConnection objects so when Connection::remove() is called we don't crash since
	// ConnectionServer no longer exists
	while( !instance().connectedMap.empty() )
	{
		SuidMap::iterator i = instance().connectedMap.begin();
		if( i != instance().connectedMap.end() )
		{
			ClientConnection * c = (*i).second;
			uint32 suid = (*i).first;
			IGNORE_RETURN(instance().connectedMap.erase(suid));
			if( c )
			{
				ConnectionServer::dropClient(c, "ConnectionServer shutting down.");
				delete c;
			}
		}
	}

	if( customerService )
	{
		delete customerService;
		customerService = 0;
	}

	if( chatService )
	{
		delete chatService;
		chatService = 0;
	}

	if( centralConnection )
	{
		delete centralConnection;
		centralConnection = 0;
	}

	if( gameService )
	{
		delete gameService;
		gameService = 0;
	}

}
//-----------------------------------------------------------------------

void ConnectionServer::setupConnections()
{
	// set up message connections
	connectToMessage("ConnectionGameServerConnect");
	connectToMessage("SetConnectionServerPublic");

	// set up port to listen for clients, central, and game servers

	connectToMessage("CentralConnectionOpened");
	connectToMessage("CentralConnectionClosed");

	NetworkSetupData setup;
	setup.port = ConfigConnectionServer::getGameServicePort();
	setup.bindInterface = ConfigConnectionServer::getGameServiceBindInterface();
	setup.maxConnections = 100;

	gameService = new Service(ConnectionAllocator<GameConnection>(), setup);
	connectToMessage("GameConnectionOpened");
	connectToMessage("GameConnectionClosed");

	// connect to central server
	centralConnection = new CentralConnection(ConfigConnectionServer::getCentralServerAddress(), ConfigConnectionServer::getCentralServerPort());

	setup.port = 0;
	setup.bindInterface = ConfigConnectionServer::getChatServiceBindInterface();
	chatService = new Service(ConnectionAllocator<ChatServerConnection>(), setup);
	connectToMessage("ChatServerConnectionOpened");
	connectToMessage("ChatServerConnectionClosed");

	setup.bindInterface = ConfigConnectionServer::getCustomerServiceBindInterface();
	customerService = new Service(ConnectionAllocator<CustomerServiceConnection>(), setup);
	connectToMessage("CustomerServiceConnectionOpened");
	connectToMessage("CustomerServiceConnectionClosed");

	connectToMessage("ConnectionServerId");
	connectToMessage("ConnectionKeyPush");
	connectToMessage("LoginKeyPush");
	connectToMessage("CharacterListMessage");
	
	//Create Characters Messages
	connectToMessage("ClientCreateCharacter");
	connectToMessage("ConnectionCreateCharacterSuccess");
	connectToMessage("ConnectionCreateCharacterFailed");
	connectToMessage("NewCharacterCreated");
	connectToMessage("GameServerForLoginMessage");

	// name query messages
	connectToMessage("ClientRandomNameRequest"); // from client
	connectToMessage("RandomNameResponse"); // from game server
	
	connectToMessage("ClientVerifyAndLockNameRequest"); // from client
	connectToMessage("VerifyAndLockNameResponse"); // from game server

	connectToMessage("ValidateCharacterForLoginReplyMessage");
	connectToMessage("ValidateAccountReplyMessage");

	connectToMessage("ProfilerOperationMessage");
	connectToMessage("CentralConnectionClosed");
	connectToMessage("CentralConnectionOpened");
	connectToMessage("ChunkCompleteMessage");
	connectToMessage("FrameEndMessage");
	connectToMessage("GameConnectionClosed");
	connectToMessage("GameGameServerConnect");
	connectToMessage("GameServerReadyMessage");
	connectToMessage("GameServerUniverseLoadedMessage");
	connectToMessage("PersistedPlayerMessage");
	connectToMessage("PreloadListMessage");
	connectToMessage("PreloadRequestCompleteMessage");
	connectToMessage("ProfilerOperationMessage");
	connectToMessage("RequestGameServerForLoginMessage");
	connectToMessage("RequestSceneTransfer");
	connectToMessage("ShutdownMessage");
	connectToMessage("TaskConnectionOpened");
	connectToMessage("UnloadedPlayerMessage");
	connectToMessage("WatcherConnectionClosed");
	connectToMessage("WatcherConnectionOpened");
	connectToMessage("ExcommunicateGameServerMessage");
	connectToMessage("CntrlSrvDropDupeConns");
}

//----------------------------------------------------------------------

uint16 ConnectionServer::getPingPort         ()
{
	static ConnectionServer & cs = instance();
	return cs.pingSocket->getBindAddress ().getHostPort ();
}

//-----------------------------------------------------------------------

Client* ConnectionServer::getClient(const NetworkId & oid)
{
	static ConnectionServer & cs = instance();
	ClientMap::const_iterator iter = cs.clientMap.find(oid);
	if (iter != cs.clientMap.end())
		return (*iter).second;
	return 0;
}

//-----------------------------------------------------------------------

const ConnectionServer::ClientMap & ConnectionServer::getClientMap()
{
	return instance().clientMap;
}

//-----------------------------------------------------------------------

GameConnection* ConnectionServer::getGameConnection(uint32 gameServerId)
{
	static ConnectionServer & cs = instance();
	const GameServerMap::const_iterator i = cs.gameServerMap.find(gameServerId);
	if (i != cs.gameServerMap.end())
		return (*i).second;
	
	return NULL;
}

//-----------------------------------------------------------------------

GameConnection* ConnectionServer::getAnyGameConnection()
{
	static ConnectionServer & cs = instance();
	if (!cs.gameServerMap.empty())
		return cs.gameServerMap.begin()->second;

	return NULL;
}

//-----------------------------------------------------------------------

int ConnectionServer::getPingTrafficNumBytes()
{
	static ConnectionServer & cs = instance();
	return cs.m_pingTrafficNumBytes;
}

//-----------------------------------------------------------------------

int ConnectionServer::getNumberOfClients()
{
	static ConnectionServer & cs = instance();
	return static_cast<int>(cs.connectedMap.size());
}

//-----------------------------------------------------------------------

int ConnectionServer::getNumberOfFreeTrials()
{
	static ConnectionServer & cs = instance();
	return static_cast<int>(cs.freeTrials.size());
}

//-----------------------------------------------------------------------

int ConnectionServer::getNumberOfGameServers()
{
	static ConnectionServer & cs = instance();
	return static_cast<int>(cs.gameServerMap.size());
}

//-----------------------------------------------------------------------

void ConnectionServer::removeConnectedCharacter(uint32 suid)
{
	static ConnectionServer & cs = instance();
	cs.removeFromConnectedMap(suid);
}
//-----------------------------------------------------------------------

void ConnectionServer::sendToCentralProcess(const GameNetworkMessage & msg)
{
	static ConnectionServer & cs = instance();
	if (cs.centralConnection)
		cs.centralConnection->send(msg, true);
	else
		WARNING(true, ("Connection tried to send a message Central, but there is no Central Connection"));
}

//-----------------------------------------------------------------------

CentralConnection * ConnectionServer::getCentralConnection()
{
	static ConnectionServer & cs = instance();
	return cs.centralConnection;
}

//-----------------------------------------------------------------------

void ConnectionServer::installSessionValidation()
{
	int i = 0;
	std::vector<char const *> sessionServers;
	int const numberOfSessionServers = ConfigConnectionServer::getNumberOfSessionServers();
	for (i = 0; i < numberOfSessionServers; ++i)
	{
		char const * const p = ConfigConnectionServer::getSessionServer(i);
		if (p)
		{
			REPORT_LOG(true, ("Using session server %s\n", p));
			sessionServers.push_back(p);
		}
	}

	// if there were none specified, use defaults
	FATAL(i == 0, ("No session servers specified for session API"));
	m_sessionApiClient = new SessionApiClient(&sessionServers[0], i);
}

// ----------------------------------------------------------------------

void ConnectionServer::addToClientMap(const NetworkId &oid, ClientConnection* cconn)
{
	Client * newClient = new Client(cconn, oid);
	NOT_NULL(newClient);

	clientMap[oid] = newClient;

	//associate the ClientConnection object.
	cconn->setClient(newClient);
}

// ----------------------------------------------------------------------

void ConnectionServer::removeFromClientMap(const NetworkId &oid)
{
	ClientMap::iterator i = clientMap.find(oid);
	if (i != clientMap.end())
	{
		clientMap.erase(i);
	}
}

//-----------------------------------------------------------------------

void ConnectionServer::addToConnectedMap(uint32 suid, ClientConnection* cconn)
{
	SuidMap::iterator i = connectedMap.find(suid);
	if (i == connectedMap.end())
	{
		connectedMap[suid] = cconn;
	}
	else
	{
		WARNING_STRICT_FATAL(true, ("Attepting to add duplicate connected chatacter"));
		connectedMap[suid] = cconn;
	}

	if (   ((cconn->getSubscriptionFeatures() & ClientSubscriptionFeature::FreeTrial) != 0)
	    && ((cconn->getSubscriptionFeatures() & ClientSubscriptionFeature::Base)      == 0))
	{
		freeTrials.insert(suid);
	}	

	// Update the population on the CentralServer immediately
	// since we are trying to avoid people "rushing" the server
	updatePopulationOnCentralServer();
}

// ----------------------------------------------------------------------

void ConnectionServer::removeFromConnectedMap(uint32 suid)
{
	SuidMap::iterator i = connectedMap.find(suid);
	if (i != connectedMap.end())
	{
		connectedMap.erase(i);
	}
	
	FreeTrialsSet::iterator j = freeTrials.find(suid);
	if (j != freeTrials.end())
	{
		freeTrials.erase(j);
	}

	// We could update the CentralServer population but people
	// leaving the server are not as important as people connecting
	// and so we will wait for update() to handle things
}

// ----------------------------------------------------------------------

void ConnectionServer::updatePopulationOnCentralServer()
{
	if (centralConnection)
	{
		// Total number of clients and how many of those are free trials
		const int numPlayers   = getNumberOfClients();
		const int numFreeTrial = getNumberOfFreeTrials();

		// We are concerned about too many people piling up at the beginning
		// of the tutorial, so count how many players could be a problem
		int numPlayersEmptyScene    = 0;
		int numPlayersTutorialScene = 0;
		int numPlayersFalconScene   = 0;

		// Walk through the clients and evaluate what scene they are in
		SuidMap::const_iterator i;
		for(i = connectedMap.begin(); i != connectedMap.end(); ++i)
		{
			const ClientConnection * const conn   = (*i).second;
			const Client * const           client = conn->getClient();

			if (client && client->getGameConnection())
			{
				const std::string& scene = client->getGameConnection()->getSceneName();

				if (scene.empty())
				{
					numPlayersEmptyScene += 1;
				}
				else if (scene == SCENE_NAME_TUTORIAL)
				{
					numPlayersTutorialScene += 1;
				}
				else if (scene.substr(0, SCENE_NAME_FALCON_PREFIX.length()) == SCENE_NAME_FALCON_PREFIX)
				{
					numPlayersFalconScene += 1;
				}
			}
			else
			{
				numPlayersEmptyScene += 1;
			}
		}

		const UpdatePlayerCountMessage msg(false, numPlayers, numFreeTrial, numPlayersEmptyScene, numPlayersTutorialScene, numPlayersFalconScene);
		centralConnection->send(msg,true);
	}
}


// ----------------------------------------------------------------------

SessionApiClient* ConnectionServer::getSessionApiClient()
{
	// this is causing crashes when ConnectionServer is shutdown and something calls this function
	// because instance() returns 0.
	if( s_connectionServer )
	{
		return instance().m_sessionApiClient;
	}
	else
	{
		return 0;
	}
}

// ----------------------------------------------------------------------

void ConnectionServer::setDone(char const *reasonfmt, ...)
{
	if (!done)
	{
		char reason[1024];
		va_list ap;
		va_start(ap, reasonfmt);
		IGNORE_RETURN(_vsnprintf(reason, sizeof(reason), reasonfmt, ap));//lint !e530 Symbol 'ap' not initialized
		reason[sizeof(reason)-1] = '\0';

		LOG(
			"ServerShutdown",
			(
				"ConnectionServer (pid %d) shutdown, reason: %s",
				static_cast<int>(Os::getProcessId()),
				reason));

		REPORT_LOG(
			true,
			(
				"ConnectionServer (pid %d) shutdown, reason: %s\n",
				static_cast<int>(Os::getProcessId()),
				reason));

		done = true;
		va_end(ap);
	}
}

// ======================================================================

