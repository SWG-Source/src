// ClientConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstLoginServer.h"
#include "ClientConnection.h"

#include "Archive/ByteStream.h"
#include "ConfigLoginServer.h"
#include "DatabaseConnection.h"
#include "LoginServer.h"
#include "SessionApiClient.h"
#include "serverKeyShare/KeyShare.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ClientLoginMessages.h"
#include "sharedNetworkMessages/DeleteCharacterMessage.h"
#include "sharedNetworkMessages/DeleteCharacterReplyMessage.h"
#include "sharedNetworkMessages/ErrorMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/LoginEnumCluster.h"

#include "webAPI.h"
#include <algorithm>

//-----------------------------------------------------------------------

ClientConnection::ClientConnection(UdpConnectionMT * u, TcpClient * t) :
		ServerConnection(u, t),
		m_clientId(0),
		m_isValidated(false),
		m_isSecure(false),
		m_adminLevel(-1),
		m_stationId(0),
		m_requestedAdminSuid(0),
		m_gameBits(0),
		m_subscriptionBits(0),
		m_waitingForCharacterLoginDeletion(false),
		m_waitingForCharacterClusterDeletion(false)
{
}

//-----------------------------------------------------------------------

ClientConnection::~ClientConnection()
{
}

//-----------------------------------------------------------------------

void ClientConnection::onConnectionClosed()
{
	// client has disconnected
	DEBUG_REPORT_LOG(true, ("Client %lu disconnected\n", getStationId()));
	LOG("LoginClientConnection", ("onConnectionClosed() for stationId (%lu) at IP (%s)", m_stationId, getRemoteAddress().c_str()));
	LoginServer::getInstance().removeClient(m_clientId);
	
	if (!m_isValidated)
	{
		SessionApiClient * session = LoginServer::getInstance().getSessionApiClient();
		if (session)
			session->dropClient(this);
	}
}

//-----------------------------------------------------------------------

void ClientConnection::onConnectionOpened()
{
	m_clientId = LoginServer::getInstance().addClient(*this);
	setOverflowLimit(ConfigLoginServer::getClientOverflowLimit());

	LOG("LoginClientConnection", ("onConnectionOpened() for stationId (%lu) at IP (%s)", m_stationId, getRemoteAddress().c_str()));
}

//-----------------------------------------------------------------------

void ClientConnection::onReceive(const Archive::ByteStream & message)
{
	try
	{
		//Handle all client messages here.  Do not forward out.
		Archive::ReadIterator ri = message.begin();
		GameNetworkMessage m(ri);
		ri = message.begin();
		
		//Validation check
		if (!getIsValidated() && !m.isType("LoginClientId"))
		{
			//Receiving message from unvalidated client.  Pitch it.
			DEBUG_WARNING(true, ("Received %s message from unknown, unvalidated client", m.getCmdName().c_str()));
			return;
		}
		
		if(m.isType("LoginClientId"))
		{
			// send the client the server "now" Epoch time so that the
			// client has an idea of how much difference there is between
			// the client's Epoch time and the server Epoch time
			GenericValueTypeMessage<int32> const serverNowEpochTime(
				"ServerNowEpochTime", static_cast<int32>(::time(nullptr)));
			send(serverNowEpochTime, true);

			LoginClientId id(ri); 
			
			// verify version
#if PRODUCTION == 1

			if(!ConfigLoginServer::getValidateClientVersion() || id.getVersion() == GameNetworkMessage::NetworkVersionId)
			{
				validateClient(id.getId(), id.getKey());
			}
			else
			{
				LOG("CustomerService", ("Login:LoginServer dropping client (stationId=[%lu], ip=[%s], id=[%s], key=[%s], version=[%s]) because of network version mismatch (required version=[%s])", m_stationId, getRemoteAddress().c_str(), id.getId().c_str(), id.getKey().c_str(), id.getVersion().c_str(), GameNetworkMessage::NetworkVersionId.c_str()));
				// disconnect is handled on the client side, as soon as it recieves this message
	#if _DEBUG
				LoginIncorrectClientId incorrectId(GameNetworkMessage::NetworkVersionId, ApplicationVersion::getInternalVersion());
	#else
				LoginIncorrectClientId incorrectId("", "");
	#endif // _DEBUG
				send(incorrectId, true);
			}
			
#else

			validateClient( id.getId(), id.getKey() );
		
#endif // PRODUCTION == 1

		}
		else if ( m.isType( "RequestExtendedClusterInfo" ) )
		{
			LoginServer::getInstance().sendExtendedClusterInfo( *this );
		}
		else if (m.isType("DeleteCharacterMessage"))
		{
			DeleteCharacterMessage msg(ri);
			std::vector<NetworkId>::const_iterator f = std::find(m_charactersPendingDeletion.begin(), m_charactersPendingDeletion.end(), msg.getCharacterId());			
			if ((m_waitingForCharacterLoginDeletion || m_waitingForCharacterClusterDeletion) && f != m_charactersPendingDeletion.end())
			{
				DeleteCharacterReplyMessage reply(DeleteCharacterReplyMessage::rc_ALREADY_IN_PROGRESS);
				send(reply,true);
			}
			else
			{
				if (LoginServer::getInstance().deleteCharacter(msg.getClusterId(), msg.getCharacterId(), getStationId()))
				{
					m_waitingForCharacterLoginDeletion=true;
					m_waitingForCharacterClusterDeletion=true;
					m_charactersPendingDeletion.push_back(msg.getCharacterId());
				}
				else
				{
					DeleteCharacterReplyMessage reply(DeleteCharacterReplyMessage::rc_CLUSTER_DOWN);
					send(reply,true);
				}
			}
		}
	}
	catch(const Archive::ReadException & readException)
	{
		WARNING(true, ("Archive read error (%s) on message from client. Disconnecting client.", readException.what()));
		disconnect();
	}
}

//-----------------------------------------------------------------------
// originally was used to validate station API credentials, now uses our custom api
void ClientConnection::validateClient(const std::string & id, const std::string & key)
{
	// to avoid having to re-type this stupid var all over the place
	// ideally we wouldn't copy this here, but it would be a huge pain
	const std::string trimmedId = trim(id);
	const std::string trimmedKey = trim(key);

	// and to avoid funny business with atoi and casing
	// make it a separate var than the one we send the auth server
	std::string lcaseId;
	lcaseId.resize(trimmedId.size());

  	std::transform(trimmedId.begin(),trimmedId.end(),lcaseId.begin(),::tolower);

	StationId suid = atoi(lcaseId.c_str()); 
	int authOK = 0;

	if (suid == 0)
	{
		std::hash<std::string> h;
		suid = h(lcaseId.c_str()); //lint !e603 // Symbol 'h' not initialized (it's a functor)
	}
	
	LOG("LoginClientConnection", ("validateClient() for stationId (%lu) at IP (%s), id (%s)", m_stationId, getRemoteAddress().c_str(), lcaseId.c_str()));

	std::string authURL(ConfigLoginServer::getExternalAuthUrl());

	if (!authURL.empty()) 
	{
		std::ostringstream postBuf;
		postBuf << "user_name=" << trimmedId << "&user_password=" << trimmedKey << "&stationID=" << suid << "&ip=" << getRemoteAddress();

		std::string response = webAPI::simplePost(authURL, std::string(postBuf.str()), "");

		if (response == "success")
		{
			authOK = 1;
		}
		else
		{
			ErrorMessage err("Login Failed", response);
			this->send(err, true);
		}
		
	}
	else
	{
		authOK = 1;
	}

	if (authOK) 
	{
		LoginServer::getInstance().onValidateClient(suid, lcaseId, this, true, NULL, 0xFFFFFFFF, 0xFFFFFFFF);
	}
	// else this case will never be reached, noop
}

// ----------------------------------------------------------------------------

/**
 * The character has been deleted from the login database.  1/2 of what is
 * required for character deletion.  If the character has already been deleted
 * from the cluster, send the reply message to the client.
 */
void ClientConnection::onCharacterDeletedFromLoginDatabase(const NetworkId & characterId)
{
	m_waitingForCharacterLoginDeletion = false;
	if (!m_waitingForCharacterClusterDeletion)
	{
		std::vector<NetworkId>::iterator f = std::find(m_charactersPendingDeletion.begin(), m_charactersPendingDeletion.end(), characterId);
		if(f != m_charactersPendingDeletion.end())
		{
			m_charactersPendingDeletion.erase(f);
		}
		
		DeleteCharacterReplyMessage reply(DeleteCharacterReplyMessage::rc_OK);
		send(reply,true);
		LOG("CustomerService", ("Player:deleted character %s for stationId %u at IP: %s", characterId.getValueString().c_str(), m_stationId, getRemoteAddress().c_str()));
	}
}

// ----------------------------------------------------------------------

void ClientConnection::onCharacterDeletedFromCluster(const NetworkId & characterId)
{
	m_waitingForCharacterClusterDeletion = false;
	if (!m_waitingForCharacterLoginDeletion)
	{
		std::vector<NetworkId>::iterator f = std::find(m_charactersPendingDeletion.begin(), m_charactersPendingDeletion.end(), characterId);
		if(f != m_charactersPendingDeletion.end())
		{
			m_charactersPendingDeletion.erase(f);
		}
		
		DeleteCharacterReplyMessage reply(DeleteCharacterReplyMessage::rc_OK);
		send(reply,true);
		LOG("CustomerService", ("Player:deleted character %s for stationId %u at IP: %s", characterId.getValueString().c_str(), m_stationId, getRemoteAddress().c_str()));
	}
}

// ----------------------------------------------------------------------

StationId ClientConnection::getRequestedAdminSuid() const
{
	return m_requestedAdminSuid;
}

// ======================================================================

