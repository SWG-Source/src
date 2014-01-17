// -----------------------------------------
// Client.h
// copyright 2001 Sony Online Entertainment
// -----------------------------------------


#ifndef _Included_Client_H_
#define _Included_Client_H_

#include "ChatServerConnection.h"
#include "ClientConnection.h"
#include "CustomerServiceConnection.h"
#include "GameConnection.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Watcher.h"
#include "sharedMessageDispatch/Receiver.h"
#include <string>


class ChatServerConnection;
class CustomerServiceConnection;
class ClientConnection;
class GameConnection;

namespace MessageDispatch
{
	class Callback;
}

/**
 * class Client represents one actual player logged into the cluster.
 * it knows what client connection they are on, as well as their oids and
 * game server mappings
 */

//----------------------------------------------------------------------

class Client :  public MessageDispatch::Receiver
{
public:
	
	Client(ClientConnection* cconn, const NetworkId& oid); //ControlAssumed message should set the rest, GameConnection* gconn, std::string sceneName);
	~Client();

	void                         deferChatMessage     (const Archive::ByteStream & message);
//	void                         enterRoom            (const unsigned int roomId);
	void                         flushChatMessages    ();
	ClientConnection*            getClientConnection  ();
	const NetworkId&             getNetworkId         () const;
	GameConnection*              getGameConnection    () const;
	ChatServerConnection*        getChatConnection    () const;
	CustomerServiceConnection*   getCustomerServiceConnection () const;
	const std::string&           getSceneName         () const;
	bool                         getSkipLoadScreen    () const;
	bool                         hasBeenKicked() const;
	void                         skipLoadScreen       ();
	void                         handleTransfer       (const std::string & sceneName, GameConnection* conn);
	void                         kick                 (const std::string& reason);
//	void                         leaveRoom            (const unsigned int roomId);
	void                         receiveMessage       (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	void                         setChatConnection    (ChatServerConnection * chatServerConnection);	
	void                         setCustomerServiceConnection (CustomerServiceConnection * customerServiceConnection);	

private:
	Client();
	Client(const Client&);
	Client & operator= (const Client&);
	void                    setGameConnection(GameConnection* conn);
	void                    setSceneName(const std::string &name);
	void                    onChatConnectionClosed(Connection *);
	
private:
	//@todo...right now there is a one to one mapping between oid and client.
	//this may not always be the case.
	Watcher<ChatServerConnection>      m_chatConnection;
	Watcher<CustomerServiceConnection> m_customerServiceConnection;
	ClientConnection*                  m_clientConnection;
	std::vector<Archive::ByteStream>   m_deferredChatMessages;
	bool                               m_hasBeenKicked;
	NetworkId                          m_oid;
	GameConnection*                    m_gameConnection;
//	std::set<unsigned int>             m_roomCache;
	std::string                        m_sceneName;
	bool                               m_skipLoadScreen;
	MessageDispatch::Callback *        m_callback;
};

//-----------------------------------------------------------------------

inline ChatServerConnection * Client::getChatConnection() const
{
	return m_chatConnection;
}

//-----------------------------------------------------------------------

inline CustomerServiceConnection * Client::getCustomerServiceConnection() const
{
	return m_customerServiceConnection;
}

//------------------------------------------------------------

inline ClientConnection* Client::getClientConnection()
{
	return m_clientConnection;
}

//------------------------------------------------------------

inline const NetworkId& Client::getNetworkId() const
{
	return m_oid;
}

//------------------------------------------------------------

inline GameConnection* Client::getGameConnection() const
{
	return m_gameConnection;
}

//------------------------------------------------------------

inline const std::string& Client::getSceneName() const
{
	return m_sceneName;
}

//------------------------------------------------------------

inline bool Client::hasBeenKicked() const
{
	return m_hasBeenKicked;
}


//------------------------------------------------------------

inline void Client::setSceneName(const std::string & name)
{
	m_sceneName = name;
}

//------------------------------------------------------------
#endif
