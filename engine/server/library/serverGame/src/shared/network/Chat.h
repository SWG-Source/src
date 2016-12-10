// Chat.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_Chat_H
#define	_INCLUDED_Chat_H

//-----------------------------------------------------------------------

#include "Archive/ByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

struct ChatAvatarId;
class ChatServerConnection;
class CreatureObject;
class NetworkId;
class ProsePackage;
class ServerObject;
class StringId;

//-----------------------------------------------------------------------

class Chat
{
public:
	~Chat();

//TODO: integrate these properly
	static void requestGetChannel(std::string const & roomName, std::string const & displayName, std::string const & password = "", bool persistant = false, uint32 limit = 100);
	static void requestGetChannel(std::string const & roomName, bool isPublic = true, bool persistant = false, uint32 limit = 100);
	static void requestDestroyChannel(std::string const & roomName);
	static void requestAddClientToChannel(NetworkId const & clientId, std::string const & playerName, std::string const & roomName, bool forceShortlist);
	static void requestRemoveClientFromChannel(NetworkId const & clientId, std::string const & playerName, std::string const & roomName);
	static void requestChannelCommand(std::string const & srcUser, std::string const & destUser, std::string const & destChan, uint32 commandType, uint32 banTimeout);

	static void requestBroadcastGlobalChannelMessage(std::string const & channelName, std::string const & message, bool isRemove, bool sendToAllClusters);

	static bool debugTellClientAboutRoom(const NetworkId & clientId, std::string const& roomName, std::string const & uri, std::string const & password = "", bool autoJoin = true, bool leaveCommand = false);
//END TODO


	static void addFriend                (const std::string & player, const std::string & friendName);
	static void removeFriend             (const std::string & player, const std::string & friendName);
	static void getFriendsList           (const std::string & player);
	static void addIgnore                (const std::string & player, const std::string & ignoreName);
	static void removeIgnore             (const std::string & player, const std::string & ignoreName);
	static void getIgnoreList            (const std::string & player);
	static void addModeratorToRoom       (const std::string & who, const std::string & roomPath);
	static void createRoom               (const std::string & ownerName, const bool isPublic, const std::string & roomPath, const std::string & roomTitle);
	static void createSystemRooms        (const std::string & galaxyName, const std::string & planetName);
	static void destroyRoom              (const std::string & roomPath);
	static void enterRoom                (const std::string & who, const std::string & roomPath, bool forceCreate, bool createPrivate);
	static void exitRoom                 (const std::string & who, const std::string & roomPath);
	static void removeModeratorFromRoom  (const std::string & who, const std::string & roomPath);
	static void sendInstantMessage       (const std::string & from, const std::string & to, const Unicode::String & message, const Unicode::String & oob);
	static void emptyMail                (const NetworkId &sourceNetworkId, const NetworkId &targetNetworkId);

	static Unicode::String & makePersistentMessageBody     (Unicode::String & target, const ProsePackage & pp);

	static void sendPersistentMessage    (const std::string & from,  const std::string & to, const Unicode::String & subject, const Unicode::String & message, const Unicode::String & oob);
	static void sendPersistentMessage    (const ServerObject & from, const std::string & to, const StringId & subject,        const Unicode::String & message, const Unicode::String & oob);
	static void sendPersistentMessage    (const ServerObject & from, const std::string & to, const StringId & subject,        const StringId & message,        const Unicode::String & oob);

	static void sendToChatServer         (const GameNetworkMessage & msg);
	static void sendToRoom               (const std::string & from, const std::string & roomPath, const Unicode::String & message, const Unicode::String & oob);
	static void broadcastSystemMessageToCluster (const Unicode::String & message, const Unicode::String & oob);

	static void sendQuestSystemMessage(ServerObject const & to, Unicode::String const & message, Unicode::String const & oob);

	static void sendSystemMessage        (const ServerObject & to, const StringId & message, const Unicode::String & oob);
	static void sendSystemMessage        (const ServerObject & to, const Unicode::String & message, const Unicode::String & oob);
	static void sendSystemMessage        (const ServerObject & to, const ProsePackage & pp);
	static void sendSystemMessageSimple  (const ServerObject & to, const StringId & message, const ServerObject * target);

	static void sendSystemMessage        (const std::string & to, const StringId & message, const Unicode::String & oob);
	static void sendSystemMessage        (const std::string & to, const Unicode::String & message, const Unicode::String & oob);
	static void sendSystemMessage        (const std::string & to, const ProsePackage & pp);
	static void invite                   (const std::string & to, const std::string & room); 
	static void uninvite                 (const std::string & to, const std::string & room); 
	static void inviteGroupMembers       (const NetworkId & invitor, const ChatAvatarId & groupLeader, const std::string & room, const std::vector<NetworkId> & members); 

	static unsigned int isAllowedToEnterRoom(const CreatureObject & who, const std::string & room);

	static void setChatServer            (ChatServerConnection * chatServerConnection);

	static const std::string & getGameCode ();

	static const ChatAvatarId             constructChatAvatarId (const ServerObject & to);
	static const ChatAvatarId             constructChatAvatarId (const std::string & to);

	static const ChatServerConnection *   getChatServer();

private:
	Chat();
	Chat & operator = (const Chat & rhs);
	Chat(const Chat & source);
	static Chat & instance();

	static Chat *                        m_instance;
	ChatAvatarId *                       m_serverAvatar;
	ChatServerConnection *               m_chatServer;	
	std::vector<Archive::ByteStream>  m_deferredChatMessages;
	std::string                          m_gameCode;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_Chat_H
