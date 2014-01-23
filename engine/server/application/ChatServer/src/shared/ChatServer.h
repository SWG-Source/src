// ChatServer.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

#ifndef	_INCLUDED_ChatServer_H
#define	_INCLUDED_ChatServer_H

//-----------------------------------------------------------------------

#include "ChatAPI/ChatAvatar.h"
#include "ChatAPI/ChatAPI.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include <unordered_map>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class CentralServerConnection;
class ChatInterface;
class Connection;
class ConnectionServerConnection;
class EnumerateServers;
class Service;
class GameNetworkMessage;
class GameServerConnection;
class ChatServerRoomOwner;
class CustomerServiceServerConnection;
struct ChatLogEntry;
class TransferCharacterData;
class VChatInterface;

using namespace ChatSystem;


namespace MessageDispatch {
	class Callback;
}

struct AvatarSequencePair
{
	AvatarSequencePair(unsigned s, const ChatAvatar *a) : sequence(s), avatar(a) {}
	unsigned sequence;
	const ChatAvatar *avatar;
};

struct AvatarIdSequencePair
{
	AvatarIdSequencePair(unsigned s, const ChatAvatarId &a) : sequence(s), avatar(a) {}
	unsigned sequence;
	const ChatAvatarId avatar;

private:
	AvatarIdSequencePair(AvatarIdSequencePair const &);
	AvatarIdSequencePair & operator =(AvatarIdSequencePair const &);
};

struct RoomOwnerSequencePair
{
	RoomOwnerSequencePair(unsigned s, const ChatServerRoomOwner * r) : sequence(s), roomOwner(r) {}
	unsigned sequence;
	const ChatServerRoomOwner *roomOwner;
};

struct ReturnAddress;

class ChatServer
{
public:
	ChatServer();
	~ChatServer();

	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------

	void sendResponse(ReturnAddress const & requester, const GameNetworkMessage & response);

	GameServerConnection * getGameServerConnectionFromId(unsigned int connectionId);
	unsigned registerGameServerConnection(GameServerConnection * connection);
	void unregisterGameServerConnection(unsigned const connectionId);
	void sendToGameServerById(unsigned const connectionId, GameNetworkMessage const & message);
	
	static VChatInterface* getVChatInterface();

	//TODO: remove these and integrate their functionality where it should rightly go (voice and text should be unified)
	static void requestGetChannel(ReturnAddress const & requester, std::string const &roomName, bool isPublic, bool isPersistant, uint32 limit, std::list<std::string> const & moderators);
	static void requestChannelInfo(ReturnAddress const & requester, std::string const &roomName);
	static void requestDeleteChannel(ReturnAddress const & requester, std::string const & roomName);
	static void requestAddClientToChannel(const NetworkId & id, std::string const & playerName, std::string const &roomName, bool forceShortlist);
	static void requestRemoveClientFromChannel(const NetworkId & id, std::string const & playerName, std::string const &roomName);
	static void requestChannelCommand(ReturnAddress const & requester, const std::string &srcUserName, const std::string &destUserName, 
		const std::string &destChannelAddress, unsigned command, unsigned banTimeout);

	static void requestBroadcastChannelMessage(std::string const & channelName, std::string const & textMessage, bool isRemove);

	static bool getVoiceChatLoginInfoFromId(NetworkId const & id, std::string & userName, std::string & playerName);
	static bool getVoiceChatLoginInfoFromName(std::string const & playerName, NetworkId & id);
	static bool getVoiceChatLoginInfoFromLoginName(std::string const & loginName, NetworkId & id);
	static void voiceChatGotLoginInfo(NetworkId const & id, std::string const & userName, std::string const & playerName);

	static void requestInvitePlayerToChannel(NetworkId const & sourceId, NetworkId const & targetId, std::string const & channelName);
	static void requestKickPlayerFromChannel(NetworkId const & sourceId, NetworkId const & targetId, std::string const & channelName);


	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------


	std::unordered_map<unsigned, NetworkId>                  pendingRequests;
	static ChatServer &  instance                   ();

	static bool        isValidChatAvatarName(Unicode::String const &chatName);
	static std::string getFullChatAvatarName(ChatAvatar const *chatAvatar);
	static std::string toNarrowString(ChatUnicodeString const &chatUnicodeString);
	static std::string getConnectionAddress(Connection const *connection);
	static Unicode::String getChatRoomName(ChatRoom const *chatRoom);
	static std::string getChatRoomNameNarrow(ChatRoom const *chatRoom);
	static Unicode::String toUnicodeString(ChatUnicodeString const &chatUnicodeString);

	static const ChatAvatar * getAvatarByNetworkId  (const NetworkId & id);
	static const NetworkId &  getNetworkIdByAvatarId(const ChatAvatarId & id);
	static NetworkId  sendResponseForTrackId    (unsigned trackId, const GameNetworkMessage & msg);
	static ChatInterface * getChatInterface();

	static void       setOwnerSystem            (const ChatAvatar * ownerSystem);
	static ConnectionServerConnection *  getConnectionForCharacter(const NetworkId & characterId);
	static const ChatAvatarId &  getSystemAvatarId  ();
	static const Service *  getGameService      ();
	static GameServerConnection       *  getGameServerConnection(unsigned int sequence);
	static void           addGameServerConnection(unsigned int sequence, GameServerConnection *connection);

	static void           clearGameServerConnection(const GameServerConnection *connection);
	static void           clearCustomerServiceServerConnection();

	static void           connectToCustomerServiceServer(const std::string &address, const unsigned short port);

	static void       sendToClient              (const NetworkId & id, const GameNetworkMessage & msg);
	static void       deferChatMessageFor       (const NetworkId &, const Archive::ByteStream &);

	static ChatAvatarId getAvatarIdForTrackId   (unsigned trackId);

	static void       reconnectToCentralServer  ();
	static void       onCentralServerConnectionClosed();

	static void       run                       ();
	static void       quit                      ();
	static void       chatConnectedAvatar       (const NetworkId & id, const ChatAvatar & avatar);

	static void       connectPlayer             (ConnectionServerConnection * connection, const unsigned int suid, const std::string & characterName, const NetworkId & networkId, const bool isSecure, const bool isSubscribed);
	static void       createRoom                (const NetworkId & id, const unsigned int sequence, const std::string & name, const bool isModerated, const bool isPublic, const std::string & title);
	static void       putSystemAvatarInRoom     (const std::string & roomName);
	static void       enterRoom                 (const NetworkId & id, const unsigned int sequence, const std::string & roomName);
	static void       enterRoom                 (const NetworkId & id, const unsigned int sequence, const unsigned int roomId);
	static void       enterRoom                 (const ChatAvatarId & id, const std::string & roomName, bool forceCreate, bool createPrivate);
	static void       sendInstantMessage        (const NetworkId & fromId, const unsigned int sequence, const ChatAvatarId & to, const Unicode::String & message, const Unicode::String & oob);
	static void       sendInstantMessage        (const ChatAvatarId & from, const ChatAvatarId & to, const Unicode::String & message, const Unicode::String & oob);
	static void       leaveRoom                 (const NetworkId & id, const unsigned int sequence, const unsigned int roomId);
	static void       removeAvatarFromRoom      (const NetworkId & requestor, const ChatAvatarId & avatarId, const std::string & roomName);
	static void       removeSystemAvatarFromRoom(const ChatRoom *room);
	static void       removeAvatarFromRoom      (const ChatAvatarId & avatarId, const std::string & roomName);
	static void       kickAvatarFromRoom        (const NetworkId & id, const ChatAvatarId & avatarId, const std::string & roomName);
	static void       sendRoomMessage           (const NetworkId & id, const unsigned int sequence, const unsigned int roomId, const Unicode::String & message, const Unicode::String & oob);
	static void       sendRoomMessage           (const ChatAvatarId & from, const std::string & roomName, const Unicode::String & message, const Unicode::String & oob);
	static void       sendStandardRoomMessage   (const ChatAvatarId & from, const std::string & roomName, const Unicode::String & message, const Unicode::String & oob);
	static void       destroyRoom               (const NetworkId & id, const unsigned int sequence, const unsigned int roomId);
	static void       destroyRoom               (const std::string & roomName);
	static void       addFriend                 (const NetworkId & id, const unsigned int sequence, const ChatAvatarId & friendName);
	static void       removeFriend              (const NetworkId & id, const unsigned int sequence, const ChatAvatarId & friendName);
	static void       getFriendsList            (const ChatAvatarId & characterName);
	static void       addIgnore                 (const NetworkId & id, const unsigned int sequence, const ChatAvatarId & ignoreName);
	static void       removeIgnore              (const NetworkId & id, const unsigned int sequence, const ChatAvatarId & ignoreName);
	static void       getIgnoreList             (const ChatAvatarId & characterName);
	static void       disconnectAvatar          (const ChatAvatar &);
	static void       disconnectPlayer          (const NetworkId &);
	static void       invite                    (const NetworkId & id, const ChatAvatarId & avatar, const std::string & roomName);
	static void       inviteGroupMembers        (const NetworkId & id, const ChatAvatarId & avatar, const std::string & roomName, const stdvector<NetworkId>::fwd & members);
	static void       uninvite                  (const NetworkId & id, const unsigned int sequence, const ChatAvatarId & avatar, const std::string & roomName);
	static void       queryRoom                 (const NetworkId & id, ConnectionServerConnection * connection, const unsigned int sequence, const std::string & roomName);
	static void       requestRoomList           (const NetworkId & id, ConnectionServerConnection * connection);
	static void       addModeratorToRoom        (const unsigned int sequenceId, const NetworkId & id, const ChatAvatarId & avatarId, const std::string & roomName);
	static void       removeModeratorFromRoom   (const unsigned int sequenceId, const NetworkId & id, const ChatAvatarId & avatarId, const std::string & roomName);
	static void       deletePersistentMessage   (const NetworkId & ownerId, const unsigned int messageId);
	static void       deleteAllPersistentMessages(const NetworkId &sourceNetworkId, const NetworkId &targetNetworkId);
	static void       sendPersistentMessage     (const NetworkId & fromId, const unsigned int sequence, const ChatAvatarId & to, const Unicode::String & subject, const Unicode::String & message, const Unicode::String & oob);
	static void       sendPersistentMessage     (const ChatAvatarId & from, const ChatAvatarId & to, const Unicode::String & subject, const Unicode::String & message, const Unicode::String & oob);
	static void       requestPersistentMessage  (const NetworkId &id, const unsigned int sequence, const unsigned int messageId);
	static void       banFromRoom               (const unsigned sequence, const NetworkId &banner, const ChatAvatarId &bannee, const std::string &roomName);
	static void       unbanFromRoom             (const unsigned sequence, const NetworkId &banner, const ChatAvatarId &bannee, const std::string &roomName);
	static void       removeConnectionServerConnection(ConnectionServerConnection * target);

	static void       sendToAllConnectionServers(const GameNetworkMessage &message);
	static void       broadcastToGameServers(const GameNetworkMessage &message);

	static bool getChatLog(Unicode::String const &player, std::vector<ChatLogEntry> &chatLog);
	static void logChatMessage(Unicode::String const &logPlayer, Unicode::String const &fromPlayer, Unicode::String const &toPlayer, Unicode::String const &text, Unicode::String const &channel);
	static Unicode::String const &getChannelTell();
	static Unicode::String const &getChannelEmail();

	static void fileLog(bool const forceLog, char const * const label, char const * const format, ...);
	static void requestTransferAvatar(const TransferCharacterData & request);
	static void       onCreateRoomSuccess(const std::string & lowerName, const unsigned int roomId);
	static bool isGod(const NetworkId & networkId);

	static void setUnsquelchTime(NetworkId const & character, time_t unsquelchTime);
	static void handleChatStatisticsFromGameServer(NetworkId const & character, time_t unsquelchTime, time_t chatSpamTimeEndInterval, int spatialNumCharacters, int nonSpatialNumCharacters);
private:

	void onEnumerateServers(const EnumerateServers &);
	void update();
	void removeServices();

	struct AvatarExtendedData
	{
		ChatAvatar chatAvatar;
		bool isSubscribed;
		int spatialCharCount;
		int nonSpatialCharCount;
		time_t chatSpamTimeEndInterval;
		time_t chatSpamNextTimeToSyncWithGameServer;
		time_t chatSpamNextTimeToNotifyPlayerWhenLimited;
		time_t unsquelchTime;
	};
	typedef std::unordered_map<NetworkId, AvatarExtendedData, NetworkId::Hash> ChatAvatarList;


	struct VoiceChatAvatarData
	{
		std::string loginName;
		std::string playerName;
	};

	typedef std::unordered_map<NetworkId, VoiceChatAvatarData, NetworkId::Hash> VoiceChatAvatarList;

	static AvatarExtendedData * getAvatarExtendedDataByNetworkId(const NetworkId & id);

	MessageDispatch::Callback *                                       callback;
	CentralServerConnection *                                         centralServerConnection;
	ChatAvatarList                                                    chatAvatars;
	ChatInterface *                                                   chatInterface;
	VChatInterface *                                                  voiceChatInterface;
	std::unordered_map<NetworkId, ConnectionServerConnection *, NetworkId::Hash>  clientMap;
	std::unordered_map<unsigned int, GameServerConnection *>                      gameServerConnectionMap;
	std::set<ConnectionServerConnection *>              connectionServerConnections;
	
	bool                             done;
	Service *                        gameService;
	Service *                        planetService;
	const ChatAvatar *               ownerSystem;
	ChatAvatarId                     systemAvatarId;
	static ChatServer *              m_instance;
	CustomerServiceServerConnection *customerServiceServerConnection;

	typedef std::unordered_map<unsigned int, GameServerConnection *> GameServerMap;
	GameServerMap m_gameServerConnectionRegistry;
	VoiceChatAvatarList m_voiceChatIdMap;
	std::map<std::string, NetworkId> m_voiceChatNameToIdMap;
	std::map<std::string, NetworkId> m_voiceLoginNameToIdMap;

	// Disabled

	ChatServer &operator =(ChatServer const &);
};

/*
class ChatServer
{
public:




private:
	ChatServer & operator = (const ChatServer & rhs);
	ChatServer(const ChatServer & source);

private:
	MessageDispatch::Callback *                         callback;
	CentralServerConnection *                           centralServerConnection;
	std::hash_map<NetworkId, const ChatAvatar *, NetworkId::Hash>        chatAvatars;
	ChatInterface *                                     chatInterface;
	std::hash_map<NetworkId, ConnectionServerConnection *, NetworkId::Hash>  clientMap;
	std::hash_map<unsigned int, GameServerConnection *>                      gameServerConnectionMap;
	std::set<ConnectionServerConnection *>              connectionServerConnections;
	bool                                                done;
	Service *                                           gameService;
	Service *                                           planetService;
	const ChatAvatar *                                  ownerSystem;
	ChatAvatarId                                        systemAvatarId;
	std::hash_map<unsigned, NetworkId>               pendingRequests;
	static ChatServer *                                 m_instance;

};*/

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatServer_H
