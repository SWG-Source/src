// ClientConnection.cpp
// copyright 2001 Verant Interactive

//-----------------------------------------------------------------------

#include "FirstConnectionServer.h"
#include "ClientConnection.h"

#include "Archive/ByteStream.h"
#include "ChatServerConnection.h"
#include "CustomerServiceConnection.h"
#include "ConfigConnectionServer.h"
#include "ConnectionServer.h"
#include "GameConnection.h"
#include "SessionApiClient.h"
#include "UnicodeUtils.h"
#include "serverKeyShare/KeyShare.h"
#include "serverNetworkMessages/CentralConnectionServerMessages.h"
#include "serverNetworkMessages/ChatDisconnectAvatar.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "serverNetworkMessages/RandomName.h"
#include "serverNetworkMessages/RequestGameServerForLoginMessage.h"
#include "serverNetworkMessages/ValidateAccountMessage.h"
#include "serverNetworkMessages/ValidateCharacterForLoginMessage.h"
#include "serverNetworkMessages/VerifyAndLockName.h"
#include "serverUtility/AdminAccountManager.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/Connection.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "sharedNetworkMessages/ClientPermissionsMessage.h"
#include "sharedNetworkMessages/AppendCommentMessage.h"
#include "sharedNetworkMessages/CancelTicketMessage.h"
#include "sharedNetworkMessages/ChatEnterRoom.h"
#include "sharedNetworkMessages/ChatEnterRoomById.h"
#include "sharedNetworkMessages/ChatEnum.h"
#include "sharedNetworkMessages/ChatOnEnteredRoom.h"
#include "sharedNetworkMessages/ChatPersistentMessageToServer.h"
#include "sharedNetworkMessages/ChatQueryRoom.h"
#include "sharedNetworkMessages/ConnectPlayerMessage.h"
#include "sharedNetworkMessages/CreateTicketMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/GetTicketsMessage.h"
#include "sharedNetworkMessages/NewTicketActivityMessage.h"
#include "sharedNetworkMessages/ErrorMessage.h"
#include "sharedNetworkMessages/HeartBeat.h"
#include "Session/LoginAPI/Client.h"
#include "UdpLibrary.h"

#include <algorithm>

//-----------------------------------------------------------------------

namespace ClientConnectionNamespace
{
	unsigned long gs_receiveDelayMaxMs = 16384;
}

using namespace ClientConnectionNamespace;




//-----------------------------------------------------------------------


std::map< std::string, uint32 > ClientConnection::sm_outgoingBytesMap_Working;  // working stats that will rotate after 1 minute
std::map< std::string, uint32 > ClientConnection::sm_outgoingBytesMap_Stats;    // computed stats from the last minute
uint32                          ClientConnection::sm_outgoingBytesMap_Worktime = 0 ; // time we started filling in the working map



//-----------------------------------------------------------------------

ClientConnection::ClientConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t),
m_accountName(""),
m_canCreateRegularCharacter(false),
m_canCreateJediCharacter(false),
m_hasRequestedCharacterCreate(false),
m_hasCreatedCharacter(false),
m_pendingCharacterCreate(NULL),
m_canSkipTutorial(false),
m_characterId(NetworkId::cms_invalid),
m_characterName(),
m_startPlayTime(0),
m_lastActiveTime(0),
m_activePlayTimeDuration(0),
m_client(0),
m_containerId(NetworkId::cms_invalid),
m_featureBitsGame(0),
m_featureBitsSubscription(0),
m_hasBeenSentToGameServer(false),
m_hasBeenValidated(false),
m_hasSelectedCharacter(false),
m_isSecure(false),
m_isAdminAccount(false),
m_hasCSLoggedAccountFeatureIds(false),
m_suid(0),
m_requestedSuid(0),
m_usingAdminLogin(false),
m_targetCoordinates(),
m_targetScene(""),
m_validatingCharacter(false),
m_receiveHistoryBytes(0),
m_receiveHistoryPackets(0),
m_receiveHistoryMs(0),
m_receiveLastTimeMs(0),
m_sendLastTimeMs(0),
m_sessionId(""),
m_sessionValidated(false),
m_connectionServerLag(0),
m_gameServerLag(0),
m_countSpamLimitResetTime(0),
m_entitlementTotalTime(0),
m_entitlementEntitledTime(0),
m_entitlementTotalTimeSinceLastLogin(0),
m_entitlementEntitledTimeSinceLastLogin(0),
m_buddyPoints(0),
m_sendToStarport(false),
m_pendingChatEnterRoomRequests(),
m_pendingChatQueryRoomRequests()
{
	static const std::string loginTrace("TRACE_LOGIN");
	LOG(loginTrace, ("new ClientConnection"));

	setNoDataTimeout(600000);
}

//-----------------------------------------------------------------------

ClientConnection::~ClientConnection()
{
	bool hasBeenKicked = false;

	if (ConnectionServer::getClientConnection(m_suid) == this)
	{
		ConnectionServer::removeConnectedCharacter(m_suid);
	}
	if (m_client)
	{
		hasBeenKicked = m_client->hasBeenKicked();
		delete m_client;
		m_client = NULL;
	}

	// tell Session to stop recording play time for the character
	if (m_hasBeenValidated && m_sessionValidated && ConnectionServer::getSessionApiClient() && (m_lastActiveTime > 0) && ConfigConnectionServer::getSessionRecordPlayTime())
	{
		LOG("CustomerService", ("Login:%s calling SessionStopPlay() for %s/%s/%s (%s). Active play time: %s", ClientConnection::describeAccount(this).c_str(), this->getSessionId().c_str(), ConfigConnectionServer::getClusterName(), this->getCharacterName().c_str(), this->getCharacterId().getValueString().c_str(), this->getCurrentActivePlayTimeDuration().c_str()));

		// log total active play time for the session to the balance log
		LOG("GameBalance", ("balancelog:%s calling SessionStopPlay() for %s/%s/%s (%s). Active play time: %s", ClientConnection::describeAccount(this).c_str(), this->getSessionId().c_str(), ConfigConnectionServer::getClusterName(), this->getCharacterName().c_str(), this->getCharacterId().getValueString().c_str(), this->getActivePlayTimeDuration().c_str()));

		ConnectionServer::getSessionApiClient()->stopPlay(*this);
	}

	if (ConnectionServer::getSessionApiClient())
	{
		ConnectionServer::getSessionApiClient()->dropClient(this, hasBeenKicked);
	}

	std::map<unsigned long, GameClientMessage*>::const_iterator iter;
	for (iter = m_pendingChatEnterRoomRequests.begin(); iter !=  m_pendingChatEnterRoomRequests.end(); ++iter)
	{
		delete iter->second;
	}
	m_pendingChatEnterRoomRequests.clear();

	for (iter = m_pendingChatQueryRoomRequests.begin(); iter !=  m_pendingChatQueryRoomRequests.end(); ++iter)
	{
		delete iter->second;
	}
	m_pendingChatQueryRoomRequests.clear();

	delete m_pendingCharacterCreate;
	m_pendingCharacterCreate = NULL;
}


//-----------------------------------------------------------------------

const NetworkId & ClientConnection::getCharacterId() const
{
	return m_characterId;
}

//-----------------------------------------------------------------------

const std::string & ClientConnection::getCharacterName() const
{
	return m_characterName;
}

//-----------------------------------------------------------------------

std::string ClientConnection::getPlayTimeDuration() const
{
	int playTimeDuration = 0;

	if (m_startPlayTime > 0)
		playTimeDuration = static_cast<int>(::time(NULL) - m_startPlayTime);

	return CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(playTimeDuration));
}

//-----------------------------------------------------------------------

std::string ClientConnection::getActivePlayTimeDuration() const
{
	int activePlayTimeDuration = static_cast<int>(m_activePlayTimeDuration);

	if (m_lastActiveTime > 0)
		activePlayTimeDuration += static_cast<int>(::time(NULL) - m_lastActiveTime);

	return CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(activePlayTimeDuration));
}

//-----------------------------------------------------------------------

std::string ClientConnection::getCurrentActivePlayTimeDuration() const
{
	int activePlayTimeDuration = 0;

	if (m_lastActiveTime > 0)
		activePlayTimeDuration = static_cast<int>(::time(NULL) - m_lastActiveTime);

	return CalendarTime::convertSecondsToHMS(static_cast<unsigned int>(activePlayTimeDuration));
}

//-----------------------------------------------------------------------

void ClientConnection::sendPlayTimeInfoToGameServer() const
{
	if (m_client && m_client->getGameConnection())
	{
		// update the game server with play time info
		GenericValueTypeMessage<std::pair<int32, std::pair<int32, unsigned long> > > const msgPlayTimeInfo(
		    "UpdateSessionPlayTimeInfo",
		    std::make_pair(static_cast<int32>(m_startPlayTime),
		        std::make_pair(static_cast<int32>(m_lastActiveTime), m_activePlayTimeDuration)
		    )
		);

		std::vector<NetworkId> v;
		v.push_back(m_client->getNetworkId());
		GameClientMessage const gcm(v, true, msgPlayTimeInfo);
		m_client->getGameConnection()->send(gcm, true);
	}
}

// ----------------------------------------------------------------------

void ClientConnection::handleSelectCharacterMessage(const SelectCharacter& msg)
{
	//Only accept this message from clients who have been validated and
	//haven't already selected.
	if (m_hasSelectedCharacter || m_validatingCharacter || !m_hasBeenValidated || !m_sessionValidated)
	{
		if(m_hasSelectedCharacter)
		{
			LOG("TraceCharacterSelection", ("%d cannot select a character because the client has already selected a character", getSUID()));
		}
		if(m_validatingCharacter)
		{
			LOG("TraceCharacterSelection", ("%d cannot select a character because the client has not yet received validation", getSUID()));
		}
		if(!m_hasBeenValidated)
		{
			LOG("TraceCharacterSelection", ("%d cannot select a character because the client has not been validated", getSUID()));
		}
		if (!m_sessionValidated)
		{
			LOG("TraceCharacterSelection", ("%d cannot select a character because the client has not been session validated", getSUID()));
		}

		return;
	}

	m_validatingCharacter = true;

	// The client is picking a character from the list the Login Server gave him.
	// But we don't trust him not to cheat, so we double-check that he really
	// owns the character he selected.

	ValidateCharacterForLoginMessage vclm(getSUID(), msg.getId());
	ConnectionServer::sendToCentralProcess(vclm);
	LOG("TraceCharacterSelection", ("%d selected %s for login. Sending a validation request to CentralServer to verify this client can use this character", getSUID(), msg.getId().getValueString().c_str()));

}

// ----------------------------------------------------------------------

/**
 * We got a message telling us what game server to use for the player (who
 * is in the process of logging in).
 * Connect the player with the game server.
 */
void ClientConnection::handleGameServerForLoginMessage(uint32 serverId)
{
	DEBUG_WARNING(serverId==0,("Got handleGameServerForLoginMessage with serverId=0.\n"));
	IGNORE_RETURN( sendToGameServer(serverId) );
}

//----------------------------------------------------------------------

/**
 * Examine the ID the client sends us.  If it looks OK, send it to Central
 * to be validated.  Central will reply with a list of permissions.
 * @see onIdValidated
 */
void ClientConnection::handleClientIdMessage(const ClientIdMsg& msg)
{
	//Only check clients that have not been validated.
	if (m_hasBeenValidated)
		return;

	DEBUG_FATAL(m_hasSelectedCharacter, ("Trying to validate a client who already has a character selected.\n"));
	bool result = false;
	char sessionId[apiSessionIdWidth];

	m_gameBitsToClear = msg.getGameBitsToClear();

	if(msg.getTokenSize() > 0)
	{
		Archive::ByteStream t(msg.getToken(), msg.getTokenSize());
		Archive::ReadIterator ri(t);
		KeyShare::Token	token(ri);

		if (!ConfigConnectionServer::getValidateStationKey())
		{
			// get SUID from token
			result = ConnectionServer::decryptToken(token, m_suid, m_isSecure, m_accountName);
		}
		else
		{
			result = ConnectionServer::decryptToken(token, sessionId, m_requestedSuid);
		}

		static const std::string loginTrace("TRACE_LOGIN");
		LOG(loginTrace, ("ClientConnection SUID = %d", m_suid));

	}
	if (result)
	{
		//check for duplicate login
		ClientConnection * oldConnection = ConnectionServer::getClientConnection(m_suid);
		if (oldConnection)
		{
			//There is already someone connected to this cluster with this suid.
			LOG("Network", ("SUID %d already logged in, disconnecting client.\n", m_suid));

			ConnectionServer::dropClient(oldConnection, "Already Connected");

			disconnect();
			return;
		}

		// verify version
		if (ConfigConnectionServer::getValidateClientVersion() && msg.getVersion() != GameNetworkMessage::NetworkVersionId)
		{
			std::string strSessionId(sessionId, apiSessionIdWidth);
			strSessionId += '\0';

			const int bufferSize = 255 + apiSessionIdWidth;
			char * buffer = new char[bufferSize];
			snprintf(buffer, bufferSize-1, "network version mismatch: got (ip=[%s], sessionId=[%s], version=[%s]), required (version=[%s])", getRemoteAddress().c_str(), strSessionId.c_str(), msg.getVersion().c_str(), GameNetworkMessage::NetworkVersionId.c_str());
			buffer[bufferSize-1] = '\0';

			ConnectionServer::dropClient(this, std::string(buffer));
			disconnect();

			delete[] buffer;

			return;
		}

		if (ConfigConnectionServer::getValidateStationKey())
		{
			SessionApiClient * session = ConnectionServer::getSessionApiClient();
			NOT_NULL(session);
			if(session)
			{
				session->validateClient(this, sessionId);
			}
			else
			{
				ConnectionServer::dropClient(this, "SessionApiClient is not available!");
				disconnect();
			}
		}
		else
		{
			m_suid = atoi(m_accountName.c_str());
			if (m_suid == 0)
			{
				std::hash<std::string> h;
				m_suid = h(m_accountName.c_str());
			}
			onValidateClient(m_suid, m_accountName, m_isSecure, NULL, ConfigConnectionServer::getDefaultGameFeatures(), ConfigConnectionServer::getDefaultSubscriptionFeatures(), 0, 0, 0, 0, ConfigConnectionServer::getFakeBuddyPoints());
		}
	}
	else
	{
		// They sent us a token that was no good -- either a hack attempt, or
		// possibly it was just too old.
		LOG("ClientDisconnect", ("SUID %d passed a bad token to the connections erver. Disconnecting.", m_suid));
		disconnect();
	}
}

//-----------------------------------------------------------------------

void ClientConnection::onIdValidated(bool canLogin, bool canCreateRegularCharacter, bool canCreateJediCharacter, bool canSkipTutorial, std::vector<std::pair<NetworkId, std::string> > const & consumedRewardEvents, std::vector<std::pair<NetworkId, std::string> > const & claimedRewardItems)
{
	//@todo start session with station.
	//@todo add more permissions to this message as needed.

	// resume character creation
	if (m_pendingCharacterCreate)
	{
		if (!m_pendingCharacterCreate->getUseNewbieTutorial() && !canSkipTutorial)
		{
			LOG("TraceCharacterCreation", ("%d failed character creation. The client is not allowed to skip the tutorial", getSUID()));
			// This is probably a hack attempt, because the Client was already told they couldn't skip the tutorial
			LOG("ClientDisconnect", ("Disconnecting %u because they tried to skip the tutorial without permission.\n",getSUID()));
			disconnect();
		}
		else if (m_pendingCharacterCreate->getJedi() && !canCreateJediCharacter)
		{
			LOG("TraceCharacterCreation", ("%d failed character creation. The request character type was Jedi, but this client cannot create a Jedi character", getSUID()));
			// This is probably a hack attempt, because the Client was already told they couldn't create a character
			LOG("ClientDisconnect", ("Disconnecting %u because they tried to create a Jedi character without permission.\n",getSUID()));
			disconnect();
		}
		else if (!m_pendingCharacterCreate->getJedi() && !canCreateRegularCharacter)
		{
			LOG("TraceCharacterCreation", ("%d failed character creation. The client is not allowed to create any characters", getSUID()));
			// This is probably a hack attempt, because the Client was already told they couldn't create a character
			LOG("ClientDisconnect", ("Disconnecting %u because they tried to create a regular character without permission.\n",getSUID()));
			disconnect();
		}
		else
		{
			ConnectionServer::sendToCentralProcess(*m_pendingCharacterCreate);
			LOG("TraceCharacterCreation", ("%d character creation request sent to CentralServer", getSUID()));

			m_hasRequestedCharacterCreate = true;
		}

		delete m_pendingCharacterCreate;
		m_pendingCharacterCreate = NULL;

		return;
	}

	// Save lists of claimed rewards, which won't be used again until later in the login sequence
	m_consumedRewardEvents = consumedRewardEvents;
	m_claimedRewardItems = claimedRewardItems;


	int level=0;
	if (AdminAccountManager::isAdminAccount(Unicode::toLower(getAccountName()),level) && (level !=0)) // Note:  not checking IP, so that owners of god accounts can create characters to play from home without having to erase the characters they use for work
	{
		canLogin = true;
		canCreateRegularCharacter = true;
		canSkipTutorial = true;
		m_isAdminAccount = true;
	}

	ClientPermissionsMessage c(canLogin, canCreateRegularCharacter, canCreateJediCharacter, canSkipTutorial);
	send(c, true);

	DEBUG_REPORT_LOG(true,("Permissions for %lu:\n",getSUID()));
	DEBUG_REPORT_LOG(canLogin,("\tcanLogin\n"));
	DEBUG_REPORT_LOG(canCreateRegularCharacter,("\tcanCreateRegularCharacter\n"));
	DEBUG_REPORT_LOG(canCreateJediCharacter,("\tcanCreateJediCharacter\n"));
	DEBUG_REPORT_LOG(canSkipTutorial,("\tcanSkipTutorial\n"));
	DEBUG_REPORT_LOG(!(canLogin || canCreateRegularCharacter || canCreateJediCharacter || canSkipTutorial),("\tnone\n"));

	if (canLogin)
	{
		m_hasBeenValidated = true;
		m_canCreateRegularCharacter = canCreateRegularCharacter;
		m_canCreateJediCharacter = canCreateJediCharacter;
		m_canSkipTutorial = canSkipTutorial;
	}
	else
	{
		LOG("TRACE_LOGIN", ("%d does not have permissions to log in", getSUID()));
		LOG("ClientDisconnect", ("Client (SUID %u) does not have permissions to log in. Disconnecting.", getSUID()));
		disconnect();
	}
}

//-----------------------------------------------------------------------

void ClientConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();
	static MessageConnectionCallback m("ClientConnectionClosed");
	emitMessage(m);

	LOG("TRACE_LOGIN", ("%d closed connection", getSUID()));
	if (m_client)
	{
		if (!m_client->hasBeenKicked())
		{
			LOG("CustomerService", ("Login:%s Dropped Reason: Client Dropped Connection. Character: %s (%s). Play time: %s. Active play time: %s", describeAccount(this).c_str(), getCharacterName().c_str(), getCharacterId().getValueString().c_str(), getPlayTimeDuration().c_str(), getActivePlayTimeDuration().c_str()));
		}
		ChatServerConnection * chatConnection = m_client->getChatConnection();
		if(chatConnection)
		{
			ChatDisconnectAvatar m(m_characterId);
			chatConnection->send(m, true);
		}
		// We cannot do this here, as this connection will be deleted on
		// return from this function already.
		//m_client->kick();
	}
}

//-----------------------------------------------------------------------

void ClientConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();
	static MessageConnectionCallback m("ClientConnectionOpened");
	emitMessage(m);
	setOverflowLimit(ConfigConnectionServer::getClientOverflowLimit());
}

//-----------------------------------------------------------------------

void ClientConnection::onConnectionOverflowing  (const unsigned int bytesPending)
{
	char errbuf[1024];
	snprintf(errbuf, sizeof(errbuf), "Connection overflow from server to client, %d bytes. Disconnected.", bytesPending);
	std::string name("Connection overflow server->client");
	std::string desc(errbuf);
	LOG("Network", ("Disconnect: Client connection overflowing. %d bytes pending", bytesPending));

	std::vector<std::pair<std::string, int> >::const_iterator i;
	for(i = m_pendingPackets.begin(); i != m_pendingPackets.end(); ++i)
	{
		LOG("Network", ("Overflow packets this frame: [%s] %d bytes", i->first.c_str(), i->second));
	}

//	ErrorMessage err(name, desc, false);
//	send(err, true);
	WARNING(true, (errbuf));
	LOG("ClientDisconnect", ("About to drop client (character) %s because the connection is overflowing\n", m_characterName.c_str()));

	snprintf(errbuf, sizeof(errbuf)-1, "Connection Overflow (bytes pending=%u)", bytesPending);
	errbuf[sizeof(errbuf)-1] = '\0';
	ConnectionServer::dropClient(this, std::string(errbuf));
}

//-----------------------------------------------------------------------

bool ClientConnection::checkSpamLimit(unsigned int messageSize)
{
	if (!ConfigConnectionServer::getSpamLimitEnabled())
		return true;

	unsigned long curTimeMs = Clock::timeMs();
	if (m_receiveLastTimeMs)
	{
		++m_receiveHistoryPackets;
		m_receiveHistoryBytes += messageSize;
		m_receiveHistoryMs += curTimeMs-m_receiveLastTimeMs;

		// rescale the history information if we've exceeded the reset time; this
		// must be done before the spam check below or else we may run into overflow
		// issues because m_receiveHistoryMs could be pretty large if we haven't
		// received anything from the client for a while
		while (m_receiveHistoryMs > ConfigConnectionServer::getSpamLimitResetTimeMs())
		{
			++m_countSpamLimitResetTime;

			unsigned int resetScale = ConfigConnectionServer::getSpamLimitResetScaleFactor();
			m_receiveHistoryMs      /= resetScale;
			m_receiveHistoryBytes   /= resetScale;
			m_receiveHistoryPackets /= resetScale;
		}

		// check for exceeding limits, but wait for at least
		// one reset cycle so that there has been enough
		// elapsed time, so we won't get a false positive
		if (m_countSpamLimitResetTime)
		{
			if (m_receiveHistoryBytes >= m_receiveHistoryMs*ConfigConnectionServer::getSpamLimitBytesPerSec()/1000)
			{
				LOG("Network", ("Client %s disconnected for exceeding bytes/sec limit (bytes=%u, time=%lums)\n", getCharacterId().getValueString().c_str(), m_receiveHistoryBytes, m_receiveHistoryMs));
				return false;
			}
			if (m_receiveHistoryPackets >= m_receiveHistoryMs*ConfigConnectionServer::getSpamLimitPacketsPerSec()/1000)
			{
				LOG("Network", ("Client %s disconnected for exceeding packets/sec limit (packets=%u, time=%lums)\n", getCharacterId().getValueString().c_str(), m_receiveHistoryPackets, m_receiveHistoryMs));
				return false;
			}
		}
	}
	m_receiveLastTimeMs = curTimeMs;
	return true;
}

//-----------------------------------------------------------------------

void ClientConnection::onReceive(const Archive::ByteStream & message)
{
	try
	{
		if (!checkSpamLimit(message.getSize()))
		{
			ConnectionServer::dropClient(this, "Spam Detected");
			return;
		}

		unsigned long curTimeMs = Clock::timeMs();
		if (m_sendLastTimeMs + std::min(gs_receiveDelayMaxMs, static_cast<unsigned long>(Clock::frameTime()*1000.0f)) < curTimeMs)
		{
			static HeartBeat h;
			send(h, false);
		}

		Archive::ReadIterator ri = message.begin();
		GameNetworkMessage m(ri);
		ri = message.begin();

		//Clients with a selected character get routed to a game server.
		//@todo check for filtering out bad messages.
		if (m_hasSelectedCharacter)
		{
			// if it is a chat message, send it directly to the chat server
			if(
				m.isType("ChatAddFriend") ||
				m.isType("ChatAddModeratorToRoom")  ||
				m.isType("ChatBanAvatarFromRoom") ||
				m.isType("ChatCreateRoom") ||
				m.isType("ChatDeletePersistentMessage") ||
				m.isType("ChatDeleteAllPersistentMessages") ||
				m.isType("ChatDestroyRoom") ||
				m.isType("ChatInstantMessageToCharacter") ||
				m.isType("ChatInviteAvatarToRoom") ||
				m.isType("ChatKickAvatarFromRoom") ||
				m.isType("ChatRemoveAvatarFromRoom")  ||
				m.isType("ChatRemoveFriend") ||
				m.isType("ChatRemoveModeratorFromRoom")  ||
				m.isType("ChatRequestPersistentMessage") ||
				m.isType("ChatRequestRoomList") ||
				m.isType("ChatSendToRoom") ||
				m.isType("ChatUninviteFromRoom") ||
				m.isType("ChatUnbanAvatarFromRoom") ||
				m.isType("VerifyPlayerNameMessage") ||
				m.isType("VoiceChatRequestPersonalChannel") ||
				m.isType("VoiceChatInvite") ||
				m.isType("VoiceChatKick") ||
				m.isType("VoiceChatRequestChannelInfo")
				)
			{
				DEBUG_REPORT_LOG(true, ("ConnServ: ClientConnection::onReceive()\n"));

				NOT_NULL(m_client);
				if(m_client)
				{
					static std::vector<NetworkId> v;
					v.clear();
					v.push_back(m_client->getNetworkId());
					GameClientMessage gcm(v, true, ri);
					if(m_client->getChatConnection())
					{
						m_client->getChatConnection()->send(gcm , true);
					}
					else
					{
						// defer chat messages until a server is back online
						Archive::ByteStream bs;
						m.pack(bs);
						m_client->deferChatMessage(bs);
					}
				}
				else
				{
					ConnectionServer::dropClient(this, "m_client is null while receiving a message!");
					disconnect();
				}
			}
			// ChatEnterRoom and ChatEnterRoomById needs to go to the game server to determine
			// if the character is not allowed to enter the room because of game rule restrictions;
			// only if that test pass do we forward the message on to the chat server to request
			// to enter the room
			else if (m.isType("ChatEnterRoom") ||
				m.isType("ChatEnterRoomById")
				)
			{
				NOT_NULL(m_client);

				unsigned int sequence;
				std::string roomName;

				Archive::ReadIterator cri = message.begin();
				if (m.isType("ChatEnterRoom"))
				{
					ChatEnterRoom const cer(cri);
					sequence = cer.getSequence();
					roomName = cer.getRoomName();
				}
				else
				{
					ChatEnterRoomById const cerbi(cri);
					sequence = cerbi.getSequence();
					roomName = cerbi.getRoomName();
				}

				if(m_client && m_client->getGameConnection())
				{
					if (m_pendingChatEnterRoomRequests.count(sequence) == 0)
					{
						GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::string>, unsigned int> > const cervr(
							"ChatEnterRoomValidationRequest",
							std::make_pair(
							std::make_pair(m_client->getNetworkId(), roomName),
							sequence));

						m_client->getGameConnection()->send(cervr, true);

						// queue up request until game server responds
						static std::vector<NetworkId> v;
						v.clear();
						v.push_back(m_client->getNetworkId());
						m_pendingChatEnterRoomRequests[sequence] = new GameClientMessage(v, true, ri);
					}
				}
				else
				{
					// send back response to client saying game server not available

					// the client only cares about sequence and result when it's a failure
					ChatOnEnteredRoom fail(sequence, SWG_CHAT_ERR_NO_GAME_SERVER, 0, ChatAvatarId()); 
					send(fail, true);
				}
			}
			// ChatQueryRoom needs to go to the game server to determine if the character is
			// not allowed to query the room because of game rule restrictions; only if that
			// test pass do we forward the message on to the chat server for completion
			else if (m.isType("ChatQueryRoom"))
			{
				NOT_NULL(m_client);

				Archive::ReadIterator cri = message.begin();
				ChatQueryRoom cqr(cri);

				if(m_client && m_client->getGameConnection())
				{
					if (m_pendingChatQueryRoomRequests.count(cqr.getSequence()) == 0)
					{
						GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::string>, unsigned int> > const cqrvr(
							"ChatQueryRoomValidationRequest",
							std::make_pair(
							std::make_pair(m_client->getNetworkId(), cqr.getRoomName()),
							cqr.getSequence()));

						m_client->getGameConnection()->send(cqrvr, true);

						// queue up request until game server responds
						static std::vector<NetworkId> v;
						v.clear();
						v.push_back(m_client->getNetworkId());
						m_pendingChatQueryRoomRequests[cqr.getSequence()] = new GameClientMessage(v, true, ri);
					}
				}
			}
			// ChatInviteGroupToRoom needs to go to the game server to get group information
			else if (m.isType("ChatInviteGroupToRoom"))
			{
				NOT_NULL(m_client);
				if(m_client)
				{
					if (m_client->getGameConnection())
					{
						static std::vector<NetworkId> v;
						v.clear();
						v.push_back(m_client->getNetworkId());
						GameClientMessage gcm(v, true, ri);
						m_client->getGameConnection()->send(gcm, true);
					}
					else
					{
						// defer chat messages until a server is back online
						Archive::ByteStream bs;
						m.pack(bs);
						m_client->deferChatMessage(bs);
					}
				}
				else
				{
					ConnectionServer::dropClient(this, "m_client is null while receiving a message!");
					disconnect();
				}
			}
			// ChatPersistentMessageToServer may need to be passed off to the game server for guild or citizens messages
			else if (m.isType("ChatPersistentMessageToServer"))
			{
				NOT_NULL(m_client);
				if(m_client)
				{
					static std::vector<NetworkId> v;
					v.clear();
					v.push_back(m_client->getNetworkId());

					Archive::ReadIterator cri = message.begin();
					ChatPersistentMessageToServer chat(cri);
					std::string const &toName = chat.getToCharacterName().name;
					if (!_stricmp(toName.c_str(), "guild") || !_strnicmp(toName.c_str(), "guild ", 6) || !_stricmp(toName.c_str(), "citizens"))
					{
						if (m_client->getGameConnection())
						{
							GameClientMessage gcm(v, true, ri);
							m_client->getGameConnection()->send(gcm, true);
						}
					}
					else
					{
						if (m_client->getChatConnection())
						{
							GameClientMessage gcm(v, true, ri);
							m_client->getChatConnection()->send(gcm, true);
						}
						else
						{
							// defer chat messages until a server is back online
							Archive::ByteStream bs;
							m.pack(bs);
							m_client->deferChatMessage(bs);
						}
					}
				}
				else
				{
					ConnectionServer::dropClient(this, "m_client is null while receiving a message!");
					disconnect();
				}
			}
			// if it is a cs message, send it directly to the cs server
			else if (
				m.isType("ConnectPlayerMessage") ||
				m.isType("DisconnectPlayerMessage")  ||
				m.isType("CreateTicketMessage")  ||
				m.isType("AppendCommentMessage")  ||
				m.isType("CancelTicketMessage")  ||
				m.isType("GetTicketsMessage")  ||
				m.isType("GetCommentsMessage")  ||
				m.isType("SearchKnowledgeBaseMessage")  ||
				m.isType("GetArticleMessage")  ||
				m.isType("RequestCategoriesMessage") ||
				m.isType("NewTicketActivityMessage")
				)
			{
				NOT_NULL(m_client);
				if(m_client)
				{
					CustomerServiceConnection *customerServiceConnection = m_client->getCustomerServiceConnection();

					//DEBUG_REPORT_LOG(true, ("CONSRV::CS - suid: %i\n", getSUID()));

					if (customerServiceConnection != NULL)
					{
						static std::vector<NetworkId> v;
						v.clear();
						v.push_back(m_client->getNetworkId());
						if (m.isType("ConnectPlayerMessage"))
						{
							ConnectPlayerMessage message(ri);
							message.setStationId(getSUID());

							GameClientMessage gcm(v, true, message);
							customerServiceConnection->send(gcm , true);
						}
						else if (m.isType("CreateTicketMessage"))
						{
							CreateTicketMessage message(ri);
							message.setStationId(getSUID());

							GameClientMessage gcm(v, true, message);
							customerServiceConnection->send(gcm , true);
						}
						else if (m.isType("AppendCommentMessage"))
						{
							AppendCommentMessage message(ri);
							message.setStationId(getSUID());

							GameClientMessage gcm(v, true, message);
							customerServiceConnection->send(gcm , true);
						}
						else if (m.isType("CancelTicketMessage"))
						{
							CancelTicketMessage message(ri);
							message.setStationId(getSUID());

							GameClientMessage gcm(v, true, message);
							customerServiceConnection->send(gcm , true);
						}
						else if (m.isType("GetTicketsMessage"))
						{
							GetTicketsMessage message(ri);
							message.setStationId(getSUID());

							GameClientMessage gcm(v, true, message);
							customerServiceConnection->send(gcm , true);
						}
						else if (m.isType("NewTicketActivityMessage"))
						{
							NewTicketActivityMessage message(ri);
							message.setStationId(getSUID());

							GameClientMessage gcm(v, true, message);
							customerServiceConnection->send(gcm , true);
						}
						else
						{
							GameClientMessage gcm(v, true, ri);
							customerServiceConnection->send(gcm , true);
						}
						//else
						//{
						//	/*// defer chat messages until a server is back online
						//	  Archive::ByteStream bs;
						//	  m.pack(bs);
						//	  m_client->deferChatMessage(bs);
						//	*/
						//}
					}
				}
				else
				{
					ConnectionServer::dropClient(this, "m_client is null while receiving a message!");
					disconnect();
				}
			}
			else if (m.isType("28afefcc187a11dc888b001")) // obfuscation for ClientInactivityMessage message
			{
				GenericValueTypeMessage<bool> msg(ri);

				if (m_hasBeenValidated && m_sessionValidated)
				{
					// client went inactive
					if (msg.getValue())
					{
						if (m_lastActiveTime > 0)
						{
							// record the amount of active time
							m_activePlayTimeDuration += static_cast<unsigned long>(::time(NULL) - m_lastActiveTime);

							// tell Session to stop recording play time for the character
							if (ConnectionServer::getSessionApiClient() && ConfigConnectionServer::getSessionRecordPlayTime())
							{
								LOG("CustomerService", ("Login:%s calling SessionStopPlay() for %s/%s/%s (%s). Active play time: %s", ClientConnection::describeAccount(this).c_str(), this->getSessionId().c_str(), ConfigConnectionServer::getClusterName(), this->getCharacterName().c_str(), this->getCharacterId().getValueString().c_str(), this->getCurrentActivePlayTimeDuration().c_str()));
								ConnectionServer::getSessionApiClient()->stopPlay(*this);
							}

							// client is no longer active; this needs to be set after the LOG() statement
							// above because getCurrentActivePlayTimeDuration() uses m_lastActiveTime
							m_lastActiveTime = 0;

							// update the play time info on the game server
							sendPlayTimeInfoToGameServer();

							// drop inactive character
							if (ConfigConnectionServer::getDisconnectOnInactive())
							{
								LOG("ClientDisconnect", ("Disconnecting %u because the player was inactive for too long.",getSUID()));
								ConnectionServer::dropClient(this, "Client inactivity");
								disconnect();
							}
							else if (ConfigConnectionServer::getDisconnectFreeTrialOnInactive() && ((m_featureBitsSubscription & ClientSubscriptionFeature::Base) == 0))
							{
								LOG("ClientDisconnect", ("Disconnecting (free trial) %u because the player was inactive for too long.",getSUID()));
								ConnectionServer::dropClient(this, "Client inactivity (free trial)");
								disconnect();
							}
						}
					}
					// client went active
					else
					{
						if (m_lastActiveTime == 0)
						{
							// record the time client went active
							m_lastActiveTime = ::time(NULL);

							// tell Session to start recording play time for the character
							if (ConnectionServer::getSessionApiClient() && ConfigConnectionServer::getSessionRecordPlayTime())
							{
								LOG("CustomerService", ("Login:%s calling SessionStartPlay() for %s/%s/%s (%s)", ClientConnection::describeAccount(this).c_str(), this->getSessionId().c_str(), ConfigConnectionServer::getClusterName(), this->getCharacterName().c_str(), this->getCharacterId().getValueString().c_str()));
								ConnectionServer::getSessionApiClient()->startPlay(*this);
							}

							// update the play time info on the game server
							sendPlayTimeInfoToGameServer();
						}
					}
				}
			}
			else
			{
				//Forward on to Game Server
				DEBUG_REPORT_LOG((!m_client || !m_client->getGameConnection()), ("Warn, received game message with no game connection.  This may happen for a short time after a GameServer crashes.  If it continues to happen, it indicates a bug.\n"));

				if (m_client && m_client->getGameConnection())
				{
					static std::vector<NetworkId> v;
					v.clear();
					v.push_back(m_client->getNetworkId());
					GameClientMessage gcm(v, true, ri);
					m_client->getGameConnection()->send(gcm, true);
				}
			}
		}

		else if(m.isType("ClientIdMsg"))
		{
			DEBUG_REPORT_LOG(true,("Recieved ClientIdMsg\n"));
			ClientIdMsg	k(ri);

			handleClientIdMessage(k);
		}
		else if(m.isType("SelectCharacter"))
		{
			SelectCharacter s(ri);
			DEBUG_REPORT_LOG(true,("Recvd SelectCharacter message for %s.\n", s.getId().getValueString().c_str()));

			handleSelectCharacterMessage(s);
		}
		else if(m.isType("ClientCreateCharacter"))
		{
			if (m_hasBeenValidated && !m_hasSelectedCharacter) //lint !e774 no this doesn't always eval to true
			{
				ClientCreateCharacter clientCreate(ri);
				DEBUG_REPORT_LOG(true,("Got ClientCreateCharacter message for %lu with name %s\n", m_suid, Unicode::wideToNarrow(clientCreate.getCharacterName()).c_str()));
				LOG("TraceCharacterCreation", ("%d sent ClientCreateCharacter(charaterName=%s, templateName=%s, scaleFactor=%f, startingLocation=%s, hairTemplateName=%s, profession=%s, jedi=%d, useNewbieTutorial=%d, skillTemplate=%s, workingSkill=%s)",
							getSUID(),
							Unicode::wideToNarrow(clientCreate.getCharacterName()).c_str(),
							clientCreate.getTemplateName().c_str(),
							clientCreate.getScaleFactor(),
							clientCreate.getStartingLocation().c_str(),
							clientCreate.getHairTemplateName().c_str(),
							clientCreate.getProfession().c_str(),
							static_cast<int>(clientCreate.getJedi()),
							static_cast<int>(clientCreate.getUseNewbieTutorial()),
							clientCreate.getSkillTemplate().c_str(),
							clientCreate.getWorkingSkill().c_str()));

				if (!clientCreate.getUseNewbieTutorial() && !m_canSkipTutorial)
				{
					LOG("TraceCharacterCreation", ("%d failed character creation. The client is not allowed to skip the tutorial", getSUID()));
					// This is probably a hack attempt, because the Client was already told they couldn't skip the tutorial
					LOG("ClientDisconnect", ("Disconnecting %u because they tried to skip the tutorial without permission.\n",getSUID()));
					disconnect();
				}
				else if (clientCreate.getJedi() && !m_canCreateJediCharacter)
				{
					LOG("TraceCharacterCreation", ("%d failed character creation. The request character type was Jedi, but this client cannot create a Jedi character", getSUID()));
					// This is probably a hack attempt, because the Client was already told they couldn't create a character
					LOG("ClientDisconnect", ("Disconnecting %u because they tried to create a Jedi character without permission.\n",getSUID()));
					disconnect();
				}
				else if (!clientCreate.getJedi() && !m_canCreateRegularCharacter)
				{
						LOG("TraceCharacterCreation", ("%d failed character creation. The client is not allowed to create any characters", getSUID()));
						// This is probably a hack attempt, because the Client was already told they couldn't create a character
						LOG("ClientDisconnect", ("Disconnecting %u because they tried to create a regular character without permission.\n",getSUID()));
						disconnect();
				}
				else if (m_hasRequestedCharacterCreate)
				{
					LOG("TraceCharacterCreation", ("%d failed character creation. The client has already requested character creation on this connection", getSUID()));
					LOG("ClientDisconnect", ("Disconnecting %u because the client has already requested character creation on this connection.\n",getSUID()));
					disconnect();
				}
				else if (m_hasCreatedCharacter)
				{
					LOG("TraceCharacterCreation", ("%d failed character creation. A character has been created on this or another galaxy for this account while this connection was up", getSUID()));
					LOG("ClientDisconnect", ("Disconnecting %u because a character has been created on this or another galaxy for this account while this connection was up.\n",getSUID()));
					disconnect();
				}
				else if (clientCreate.getCharacterName().length()==0)
				{
					LOG("TraceCharacterCreation", ("%d failed character creation. The character's name is empty", getSUID()));
					LOG("ClientDisconnect",("Disconnecting %u because they tried to create a character with no name.\n",getSUID()));
					disconnect();
				}
				else
				{
					Unicode::String biography(clientCreate.getBiography());
					if (biography.length() > 1024)
					{
						IGNORE_RETURN( biography.erase(1024) );
						DEBUG_REPORT_LOG(true,("Biography shortened to 1024 characters.\n"));
					}
					
					if (m_isAdminAccount)
					{
						ConnectionCreateCharacter connectionCreate(
							m_suid,
							clientCreate.getCharacterName(),
							clientCreate.getTemplateName(),
							clientCreate.getScaleFactor(),
							clientCreate.getStartingLocation(),
							clientCreate.getAppearanceData(),
							clientCreate.getHairTemplateName(),
							clientCreate.getHairAppearanceData(),
							clientCreate.getProfession(),
							clientCreate.getJedi(),
							biography,
							clientCreate.getUseNewbieTutorial(),
							clientCreate.getSkillTemplate(),
							clientCreate.getWorkingSkill(),
							m_isAdminAccount,
							false,
							m_featureBitsGame);

						ConnectionServer::sendToCentralProcess(connectionCreate);
						LOG("TraceCharacterCreation", ("%d character creation request sent to CentralServer", getSUID()));
					}
					else
					{
						// for regular players, do one final check with the LoginServer
						// to make sure the character can be created (i.e. that character
						// limits have not been exceeded)
						delete m_pendingCharacterCreate;
						m_pendingCharacterCreate = new ConnectionCreateCharacter(
							m_suid,
							clientCreate.getCharacterName(),
							clientCreate.getTemplateName(),
							clientCreate.getScaleFactor(),
							clientCreate.getStartingLocation(),
							clientCreate.getAppearanceData(),
							clientCreate.getHairTemplateName(),
							clientCreate.getHairAppearanceData(),
							clientCreate.getProfession(),
							clientCreate.getJedi(),
							biography,
							clientCreate.getUseNewbieTutorial(),
							clientCreate.getSkillTemplate(),
							clientCreate.getWorkingSkill(),
							m_isAdminAccount,
							false,
							m_featureBitsGame);

						LOG("TraceCharacterCreation", ("%d character creation request awaiting final verification from LoginServer", getSUID()));

						ValidateAccountMessage vcm(m_suid, 0, m_featureBitsSubscription);
						ConnectionServer::sendToCentralProcess(vcm);
					}

					m_hasRequestedCharacterCreate = true;
				}
			}
		}
		else if(m.isType("ClientRandomNameRequest"))
		{
			ClientRandomNameRequest clientRandomName(ri);

			RandomNameRequest randomNameRequest(m_suid, clientRandomName.getCreatureTemplate());
			ConnectionServer::sendToCentralProcess(randomNameRequest);
			LOG("TraceCharacterCreation", ("%d requested a random name. Request sent to CentralServer", getSUID()));
		}
		else if(m.isType("ClientVerifyAndLockNameRequest"))
		{
			ClientVerifyAndLockNameRequest clientVerifyAndLockNameRequest(ri);

			VerifyAndLockNameRequest verifyAndLockNameRequest(m_suid, NetworkId::cms_invalid, clientVerifyAndLockNameRequest.getTemplateName(), clientVerifyAndLockNameRequest.getCharacterName(), m_featureBitsGame);
			ConnectionServer::sendToCentralProcess(verifyAndLockNameRequest);
			LOG("TraceCharacterCreation", ("%d requested a verify and lock of name: %s. Request sent to CentralServer", getSUID(), Unicode::wideToNarrow(verifyAndLockNameRequest.getCharacterName()).c_str()));
		}
		else if(m.isType("LagRequest"))
		{
			/*
			handleLagRequest();
			*/
		}
	}
	catch(const Archive::ReadException & readException)
	{
		WARNING(true, ("Archive read error (%s) from client. Disconnecting client", readException.what()));
		LOG("ClientDisconnect", ("Archive read error (%s) from client. Disconnecting client", readException.what()));
		disconnect();
	}
}

//-----------------------------------------------------------------------

void ClientConnection::handleLagRequest()
{
	// client is requesting a lag ping (reliable trace)
	GameNetworkMessage response("ConnectionServerLagResponse");
	send(response, true);

	if(m_hasSelectedCharacter && m_client && m_client->getGameConnection())
	{
		// send to game server
		GameNetworkMessage request("LagRequest");
		std::vector<NetworkId> v;
		v.push_back(m_characterId);
		GameClientMessage gcm(v, true, request);
		m_client->getGameConnection()->send(gcm, true);
	}
	else
	{
		// send game response immediately
		GameNetworkMessage gameResponse("GameServerLagResponse");
		send(gameResponse, true);
	}

}

//-----------------------------------------------------------------------

/** character has selected a character and calls this function
* to associate the connection with the new client they created with that
* character.  We no longer need the character map.
*/
void ClientConnection::setClient(Client* newClient)
{
	//This fatal is here to try to catch the reconnect bug.
	WARNING_STRICT_FATAL(m_client, ("Attempting to set the client on a connection that already has one. Client %s\n", getCharacterName().c_str()));
	// jrandall - I've removed the fatal because it is blocking some people from getting some work
	// done. I'm on a high priority fix at the moment. If this warning starts appearing,
	// set a break point or something.
	//DEBUG_FATAL(client, ("Attempting to set the client on a connection that already has one.\n"));
	m_client = newClient;

	// todo put this in:    characterMap.clear();
}

//-----------------------------------------------------------------------

void ClientConnection::send(const GameNetworkMessage & message, const bool reliable)
{
	m_sendLastTimeMs = Clock::timeMs();


	if ( sm_outgoingBytesMap_Worktime == 0 )
		sm_outgoingBytesMap_Worktime = m_sendLastTimeMs;
	else if ( (m_sendLastTimeMs - sm_outgoingBytesMap_Worktime) > 60000 )   // 60 seconds
	{
		sm_outgoingBytesMap_Stats =  sm_outgoingBytesMap_Working;
		std::map< std::string, uint32 >::iterator iter;
		for ( iter = sm_outgoingBytesMap_Working.begin(); iter != sm_outgoingBytesMap_Working.end(); ++iter )
		{
			iter->second = 0;
		}
		sm_outgoingBytesMap_Worktime = m_sendLastTimeMs;
	}
	sm_outgoingBytesMap_Working[ message.getCmdName() ] += message.getByteStream().getSize();


	ServerConnection::send(message, reliable);
}

//-----------------------------------------------------------------------

std::map< std::string, uint32 >& ClientConnection::getPacketBytesPerMinStats()
{ 
	uint32 now = Clock::timeMs();
        if ( sm_outgoingBytesMap_Worktime == 0 )
                sm_outgoingBytesMap_Worktime = now;
        else if ( (now - sm_outgoingBytesMap_Worktime) > 60000 )   // 60 seconds
        {
                sm_outgoingBytesMap_Stats =  sm_outgoingBytesMap_Working;
                std::map< std::string, uint32 >::iterator iter;
                for ( iter = sm_outgoingBytesMap_Working.begin(); iter != sm_outgoingBytesMap_Working.end(); ++iter )
                {
                        iter->second = 0;
                }
                sm_outgoingBytesMap_Worktime = now;
        }

	return sm_outgoingBytesMap_Stats;
}

//-----------------------------------------------------------------------

void ClientConnection::handleChatEnterRoomValidationResponse(unsigned int sequence, unsigned int result)
{
	std::map<unsigned long, GameClientMessage*>::iterator iterFind = m_pendingChatEnterRoomRequests.find(sequence);
	if (iterFind != m_pendingChatEnterRoomRequests.end())
	{
		if (result == CHATRESULT_SUCCESS)
		{
			if (m_client && m_client->getChatConnection())
			{
				// game server says it's ok to enter the chat room,
				// so forward the request on to the chat server 
				m_client->getChatConnection()->send(*(iterFind->second), true);
			}
			else
			{
				// send back response to client saying chat server not available

				// the client only cares about sequence and result when it's a failure
				ChatOnEnteredRoom fail(sequence, SWG_CHAT_ERR_CHAT_SERVER_UNAVAILABLE, 0, ChatAvatarId()); 
				send(fail, true);
			}
		}
		else
		{
			// send back response to client saying game server denied enter room request

			// the client only cares about sequence and result when it's a failure
			ChatOnEnteredRoom fail(sequence, result, 0, ChatAvatarId()); 
			send(fail, true);
		}

		delete iterFind->second;
		m_pendingChatEnterRoomRequests.erase(iterFind);
	}
}

//-----------------------------------------------------------------------

void ClientConnection::handleChatQueryRoomValidationResponse(unsigned int sequence, bool success)
{
	std::map<unsigned long, GameClientMessage*>::iterator iterFind = m_pendingChatQueryRoomRequests.find(sequence);
	if (iterFind != m_pendingChatQueryRoomRequests.end())
	{
		if (success)
		{
			if (m_client && m_client->getChatConnection())
			{
				// game server says it's ok to query the chat room,
				// so forward the request on to the chat server 
				m_client->getChatConnection()->send(*(iterFind->second), true);
			}
		}

		delete iterFind->second;
		m_pendingChatQueryRoomRequests.erase(iterFind);
	}
}

//-----------------------------------------------------------------------

void ClientConnection::sendByteStream(const Archive::ByteStream& bs, bool reliable)
{
	Connection::send(bs, reliable);
}

//-----------------------------------------------------------------------

/**
 * Send the client to an arbitratry game server based on the current scene
 */
const bool ClientConnection::sendToGameServer()
{
	GameConnection * c = const_cast<GameConnection *>(ConnectionServer::getGameConnection(m_targetScene));
	return sendToGameServer(c);
}

// ----------------------------------------------------------------------

/**
 * Send the client to a particular game server, sepcified by process id.
 */

const bool ClientConnection::sendToGameServer(uint32 gameServerId)
{
	GameConnection * c = const_cast<GameConnection *>(ConnectionServer::getGameConnection(gameServerId));
	return sendToGameServer(c);
}

// ----------------------------------------------------------------------

bool ClientConnection::sendToGameServer(GameConnection *c)
{
	bool result = false;

	if(c && m_hasSelectedCharacter && m_hasBeenValidated)
	{
		//create a new client
		ConnectionServer::addNewClient(this,
			m_characterId,
			c,
			m_targetScene,
			m_sendToStarport);

		LoggedInMessage m(m_suid);
		ConnectionServer::sendToCentralProcess(m);
		result = true;
		m_hasBeenSentToGameServer = true;
	}
	return result;
}

//-----------------------------------------------------------------------

/**
 * Called when DBProcess responds to our ValidateCharacterForLoginMessage.
 * Now we know whether the character is valid and where in the world it
 * is located.
 */
void ClientConnection::onCharacterValidated(bool isValid, const NetworkId &character, const std::string &characterName, const NetworkId &container, const std::string &scene, const Vector &coordinates)
{
	if (!m_validatingCharacter)
	{
		LOG("TraceCharacterSelection", ("%d received a validation response, but is not in the process of validation", getSUID()));
		DEBUG_REPORT_LOG(true,("Got unexpected onCharacterValidated() for account %lu.\n",getSUID()));
		return;
	}
	m_validatingCharacter=false;

	if (isValid)
	{

		LOG("TraceCharacterSelection", ("%d received a validation response. The character (%s: %s) at (%s,%f,%f,%f,%s) selected is valid", getSUID(), character.getValueString().c_str(), characterName.c_str(), scene.c_str(), coordinates.x, coordinates.y, coordinates.z, container.getValueString().c_str()));
		LOG("CustomerService", ("Login:%s received a validation response. The character (%s: %s) at (%s,%f,%f,%f,%s) selected is valid", describeAccount(this).c_str(), character.getValueString().c_str(), characterName.c_str(), scene.c_str(), coordinates.x, coordinates.y, coordinates.z, container.getValueString().c_str()));
		m_targetScene = scene;
		m_targetCoordinates = coordinates;
		m_characterId = character;
		m_containerId = container;
		m_characterName = characterName;

		m_hasSelectedCharacter = true;



		// If they don't have access to mustafar but are on the planet, move them to a safe spot
		uint32 features = getGameFeatures();
		if ( (strncmp(scene.c_str(), "mustafar", strlen("mustafar")) == 0 ) &&
			(  (features & ClientGameFeature::TrialsOfObiwanRetail) == 0 )  )
		{
			// mos eisley starport
			m_sendToStarport = true;
			m_targetScene = "tatooine";
			m_targetCoordinates.x = 3528;
			m_targetCoordinates.y = 4;
			m_targetCoordinates.z = -4804;

                	// Make sure Central knows the right sceneId for the player
                	GenericValueTypeMessage<std::pair<NetworkId, std::pair<std::string, bool> > > const msg(
                        	"SetSceneForPlayer",
                        	std::make_pair(
                                m_characterId,
                                std::make_pair("tatooine", false)));
			ConnectionServer::sendToCentralProcess(msg);


			LOG("TraceCharacterSelection", ("Character didn't have Mustafar feature bit, moving to Tattoine."));
		}


		// ask CentralServer to suggest a game server for this character
		// (Central will forward the request to a Planet Server)

		RequestGameServerForLoginMessage requestmsg(getSUID(), m_characterId, m_containerId, m_targetScene, m_targetCoordinates, false);
		if(ConnectionServer::getCentralConnection())
			ConnectionServer::getCentralConnection()->send(requestmsg, true);
		else
		{
			LOG("ClientDisconnect",("Can't handle login of character %s because there is no connection to Central.\n",m_characterId.getValueString().c_str()));
			ErrorMessage err("Validation Failed", "The connection to the central server is down.  Please try again later.");
			send(err, true);

			disconnect();
		}
	}
	else
	{
		LOG("TraceCharacterSelection", ("%d validation failed, disconnecting client", getSUID()));
		ErrorMessage err("Validation Failed", "Your character was denied login by the database.");
		send(err, true);

		LOG("ClientDisconnect", ("Denying login for account %u.\n",getSUID()));
		disconnect();
	}
}

//------------------------------------------------------------------------------------------

void ClientConnection::onValidateClient (uint32 suid, const std::string & username, bool secure, const char* id, const uint32 gameFeatures, const uint32 subscriptionFeatures, unsigned int entitlementTotalTime, unsigned int entitlementEntitledTime, unsigned int entitlementTotalTimeSinceLastLogin, unsigned int entitlementEntitledTimeSinceLastLogin, int buddyPoints)
{
	UNREF(id);
	m_sessionValidated = true;
	m_suid = suid;
	m_accountName = username;
	m_featureBitsGame = gameFeatures;
	m_featureBitsSubscription = subscriptionFeatures;
	m_isSecure = secure;
	m_entitlementTotalTime = entitlementTotalTime;
	m_entitlementEntitledTime = entitlementEntitledTime;
	m_entitlementTotalTimeSinceLastLogin = entitlementTotalTimeSinceLastLogin;
	m_entitlementEntitledTimeSinceLastLogin = entitlementEntitledTimeSinceLastLogin;
	m_buddyPoints = buddyPoints;

	if (id)
		m_sessionId = id;

	if (m_requestedSuid != 0 && suid != m_requestedSuid)
	{
		//verify internal, secure, is on the god list
		bool loginOK=false;
		if(!secure)
			LOG("CustomerService",("AdminLogin:  User %s (account %li) attempted to log into account %li, but was not using a SecureID token", username.c_str(),suid, m_requestedSuid));
		else
		{
			if (!AdminAccountManager::isInternalIp(getRemoteAddress()))
				LOG("CustomerService",("AdminLogin:  User %s (account %li) attempted to log into account %li, but was not logging in from an internal IP", username.c_str(),suid, m_requestedSuid));
			else
			{
				int adminLevel=0;
				if (!AdminAccountManager::isAdminAccount(Unicode::toLower(username), adminLevel) || adminLevel < 10)
					LOG("CustomerService",("AdminLogin:  User %s (account %li) attempted to log into account %li, but did not have sufficient permissions", username.c_str(),suid, m_requestedSuid));
				else
				{
					LOG("CustomerService",("AdminLogin:  User %s (account %li) logged into account %li", username.c_str(),m_suid,m_requestedSuid));
					DEBUG_REPORT_LOG(true,("AdminLogin:  User %s (account %li) logged into account %li\n", username.c_str(),m_suid,m_requestedSuid));
					m_suid = m_requestedSuid;
					m_usingAdminLogin = true;
					loginOK=true;
				}
			}
		}
		if (!loginOK)
		{
			disconnect();
			return;
		}
	}

	m_featureBitsGame &= ~ConfigConnectionServer::getDisabledFeatureBits();
	
	//Configoption to enable JTL features for beta players so that our code can pretend everything uses the JTL Retail bit
	if (ConfigConnectionServer::getSetJtlRetailIfBetaIsSet())
	{
		if (ClientGameFeature::SpaceExpansionBeta & m_featureBitsGame)
		{
			m_featureBitsGame |= ClientGameFeature::SpaceExpansionRetail;
		}
	}

	//Configoption to enable Obiwan features for beta players so that our code can pretend everything uses the Obiwan Retail bit
	if (ConfigConnectionServer::getSetTrialsOfObiwanRetailIfBetaIsSet())
	{
		if (ClientGameFeature::TrialsOfObiwanBeta & m_featureBitsGame)
		{
			//-- add retail bit only if player does not have the preorder bit
			if ((m_featureBitsGame & ClientGameFeature::TrialsOfObiwanPreorder) == 0)
				m_featureBitsGame |= ClientGameFeature::TrialsOfObiwanRetail;
		}
		else
	{
		// Clear bits from players who might have them for real, but aren't in the beta
			m_featureBitsGame &= ~ClientGameFeature::TrialsOfObiwanRetail;
			m_featureBitsGame &= ~ClientGameFeature::TrialsOfObiwanPreorder;
		}
	}
		
	//-- Obiwan Preorders get the Retail bit as well... All rewards etc...
	if (ClientGameFeature::TrialsOfObiwanPreorder & m_featureBitsGame)
		{
		m_featureBitsGame |= ClientGameFeature::TrialsOfObiwanRetail;
	}

	// Restrictions for "new free trial" account
	if (   ((m_featureBitsSubscription & ClientSubscriptionFeature::FreeTrial2) != 0)
		&& ((m_featureBitsSubscription & ClientSubscriptionFeature::Base)      == 0))
	{
		// "new free trial" account don't have access to RoW until they convert
		m_featureBitsGame &= ~ClientGameFeature::Episode3ExpansionRetail;
		m_featureBitsGame &= ~ClientGameFeature::Episode3PreorderDownload;

		// ClientGameFeature::FreeTrial2 indicates this is a converted "new free trial"
		// account, and since this account hasn't converted yet, we remove this bit
		m_featureBitsGame &= ~ClientGameFeature::FreeTrial2;
	}

	// Clear feature bits that only apply if the account is paying (i.e. the sub base bit is set)
	if ((m_featureBitsSubscription & ClientSubscriptionFeature::Base) == 0)
	{
		m_featureBitsGame &= ~ClientGameFeature::HousePackupReward;
		m_featureBitsGame &= ~ClientGameFeature::BuddyProgramReward;
	}

	//hack to prevent non-jtl users from using jtl assets.  In this hack, clients who didn't patch through jtl patcher will send us information requesting we clear their jtl bit since they are supposed to go through the jtl patcher.  We can remove this hack once the launch pad takes care of this for us.
	m_featureBitsGame &= ~m_gameBitsToClear;
	//end hack
	
	ValidateAccountMessage vcm(m_suid, 0, m_featureBitsSubscription);
	ConnectionServer::sendToCentralProcess(vcm);
	ConnectionServer::addConnectedClient(m_suid, this);

	uint32 const requiredSubscriptionBits = ConfigConnectionServer::getRequiredSubscriptionBits();
	if (requiredSubscriptionBits != 0)
	{
		if ((subscriptionFeatures & requiredSubscriptionBits) != requiredSubscriptionBits)
		{
			LOG("ClientDisconnect", ("Suid %d (%s) by session denial reason 'Invalid Subscription Bits'.", suid, username.c_str()));
			LOG("CustomerService", ("Login: %s by session denial reason 'Invalid Subscription Bits'.", describeAccount(this).c_str()));
			disconnect();
			return;
		}
	}

	uint32 const requiredGameBits = ConfigConnectionServer::getRequiredGameBits();
	if (requiredGameBits != 0)
	{
		if ((gameFeatures & requiredGameBits) != requiredGameBits)
		{
			LOG("ClientDisconnect", ("Suid %d (%s) by session denial reason 'Invalid Game Bits'.", suid, username.c_str()));
			LOG("CustomerService", ("Login: %s by session denial reason 'Invalid Game Bits'.", describeAccount(this).c_str()));
			disconnect();
			return;
		}
	}

	// tell client the server-side game and subscription feature bits and which ConnectionServer we are and the current server Epoch time
	GenericValueTypeMessage<std::pair<std::pair<unsigned long, unsigned long>, std::pair<int, int32> > > const msgFeatureBits("AccountFeatureBits", std::make_pair(std::make_pair(gameFeatures, subscriptionFeatures), std::make_pair(ConfigConnectionServer::getConnectionServerNumber(), static_cast<int32>(::time(NULL)))));
	send(msgFeatureBits, true);

	std::string const gameFeaturesDescription = ClientGameFeature::getDescription(gameFeatures);
	std::string const subscriptionFeaturesDescription = ClientSubscriptionFeature::getDescription(subscriptionFeatures);

	LOG("CustomerService", ("Login:%s at IP: %s:%hu has connected with game code 0x%x (%s) and sub code 0x%x (%s) (entitlement total: %u/%u, since last login: %u/%u)", describeAccount(this).c_str(), getRemoteAddress().c_str(), getRemotePort(), gameFeatures, gameFeaturesDescription.c_str(), subscriptionFeatures, subscriptionFeaturesDescription.c_str(), m_entitlementEntitledTime, m_entitlementTotalTime, m_entitlementEntitledTimeSinceLastLogin, m_entitlementTotalTimeSinceLastLogin));

	// ask CentralServer to tell all other ConnectionServers on this galaxy to drop duplicate connections for this account
	// and ask CentralServer (via LoginServer) to tell all ConnectionServers on other galaxies to drop duplicate connections for this account
	if (!m_usingAdminLogin && !m_isSecure)
	{
		GenericValueTypeMessage<std::pair<uint32, std::string> > const dropDuplicateConnections("ConnSrvDropDupeConns", std::make_pair(m_suid, m_sessionId));
		ConnectionServer::sendToCentralProcess(dropDuplicateConnections);
	}
}

//-----------------------------------------------------------------------

std::string ClientConnection::describeAccount(const ClientConnection * c)
{
	std::string result = "";
	if(c)
	{
		char idbuf[512] = {"\0"};
		const std::string & sessionId = c->getSessionId();
		if (sessionId.empty())
		{
			snprintf(idbuf, sizeof(idbuf), " (%lu)", c->m_suid);
		}
		else
		{
			snprintf(idbuf, sizeof(idbuf), " (%lu, %s)", c->m_suid, sessionId.c_str());
		}

		result = c->m_accountName;
		result += idbuf;
	}
	return result;
}

// ----------------------------------------------------------------------

std::vector<std::pair<NetworkId, std::string> > const & ClientConnection::getConsumedRewardEvents() const
{
	return m_consumedRewardEvents;
}

// ----------------------------------------------------------------------

std::vector<std::pair<NetworkId, std::string> > const & ClientConnection::getClaimedRewardItems() const
{
	return m_claimedRewardItems;
}

// ----------------------------------------------------------------------

bool ClientConnection::isUsingAdminLogin() const
{
	return m_usingAdminLogin;
}

// ----------------------------------------------------------------------

int ClientConnection::getBuddyPoints() const
{
	return m_buddyPoints;
}

// ======================================================================
