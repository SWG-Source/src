// -----------------------------------------
// Client.cpp
// copyright 2001 Sony Online Entertainment
// -----------------------------------------

#include "FirstConnectionServer.h"

#include "ChatServerConnection.h"
#include "CustomerServiceConnection.h"
#include "Client.h"
#include "ClientConnection.h"
#include "serverNetworkMessages/ChatConnectAvatar.h"
#include "serverNetworkMessages/ChatDisconnectAvatar.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedLog/Log.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ChatEnterRoomById.h"
#include "sharedNetworkMessages/ChatServerStatus.h"
#include "sharedNetworkMessages/DisconnectPlayerMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

//----------------------------------------------------------------------

Client::Client(ClientConnection * cconn, const NetworkId& oid) :
Receiver(),
m_chatConnection(0),
m_customerServiceConnection(0),
m_clientConnection(cconn),
m_deferredChatMessages(),
m_hasBeenKicked(false),
m_oid(oid),
m_gameConnection(0),
//m_roomCache(),
m_sceneName(),
m_skipLoadScreen(false),
m_callback(new MessageDispatch::Callback)
{
	static const std::string loginTrace("TRACE_LOGIN");
	LOG(loginTrace, ("new Client(%s)", m_oid.getValueString().c_str()));
	
	// until/unless a chat server connection is set, the client will
	// listen for a new chat server coming online.
	// This handles the case where a client connects and there
	// are no chat servers currently running (e.g. a crash happens
	// at the point in time a client is connecting)
	connectToMessage("ChatServerConnectionOpened"); 
	connectToMessage("CustomerServiceConnectionOpened"); 
	m_callback->connect(*this, &Client::onChatConnectionClosed);
}

//----------------------------------------------------------------------

Client::~Client()
{
	ConnectionServer::dropClient(m_clientConnection, "Destroying Client Object");
	if(m_chatConnection)
	{
		m_chatConnection->removeClient(this);
		m_chatConnection = 0;
	}

	if(m_customerServiceConnection)
	{
		std::vector<NetworkId> v;
		v.clear();
		v.push_back(getNetworkId());
		DisconnectPlayerMessage message;
		GameClientMessage gcm(v, true, message);
		m_customerServiceConnection->send(gcm, true);

		m_customerServiceConnection->removeClient(this);
		m_customerServiceConnection = 0;
	}
	
	if (m_oid != NetworkId::cms_invalid && ConnectionServer::getClient(m_oid))
	{
		WARNING_STRICT_FATAL(true, ("Attempting to delete client %d without removing him from map\n", m_oid.getValueString().c_str()));
	}
	m_clientConnection = 0;

	setGameConnection(0);
	m_gameConnection = 0;
	delete m_callback;
}

//-----------------------------------------------------------------------

void Client::onChatConnectionClosed(Connection * closedConnection)
{
	if(m_chatConnection == closedConnection)
	{
		m_chatConnection->removeClient(this);
		m_chatConnection = 0;
	}
}

//-----------------------------------------------------------------------

void Client::deferChatMessage(const Archive::ByteStream & message)
{
	m_deferredChatMessages.push_back(message);
}

//-----------------------------------------------------------------------
/*
void Client::enterRoom(const unsigned int roomId)
{
	IGNORE_RETURN ( m_roomCache.insert(roomId) );
}
*/

//-----------------------------------------------------------------------

void Client::flushChatMessages() 
{
	if(m_chatConnection)
	{
		std::vector<Archive::ByteStream>::const_iterator i;
		for(i = m_deferredChatMessages.begin(); i != m_deferredChatMessages.end(); ++i)
		{
			m_chatConnection->Connection::send((*i), true);
		}
		m_deferredChatMessages.clear();
	}
}

//------------------------------------------------------------

void Client::handleTransfer(const std::string & sceneName, GameConnection* conn)
{
	NOT_NULL(conn);
	//Set the new scene name
	setSceneName(sceneName);
	
	//If we are already connected to a game server drop them
	if (getGameConnection() && getGameConnection() != conn)
	{
		DropClient msg(getNetworkId());
		getGameConnection()->send(msg, true);
	}
	//Set the new game server
	setGameConnection(conn);
	m_skipLoadScreen = false;
}

//-----------------------------------------------------------------------
/*
void Client::leaveRoom(const unsigned int roomId)
{
	std::set<unsigned int>::iterator f = m_roomCache.find(roomId);
	if(f != m_roomCache.end())
		m_roomCache.erase(f);
}
*/
//------------------------------------------------------------

void Client::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	if(message.isType("GameConnectionClosed"))
	{
		// Game server has crashed.  With luck, we'll get transferred to a new server shortly
		// So, clear our connection and put us on a queue.  If we don't get transferred in a
		// reasonable about of time, we'll be dropped.
		
		m_gameConnection = 0;
		ConnectionServer::addRecoveringClient(getNetworkId());
	}
	else if(message.isType("ChatServerConnectionOpened"))
	{
		const ChatServerConnection & chatConnection = static_cast<const ChatServerConnection &>(source);
		setChatConnection(const_cast<ChatServerConnection*>(&chatConnection));
	}
	else if(message.isType("CustomerServiceConnectionOpened"))
	{
		const CustomerServiceConnection & customerServiceConnection = static_cast<const CustomerServiceConnection &>(source);
		setCustomerServiceConnection(const_cast<CustomerServiceConnection*>(&customerServiceConnection));
	}
}

//-----------------------------------------------------------------------

void Client::setChatConnection(ChatServerConnection * c)
{
	if(m_chatConnection)
	{
		disconnectFromEmitter(*m_chatConnection, "ChatServerConnectionClosed");
		// advise the old chat server that this client is disconnecting
		// from it
		NOT_NULL(m_clientConnection);
		ChatDisconnectAvatar m(m_clientConnection->getCharacterId());
		m_chatConnection->send(m, true);
	}
	else
	{
			disconnectFromMessage("ChatServerConnectionOpened");
	}
	
	m_chatConnection = c;

	if(c && m_clientConnection)
	{
		// advise the new chat server that the client is connecting
		connectToEmitter(*m_chatConnection, "ChatServerConnectionClosed");
		ChatConnectAvatar connectAvatar(m_clientConnection->getCharacterName(), m_clientConnection->getCharacterId(), m_clientConnection->getSUID(), m_clientConnection->getIsSecure(), ((m_clientConnection->getSubscriptionFeatures() & ClientSubscriptionFeature::Base) != 0));
		m_chatConnection->send(connectAvatar, true);
		m_chatConnection->addClient(this);

		/*
		std::vector<NetworkId> id;
		id.push_back(getNetworkId());
		std::set<unsigned int>::const_iterator i;
		for(i = m_roomCache.begin(); i != m_roomCache.end(); ++i)
		{
			ChatEnterRoomById chat(0, (*i));
			GameClientMessage gcm(id, true, chat);
			c->send(gcm, true);
		}
		m_roomCache.clear();
		*/

		flushChatMessages();

		ChatServerStatus status(true);
		m_clientConnection->send(status, true);
	}
	else
	{
		connectToMessage("ChatServerConnectionOpened");
		ChatServerStatus status(false);
		NOT_NULL(m_clientConnection);
		m_clientConnection->send(status, false);
	}
}

//-----------------------------------------------------------------------

void Client::setCustomerServiceConnection(CustomerServiceConnection * c)
{
	if(m_customerServiceConnection)
	{
		disconnectFromEmitter(*m_customerServiceConnection, "CustomerServiceConnectionClosed");
		NOT_NULL(m_clientConnection);
	}
	else
	{
		disconnectFromMessage("CustomerServiceConnectionOpened");
	}
	
	m_customerServiceConnection = c;

	if(c && m_clientConnection)
	{
		connectToEmitter(*m_customerServiceConnection, "CustomerServiceConnectionClosed");
		m_customerServiceConnection->addClient(this);
	}
	else
	{
		connectToMessage("CustomerServiceConnectionOpened");
	}
}

//------------------------------------------------------------

void Client::setGameConnection(GameConnection* conn)
{
	if (m_gameConnection != conn)
	{
		if (m_gameConnection)
		{
			disconnectFromEmitter(*m_gameConnection, "GameConnectionClosed");
		}
		if (conn)
		{
			connectToEmitter(*conn, "GameConnectionClosed");
		}
		m_gameConnection = conn;
	}
}

//------------------------------------------------------------

void Client::kick(const std::string& reason) 
{
	ClientConnection *c = getClientConnection();
	ConnectionServer::dropClient(c, reason);

	m_hasBeenKicked = true;
	if (getClientConnection())
	{
		LOG("CustomerService", ("Login:%s Dropped Reason: %s. Character: %s (%s). Play time: %s. Active play time: %s", ClientConnection::describeAccount(c).c_str(), reason.c_str(), getClientConnection()->getCharacterName().c_str(), getClientConnection()->getCharacterId().getValueString().c_str(), getClientConnection()->getPlayTimeDuration().c_str(), getClientConnection()->getActivePlayTimeDuration().c_str()));
	}
	else
	{
		LOG("CustomerService", ("Login: Account unknown (lost client connection) id %s Dropped Reason: %s. Character: <unknown>. Play time: <unknown>. Active play time: <unknown>", m_oid.getValueString().c_str(), reason.c_str()));
	}
}

//------------------------------------------------------------

bool Client::getSkipLoadScreen() const
{
	return m_skipLoadScreen;
}

//------------------------------------------------------------

void Client::skipLoadScreen()
{
	m_skipLoadScreen = true;
}

//----------------------------------------------------------------------
