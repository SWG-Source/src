// ClientConnection.h
// copyright 2001 Verant Interactive

#ifndef	_ClientConnection_H
#define	_ClientConnection_H

//-----------------------------------------------------------------------

#include "Client.h"
#include "Unicode.h"
#include "serverUtility/ServerConnection.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/StationId.h"
#include "sharedMath/Vector.h"
#include <map>

class ClientIdMsg;
class SelectCharacter;
class GameConnection;
class GameClientMessage;
class ConnectionCreateCharacter;

//-----------------------------------------------------------------------

class ClientConnection : public ServerConnection
{
public:
	struct CharacterData
	{
		Unicode::String  name;
		std::string      narrowName;
		std::string      location;
		std::string      objectTemplate;
		NetworkId        characterId;
		NetworkId        containerId;
		Vector           coordinates;     
	};

	explicit ClientConnection(UdpConnectionMT *, TcpClient *);
	virtual ~ClientConnection();

	void               addCharacter (const Unicode::String & name,
	                                 const NetworkId &characterId,
	                                 const std::string & location,
	                                 const std::string & objectTemplate,
	                                 const NetworkId &containerId,
	                                 const Vector & coordinates);

	const std::string &           getAccountName           () const;
	const NetworkId &             getCharacterId           () const;
	const std::string &           getCharacterName         () const;
	void                          setStartPlayTime         (time_t startPlayTime);
	time_t                        getStartPlayTime         () const;
	std::string                   getPlayTimeDuration      () const;
	std::string                   getActivePlayTimeDuration() const;
	std::string                   getCurrentActivePlayTimeDuration() const;
	void                          sendPlayTimeInfoToGameServer() const;
	const Client *                getClient                () const;
	Client *                      getClient                ();
	void                          setClient                (Client* client);

	uint32                        getSUID                  () const;

	const bool                    getHasSelectedCharacter  () const;
	bool                          getHasBeenSentToGameServer () const;
	bool                          getIsSecure              () const;
	const std::string &           getSessionId() const;
	const std::string &           getTargetSecene          () const;
	unsigned int                  getGameFeatures() const;
	unsigned int                  getSubscriptionFeatures() const;
	bool                          getHasCSLoggedAccountFeatureIds() const;
	void                          setHasCSLoggedAccountFeatureIds(bool hasCSLoggedAccountFeatureIds);
	unsigned int                  getEntitlementTotalTime  () const;
	unsigned int                  getEntitlementEntitledTime () const;
	unsigned int                  getEntitlementTotalTimeSinceLastLogin () const;
	unsigned int                  getEntitlementEntitledTimeSinceLastLogin () const;
	int                           getBuddyPoints() const;
	std::vector<std::pair<NetworkId, std::string> > const & getConsumedRewardEvents() const;
	std::vector<std::pair<NetworkId, std::string> > const & getClaimedRewardItems() const;
	bool                          isUsingAdminLogin        () const;
	bool                          getCanSkipTutorial        () const;	

	virtual void                  onConnectionClosed       ();
	virtual void                  onConnectionOpened       ();
	virtual void                  onConnectionOverflowing  (const unsigned int bytesPending);
	virtual void                  onReceive                (const Archive::ByteStream & message);
	virtual void                  send                     (const GameNetworkMessage & message, const bool reliable);
	void                          sendByteStream           (const Archive::ByteStream &bs, bool reliable);
	const bool                    sendToGameServer         (uint32 gameServerId);
	const bool                    sendToGameServer         ();

	void                          handleGameServerForLoginMessage(uint32 serverId);
	void                          onIdValidated(bool canLogin, bool canCreateRegularCharacter, bool canCreateJediCharacter, bool canSkipTutorial, std::vector<std::pair<NetworkId, std::string> > const & consumedRewardEvents, std::vector<std::pair<NetworkId, std::string> > const & claimedRewardItems);
	void                          onValidateClient    (StationId id, const std::string & username, bool, const char*, uint32 gameFeatures, uint32 subscriptionFeatures, unsigned int entitlementTotalTime, unsigned int entitlementEntitledTime, unsigned int entitlementTotalTimeSinceLastLogin, unsigned int entitlementEntitledTimeSinceLastLogin, int buddyPoints);
	void                          onCharacterValidated(bool isValid, const NetworkId &character, const std::string &characterName, const NetworkId &container, const std::string &scene, const Vector &coordinates);
	static std::string            describeAccount(const ClientConnection *);

	static std::map< std::string, uint32 >&  getPacketBytesPerMinStats();

	void                          handleChatEnterRoomValidationResponse(unsigned int sequence, unsigned int result);
	void                          handleChatQueryRoomValidationResponse(unsigned int sequence, bool success);

	void                          setHasRequestedCharacterCreate(bool value);
	void                          setHasCreatedCharacter(bool value);

private:
	ClientConnection();
	ClientConnection(const ClientConnection&);
	ClientConnection& operator=(const ClientConnection&);
	void                          handleLagRequest         ();
	bool                          checkSpamLimit           (unsigned int messageSize);

	static std::map< std::string, uint32 >   sm_outgoingBytesMap_Working;  // working stats that will rotate after 1 minute 
	static std::map< std::string, uint32 >   sm_outgoingBytesMap_Stats;    // computed stats from the last minute 
	static uint32				 sm_outgoingBytesMap_Worktime; // time we started filling in the working map

	std::string                       m_accountName;
	bool                              m_canCreateRegularCharacter;
	bool                              m_canCreateJediCharacter;
	
	// to prevent exploit, a connection to the client is allowed to only request
	// creating a new character once; we will disconnect the connection if we
	// receive a second request; this will force the client to connect again,
	// at which time another validation will be done to see if creating a new
	// character is allowed
	bool                              m_hasRequestedCharacterCreate;

	// to prevent exploit, a connection to the client is allowed to
	// create a new character once; we will disconnect the connection if we
	// receive a request to create another character; this will force the
	// client to connect again, at which time another validation will be
	// done to see if creating a new character is allowed
	bool                              m_hasCreatedCharacter;

	// hang on to a character create request while we check with the LoginServer
	// one last time to make sure the account can create a new character
	ConnectionCreateCharacter *       m_pendingCharacterCreate;

	bool                              m_canSkipTutorial;
	NetworkId                         m_characterId;
	std::string                       m_characterName;
	time_t                            m_startPlayTime; // time when the player started playing the character
	time_t                            m_lastActiveTime; // the client will detect when the player is "active" or "inactive"; this keeps track of the last time that the client said the player was "active"; if 0, it means the client is currently "inactive"
	unsigned long                     m_activePlayTimeDuration; // total amount of play time player was active (i.e. at the mouse/keyboard/joystick)
	Client *                          m_client;
	NetworkId                         m_containerId;
	uint32                            m_featureBitsGame;
	uint32                            m_featureBitsSubscription;
	uint32                            m_gameBitsToClear;
	bool                              m_hasBeenSentToGameServer;
	bool                              m_hasBeenValidated;
	bool                              m_hasSelectedCharacter;
	bool                              m_isSecure;
	bool                              m_isAdminAccount; // Note:  means this account is on the admin list, not that the account has god powers in this session.  Using SecureId and being on the right IP subnet are also required to run god commands.  m_isAdminAccount will be true for admins playing from home, for example.
	bool                              m_hasCSLoggedAccountFeatureIds;
	StationId                         m_suid;
	StationId                         m_requestedSuid;
	bool                              m_usingAdminLogin;
	Vector                            m_targetCoordinates;
	std::string                       m_targetScene;
	bool                              m_validatingCharacter;
	unsigned int                      m_receiveHistoryBytes;
	unsigned int                      m_receiveHistoryPackets;
	unsigned long                     m_receiveHistoryMs;
	unsigned long                     m_receiveLastTimeMs;
	mutable unsigned long             m_sendLastTimeMs;
	std::string                       m_sessionId;
	bool                              m_sessionValidated;
	int                               m_connectionServerLag;
	int                               m_gameServerLag;
	unsigned long                     m_countSpamLimitResetTime;
	unsigned int                      m_entitlementTotalTime;
	unsigned int                      m_entitlementEntitledTime;
	unsigned int                      m_entitlementTotalTimeSinceLastLogin;
	unsigned int                      m_entitlementEntitledTimeSinceLastLogin;
	int                               m_buddyPoints;
	std::vector<std::pair<NetworkId, std::string> > m_consumedRewardEvents;
	std::vector<std::pair<NetworkId, std::string> > m_claimedRewardItems;

	bool				  m_sendToStarport;

	// chat enter room requests that came from the client that's awaiting
	// game sever approval before being forwarded to the chat server
	std::map<unsigned long, GameClientMessage*> m_pendingChatEnterRoomRequests;

	// ChatQueryRoom requests that came from the client that's awaiting
	// game sever approval before being forwarded to the chat server
	std::map<unsigned long, GameClientMessage*> m_pendingChatQueryRoomRequests;

	//Message handler functions
	void                              handleClientIdMessage(const ClientIdMsg& msg);
	void                              handleSelectCharacterMessage(const SelectCharacter& msg);
	bool                              validateSelection(const Unicode::String & characterName);

	bool                              sendToGameServer(GameConnection *c);
};

//-----------------------------------------------------------------------

inline const std::string & ClientConnection::getAccountName() const
{
	return m_accountName;
}

//-----------------------------------------------------------------------

inline void ClientConnection::setStartPlayTime(time_t startPlayTime)
{
	m_startPlayTime = startPlayTime;
}

//-----------------------------------------------------------------------

inline time_t ClientConnection::getStartPlayTime() const
{
	return m_startPlayTime;
}

//-----------------------------------------------------------------------

inline const Client* ClientConnection::getClient() const
{
	return m_client;
}

//-----------------------------------------------------------------------

inline Client* ClientConnection::getClient()
{
	return m_client;
}

//-----------------------------------------------------------------------

inline const bool ClientConnection::getHasSelectedCharacter() const
{
	return m_hasSelectedCharacter;
}

//-----------------------------------------------------------------------

inline const std::string & ClientConnection::getTargetSecene() const
{
	return m_targetScene;
}

//-----------------------------------------------------------------------

inline uint32 ClientConnection::getSUID() const
{
	return m_suid;
}

//-----------------------------------------------------------------------

inline bool ClientConnection::getHasBeenSentToGameServer() const
{
	return m_hasBeenSentToGameServer;
}

// ----------------------------------------------------------------------

inline bool ClientConnection::getIsSecure() const
{
	return m_isSecure;
}

// ----------------------------------------------------------------------

inline const std::string & ClientConnection::getSessionId() const
{
	return m_sessionId;
}

// ----------------------------------------------------------------------

inline unsigned int ClientConnection::getGameFeatures() const
{
	return m_featureBitsGame;
}

// ----------------------------------------------------------------------

inline unsigned int ClientConnection::getSubscriptionFeatures() const
{
	return m_featureBitsSubscription;
}

// ----------------------------------------------------------------------

inline unsigned int ClientConnection::getEntitlementTotalTime() const
{
	return m_entitlementTotalTime;
}

// ----------------------------------------------------------------------

inline unsigned int ClientConnection::getEntitlementEntitledTime() const
{
	return m_entitlementEntitledTime;
}

// ----------------------------------------------------------------------

inline unsigned int ClientConnection::getEntitlementTotalTimeSinceLastLogin() const
{
	return m_entitlementTotalTimeSinceLastLogin;
}

// ----------------------------------------------------------------------

inline unsigned int ClientConnection::getEntitlementEntitledTimeSinceLastLogin() const
{
	return m_entitlementEntitledTimeSinceLastLogin;
}

// ----------------------------------------------------------------------

inline bool ClientConnection::getCanSkipTutorial() const
{
    return m_canSkipTutorial;
}

// ----------------------------------------------------------------------

inline void ClientConnection::setHasRequestedCharacterCreate(bool value)
{
	m_hasRequestedCharacterCreate = value;
}

// ----------------------------------------------------------------------

inline void ClientConnection::setHasCreatedCharacter(bool value)
{
	m_hasCreatedCharacter = value;
}

// ----------------------------------------------------------------------

inline bool ClientConnection::getHasCSLoggedAccountFeatureIds() const
{
	return m_hasCSLoggedAccountFeatureIds;
}

// ----------------------------------------------------------------------

inline void ClientConnection::setHasCSLoggedAccountFeatureIds(bool hasCSLoggedAccountFeatureIds)
{
	m_hasCSLoggedAccountFeatureIds = hasCSLoggedAccountFeatureIds;
}

// ----------------------------------------------------------------------

#endif	// _ClientConnection_H
