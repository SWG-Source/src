// ChatInterface.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstChatServer.h"
#include "ChatAPI/ChatAvatar.h"
#include "ChatInterface.h"
#include "ChatAPI/ChatFriendStatus.h"
#include "ChatAPI/ChatIgnoreStatus.h"
#include "ChatAPI/ChatRoom.h"
#include "ChatAPI/PersistentMessage.h"
#include "ChatAPI/ChatEnum.h"
#include "ChatServer.h"
#include "ChatServerAvatarOwner.h"
#include "ConfigChatServer.h"
#include "ConnectionServerConnection.h"
#include "GameServerConnection.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/Service.h"
#include "sharedNetworkMessages/ChatFriendsListUpdate.h"
#include "sharedNetworkMessages/ChatInstantMessageToClient.h"
#include "sharedNetworkMessages/ChatOnAddFriend.h"
#include "sharedNetworkMessages/ChatOnAddModeratorToRoom.h"
#include "sharedNetworkMessages/ChatOnBanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnChangeFriendStatus.h"
#include "sharedNetworkMessages/ChatOnChangeIgnoreStatus.h"
#include "sharedNetworkMessages/ChatOnConnectAvatar.h"
#include "sharedNetworkMessages/ChatOnCreateRoom.h"
#include "sharedNetworkMessages/ChatOnDeleteAllPersistentMessages.h"
#include "sharedNetworkMessages/ChatOnDestroyRoom.h"
#include "sharedNetworkMessages/ChatOnEnteredRoom.h"
#include "sharedNetworkMessages/ChatOnGetFriendsList.h"
#include "sharedNetworkMessages/ChatOnGetIgnoreList.h"
#include "sharedNetworkMessages/ChatOnInviteToRoom.h"
#include "sharedNetworkMessages/ChatOnKickAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnLeaveRoom.h"
#include "sharedNetworkMessages/ChatOnReceiveRoomInvitation.h"
#include "sharedNetworkMessages/ChatOnRemoveModeratorFromRoom.h"
#include "sharedNetworkMessages/ChatOnSendInstantMessage.h"
#include "sharedNetworkMessages/ChatOnSendPersistentMessage.h"
#include "sharedNetworkMessages/ChatOnSendRoomInvitation.h"
#include "sharedNetworkMessages/ChatOnSendRoomMessage.h"
#include "sharedNetworkMessages/ChatOnUnbanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnUninviteFromRoom.h"
#include "sharedNetworkMessages/ChatQueryRoomResults.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClient.h"
#include "sharedNetworkMessages/ChatQueryRoomResults.h"
#include "sharedNetworkMessages/ChatRoomData.h"
#include "sharedNetworkMessages/ChatRoomList.h"
#include "sharedNetworkMessages/ChatRoomMessage.h"
#include "sharedNetworkMessages/ChatSystemMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "StringId.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "UnicodeUtils.h"
#include <cstdio>

using namespace ChatSystem;

//----------------------------------------------------------------------

namespace ChatInterfaceNamespace
{
    char convertPmStatus (const int value)
	{
		if (value == ChatSystem::PERSISTENT_NEW)
			return 'N';
		else if (value == ChatSystem::PERSISTENT_UNREAD)
			return 'U';
		else if (value == ChatSystem::PERSISTENT_READ)
			return 'R';
		else if (value == ChatSystem::PERSISTENT_TRASH)
			return 'T';
		else if (value == ChatSystem::PERSISTENT_DELETED)
			return 'D';

		return 0;
	}

	// Control log output for chat room related methods
	bool const s_enableChatRoomLogs = false;

	static int s_intervalToSendHeadersToClientSeconds;
	static int s_maxHeadersToSendToClientPerInterval;

	// to prevent sending duplicate room chat message
	std::unordered_map<unsigned, std::pair<unsigned, unsigned> > s_mostRecentRoomChatMessage;
}

using namespace ChatInterfaceNamespace;

//----------------------------------------------------------------------

//----------------------------------------------------------------------

void splitRoomName(std::string roomName, Unicode::String &name, Unicode::String &address)
{
	if (roomName.find("SOE") != 0)
	{
		roomName = "SOE." + roomName;
	}

	size_t pos = roomName.find(".");
	while (pos != std::string::npos)
	{
		roomName[pos] = '+';
		pos = roomName.find(".");
	}

	pos = roomName.rfind("+");
	if (pos != std::string::npos)
	{
		address = Unicode::narrowToWide(roomName.substr(0, pos));
		name = Unicode::narrowToWide(roomName.substr(pos+1));
	}

	/*char *tmp = roomName.c_str();
	char *loc = tmp;

	int numPlus = 0;
	while (*loc != 0)
	{
		if (*loc == '+')
		{
			numPlus++;
		}
		if (numPlus == 3)
		{
			//i.e SOE.SWG.bria
			*loc = 0;
			address = Unicode::narrowToWide(std::string(tmp));
			name = Unicode::narrowToWide(std::string(loc+1));
		}
		++loc;
	*/
}

//-----------------------------------------------------------------------

void makeAvatarId(const ChatUnicodeString &name, const ChatUnicodeString &addr, ChatAvatarId &id)
{
	std::string address = Unicode::wideToNarrow(Unicode::String(addr.string_data, addr.string_length));
	if ((address.find("SOE") == 0) && (address.length() > 4))
	{
		address = address.substr(4);
	}
	else
	{
		DEBUG_WARNING(true, ("makeAvatarId() received an invalid address"));
	}

	char text[1024];
	snprintf(text, sizeof(text), "%s", address.c_str());

	char *loc = strstr(text, "+");
	if (loc)
	{
		*loc = 0;
		id.gameCode = text;
		id.cluster = (loc+1);
		id.name = Unicode::wideToNarrow(Unicode::String(name.string_data, name.string_length));
		//tmp, due to a bug in the backend.
		if (id.gameCode == "PS")
		{
			id.gameCode = "SWG";
		}
	}
}

//-----------------------------------------------------------------------

void makeAvatarId(const ChatAvatar & avatar, ChatAvatarId & id)
{
	makeAvatarId(avatar.getName(), avatar.getAddress(), id);
}

//-----------------------------------------------------------------------

void makeRoomName(const ChatUnicodeString &roomAddress, std::string &name)
{
	std::string narrowRoomAddress = Unicode::wideToNarrow(Unicode::String(roomAddress.string_data,roomAddress.string_length));

	if ((narrowRoomAddress.find("SOE") == 0) && (narrowRoomAddress.length() > 4))
	{
		narrowRoomAddress = narrowRoomAddress.substr(4);
	}

	name = narrowRoomAddress;

	size_t pos = name.find("+");
	while (pos != std::string::npos)
	{
		name[pos] = '.';
		pos = name.find("+");
	}
}

//-----------------------------------------------------------------------

void makeRoomName(const ChatRoom *room, std::string &name)
{
	name.clear();
	if (room)
	{
		makeRoomName(room->getAddress(), name);
	}
}

//-----------------------------------------------------------------------

void makeRoomName(const RoomSummary *room, std::string &name)
{
	name.clear();
	if (room)
	{
		makeRoomName(room->getRoomAddress(), name);
	}
}

//-----------------------------------------------------------------------

void makeRoomData(const ChatRoom & room, ChatRoomData & roomData)
{
	NOT_NULL(&room);
	roomData.id = room.getRoomID();

	if (room.getRoomAttributes() & ROOMATTR_PRIVATE)
	{
		roomData.roomType = CHAT_ROOM_PRIVATE;
	}
	else
	{
		roomData.roomType = CHAT_ROOM_PUBLIC;
	}

	if (room.getRoomAttributes() & ROOMATTR_MODERATED)
	{
		roomData.moderated = 1;
	}
	makeRoomName(&room, roomData.path);
	makeAvatarId(room.getCreatorName(), room.getCreatorAddress(), roomData.owner);
	makeAvatarId(room.getCreatorName(), room.getCreatorAddress(), roomData.creator);

	roomData.title = Unicode::String(room.getRoomTopic().string_data, room.getRoomTopic().string_length);

	roomData.moderators.clear();

	ModeratorIterator m = room.getFirstModerator();
	for(; !m.outOfBounds(); ++m)
	{
		ChatAvatarId id;
		makeAvatarId(*(*m), id);
		roomData.moderators.push_back(id);
	}

	roomData.invitees.clear();
	InviteIterator i = room.getFirstInvited();
	for(; !i.outOfBounds(); ++i)
	{
		ChatAvatarId id;
		makeAvatarId(*(*i), id);
		roomData.invitees.push_back(id);
	}
}

//-----------------------------------------------------------------------

ChatInterface::ChatInterface( const std::string & strGameCode,
							 const std::string & strGatewayServerIP,
							 const unsigned short sGatewayServerPort,
							 const std::string & registrarHost,
							 const unsigned short registrarPort) :
		ChatAPI(registrarHost.c_str(), registrarPort, strGatewayServerIP.c_str(), sGatewayServerPort),
		avatarMap(),
		pendingAvatars(),
		pendingRoomQueries(),
		roomQueriesThisFrame(0),
		roomList(),
		deferredChatMessages(),
		queuedHeaders(),
		trackingRequestGetAnyAvatarForDestroy()
{
	ChatServer::fileLog(true, "ChatInterface", "connecting to API (gameCode=%s, gatewayServer=%s:%d registrar=%s:%d", strGameCode.c_str(), strGatewayServerIP.c_str(), sGatewayServerPort, registrarHost.c_str(), registrarPort);
	UNREF(strGameCode); 

	s_intervalToSendHeadersToClientSeconds = ConfigChatServer::getIntervalToSendHeadersToClientSeconds();
	s_maxHeadersToSendToClientPerInterval = ConfigChatServer::getMaxHeadersToSendToClientPerInterval();
}

//-----------------------------------------------------------------------

ChatInterface::~ChatInterface()
{
	for (std::map<ChatAvatarId, std::pair<unsigned long, std::deque<const ChatPersistentMessageToClient *> > >::const_iterator iter = queuedHeaders.begin(); iter != queuedHeaders.end(); ++iter)
	{
		for (std::deque<const ChatPersistentMessageToClient *>::const_iterator iter2 = iter->second.second.begin(); iter2 != iter->second.second.end(); ++iter2)
			delete *iter2;
	}

	queuedHeaders.clear();
}

//-----------------------------------------------------------------------

void ChatInterface::OnConnect()
{
	ChatServer::fileLog(true, "ChatInterface", "OnConnect() ChatAPI has established connection to the ChatServer backend and is ready to accept requests.  Connecting SYSTEM avatar.");

	ConnectPlayer(10, "SYSTEM", NetworkId::cms_invalid);
}

//-----------------------------------------------------------------------

void ChatInterface::OnDisconnect()
{
	ChatServer::fileLog(true, "ChatInterface", "OnDisconnect() ChatAPI has lost connection to the ChatServer backend. This may be followed by an OnFailoverBegin command if the connection is re-established and failover mode is initiated.");
}

//-----------------------------------------------------------------------

void ChatInterface::OnFailoverBegin()
{
	ChatServer::fileLog(true, "ChatInterface", "OnFailoverBegin() ChatAPI has restored connection to the ChatServer backend and is attempting failover to restore validity of its state. It is retaining its queued requests for server submission upon OnFailoverComplete.");
}

//-----------------------------------------------------------------------

void ChatInterface::OnFailoverComplete()
{
	ChatServer::fileLog(true, "ChatInterface", "OnFailoverComplete() ChatAPI has restored its connection to the ChatServer backend and restored its state. It is submitting the queued requests and ready to accept new requests.");
}

//-----------------------------------------------------------------------

void ChatInterface::OnGetRoom(unsigned track, unsigned result, const ChatRoom *room, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnGetRoom() track(%u) result(%u) room(%s)", track, result, ChatServer::getChatRoomNameNarrow(room).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnGetRoom");
	if (result != CHATRESULT_SUCCESS || !room)
	{
		return;
	}
	std::string roomName;
	if (room)
		makeRoomName(room, roomName);
	//printf("!!!!!!!!!!!!got room %s\n", roomName.c_str());
	if (strstr(roomName.c_str(), "group") == nullptr)
	{
		ChatServer::putSystemAvatarInRoom(roomName);
	}
	std::string lowerRoomName = toLower(roomName);
	unsigned sequence = (unsigned)user;
	std::unordered_map<std::string, ChatServerRoomOwner>::iterator f = roomList.find(lowerRoomName);
	if (f != roomList.end() && room)
	{
		(*f).second.updateRoomData(room);
	}
	else
	{
		if (room)
		{
			ChatServerRoomOwner * o = new ChatServerRoomOwner(room);

			IGNORE_RETURN(roomList.insert(std::make_pair(lowerRoomName, *o)));

			delete o;
		}
	}

	ChatServerRoomOwner *owner = nullptr;
	if (room)
		owner = getRoomOwner(room->getRoomID());

	if (owner)
	{
		ChatAvatarId av;
		std::vector<ChatAvatarId> avatars;
		
		if(room)
		{
			for(AvatarIterator a = room->getFirstAvatar(); !a.outOfBounds(); ++a)
			{
				makeAvatarId(*(*a), av);
				avatars.push_back(av);
			}
			std::vector<ChatAvatarId> moderators;
			for(ModeratorIterator m = room->getFirstModerator(); !m.outOfBounds(); ++m)
			{
				makeAvatarId(*(*m), av);
				moderators.push_back(av);
			}
			std::vector<ChatAvatarId> invitees;
			for(InviteIterator i = room->getFirstInvited(); !i.outOfBounds(); ++i)
			{
				makeAvatarId(*(*i), av);
				invitees.push_back(av);
			}
			std::vector<ChatAvatarId> banned;
			for(BanIterator b = room->getFirstBanned(); !b.outOfBounds(); ++b)
			{
				makeAvatarId(*(*b), av);
				banned.push_back(av);
			}
			
			ChatQueryRoomResults results(sequence, owner->getRoomData(), avatars, moderators, invitees, banned);
			ChatServer::sendResponseForTrackId(track, results);
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::queryRoom(const NetworkId & id, ConnectionServerConnection * connection, const unsigned int sequence, const std::string & roomName)
{
	UNREF(connection);
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "queryRoom() id(%s) connection(%s) sequence(%u) roomName(%s)", id.getValueString().c_str(), ChatServer::getConnectionAddress(connection).c_str(), sequence, roomName.c_str());

	std::string tmpName = "SOE+" + roomName;
	size_t pos = tmpName.find(".");
	while (pos != std::string::npos)
	{
		tmpName[pos] = '+';
		pos = tmpName.find(".");
	}

	Unicode::String wideName = Unicode::narrowToWide(tmpName);
	unsigned track = RequestGetRoom(ChatUnicodeString(wideName.data(), wideName.size()), (void *)sequence);
	ChatServer::instance().pendingRequests[track] = id;
	
}

//-----------------------------------------------------------------------

const ChatServerRoomOwner * ChatInterface::getRoomByName(const std::string & roomName) const
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "getRoomByName() roomName(%s)", roomName.c_str());

	std::string lowerRoomName = toLower(roomName);
	const ChatServerRoomOwner * result = 0;
	std::unordered_map<std::string, ChatServerRoomOwner>::const_iterator f = roomList.find(lowerRoomName);
	if(f != roomList.end())
	{
		result = &(*f).second;
	}
	return result;
}

//-----------------------------------------------------------------------

void ChatInterface::deferChatMessageFor(const NetworkId & id, const Archive::ByteStream & bs)
{
	ChatServer::fileLog(false, "ChatInterface", "deferChatMessageFor() id(%s)", id.getValueString().c_str());

	std::vector<Archive::ByteStream> & v = deferredChatMessages[id];
	v.push_back(bs);
}

//-----------------------------------------------------------------------

void ChatInterface::sendMessageToAvatar(const ChatAvatarId & avatar, const GameNetworkMessage & message)
{
	ChatServer::fileLog(false, "ChatInterface", "sendMessageToAvatar() avatar(%s) message(%s)", avatar.getFullName().c_str(), message.getCmdName().c_str());

	ChatServerAvatarOwner * to = 0;
	std::map<ChatAvatarId, ChatServerAvatarOwner *>::const_iterator f = avatarMap.find(avatar);
	if(f != avatarMap.end())
		to = (*f).second;
	if(to)
	{
		ConnectionServerConnection * connection = to->getPlayerConnection();
		if(connection)
		{
			const NetworkId & id = to->getNetworkId();
			if(id != NetworkId::cms_invalid)
			{
				connection->sendToClient(id, message);
			}
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::sendMessageToAvatar(const ChatAvatar & avatar, const GameNetworkMessage & message)
{
	ChatServer::fileLog(false, "ChatInterface", "sendMessageToAvatar() avatar(%s) message(%s)", ChatServer::getFullChatAvatarName(&avatar).c_str(), message.getCmdName().c_str());

	ChatServerAvatarOwner * to = getAvatarOwner(&avatar);
	
	if(! to)
	{
		ChatAvatarId id;
		makeAvatarId(avatar, id);
		std::map<ChatAvatarId, ChatServerAvatarOwner *>::const_iterator f = avatarMap.find(id);
		if(f != avatarMap.end())
		{
			to = (*f).second;
		}
	}
	if(to)
	{
		ConnectionServerConnection * connection = to->getPlayerConnection();
		if(connection)
		{
			const NetworkId & id = to->getNetworkId();
			if(id != NetworkId::cms_invalid)
			{
				connection->sendToClient(id, message);
			}
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::sendMessageToAllAvatars(const GameNetworkMessage & message)
{
	ChatServer::fileLog(false, "ChatInterface", "sendMessageToAllAvatars() message(%s)", message.getCmdName().c_str());

	ChatServer::sendToAllConnectionServers(message);
/*
	//send to all connected avatars
	std::map<ChatAvatarId, ChatServerAvatarOwner *>::const_iterator f = avatarMap.begin();
	while (f != avatarMap.end())
	{
		ChatServerAvatarOwner *to = (*f).second;
		if(to)
		{
			ConnectionServerConnection * connection = to->getPlayerConnection();
			if(connection)
			{
				const NetworkId & id = to->getNetworkId();
				if(id != NetworkId::cms_invalid)
				{
					connection->sendToClient(id, message);
				}
			}
		}
		++f;
	}

	//send to all pending avatars
	std::unordered_map<std::string, NetworkId>::iterator p = pendingAvatars.begin();
	while (p != pendingAvatars.end())
	{
		ConnectionServerConnection * connection = 
			ChatServer::getConnectionForCharacter((*p).second);
		if(connection)
		{
			const NetworkId & id = (*p).second;
			if(id != NetworkId::cms_invalid)
			{
				connection->sendToClient(id, message);
			}
		}
		++p;
	}
	*/
}

//-----------------------------------------------------------------------

std::string ChatInterface::getChatName(const NetworkId &id)
{
	ChatServer::fileLog(false, "ChatInterface", "getChatName() id(%s)", id.getValueString().c_str());

    std::unordered_map<std::string, NetworkId>::iterator i;
    for (i = pendingAvatars.begin(); i != pendingAvatars.end(); ++i)
    {
        if (id == (*i).second)
        {
            return (*i).first;
        }
    }
    static std::string tmp("somebody");
    return tmp;
}

//-----------------------------------------------------------------------

bool ChatInterface::sendMessageToPendingAvatar(const ChatAvatarId &id, const GameNetworkMessage &message)
{
	ChatServer::fileLog(false, "ChatInterface", "sendMessageToPendingAvatar() id(%s) message(%s)", id.getFullName().c_str(), message.getCmdName().c_str());

	std::unordered_map<std::string, NetworkId>::iterator f = pendingAvatars.find(id.name);
	if (f != pendingAvatars.end())
	{
		ConnectionServerConnection * connection = 
			ChatServer::getConnectionForCharacter((*f).second);
		
		if (connection)
		{
			const NetworkId & id = (*f).second;
			if(id != NetworkId::cms_invalid)
			{
				connection->sendToClient(id, message);
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------

const std::unordered_map<std::string, ChatServerRoomOwner> & ChatInterface::getRoomList() const
{
	return roomList;
}

//-----------------------------------------------------------------------

void ChatInterface::updateRooms()
{
	std::unordered_map<std::string, ChatServerRoomOwner>::const_iterator i;
	for(i = roomList.begin(); i != roomList.end(); ++i)
	{
		ChatServerRoomOwner & roomOwner = const_cast<ChatServerRoomOwner &>((*i).second);
		const ChatRoom *room = roomOwner.getRoom();
		if (room)
		{
			roomOwner.updateRoomData(room);
			/*
			AvatarIterator i = room->getFirstAvatar();
			if (!i.outOfBounds())
			{
				++i;
			}
			if (i.outOfBounds() && (!(room->getRoomAttributes() & ROOMATTR_PERSISTENT)))
			{
				ChatServer::removeSystemAvatarFromRoom(room);
				roomOwner.flagForDelete();
			}*/
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::requestRoomList(const NetworkId & id, ConnectionServerConnection * connection)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "requestRoomList() id(%s) connection(%s)", id.getValueString().c_str(), ChatServer::getConnectionAddress(connection).c_str());

	if (roomQueriesThisFrame >= ConfigChatServer::getMaxRoomQueriesPerFrame())
	{
//		printf("%Ld requested room query, but we've had too many queries this frame.  Queuing it\n", id.getValue());
		pendingRoomQueries.insert(id);
		return;
	}

	++roomQueriesThisFrame;

	std::unordered_map<std::string, ChatServerRoomOwner>::const_iterator i;
	std::vector<ChatRoomData> rooms;
	const ChatAvatar * target = ChatServer::getAvatarByNetworkId(id);
	ChatAvatarId requestingAvatar;
	if(target)
	{
		makeAvatarId(*target, requestingAvatar);
		for(i = roomList.begin(); i != roomList.end(); ++i)
		{
			ChatServerRoomOwner & roomOwner = const_cast<ChatServerRoomOwner &>((*i).second);
			const ChatRoomData & roomData = roomOwner.getRoomData();
			if(
				roomData.roomType == CHAT_ROOM_PUBLIC || //lint !e641 enum to int 
				roomData.creator == requestingAvatar 
			) 
			{
				rooms.push_back(roomData);
			}
			else
			{
				std::vector<ChatAvatarId>::const_iterator j;
				for(j = roomData.invitees.begin(); j != roomData.invitees.end(); ++j)
				{
					if (requestingAvatar == (*j))
					{
						rooms.push_back(roomData);
						break;
					}
				}

			}
		}
	}
	ChatRoomList r(rooms);
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "requestRoomList() sending room list of %u rooms to (%s)", r.getRoomData().size(), id.getValueString().c_str());

	if (connection)
		connection->sendToClient(id, r);
}

//-----------------------------------------------------------------------

std::string makeCanonicalName(const std::string &characterName)
{
/*
	static const std::string dot(".");
	std::string retVal;
	char *str = strdup(characterName.c_str());
	char *tmp = strtok(str, ".");
	if (tmp)
	{
		retVal = toUpper(tmp);
	}
	tmp = strtok(nullptr, ".");
	if (tmp)
	{
		retVal += (dot + tmp);
	}
	tmp = strtok(nullptr, ".");
	if (tmp)
	{
		retVal += (dot + tmp);
	}
	printf("canonicalized(%s -> %s)\n", characterName.c_str(), retVal.c_str());
	free(str);
	return retVal;
	*/
	if (characterName == "SYSTEM")
	{
		return characterName;
	}
	else
	{
		return toLower(characterName);
	}
}

//-----------------------------------------------------------------------

struct ConnectPlayerInfo
{
	Unicode::String name;
	Unicode::String address;
	unsigned int suid;
};

struct QueuedLoginInfo
{
	std::string  name;
	NetworkId    id;
	unsigned int suid;
};

static std::vector<QueuedLoginInfo *> queuedLogins;

//-----------------------------------------------------------------------

void ChatInterface::ConnectPlayer(unsigned int suid, const std::string & characterName, const NetworkId & networkId)
{
	ChatServer::fileLog(false, "ChatInterface", "ConnectPlayer() suid(%u) characterName(%s) networkId(%s)", suid, characterName.c_str(), networkId.getValueString().c_str());

	unsigned int loginFlowControlRate = ConfigChatServer::getLoginFlowControlRate(); 
	if (loginFlowControlRate && pendingAvatars.size() > loginFlowControlRate)
	{
		QueuedLoginInfo *info = new QueuedLoginInfo;
		info->name = characterName;
		info->id = networkId;
		info->suid = suid;

		queuedLogins.push_back(info);

//		printf("Queuing login of avatar (%s)\n", characterName.c_str());
		return;
	}

	static const std::string game("SOE+SWG");
	IGNORE_RETURN(pendingAvatars.insert(std::make_pair(characterName, networkId)));

	Unicode::String name = Unicode::narrowToWide(characterName);
	Unicode::String address = Unicode::narrowToWide(game + "+" + ConfigChatServer::getClusterName());
	//Unicode::String address = Unicode::narrowToWide("SOE+SWG+Test Server");

	ConnectPlayerInfo *info = new ConnectPlayerInfo;
	info->name = name;
	info->address = address;
	info->suid = suid;

	IGNORE_RETURN(RequestLoginAvatar(suid, ChatUnicodeString(name.data(), name.size()), ChatUnicodeString(address.data(), address.size()), (void *)info));
}

//-----------------------------------------------------------------------

void ChatInterface::DestroyAvatar(const std::string & characterName)
{
	std::string const avatarName(Unicode::toLower(characterName));
	std::string const avatarNameNormalized(avatarName, 0, avatarName.find(' '));
	ChatAvatarId const avatar("SWG", ConfigChatServer::getClusterName(), avatarNameNormalized);

	// track how many times we have called RequestGetAnyAvatar() for this particular DestroyAvatar
	// operation, so that we can stop if we somehow get stuck in an infinite loop 
	unsigned const trackID = RequestGetAnyAvatar(ChatUnicodeString(avatar.getName()), ChatUnicodeString(avatar.getAPIAddress()), nullptr);
	trackingRequestGetAnyAvatarForDestroy[trackID] = std::make_pair(std::make_pair(ChatUnicodeString(avatar.getName()), ChatUnicodeString(avatar.getAPIAddress())), 1);
}

//-----------------------------------------------------------------------

void ChatInterface::checkQueuedLogins()
{
	roomQueriesThisFrame = 0;
	unsigned int loginFlowControlRate = ConfigChatServer::getLoginFlowControlRate(); 
	if (!loginFlowControlRate)
	{
		return;
	}
	if (queuedLogins.size() > 0)
	{
		while (queuedLogins.size() > 0 && pendingAvatars.size() < loginFlowControlRate)
		{
			std::vector<QueuedLoginInfo *>::iterator i = queuedLogins.begin();
			QueuedLoginInfo *info = (*i);

			ConnectPlayer(info->suid, info->name, info->id);

//			printf("Servicing queued login of avatar (%s)\n", info->name.c_str());

			delete info;
			queuedLogins.erase(i);
		}
	}

	while (roomQueriesThisFrame < ConfigChatServer::getMaxRoomQueriesPerFrame() &&
		   !pendingRoomQueries.empty())
	{
		std::set<NetworkId>::iterator i = pendingRoomQueries.begin();	
//		printf("Servicing queued room query for %Ld.\n", (*i).getValue());
		ConnectionServerConnection * c = ChatServer::getConnectionForCharacter((*i));
		if (c)
		{
			requestRoomList((*i), c);	
		}
		pendingRoomQueries.erase(i);
	}
}

//-----------------------------------------------------------------------

void ChatInterface::sendQueuedHeadersToClient()
{
	const unsigned long currentTime = Clock::timeSeconds();
	const unsigned long queuedHeadersSendTime = currentTime + static_cast<unsigned long>(s_intervalToSendHeadersToClientSeconds);

	int numberHeadersSent = 0;
	const ChatPersistentMessageToClient * header = nullptr;

	for (std::map<ChatAvatarId, std::pair<unsigned long, std::deque<const ChatPersistentMessageToClient *> > >::iterator iter = queuedHeaders.begin(); iter != queuedHeaders.end();)
	{
		if (iter->second.first < currentTime)
		{
			numberHeadersSent = 0;

			while ((!iter->second.second.empty()) && (numberHeadersSent < s_maxHeadersToSendToClientPerInterval))
			{
				header = iter->second.second.front();
				iter->second.second.pop_front();

				sendMessageToAvatar(iter->first, *header);
				++numberHeadersSent;

				delete header;
			}

			iter->second.first = queuedHeadersSendTime;
		}

		if (iter->second.second.empty())
		{
			queuedHeaders.erase(iter++);
		}
		else
		{
			++iter;
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::clearQueuedHeadersForAvatar(const ChatAvatarId & avatarId)
{
	std::map<ChatAvatarId, std::pair<unsigned long, std::deque<const ChatPersistentMessageToClient *> > >::iterator iter = queuedHeaders.find(avatarId);

	if (iter != queuedHeaders.end())
	{
		for (std::deque<const ChatPersistentMessageToClient *>::const_iterator iter2 = iter->second.second.begin(); iter2 != iter->second.second.end(); ++iter2)
			delete *iter2;

		queuedHeaders.erase(iter);
	}
}

//-----------------------------------------------------------------------

void ChatInterface::addQueuedHeaderForAvatar(const ChatAvatarId & avatarId, const ChatPersistentMessageToClient * header, unsigned long sendTime)
{
	if (header == nullptr)
		return;

	std::pair<unsigned long, std::deque<const ChatPersistentMessageToClient *> > & queuedHeader = queuedHeaders[avatarId];
	queuedHeader.first = sendTime;
	queuedHeader.second.push_back(header);
}

//-----------------------------------------------------------------------

ChatServerAvatarOwner *ChatInterface::getAvatarOwner(const ChatAvatar *avatar)
{
	if (avatar)
	{
		ChatAvatarId id;
		makeAvatarId(*avatar, id);

		std::map<ChatAvatarId, ChatServerAvatarOwner *>::iterator f = avatarMap.find(id);
		if (f != avatarMap.end())
		{
			return (*f).second;
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------
// Response to RequestAddModerator(...)
void ChatInterface::OnAddModerator(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnAddModerator() track(%u) result(%u) srcAvatar(%s) destRoom(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnAddModerator");

	AvatarIdSequencePair *pair = (AvatarIdSequencePair *)user;

	unsigned sequence = 0;
	ChatAvatarId destId;
	if (pair)
	{
		destId = pair->avatar;
		sequence = pair->sequence;

		delete pair;
		pair = nullptr;
	}

	ChatAvatarId srcId;
	if (srcAvatar)
	{
		makeAvatarId(*srcAvatar, srcId);
	}

	std::string roomName;
	if (destRoom)
	{
		makeRoomName(destRoom, roomName);
	}

	// Tell the player making the request whether it was successful
	ChatOnAddModeratorToRoom msg(result, sequence, destId, srcId, roomName);
	NetworkId responseNetworkId = ChatServer::sendResponseForTrackId(track, msg);

	// If we had a success, tell everyone else in the room about it
	if (result == CHATRESULT_SUCCESS)
	{
		if (destRoom)
		{
			ChatAvatarId responseId;
			{
				ChatAvatar const * const responseAvatar = ChatServer::getAvatarByNetworkId( responseNetworkId );
				if (responseAvatar)
				{
					makeAvatarId(*responseAvatar, responseId);
				}
			}

			AvatarIterator i = destRoom->getFirstAvatar();
			for(; !i.outOfBounds(); ++i)
			{
				ChatAvatarId id;
				makeAvatarId(*(*i), id);

				// We already sent a response to the avatar that made the request
				if (id != responseId)
				{
					sendMessageToAvatar(id, msg);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveAddModeratorRoom
//    Called when srcAvatar has granted destAvatar moderator privileges in
//    destRoom and this API has avatars that are in destRoom.
void ChatInterface::OnReceiveAddModeratorRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveAddModeratorRoom() srcAvatar(%s) destAvatar(%s) destRoom(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveAddModeratorRoom");

	if (destRoom)
	{
		std::string roomName;
		makeRoomName(destRoom, roomName);

		// Update info about the room
		std::unordered_map<std::string, ChatServerRoomOwner>::iterator f = roomList.find(toLower(roomName));
		if(f != roomList.end())
			(*f).second.updateRoomData(destRoom);

		// See who created the room
		ChatAvatarId creatorAvatarId;
		makeAvatarId(destRoom->getCreatorName(), destRoom->getCreatorAddress(), creatorAvatarId);

		// Clients don't want to know who belongs to the system created rooms
		// NOTE: The name for the system avatar should be the same for all clusters
		if (!Unicode::caseInsensitiveCompare(creatorAvatarId.getName(), ChatServer::getSystemAvatarId().getName()))
		{
			ChatServer::fileLog(false, "ChatInterface", "OnReceiveAddModeratorRoom() updating clients with member info for chatRoom(%s)", ChatServer::getChatRoomNameNarrow(destRoom).c_str());

			// Tell all the avatars in the room the new member info
			updateRoomForThisChatAPI(destRoom, destAvatar);
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveAddModeratorAvatar
//    Called when srcAvatar has granted destAvatar moderator privileges in
//    the room described by roomName and roomAddress, and destAvatar
//    is on this API.
void ChatInterface::OnReceiveAddModeratorAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveAddModeratorAvatar() srcAvatar(%s) destAvatar(%s) roomName(%s) roomAddress(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::toNarrowString(roomName).c_str(), ChatServer::toNarrowString(roomAddress).c_str());
}

//-----------------------------------------------------------------------
// Response to RequestRemoveModerator(...)
void ChatInterface::OnRemoveModerator(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnRemoveModerator() track(%u) result(%u) srcAvatar(%s) destRoom(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnRemoveModerator");

	AvatarIdSequencePair *pair = (AvatarIdSequencePair *)user;

	unsigned sequence = 0;
	ChatAvatarId destId;
	if (pair)
	{
		destId = pair->avatar;
		sequence = pair->sequence;

		delete pair;
		pair = nullptr;
	}

	ChatAvatarId srcId;
	if (srcAvatar)
	{
		makeAvatarId(*srcAvatar, srcId);
	}

	std::string roomName;
	if (destRoom)
	{
		makeRoomName(destRoom, roomName);
	}

	// Tell the player making the request whether it was successful
	ChatOnRemoveModeratorFromRoom msg(result, sequence, destId, srcId, roomName);
	NetworkId responseNetworkId = ChatServer::sendResponseForTrackId(track, msg);

	// If we had a success, tell everyone else in the room about it
	if (result == CHATRESULT_SUCCESS)
	{
		if (destRoom)
		{
			ChatAvatarId responseId;
			{
				ChatAvatar const * const responseAvatar = ChatServer::getAvatarByNetworkId( responseNetworkId );
				if (responseAvatar)
				{
					makeAvatarId(*responseAvatar, responseId);
				}
			}

			AvatarIterator i = destRoom->getFirstAvatar();
			for(; !i.outOfBounds(); ++i)
			{
				ChatAvatarId id;
				makeAvatarId(*(*i), id);

				// We already sent a response to the avatar that made the request
				if (id != responseId)
				{
					sendMessageToAvatar(id, msg);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveRemoveModeratorRoom
//    Called when srcAvatar has removed destAvatar moderator privileges in
//    destRoom and this API has avatars that are in destRoom.
void ChatInterface::OnReceiveRemoveModeratorRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveRemoveModeratorRoom() srcAvatar(%s) destAvatar(%s) destRoom(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveRemoveModeratorRoom");

	if (destRoom)
	{
		std::string roomName;
		makeRoomName(destRoom, roomName);

		// Update info about the room
		std::unordered_map<std::string, ChatServerRoomOwner>::iterator f = roomList.find(toLower(roomName));
		if(f != roomList.end())
			(*f).second.updateRoomData(destRoom);

		// See who created the room
		ChatAvatarId creatorAvatarId;
		makeAvatarId(destRoom->getCreatorName(), destRoom->getCreatorAddress(), creatorAvatarId);

		// Clients don't want to know who belongs to the system created rooms
		// NOTE: The name for the system avatar should be the same for all clusters
		if (!Unicode::caseInsensitiveCompare(creatorAvatarId.getName(), ChatServer::getSystemAvatarId().getName()))
		{
			ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveRemoveModeratorRoom() updating clients with member info for chatRoom(%s)", ChatServer::getChatRoomNameNarrow(destRoom).c_str());

			// Tell all the avatars in the room the new member info
			updateRoomForThisChatAPI(destRoom, 0);
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveRemoveModeratorAvatar
//    Called when srcAvatar has removed destAvatar moderator privileges from
//    the room described by roomName and roomAddress, and destAvatar is on
//    this API.
void ChatInterface::OnReceiveRemoveModeratorAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveRemoveModeratorAvatar() srcAvatar(%s) destAvatar(%s) roomName(%s) roomAddress(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::toNarrowString(roomName).c_str(), ChatServer::toNarrowString(roomAddress).c_str());
}

//-----------------------------------------------------------------------

void ChatInterface::OnAddFriend(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnAddFriend() track(%u) result(%u) srcAvatar(%s) destName(%s) destAddress(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::toNarrowString(destName).c_str(), ChatServer::toNarrowString(destAddress).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnAddFriend");
	UNREF(user);
	if(result == CHATRESULT_SUCCESS) //lint !e641 Converting enum 'ChatResult' to int // jrandall that's what the API exposes
	{
		// send message to client!
		ChatAvatarId d;
		makeAvatarId(destName, destAddress, d);
		ChatOnAddFriend chat(0, result); //lint !e713 Loss of precision (arg. no. 2) (unsigned int to int)
		IGNORE_RETURN(ChatServer::sendResponseForTrackId(track, chat));
	}
	else
	{
		ChatOnAddFriend failed(0, result); //lint !e713 Loss of precision (arg. no. 2) (unsigned int to int)
		IGNORE_RETURN(ChatServer::sendResponseForTrackId(track, failed));
	}

	//game server support
	ChatAvatarId d;
	makeAvatarId(destName, destAddress, d);

	ChatServerAvatarOwner * mo = nullptr;
	if (srcAvatar)
		mo = getAvatarOwner(srcAvatar);

	if(mo)
	{
		ChatOnChangeFriendStatus update(0, mo->getNetworkId(), d, true, result);
		ConnectionServerConnection * mc = mo->getPlayerConnection();
		if(mc)
		{
			mc->sendToClient(mo->getNetworkId(), update);
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnRemoveFriend(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnRemoveFriend() track(%u) result(%u) srcAvatar(%s) destName(%s) destAddress(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::toNarrowString(destName).c_str(), ChatServer::toNarrowString(destAddress).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnRemoveFriend");
	UNREF(user);
	UNREF(track);
	// send message to client!
	ChatAvatarId d;
	makeAvatarId(destName, destAddress, d);

	ChatServerAvatarOwner * mo = nullptr;
	if (srcAvatar)
		mo =  getAvatarOwner(srcAvatar);

	if(mo)
	{
		ChatOnChangeFriendStatus update(0, mo->getNetworkId(), d, false, result);
		ConnectionServerConnection * mc = mo->getPlayerConnection();
		if(mc)
		{
			mc->sendToClient(mo->getNetworkId(), update);
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnIgnoreStatus(unsigned track, unsigned result, const ChatAvatar *srcAvatar, unsigned listLength, const ChatIgnoreStatus *ignoreList, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnIgnoreStatus() track(%u) result(%u) srcAvatar(%s) listLength(%u)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), listLength);

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnIgnoreStatus");
	UNREF(user);
	UNREF(result);
//	REPORT_LOG(true, ("OnIgnoreStatus(%u)\n", result));
	if (track != 0 && result == CHATRESULT_SUCCESS)
	{
		std::vector<ChatAvatarId> idList;

		if (ignoreList)
		{
			for (unsigned i = 0; i < listLength; ++i)
			{
				ChatAvatarId id;
				makeAvatarId(ignoreList[i].getName(), ignoreList[i].getAddress(), id);
				idList.push_back(id);
			}
		}

		ChatServerAvatarOwner * mo = nullptr;
		if (srcAvatar)
			mo =  getAvatarOwner(srcAvatar);
		if(mo)
		{
			ChatOnGetIgnoreList msg(mo->getNetworkId(), idList);
			ConnectionServerConnection * mc = mo->getPlayerConnection();
			if(mc)
			{
				mc->sendToClient(mo->getNetworkId(), msg);
			}
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnFriendStatus(unsigned track, unsigned result, const ChatAvatar *srcAvatar, unsigned listLength, const ChatFriendStatus *friendList, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnFriendStatus() track(%u) result(%u) srcAvatar(%s) listLength(%u)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), listLength);

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnFriendStatus");
	UNREF(user);
	UNREF(result);
	if (track != 0 && result == CHATRESULT_SUCCESS)
	{
		std::vector<ChatAvatarId> idList;
		if (friendList)
		{
			for (unsigned i = 0; i < listLength; ++i)
			{
				ChatAvatarId id;
				makeAvatarId(friendList[i].getName(), friendList[i].getAddress(), id);
				idList.push_back(id);
			}
		}
		ChatServerAvatarOwner * mo = nullptr;
		if (srcAvatar)
			mo =  getAvatarOwner(srcAvatar);
		if(mo)
		{
			ChatOnGetFriendsList msg(mo->getNetworkId(), idList);
			ConnectionServerConnection * mc = mo->getPlayerConnection();
			if(mc)
			{
				mc->sendToClient(mo->getNetworkId(), msg);
			}
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnReceiveFriendLogin(const ChatAvatar *srcAvatar, const ChatUnicodeString &srcAddress, const ChatAvatar *destAvatar)
{
	ChatServer::fileLog(false, "ChatInterface", "OnReceiveFriendLogin() srcAvatar(%s) srcAddress(%s) destAvatar(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::toNarrowString(srcAddress).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveFriendLogin");
	UNREF(srcAddress);
	ChatServerAvatarOwner * owner = nullptr;
	if (destAvatar)
		owner = getAvatarOwner(destAvatar);
	if(owner)
	{
		ConnectionServerConnection * connection = owner->getPlayerConnection();
		if(connection)
		{
			const NetworkId & id = owner->getNetworkId();
			if((id != NetworkId::cms_invalid) && (srcAvatar))
			{
				ChatAvatarId d;
				makeAvatarId(*srcAvatar, d);
				ChatFriendsListUpdate update(d, true);
				connection->sendToClient(id, update);
			}
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnReceiveFriendLogout(const ChatAvatar *srcAvatar, const ChatUnicodeString &srcAddress, const ChatAvatar *destAvatar)
{
	ChatServer::fileLog(false, "ChatInterface", "OnReceiveFriendLogout() srcAvatar(%s) srcAddress(%s) destAvatar(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::toNarrowString(srcAddress).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveFriendLogout");
	UNREF(srcAddress);
	ChatServerAvatarOwner * owner = nullptr;
	if (destAvatar)
		owner = getAvatarOwner(destAvatar);
	if(owner)
	{
		ConnectionServerConnection * connection = owner->getPlayerConnection();
		if(connection)
		{
			const NetworkId & id = owner->getNetworkId();
			if((id != NetworkId::cms_invalid) && (srcAvatar))
			{
				ChatAvatarId d;
				makeAvatarId(*srcAvatar, d);
				ChatFriendsListUpdate update(d, false);
				connection->sendToClient(id, update);
			}
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnAddIgnore(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnAddIgnore() track(%u) result(%u) srcAvatar(%s) destName(%s) destAddress(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::toNarrowString(destName).c_str(), ChatServer::toNarrowString(destAddress).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnAddIgnore");
	UNREF(user);

	if(srcAvatar)
	{
		//game server support
		ChatAvatarId d;
		makeAvatarId(destName, destAddress, d);
		ChatAvatarId r;
		makeAvatarId(*srcAvatar, r);
	
		ChatServerAvatarOwner * mo = getAvatarOwner(srcAvatar);
		if(mo)
		{
			ChatOnChangeIgnoreStatus update(0, mo->getNetworkId(), d, true, result);
			ConnectionServerConnection * mc = mo->getPlayerConnection();
			if(mc)
			{
				mc->sendToClient(mo->getNetworkId(), update);
			}
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnRemoveIgnore(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatUnicodeString &destName, const ChatUnicodeString &destAddress, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnRemoveIgnore() track(%u) result(%u) srcAvatar(%s) destName(%s) destAddress(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::toNarrowString(destName).c_str(), ChatServer::toNarrowString(destAddress).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnRemoveIgnore");
	UNREF(user);

	//game server support
	ChatAvatarId d;
	makeAvatarId(destName, destAddress, d);

	ChatServerAvatarOwner * mo = nullptr;
	if (srcAvatar)
		mo = getAvatarOwner(srcAvatar);
	if(mo)
	{
		ChatOnChangeIgnoreStatus update(0, mo->getNetworkId(), d, false, result);
		ConnectionServerConnection * mc = mo->getPlayerConnection();
		if(mc)
		{
			mc->sendToClient(mo->getNetworkId(), update);
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnLoginAvatar(unsigned track, unsigned result, const ChatAvatar *newAvatar, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnLoginAvatar() track(%u) result(%u) newAvatar(%s)", track, result, ChatServer::getFullChatAvatarName(newAvatar).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnLoginAvatar");

	static unsigned lastTrack = 12345678;
	if (track == lastTrack)
	{
		DEBUG_WARNING(true, ("Received duplicate track"));
		return;
	}
	lastTrack = track;

	if (newAvatar)
		s_mostRecentRoomChatMessage.erase(newAvatar->getAvatarID());

	// login to destroy chat avatar
	std::map<unsigned, std::pair<std::pair<ChatUnicodeString, ChatUnicodeString>, int> >::const_iterator iterFind = trackingRequestGetAnyAvatarForDestroy.find(track);
	if (iterFind != trackingRequestGetAnyAvatarForDestroy.end())
	{
		if ((result == CHATRESULT_SUCCESS) && newAvatar)
		{
			// destroy chat avatar
			unsigned const trackID = RequestDestroyAvatar(newAvatar, nullptr);
			trackingRequestGetAnyAvatarForDestroy[trackID] = iterFind->second;
		}

		trackingRequestGetAnyAvatarForDestroy.erase(track);
		return;
	}

	ConnectPlayerInfo *info = (ConnectPlayerInfo *)user;
	if (info)
	{
		if (result == 1) //timeout
		{
//			printf("!!!!!!!!!!avatar login timed out, trying again\n");
			ConnectPlayerInfo *info2 = new ConnectPlayerInfo;
			info2->name = info->name;
			info2->address = info->address;
			info2->suid = info->suid;
			IGNORE_RETURN(RequestLoginAvatar(info->suid, ChatUnicodeString(info->name.data(), info->name.size()), ChatUnicodeString(info->address.data(), info->address.size()), (void *)info2));
			delete info;
			return;
		}
	}
//	ChatAvatarId foobar;
//	if (newAvatar)
//	{
//		makeAvatarId(*newAvatar, foobar);
//	}
//	printf("OnLoginAvatar(%u) for %s.%s.%s\n", result, foobar.gameCode.c_str(), foobar.cluster.c_str(), foobar.name.c_str());
	//DEBUG_WARNING(result != CHATRESULT_SUCCESS, ("Chat Connect avatar %s.%s.%s failed! : %s\n", avatar.GetGameCode().c_str(), avatar.GetGameServerName().c_str(), avatar.GetCharacterName().c_str(), getChatResultCodeName(resultCode).c_str())); //lint !e641 converting enum to int
	// is this the special system avatar?

	if (result != CHATRESULT_SUCCESS)
	{
		
		ChatAvatarId failedAvatarId;
		makeAvatarId(ChatUnicodeString(info->name.data(), info->name.size()), ChatUnicodeString(info->address.data(), info->address.size()), failedAvatarId); 
		std::unordered_map<std::string, NetworkId>::iterator f = pendingAvatars.find(failedAvatarId.name);
		if (f != pendingAvatars.end())
		{
			pendingAvatars.erase(f);
		}
		DEBUG_WARNING(result != CHATRESULT_SUCCESS, ("Chat Connect avatar failed for %s.%s.%s! : %u\n", failedAvatarId.gameCode.c_str(),failedAvatarId.cluster.c_str(), failedAvatarId.name.c_str(), result)); //lint !e641 converting enum to int
		delete info;
		return;
	}
	delete info;

	ChatAvatarId id;
	if (newAvatar)
		makeAvatarId(*newAvatar, id);

	if(id == ChatServer::getSystemAvatarId())
	{
		//REPORT_LOG(true, ("\"System avatar\" load confirmed\n"));
		if (newAvatar)
		{
			ChatServer::fileLog(true, "ChatInterface", "OnLoginAvatar() ChatAPI SYSTEM avatar connected with name=(%s), address=(%s), server=(%s), gateway=(%s), loginLocation=(%s)",
			    Unicode::wideToNarrow(Unicode::String(newAvatar->getName().string_data, newAvatar->getName().string_length)).c_str(),
			    Unicode::wideToNarrow(Unicode::String(newAvatar->getAddress().string_data, newAvatar->getAddress().string_length)).c_str(),
			    Unicode::wideToNarrow(Unicode::String(newAvatar->getServer().string_data, newAvatar->getServer().string_length)).c_str(),
			    Unicode::wideToNarrow(Unicode::String(newAvatar->getGateway().string_data, newAvatar->getGateway().string_length)).c_str(),
			    Unicode::wideToNarrow(Unicode::String(newAvatar->getLoginLocation().string_data, newAvatar->getLoginLocation().string_length)).c_str());

			ChatServer::setOwnerSystem(newAvatar);
			static Unicode::String wideSWG = Unicode::narrowToWide("SOE+SWG");
			static Unicode::String wideFilter = Unicode::narrowToWide("");
			static ChatUnicodeString swgNode(wideSWG.data(), wideSWG.size());
			static ChatUnicodeString filter(wideFilter.data(), wideFilter.size());
			IGNORE_RETURN(RequestGetRoomSummaries(swgNode, filter, nullptr));
		}
	}
	else
	{
		// look up a connection for this avatar
		std::unordered_map<std::string, NetworkId>::iterator f = pendingAvatars.find(id.name);
		if(f != pendingAvatars.end())
		{
			GenericValueTypeMessage<NetworkId> const avatarConnectedMessage("ChatAvatarConnected", (*f).second);
			ChatServer::broadcastToGameServers(avatarConnectedMessage);
			ConnectionServerConnection * c = ChatServer::getConnectionForCharacter((*f).second);
			if(c && newAvatar)
			{
				ChatServerAvatarOwner * owner = new ChatServerAvatarOwner(c, (*f).second);
				ChatAvatarId avId;
				makeAvatarId(*newAvatar, avId);
				avatarMap[avId] = owner;

				if(ChatServer::isGod(f->second))
				{
					RequestSetAvatarAttributes(newAvatar, AVATARATTR_GM, nullptr);
				}
				else
				{
					RequestSetAvatarAttributes(newAvatar, 0, nullptr);
				}

				ChatServer::chatConnectedAvatar((*f).second, *newAvatar);
				
				// flush chat messages pending for this avatar
				std::unordered_map<NetworkId, std::vector<Archive::ByteStream>, NetworkId::Hash >::iterator df = deferredChatMessages.find((*f).second);
				if(df != deferredChatMessages.end())
				{
					std::vector<Archive::ByteStream> & v = (*df).second;
					std::vector<Archive::ByteStream>::const_iterator i;
					for(i = v.begin(); i != v.end(); ++i)
					{
						const_cast<ConnectionServerConnection *>(c)->onReceive((*i));
					}
					deferredChatMessages.erase(df);
				}
				//ChatServer::getFriendsList(id);
				
				clearQueuedHeadersForAvatar(avId);
				IGNORE_RETURN(RequestGetPersistentHeaders(newAvatar, nullptr));
				//REPORT_LOG(true, ("Connection to chat system for %s.%s.%s confirmed\n", avatar.GetGameCode().c_str(), avatar.GetGameServerName().c_str(), avatar.GetCharacterName().c_str()));
			}//lint !e429 Custodial pointer 'owner' has not been freed or returned // jrandall avatar owns it
			pendingAvatars.erase(f);
		}
		else
		{
			// no way to route messages to avatar, they disconnected between
			// the time they connected to the connection server and the time
			// this message arrived from the chat backend.
			if (newAvatar)
				IGNORE_RETURN(RequestLogoutAvatar(newAvatar, nullptr));
		}
	}
	ChatOnConnectAvatar const connect;
	sendMessageToAvatar(id, connect);	

	
	// make my owner object
}

//-----------------------------------------------------------------------

void ChatInterface::OnCreateRoom(unsigned track,unsigned result, const ChatRoom *newRoom, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnCreateRoom() track(%u) result(%u) newRoom(%s)", track, result, ChatServer::getChatRoomNameNarrow(newRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnCreateRoom");
	UNREF(user);

	unsigned sequence = (unsigned)user;
	static Unicode::String wideSWG = Unicode::narrowToWide("SOE+SWG");
	static Unicode::String wideFilter = Unicode::narrowToWide("");
	static ChatUnicodeString swgNode(wideSWG.data(), wideSWG.size());
	static ChatUnicodeString filter(wideFilter.data(), wideFilter.size());
	//printf("!!!!Calling GetRoomSummaries from OnCreateRoom\n");
	//IGNORE_RETURN(RequestGetRoomSummaries(swgNode, filter, nullptr));

	ChatRoomData roomData;

	if((result == CHATRESULT_SUCCESS) && (newRoom)) //lint !e641 enum to int 
	{
		ChatServerRoomOwner * o = new ChatServerRoomOwner(newRoom);

		roomData = o->getRoomData();

		//Unicode::String wideRoomName(newRoom->getRoomName().string_data, newRoom->getRoomName().string_length);
		std::string roomName;
		makeRoomName(newRoom, roomName);
		if (strstr(roomName.c_str(), "group") == nullptr)
		{
			ChatServer::putSystemAvatarInRoom(roomName);
		}
		std::string lowerRoomName = toLower(roomName);
		IGNORE_RETURN(roomList.insert(std::make_pair(lowerRoomName, *o)));
		//REPORT_LOG(true, ("OnCreateRoom(%s) confirmed\n", roomData.path.c_str()));
		delete o;
		ChatServer::onCreateRoomSuccess(lowerRoomName, newRoom->getRoomID());
	} //lint !e429 Pointer 'o' not freed or returned: Set in SetRoomOwnerPtr

	ChatOnCreateRoom const c(sequence, result, roomData);
	IGNORE_RETURN(ChatServer::sendResponseForTrackId(track, c));

	Connection * const connection = safe_cast<Connection *>(ChatServer::getGameServerConnection(sequence));
	if (connection)
	{
		static Archive::ByteStream a;
		a.clear();
		c.pack(a);
		connection->send(a, true);
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnGetRoomSummaries(unsigned track, unsigned result, unsigned numFoundRooms, RoomSummary *foundRooms, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnGetRoomSummaries() track(%u) result(%u) numFoundRooms(%u)", track, result, numFoundRooms);

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnGetRoomSummaries");
	UNREF(track);
	UNREF(result);
	UNREF(user);

	if (foundRooms)
	{
		for (unsigned i = 0; i < numFoundRooms; ++i)
		{
			std::string roomName;
			makeRoomName(&(foundRooms[i]), roomName);

			std::string lowerRoomName = toLower(roomName);
			std::unordered_map<std::string, ChatServerRoomOwner>::const_iterator f = roomList.find(lowerRoomName);
			if(f == roomList.end())
			{
				RequestGetRoom(foundRooms[i].getRoomAddress(), nullptr);
				//ChatServerRoomOwner * o = new ChatServerRoomOwner((*i));
				//(*i)->SetRoomOwnerPtr(o);
				//IGNORE_RETURN(roomList.insert(std::make_pair(lowerRoomName, *o)));
				//printf("insert %s, roomList size = %u\n", lowerRoomName.c_str(), roomList.size());
			} //lint !e429 pointer not freed or returned, it is set as ownerPtr
		}
	}
}

//-----------------------------------------------------------------------

const ChatServerRoomOwner *ChatInterface::getRoomOwner(const std::string & roomName)
{
	std::string lowerName = toLower(roomName);
	std::unordered_map<std::string, ChatServerRoomOwner>::const_iterator f = roomList.find(lowerName);
	if(f != roomList.end())
	{
		return &((*f).second);
	}
	return nullptr;
}

//-----------------------------------------------------------------------

ChatServerRoomOwner *ChatInterface::getRoomOwner(unsigned roomId)
{
	std::unordered_map<std::string, ChatServerRoomOwner>::iterator f = roomList.begin();
	while (f != roomList.end())
	{
		if ((*f).second.getRoomData().id == roomId)
		{
			return &((*f).second);
		}
		++f;
	}
	return nullptr;
}

//-----------------------------------------------------------------------

void ChatInterface::OnReceiveDestroyRoom(const ChatAvatar *srcAvatar, const ChatRoom *destRoom)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveDestroyRoom() srcAvatar(%s) destRoom(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveDestroyRoom");
	ChatAvatarId destroyer;
	if (srcAvatar)
		makeAvatarId(*srcAvatar, destroyer);

	if (destRoom)
	{
		const ChatOnDestroyRoom c(0, 0, destRoom->getRoomID(), destroyer);
		AvatarIterator i = destRoom->getFirstAvatar();

		/*for(; !i.outOfBounds(); ++i)
		{
			sendMessageToAvatar(*(*i), c);
		}*/
		sendMessageToAllAvatars(c);

		std::string roomName;
		makeRoomName(destRoom, roomName);
		std::string lowerName = toLower(roomName);
		std::unordered_map<std::string, ChatServerRoomOwner>::iterator f = roomList.find(lowerName);
		if(f != roomList.end())
		{
			roomList.erase(f);
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnDestroyRoom(unsigned track, unsigned result, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnDestroyRoom() track(%u) result(%u)", track, result);

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnDestroyRoom");
	static Unicode::String wideSWG = Unicode::narrowToWide("SOE+SWG");
	static Unicode::String wideFilter = Unicode::narrowToWide("");
	static ChatUnicodeString swgNode(wideSWG.data(), wideSWG.size());
	static ChatUnicodeString filter(wideFilter.data(), wideFilter.size());
	//IGNORE_RETURN(RequestGetRoomSummaries(swgNode, filter, nullptr));

	ChatAvatarId destroyer;

	RoomOwnerSequencePair *pair = (RoomOwnerSequencePair *)user;

	const ChatServerRoomOwner * owner = nullptr;
	unsigned sequence = 0;
	if (pair)
	{
		owner = pair->roomOwner;
		sequence = pair->sequence;

		delete pair;
		pair = 0;
	}
	
	if (owner)
	{
		const ChatOnDestroyRoom c(sequence, result, owner->getRoomData().id, destroyer);
		NetworkId responseNetworkId = ChatServer::sendResponseForTrackId(track, c);

		if (result == CHATRESULT_SUCCESS)
		{
			std::string lowerName = toLower(owner->getRoomData().path);
			std::unordered_map<std::string, ChatServerRoomOwner>::iterator f = roomList.find(lowerName);
			if(f != roomList.end())
			{
				roomList.erase(f);
			}
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnLogoutAvatar(unsigned track, unsigned result, const ChatAvatar *oldAvatar, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnLogoutAvatar() track(%u) result(%u) oldAvatar(%s)", track, result, ChatServer::getFullChatAvatarName(oldAvatar).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnLogoutAvatar");
	UNREF(user);
	UNREF(result);
	UNREF(track);
	if (!oldAvatar)
	{
		return;
	}

	s_mostRecentRoomChatMessage.erase(oldAvatar->getAvatarID());

	/*ChatAvatarId id;
	makeAvatarId(*oldAvatar, id);
	std::map<ChatAvatarId, ChatServerAvatarOwner *>::iterator f = avatarMap.find(id);
	if(f != avatarMap.end())
	{*/
		ChatServer::disconnectAvatar(*oldAvatar);
	//}

	ChatAvatarId id;
	makeAvatarId(*oldAvatar, id);
	clearQueuedHeadersForAvatar(id);
}

//-----------------------------------------------------------------------

void ChatInterface::OnDestroyAvatar(unsigned track, unsigned result, const ChatAvatar *oldAvatar, void *user)
{
	UNREF(user);

	std::map<unsigned, std::pair<std::pair<ChatUnicodeString, ChatUnicodeString>, int> >::const_iterator iterFind = trackingRequestGetAnyAvatarForDestroy.find(track);
	if (iterFind != trackingRequestGetAnyAvatarForDestroy.end())
	{
		if ((result == CHATRESULT_SUCCESS) && oldAvatar)
		{
			// stop if it looks like we're in an infinite loop
			if (iterFind->second.second <= 25)
			{
				unsigned const trackID = RequestGetAnyAvatar(iterFind->second.first.first, iterFind->second.first.second, nullptr);
				trackingRequestGetAnyAvatarForDestroy[trackID] = std::make_pair(iterFind->second.first, (iterFind->second.second + 1));
			}
		}

		trackingRequestGetAnyAvatarForDestroy.erase(track);
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnGetAnyAvatar(unsigned track, unsigned result, const ChatAvatar *foundAvatar, bool loggedIn, void *user)
{
	UNREF(user);

	std::map<unsigned, std::pair<std::pair<ChatUnicodeString, ChatUnicodeString>, int> >::const_iterator iterFind = trackingRequestGetAnyAvatarForDestroy.find(track);
	if (iterFind != trackingRequestGetAnyAvatarForDestroy.end())
	{
		if ((result == CHATRESULT_SUCCESS) && foundAvatar)
		{
			// can only destroy the avatar if he is logged in
			if (loggedIn)
			{
				unsigned const trackID = RequestDestroyAvatar(foundAvatar, nullptr);
				trackingRequestGetAnyAvatarForDestroy[trackID] = iterFind->second;
			}
			// log in the chat avatar so we can destroy him
			else
			{
				unsigned const trackID = RequestLoginAvatar(foundAvatar->getUserID(), foundAvatar->getName(), foundAvatar->getAddress(), nullptr);
				trackingRequestGetAnyAvatarForDestroy[trackID] = iterFind->second;
			}
		}

		trackingRequestGetAnyAvatarForDestroy.erase(track);
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnReceiveForcedLogout(const ChatAvatar *oldAvatar)
{
	ChatServer::fileLog(false, "ChatInterface", "OnReceiveForcedLogout() oldAvatar(%s)", ChatServer::getFullChatAvatarName(oldAvatar).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveForcedLogout");
	if (oldAvatar == nullptr)
	{
		DEBUG_WARNING(true, ("We received an OnReceiveForcedLogout with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}
	ChatAvatarId id;
	makeAvatarId(*oldAvatar, id);
	std::map<ChatAvatarId, ChatServerAvatarOwner *>::iterator f = avatarMap.find(id);
	if(f != avatarMap.end())
	{
		ChatServer::disconnectAvatar(*oldAvatar);
	}
	else
	{
		DEBUG_WARNING(true, ("We received a forced logout for an avatar who is awaiting login. Usually this will happen on slow boxes when you log out and back in quickly, exploiting a race condition in the backend.  No harm should come from this."));
	}
}

//-----------------------------------------------------------------------
// Response to RequestEnterRoom(...)
void ChatInterface::OnEnterRoom(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnEnterRoom() track(%u) result(%u) srcAvatar(%s) destRoom(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnEnterRoom");

	if (result == CHATRESULT_SUCCESS && (srcAvatar == nullptr || destRoom == nullptr))
	{
		DEBUG_WARNING(true, ("We received an OnEnterRoom with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}

	unsigned sequence = (unsigned)user;

	ChatAvatarId srcId;
	if (srcAvatar)
	{
		makeAvatarId(*srcAvatar, srcId);
	}

	unsigned roomId = 0;
	if (destRoom)
	{
		roomId = destRoom->getRoomID();
	}

	// If we had a success, we need to tell the client about the room
	// so that they have the information to process entering the room
	if (result == CHATRESULT_SUCCESS)
	{
		// Send the room data for the room the avatar has entered
		// since the room may be private and may be unknown to the player
		if (destRoom)
		{
			ChatServerRoomOwner *roomOwner = getRoomOwner(destRoom->getRoomID());
			if (roomOwner)
			{
				std::vector<ChatRoomData> rooms;

				ChatRoomData data = roomOwner->getRoomData();
				rooms.push_back(data);
				
				ChatRoomList r(rooms);
				ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnEnterRoom() sending room list of %u rooms to (%s)", r.getRoomData().size(), ChatServer::getFullChatAvatarName(srcAvatar).c_str());
				sendMessageToAvatar(srcId, r); 
			}
		}
	}

	// Tell the client that made the request the results
	ChatOnEnteredRoom const msg(sequence, result, roomId, srcId);
	NetworkId responseNetworkId = ChatServer::sendResponseForTrackId(track, msg);

	// If we had a success, tell everyone else in the room about it
	if (result == CHATRESULT_SUCCESS)
	{
		// Tell the client that entered the room if they did not make the request
		ChatServerAvatarOwner * const owner = getAvatarOwner(srcAvatar);
		if (owner && (owner->getNetworkId() != responseNetworkId))
		{
			sendMessageToAvatar(srcId, msg);
		}

		if (destRoom)
		{
			std::string roomName;
			makeRoomName(destRoom, roomName);

			// Let everyone in the room know the client entered the room
			if (roomName.find(".system") == roomName.npos)
			{
				ChatAvatarId responseId;
				{
					ChatAvatar const * const responseAvatar = ChatServer::getAvatarByNetworkId( responseNetworkId );
					if (responseAvatar)
					{
						makeAvatarId(*responseAvatar, responseId);
					}
				}

				AvatarIterator i = destRoom->getFirstAvatar();
				for(; !i.outOfBounds(); ++i)
				{
					ChatAvatarId id;
					makeAvatarId(*(*i), id);

					// We already sent a response to the avatar that made the request
					if ((id != responseId) && (id != srcId))
					{
						sendMessageToAvatar(id, msg);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveEnterRoom
//    Called when srcAvatar has entered destRoom and this API has avatars
//    that are in destRoom. If srcAvatar is on this API and no others from
//    this API are in the room, this is not called.
void ChatInterface::OnReceiveEnterRoom(const ChatAvatar *srcAvatar, const ChatRoom *destRoom)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveEnterRoom() srcAvatar(%s) destRoom(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveEnterRoom");

	if (destRoom)
	{
		std::string roomName;
		makeRoomName(destRoom, roomName);

		// Update info about the room
		std::unordered_map<std::string, ChatServerRoomOwner>::iterator f = roomList.find(toLower(roomName));
		if(f != roomList.end())
			(*f).second.updateRoomData(destRoom);

		// See who created the room
		ChatAvatarId creatorAvatarId;
		makeAvatarId(destRoom->getCreatorName(), destRoom->getCreatorAddress(), creatorAvatarId);

		// Clients don't want to know who belongs to the system created rooms
		// NOTE: The name for the system avatar should be the same for all clusters
		if (!Unicode::caseInsensitiveCompare(creatorAvatarId.getName(), ChatServer::getSystemAvatarId().getName()))
		{
			ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveEnterRoom() updating clients with member info for chatRoom(%s)", ChatServer::getChatRoomNameNarrow(destRoom).c_str());

			// Tell all the avatars in the room the new member info
			updateRoomForThisChatAPI(destRoom, srcAvatar);
		}
	}
}

//-----------------------------------------------------------------------
// The OnReceive___() callbacks for chat rooms get called for every
// Chat API that has avatars in the room.  So limit updates to the
// avatars that are located on this Chat API.
void ChatInterface::updateRoomForThisChatAPI(const ChatRoom *room, const ChatAvatar *additionalAvatar)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "updateRoomForThisChatAPI() room(%s)", ChatServer::getChatRoomNameNarrow(room).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - updateRoomForThisChatAPI");

	if (room)
	{
		ChatServerRoomOwner *owner = getRoomOwner(room->getRoomID());
		if (owner)
		{
			owner->updateRoomData(room);

			ChatAvatarId av;
			std::vector<ChatAvatarId> avatars;
			
			for(AvatarIterator a = room->getFirstAvatar(); !a.outOfBounds(); ++a)
			{
				makeAvatarId(*(*a), av);
				avatars.push_back(av);
			}

			std::vector<ChatAvatarId> moderators;
			for(ModeratorIterator m = room->getFirstModerator(); !m.outOfBounds(); ++m)
			{
				makeAvatarId(*(*m), av);
				moderators.push_back(av);
			}

			std::vector<ChatAvatarId> invitees;
			for(InviteIterator i = room->getFirstInvited(); !i.outOfBounds(); ++i)
			{
				makeAvatarId(*(*i), av);
				invitees.push_back(av);
			}

			std::vector<ChatAvatarId> banned;
			for(BanIterator b = room->getFirstBanned(); !b.outOfBounds(); ++b)
			{
				makeAvatarId(*(*b), av);
				banned.push_back(av);
			}

			ChatAvatarId additionalAvatarId;
			if (additionalAvatar)
			{
				makeAvatarId(*additionalAvatar, additionalAvatarId);
			}

			// We only want to update the additional avatar once
			bool wasAdditionalAvatarUpdated = false;

			ChatQueryRoomResults results(0, owner->getRoomData(), avatars, moderators, invitees, banned);
			for(AvatarIterator avs = room->getFirstAvatar(); !avs.outOfBounds(); ++avs)
			{
				makeAvatarId(*(*avs), av);

				// Only send to avatars for this Chat API
				if (av.cluster == ChatServer::getSystemAvatarId().cluster)
				{
					ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "updateRoomForThisChatAPI() sending room members (%u avatars, %u moderators, %u invitees, %u banned) to (%s)",
					                                          avatars.size(), moderators.size(), invitees.size(), banned.size(), av.getFullName().c_str());
					sendMessageToAvatar(av, results);

					if (additionalAvatar && (av == additionalAvatarId))
					{
						wasAdditionalAvatarUpdated = true;
					}
				}
			}

			if (additionalAvatar && !wasAdditionalAvatarUpdated)
			{
				ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "updateRoomForThisChatAPI() sending room members (%u avatars, %u moderators, %u invitees, %u banned) to (%s)",
					                                        avatars.size(), moderators.size(), invitees.size(), banned.size(), additionalAvatarId.getFullName().c_str());
				sendMessageToAvatar(additionalAvatarId, results);
			}
		}
	}
}

//-----------------------------------------------------------------------
// Response to RequestAddBan(...)
void ChatInterface::OnAddBan(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnAddBan() track(%u) result(%u) srcAvatar(%s) destRoom(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnAddBan");

	if (result == CHATRESULT_SUCCESS && (srcAvatar == nullptr || destRoom == nullptr))
	{
		DEBUG_WARNING(true, ("We received an OnAddBan with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}

	AvatarIdSequencePair *pair = (AvatarIdSequencePair *)user;

	unsigned sequence = 0;
	ChatAvatarId destId;
	if (pair)
	{
		destId = pair->avatar;
		sequence = pair->sequence;

		delete pair;
		pair = nullptr;
	}

	ChatAvatarId srcId;
	if (srcAvatar)
	{
		makeAvatarId(*srcAvatar, srcId);
	}

	std::string roomName;
	if (destRoom)
	{
		makeRoomName(destRoom, roomName);
	}

	// Tell the player making the request whether the ban was successful
	ChatOnBanAvatarFromRoom const msg(sequence, result, roomName, srcId, destId);
	NetworkId responseNetworkId = ChatServer::sendResponseForTrackId(track, msg);

	// If we had a success, tell everyone else in the room about it
	if (result == CHATRESULT_SUCCESS)
	{
		if (destRoom)
		{
			ChatAvatarId responseId;
			{
				ChatAvatar const * const responseAvatar = ChatServer::getAvatarByNetworkId( responseNetworkId );
				if (responseAvatar)
				{
					makeAvatarId(*responseAvatar, responseId);
				}
			}

			AvatarIterator i = destRoom->getFirstAvatar();
			for(; !i.outOfBounds(); ++i)
			{
				ChatAvatarId id;
				makeAvatarId(*(*i), id);

				// We already sent a response to the avatar that made the request
				if (id != responseId)
				{
					sendMessageToAvatar(id, msg);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveAddBanRoom
//    Called when srcAvatar has banned destAvatar from destRoom and this
//    API has avatars that are in destRoom.
void ChatInterface::OnReceiveAddBanRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveAddBanRoom() srcAvatar(%s) destAvatar(%s) destRoom(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	// NOTE: Do not put anything in this callback that uses destAvatar.
	//       Currently there is a bug where the address and name are not set correctly in destAvatar.

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveAddBanRoom");

	if (destRoom)
	{
		std::string roomName;
		makeRoomName(destRoom, roomName);

		// Update info about the room
		std::unordered_map<std::string, ChatServerRoomOwner>::iterator f = roomList.find(toLower(roomName));
		if(f != roomList.end())
			(*f).second.updateRoomData(destRoom);

		// See who created the room
		ChatAvatarId creatorAvatarId;
		makeAvatarId(destRoom->getCreatorName(), destRoom->getCreatorAddress(), creatorAvatarId);

		// Clients don't want to know who belongs to the system created rooms
		// NOTE: The name for the system avatar should be the same for all clusters
		if (!Unicode::caseInsensitiveCompare(creatorAvatarId.getName(), ChatServer::getSystemAvatarId().getName()))
		{
			ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveAddBanRoom() updating clients with member info for chatRoom(%s)", ChatServer::getChatRoomNameNarrow(destRoom).c_str());

			// Tell all the avatars in the room the new member info
			updateRoomForThisChatAPI(destRoom, destAvatar);
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveAddBanAvatar
//    Called when srcAvatar has banned destAvatar from the room described by
//    roomName and roomAddress, and destAvatar is on this API.
void ChatInterface::OnReceiveAddBanAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveAddBanAvatar() srcAvatar(%s) destAvatar(%s) roomName(%s) roomAddress(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::toNarrowString(roomName).c_str(), ChatServer::toNarrowString(roomAddress).c_str());
}

//-----------------------------------------------------------------------
// Response to RequestRemoveBan(...)
void ChatInterface::OnRemoveBan(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnRemoveBan() track(%u) result(%u) srcAvatar(%s) destRoom(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnRemoveBan");

	if (result == CHATRESULT_SUCCESS && (srcAvatar == nullptr || destRoom == nullptr))
	{
		DEBUG_WARNING(true, ("We received an OnRemoveBan with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}

	AvatarIdSequencePair *pair = (AvatarIdSequencePair *)user;

	unsigned sequence = 0;
	ChatAvatarId destId;
	if (pair)
	{
		destId = pair->avatar;
		sequence = pair->sequence;

		delete pair;
		pair = nullptr;
	}

	ChatAvatarId srcId;
	if (srcAvatar)
	{
		makeAvatarId(*srcAvatar, srcId);
	}

	std::string roomName;
	if (destRoom)
	{
		makeRoomName(destRoom, roomName);
	}

	// Tell the player making the request whether the ban was successful
	ChatOnUnbanAvatarFromRoom const msg(sequence, result, roomName, srcId, destId);
	NetworkId responseNetworkId = ChatServer::sendResponseForTrackId(track, msg);

	// If we had a success, tell everyone else in the room about it
	if (result == CHATRESULT_SUCCESS)
	{
		if (destRoom)
		{
			ChatAvatarId responseId;
			{
				ChatAvatar const * const responseAvatar = ChatServer::getAvatarByNetworkId( responseNetworkId );
				if (responseAvatar)
				{
					makeAvatarId(*responseAvatar, responseId);
				}
			}

			AvatarIterator i = destRoom->getFirstAvatar();
			for(; !i.outOfBounds(); ++i)
			{
				ChatAvatarId id;
				makeAvatarId(*(*i), id);

				// We already sent a response to the avatar that made the request
				if (id != responseId)
				{
					sendMessageToAvatar(id, msg);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveRemoveBanRoom
//    Called when srcAvatar has removed the ban on destAvatar from
//    destRoom and this API has avatars that are in destRoom.
void ChatInterface::OnReceiveRemoveBanRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveRemoveBanRoom() srcAvatar(%s) destAvatar(%s) destRoom(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveRemoveBanRoom");

	if (destRoom)
	{
		std::string roomName;
		makeRoomName(destRoom, roomName);

		// Update info about the room
		std::unordered_map<std::string, ChatServerRoomOwner>::iterator f = roomList.find(toLower(roomName));
		if(f != roomList.end())
			(*f).second.updateRoomData(destRoom);

		// See who created the room
		ChatAvatarId creatorAvatarId;
		makeAvatarId(destRoom->getCreatorName(), destRoom->getCreatorAddress(), creatorAvatarId);

		// Clients don't want to know who belongs to the system created rooms
		// NOTE: The name for the system avatar should be the same for all clusters
		if (!Unicode::caseInsensitiveCompare(creatorAvatarId.getName(), ChatServer::getSystemAvatarId().getName()))
		{
			ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveRemoveBanRoom() updating clients with member info for chatRoom(%s)", ChatServer::getChatRoomNameNarrow(destRoom).c_str());

			// Tell all the avatars in the room the new member info
			updateRoomForThisChatAPI(destRoom, destAvatar);
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveRemoveBanAvatar
//    Called when srcAvatar has removed the ban on destAvatar from
//    the room described by roomName and roomAddress, and destAvatar is on
//    this API.
void ChatInterface::OnReceiveRemoveBanAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveRemoveBanAvatar() srcAvatar(%s) destAvatar(%s) roomName(%s) roomAddress(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::toNarrowString(roomName).c_str(), ChatServer::toNarrowString(roomAddress).c_str());
}

//-----------------------------------------------------------------------
// Response to RequestAddInvite(...)
void ChatInterface::OnAddInvite(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnAddInvite() track(%u) result(%u) srcAvatar(%s) destRoom(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnAddInvite");

	if (result == CHATRESULT_SUCCESS && (srcAvatar == nullptr || destRoom == nullptr))
	{
		DEBUG_WARNING(true, ("We received an OnAddInvite with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}

	AvatarIdSequencePair *pair = (AvatarIdSequencePair *)user;

	ChatAvatarId destId;
	if (pair)
	{
		destId = pair->avatar;

		delete pair;
		pair = nullptr;
	}

	ChatAvatarId srcId;
	if (srcAvatar)
	{
		makeAvatarId(*srcAvatar, srcId);
	}

	std::string roomName;
	if (destRoom)
	{
		makeRoomName(destRoom, roomName);
	}

	// Tell the player making the request whether the invite was successful
	ChatOnInviteToRoom const msg(result, roomName, srcId, destId);
	NetworkId responseNetworkId = ChatServer::sendResponseForTrackId(track, msg);

	if (result == CHATRESULT_SUCCESS)
	{
		if (destRoom)
		{
			// Calling this on a room will cause the API to cache the room
			// and to receive room updates for the room.
			RequestGetRoom(destRoom->getAddress(), nullptr);

			// Send the room data for the room the avatar was invited to join
			// since the room may be private and may be unknown to the player
			ChatServerRoomOwner const * const roomOwner = getRoomOwner(destRoom->getRoomID());
			if (roomOwner)
			{
				std::vector<ChatRoomData> rooms;

				ChatRoomData data = roomOwner->getRoomData();
				rooms.push_back(data);
				
				ChatRoomList const r(rooms);
				ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnAddInvite() sending room list of %u rooms to (%s)", r.getRoomData().size(), destId.getFullName().c_str());
				sendMessageToAvatar(destId, r); 
			}
		}

		// Tell the invited person about the invitation
		ChatOnReceiveRoomInvitation const chat(roomName, srcId);
		sendMessageToAvatar(destId, chat);
	}
}

//-----------------------------------------------------------------------
// OnReceiveAddInviteRoom
//    Called when srcAvatar has invited destAvatar to destRoom and this
//    API has avatars that are in this room.
void ChatInterface::OnReceiveAddInviteRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveAddInviteRoom() srcAvatar(%s) destAvatar(%s) destRoom(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveAddInviteRoom");

	if (destRoom)
	{
		std::string roomName;
		makeRoomName(destRoom, roomName);

		// Update info about the room
		std::unordered_map<std::string, ChatServerRoomOwner>::iterator f = roomList.find(toLower(roomName));
		if(f != roomList.end())
			(*f).second.updateRoomData(destRoom);

		// See who created the room
		ChatAvatarId creatorAvatarId;
		makeAvatarId(destRoom->getCreatorName(), destRoom->getCreatorAddress(), creatorAvatarId);

		// Clients don't want to know who belongs to the system created rooms
		// NOTE: The name for the system avatar should be the same for all clusters
		if (!Unicode::caseInsensitiveCompare(creatorAvatarId.getName(), ChatServer::getSystemAvatarId().getName()))
		{
			ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveAddInviteRoom() updating clients with member info for chatRoom(%s)", ChatServer::getChatRoomNameNarrow(destRoom).c_str());

			// Tell all the avatars in the room the new member info
			updateRoomForThisChatAPI(destRoom, 0);
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveAddInviteAvatar
//    Called when srcAvatar has invited destAvatar to the room described
//    by roomName and roomAddress, and destAvatar is on this API.
void ChatInterface::OnReceiveAddInviteAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveAddInviteAvatar() srcAvatar(%s) destAvatar(%s) roomName(%s) roomAddress(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::toNarrowString(roomName).c_str(), ChatServer::toNarrowString(roomAddress).c_str());
}

//-----------------------------------------------------------------------
// Response to RequestRemoveInvite(...)
void ChatInterface::OnRemoveInvite(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnRemoveInvite() track(%u) result(%u) srcAvatar(%s) destRoom(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnRemoveInvite");

	if (result == CHATRESULT_SUCCESS && (srcAvatar == nullptr || destRoom == nullptr))
	{
		DEBUG_WARNING(true, ("We received an OnRemoveInvite with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}

	AvatarIdSequencePair *pair = (AvatarIdSequencePair *)user;

	unsigned sequence = 0;
	ChatAvatarId inviteeId;
	if (pair)
	{
		inviteeId = pair->avatar;
		sequence = pair->sequence;

		delete pair;
		pair = nullptr;
	}

	std::string roomName;
	if (destRoom)
	{
		makeRoomName(destRoom, roomName);
	}

	ChatAvatarId invitorId;
	if (srcAvatar)
	{
		makeAvatarId(*srcAvatar, invitorId);
	}

	//if the sequence is 0, the message initiated from the game server.  Don't
	//bother telling the client that something failed when they didn't even 
	//ask to do it.
	if (sequence != 0)
	{
		ChatOnUninviteFromRoom const msg(sequence, result, roomName, invitorId, inviteeId);
		IGNORE_RETURN(ChatServer::sendResponseForTrackId(track, msg));
	}
	else
	{
		// We still need to somehow remove the pending request from the queue
		IGNORE_RETURN(ChatServer::getAvatarIdForTrackId(track));
	}
}

//-----------------------------------------------------------------------
// OnReceiveRemoveInviteRoom
//    Called when srcAvatar has uninvited destAvatar from destRoom and
//    this API has avatars that are in this room.
void ChatInterface::OnReceiveRemoveInviteRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveRemoveInviteRoom() srcAvatar(%s) destAvatar(%s) chatRoom(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveRemoveInviteRoom");

	if (destRoom)
	{
		std::string roomName;
		makeRoomName(destRoom, roomName);

		// Update info about the room
		std::unordered_map<std::string, ChatServerRoomOwner>::iterator f = roomList.find(toLower(roomName));
		if(f != roomList.end())
			(*f).second.updateRoomData(destRoom);

		// See who created the room
		ChatAvatarId creatorAvatarId;
		makeAvatarId(destRoom->getCreatorName(), destRoom->getCreatorAddress(), creatorAvatarId);

		// Clients don't want to know who belongs to the system created rooms
		// NOTE: The name for the system avatar should be the same for all clusters
		if (!Unicode::caseInsensitiveCompare(creatorAvatarId.getName(), ChatServer::getSystemAvatarId().getName()))
		{
			ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveRemoveInviteRoom() updating clients with member info for chatRoom(%s)", ChatServer::getChatRoomNameNarrow(destRoom).c_str());

			// Tell all the avatars in the room the new member info
			updateRoomForThisChatAPI(destRoom, 0);
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveRemoveInviteAvatar
//    Called when srcAvatar has uninvited destAvatar from the room 
//    described by roomName and roomAddress, and destAvatar is on 
//    this API.
void ChatInterface::OnReceiveRemoveInviteAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveRemoveInviteAvatar() srcAvatar(%s) destAvatar(%s) roomName(%s) roomAddress(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::toNarrowString(roomName).c_str(), ChatServer::toNarrowString(roomAddress).c_str());
}

//-----------------------------------------------------------------------
// Response to RequestLeaveRoom(...)
void ChatInterface::OnLeaveRoom(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnLeaveRoom() track(%u) result(%u) srcAvatar(%s) destRoom(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnLeaveRoom");

	if (result == CHATRESULT_SUCCESS && (srcAvatar == nullptr || destRoom == nullptr))
	{
		DEBUG_WARNING(true, ("We received an OnLeaveRoom with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}

	unsigned sequence = (unsigned)user;

	ChatAvatarId id;
	if (srcAvatar)
	{
		makeAvatarId(*srcAvatar, id);
	}

	unsigned roomId = 0;
	if (destRoom)
	{
		roomId = destRoom->getRoomID();
	}

	// Tell the person requesting to leave the success of the request
	ChatOnLeaveRoom const msg(sequence, result, roomId, id);
	IGNORE_RETURN(ChatServer::sendResponseForTrackId(track, msg));

	// We'll tell everyone else in the room about it in OnReceiveLeaveRoom()
}

//-----------------------------------------------------------------------
// OnReceiveLeaveRoom
//    Called when srcAvatar has left destRoom and this API has avatars
//    that are in destRoom.
void ChatInterface::OnReceiveLeaveRoom(const ChatAvatar *srcAvatar, const ChatRoom *destRoom)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveLeaveRoom() srcAvatar(%s) destRoom(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveLeaveRoom");

	if (destRoom)
	{
		std::string roomName;
		makeRoomName(destRoom, roomName);

		// Update info about the room
		std::unordered_map<std::string, ChatServerRoomOwner>::iterator const f = roomList.find(toLower(roomName));
		if(f != roomList.end())
			(*f).second.updateRoomData(destRoom);

		// See who created the room
		ChatAvatarId creatorAvatarId;
		makeAvatarId(destRoom->getCreatorName(), destRoom->getCreatorAddress(), creatorAvatarId);

		// Clients don't want to know who belongs to the system created rooms
		// NOTE: The name for the system avatar should be the same for all clusters
		if (!Unicode::caseInsensitiveCompare(creatorAvatarId.getName(), ChatServer::getSystemAvatarId().getName()))
		{
			ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveLeaveRoom() updating clients with member info for chatRoom(%s)", ChatServer::getChatRoomNameNarrow(destRoom).c_str());

			// Tell all the avatars in the room the new member info
			updateRoomForThisChatAPI(destRoom, srcAvatar);
		}
		// If we didn't send the updated room information, then just send the
		// message to everyone in the room that the person has left the room
		else if ((f != roomList.end()) && srcAvatar)
		{
			ChatAvatarId srcAvatarId;
			makeAvatarId(*srcAvatar, srcAvatarId);
			ChatOnLeaveRoom const msg(0, CHATRESULT_SUCCESS, destRoom->getRoomID(), srcAvatarId);

			AvatarIterator i = destRoom->getFirstAvatar();
			for(; !i.outOfBounds(); ++i)
			{
				ChatAvatarId id;
				makeAvatarId(*(*i), id);

				sendMessageToAvatar(id, msg);
			}
		}
	}
}

//-----------------------------------------------------------------------
// Response to RequestKickAvatar(...)
void ChatInterface::OnKickAvatar(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnKickAvatar() track(%u) result(%u) srcAvatar(%s) destRoom(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnKickAvatar");

	AvatarIdSequencePair *pair = (AvatarIdSequencePair *)user;

	ChatAvatarId kickedId;
	if (pair)
	{
		kickedId = pair->avatar;

		delete pair;
		pair = nullptr;
	}

	std::string roomName;
	if (destRoom)
	{
		makeRoomName(destRoom, roomName);
	}

	ChatAvatarId srcId;
	if (srcAvatar)
	{
		makeAvatarId(*srcAvatar, srcId);
	}

	// Tell the person making the kick request whether it was successful 
	ChatOnKickAvatarFromRoom const msg(result, kickedId, srcId, roomName);
	NetworkId responseNetworkId = ChatServer::sendResponseForTrackId(track, msg);

	// If we had a success, tell everyone else in the room about it
	if (result == CHATRESULT_SUCCESS)
	{
		if (destRoom)
		{
			ChatAvatarId responseId;
			{
				ChatAvatar const * const responseAvatar = ChatServer::getAvatarByNetworkId( responseNetworkId );
				if (responseAvatar)
				{
					makeAvatarId(*responseAvatar, responseId);
				}
			}

			AvatarIterator i = destRoom->getFirstAvatar();
			for(; !i.outOfBounds(); ++i)
			{
				ChatAvatarId id;
				makeAvatarId(*(*i), id);

				// We already sent a response to the avatar that made the request
				if (id != responseId)
				{
					sendMessageToAvatar(id, msg);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveKickRoom
//    Called when destAvatar has been kicked by srcAvatar from destRoom
//    and this API has avatars that are in destRoom.
void ChatInterface::OnReceiveKickRoom(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveKickRoom() srcAvatar(%s) destAvatar(%s) chatRoom(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveKickRoom");

	if (destRoom)
	{
		std::string roomName;
		makeRoomName(destRoom, roomName);

		// Update info about the room
		std::unordered_map<std::string, ChatServerRoomOwner>::iterator f = roomList.find(toLower(roomName));
		if(f != roomList.end())
			(*f).second.updateRoomData(destRoom);

		// See who created the room
		ChatAvatarId creatorAvatarId;
		makeAvatarId(destRoom->getCreatorName(), destRoom->getCreatorAddress(), creatorAvatarId);

		// Clients don't want to know who belongs to the system created rooms
		// NOTE: The name for the system avatar should be the same for all clusters
		if (!Unicode::caseInsensitiveCompare(creatorAvatarId.getName(), ChatServer::getSystemAvatarId().getName()))
		{
			ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveKickRoom() updating clients with member info for chatRoom(%s)", ChatServer::getChatRoomNameNarrow(destRoom).c_str());

			// Tell all the avatars in the room the new member info
			updateRoomForThisChatAPI(destRoom, destAvatar);
		}
	}
}

//-----------------------------------------------------------------------
// OnReceiveKickAvatar
//    Called when destAvatar has been kicked by srcAvatar from the room
//    described by roomName and roomAddress, and destAvatar is on
//    this API.
void ChatInterface::OnReceiveKickAvatar(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &roomName, const ChatUnicodeString &roomAddress)
{
	ChatServer::fileLog(s_enableChatRoomLogs, "ChatInterface", "OnReceiveKickAvatar() srcAvatar(%s) destAvatar(%s) roomName(%s) roomAddress(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::toNarrowString(roomName).c_str(), ChatServer::toNarrowString(roomAddress).c_str());
}

//-----------------------------------------------------------------------

void ChatInterface::OnGetPersistentMessage(unsigned track, unsigned result, ChatAvatar *destAvatar, const PersistentHeader *header, const ChatUnicodeString &msg, const ChatUnicodeString &oob, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnGetPersistentMessage() track(%u) result(%u) destAvatar(%s)", track, result, ChatServer::getFullChatAvatarName(destAvatar).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnGetPersistentMessage");
	if (result == CHATRESULT_SUCCESS && (destAvatar == nullptr))
	{
		DEBUG_WARNING(true, ("We received an OnGetPersistentMessage with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}
	UNREF(user);
	UNREF(track);

	if (!destAvatar || !header)
	{
		return;
	}

	if(result == CHATRESULT_SUCCESS) //lint !e641 Converting enum 'ChatResult' to int // jrandall That's what is passed to this method by the ChatAPI
	{
		ChatAvatarId fromId;
		fromId.name = Unicode::wideToNarrow(Unicode::String(header->getFromName().string_data, header->getFromName().string_length));
		std::string tmpAddress = Unicode::wideToNarrow(Unicode::String(header->getFromAddress().string_data, header->getFromAddress().string_length));
		char *str = (char *)tmpAddress.c_str();
		char *loc = strstr(str, ".");
		if (loc)
		{
			*loc = 0;
			fromId.gameCode = str;
			fromId.cluster = loc+1;
		}
		Unicode::String subject = (Unicode::String(header->getSubject().string_data, header->getSubject().string_length));
		//printf("PM from OnGetPersistentMessage\n");
		const ChatPersistentMessageToClient m(
			header->getMessageID(),
			convertPmStatus (header->getStatus()),
			fromId.gameCode,
			fromId.cluster,
			fromId.name,
			subject,
			Unicode::String(msg.string_data, msg.string_length),
			Unicode::String(oob.string_data, oob.string_length),
			header->getSentTime()									
			);
		ChatAvatarId toId;
		makeAvatarId(*destAvatar, toId);
		sendMessageToAvatar(toId, m);
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnGetPersistentHeaders(unsigned track, unsigned result, ChatAvatar *destAvatar, unsigned listLength, const PersistentHeader *list, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnGetPersistentHeaders() track(%u) result(%u) destAvatar(%s)", track, result, ChatServer::getFullChatAvatarName(destAvatar).c_str(), listLength);

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnGetPersistentHeaders");
	if (result == CHATRESULT_SUCCESS && (destAvatar == nullptr))
	{
		DEBUG_WARNING(true, ("We received an OnGetPersistentHeaders with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}
	UNREF(track);
	UNREF(user);
	if(result == CHATRESULT_SUCCESS) //lint !e641 Converting enum 'ChatResult' to int // jrandall That's what is passed to this method by the ChatAPI
	{
		if (!destAvatar || !list)
		{
			return;
		}
		ChatAvatarId toAvatar;
		makeAvatarId(*destAvatar, toAvatar);

		LOG("CustomerService", ("Login:retrieved %u persistent message headers for %s", listLength, toAvatar.getFullName().c_str()));

		clearQueuedHeadersForAvatar(toAvatar);

		int numberHeadersSent = 0;
		const unsigned long queuedHeadersSendTime = Clock::timeSeconds() + static_cast<unsigned long>(s_intervalToSendHeadersToClientSeconds);

		for(unsigned i = 0; i < listLength; ++i)
		{
			ChatAvatarId fromId;
			makeAvatarId(list[i].getFromName(), list[i].getFromAddress(), fromId);
			Unicode::String subject(list[i].getSubject().string_data, list[i].getSubject().string_length);
		//printf("PM from OnGetPersistentHeaders\n");

			if (numberHeadersSent < s_maxHeadersToSendToClientPerInterval)
			{
				const ChatPersistentMessageToClient m(
					list[i].getMessageID(),
					convertPmStatus (list[i].getStatus()),
					fromId.gameCode,
					fromId.cluster,
					fromId.name,
					subject,
					list[i].getSentTime());

				sendMessageToAvatar(toAvatar, m);

				++numberHeadersSent;
			}
			else
			{
				const ChatPersistentMessageToClient * m = new ChatPersistentMessageToClient(
					list[i].getMessageID(),
					convertPmStatus (list[i].getStatus()),
					fromId.gameCode,
					fromId.cluster,
					fromId.name,
					subject,
					list[i].getSentTime());

				addQueuedHeaderForAvatar(toAvatar, m, queuedHeadersSendTime);
			}
		}
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnReceivePersistentMessage(const ChatAvatar *destAvatar, const PersistentHeader *header)
{
	ChatServer::fileLog(false, "ChatInterface", "OnReceivePersistentMessage() destAvatar(%s)", ChatServer::getFullChatAvatarName(destAvatar).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceivePersistentMessage");
	if (destAvatar == nullptr)
	{
		DEBUG_WARNING(true, ("We received an OnREceivePersistentMessage with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}
	if (!header)
	{
		return;	
	}
	ChatAvatarId toAvatar;
	makeAvatarId(*destAvatar, toAvatar);

	ChatAvatarId fromId;
	makeAvatarId(header->getFromName(), header->getFromAddress(), fromId);
	Unicode::String subject(header->getSubject().string_data, header->getSubject().string_length);
	//printf("PM from OnReceivePersistentMessage\n");
	const ChatPersistentMessageToClient m(
		header->getMessageID(),
		convertPmStatus (header->getStatus()),
		fromId.gameCode,
		fromId.cluster,
		fromId.name,
		subject,
		header->getSentTime());
	sendMessageToAvatar(toAvatar, m);
}

//-----------------------------------------------------------------------

void ChatInterface::OnSendRoomMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, const ChatRoom *destRoom, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnSendRoomMessage() track(%u) result(%u) srcAvatar(%s) destRoom(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnSendRoomMessage");
	if (result == CHATRESULT_SUCCESS && (srcAvatar == nullptr || destRoom == nullptr))
	{
		DEBUG_WARNING(true, ("We received an OnsendRoomMessage with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}
	UNREF(srcAvatar);
	UNREF(destRoom);
	unsigned sequence = (unsigned)user;
    ChatOnSendRoomMessage chat(sequence, result);
    IGNORE_RETURN(ChatServer::sendResponseForTrackId(track, chat));
}

//-----------------------------------------------------------------------

void ChatInterface::OnReceiveRoomMessage(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatRoom *destRoom, const ChatUnicodeString &msg, const ChatUnicodeString &oob, unsigned messageID)
{
	ChatServer::fileLog(false, "ChatInterface", "OnReceiveRoomMessage() srcAvatar(%s) destAvatar(%s) destRoom(%s) messageID(%u)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str(), ChatServer::getChatRoomNameNarrow(destRoom).c_str(), messageID);

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveRoomMessage");
	if(! srcAvatar || ! destAvatar || ! destRoom)
	{
		DEBUG_WARNING(true, ("We received an OnREceiveRoomMessage with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}

	// check to prevent sending duplicate room chat message
	{
		unsigned const destAvatarId = destAvatar->getAvatarID();
		unsigned const destRoomId = destRoom->getRoomID();
		std::unordered_map<unsigned, std::pair<unsigned, unsigned> >::iterator const iterFind = s_mostRecentRoomChatMessage.find(destAvatarId);
		if (iterFind != s_mostRecentRoomChatMessage.end())
		{
			if ((iterFind->second.first == destRoomId) && (iterFind->second.second == messageID))
				return;

			iterFind->second.first = destRoomId;
			iterFind->second.second = messageID;
		}
		else
		{
			s_mostRecentRoomChatMessage.insert(std::make_pair(destAvatarId, std::make_pair(destRoomId, messageID)));
		}
	}

	ChatAvatarId fromId;
	makeAvatarId(*srcAvatar, fromId);
	GameNetworkMessage * gnm = 0;
	Unicode::String ustrMessage(msg.string_data, msg.string_length);
	Unicode::String outOfBand(oob.string_data, oob.string_length);
	ChatSystemMessage chat(ChatSystemMessage::BROADCAST, ustrMessage, outOfBand); //lint !e641 enum to int
	ChatRoomMessage m(destRoom->getRoomID(), fromId, ustrMessage, outOfBand);

	if(fromId == ChatServer::getSystemAvatarId())
	{
		gnm = &chat;
	}
	else
	{
		gnm = &m;
	}

	ChatAvatarId toId;
	makeAvatarId(*destAvatar, toId);
    
	sendMessageToAvatar(toId, *gnm);

	// don't log planet-wide chat
	const std::string chatRoomName = toLower(ChatServer::getChatRoomNameNarrow(destRoom));
	if(chatRoomName != "auction" && chatRoomName != "planet")
	{
		Unicode::String const wideTo(Unicode::narrowToWide(toId.getFullName()));
		Unicode::String const wideFrom(Unicode::narrowToWide(fromId.getFullName()));
	
		ChatServer::instance().logChatMessage(wideTo, wideFrom, Unicode::emptyString, ChatServer::toUnicodeString(msg), ChatServer::getChatRoomName(destRoom));
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnSendInstantMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnSendInstantMessage() track(%u) result(%u) srcAvatar(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnSendInstantMessage");

	if (result == CHATRESULT_SUCCESS && (srcAvatar == nullptr))
	{
		DEBUG_WARNING(true, ("We received an OnSendInstantMessage with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}
	UNREF(srcAvatar);

	unsigned sequence = (unsigned)user;

	ChatOnSendInstantMessage chat(sequence, result);
	IGNORE_RETURN(ChatServer::sendResponseForTrackId(track, chat));
}

//-----------------------------------------------------------------------

void ChatInterface::OnReceiveInstantMessage(const ChatAvatar *srcAvatar, const ChatAvatar *destAvatar, const ChatUnicodeString &msg, const ChatUnicodeString &oob)
{
	ChatServer::fileLog(false, "ChatInterface", "OnReceiveInstantMessage() srcAvatar(%s) destAvatar(%s)", ChatServer::getFullChatAvatarName(srcAvatar).c_str(), ChatServer::getFullChatAvatarName(destAvatar).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnReceiveInstantMessage");
	if ((srcAvatar == nullptr || destAvatar == nullptr))
	{
		DEBUG_WARNING(true, ("We received an OnReceiveInstantMessage with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}
	ChatAvatarId fromId;
	makeAvatarId(*srcAvatar, fromId);
	ChatAvatarId toId;
	makeAvatarId(*destAvatar, toId);
	if(fromId == ChatServer::getSystemAvatarId())
	{
		ChatSystemMessage chat(
			ChatSystemMessage::PERSONAL, 
			Unicode::String(msg.string_data, msg.string_length), 
			Unicode::String(oob.string_data, oob.string_length)); //lint !e641 enum to int
		sendMessageToAvatar(toId, chat);
	}
	else
	{
		
		ChatInstantMessageToClient chat(fromId, Unicode::String(msg.string_data, msg.string_length), 
			Unicode::String(oob.string_data, oob.string_length));
		sendMessageToAvatar(toId, chat);
		Unicode::String const wideTo(Unicode::narrowToWide(toId.getFullName()));
		Unicode::String const wideFrom(Unicode::narrowToWide(fromId.getFullName()));
		ChatServer::instance().logChatMessage(wideTo, wideFrom, wideTo, ChatServer::toUnicodeString(msg), ChatServer::getChannelTell());
		ChatServer::instance().logChatMessage(wideFrom, wideFrom, wideTo, ChatServer::toUnicodeString(msg), ChatServer::getChannelTell());
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnSendPersistentMessage(unsigned track, unsigned result, const ChatAvatar *srcAvatar, void *user)
{
	ChatServer::fileLog(false, "ChatInterface", "OnSendPersistentMessage() track(%u) result(%u) srcAvatar(%s)", track, result, ChatServer::getFullChatAvatarName(srcAvatar).c_str());

	PROFILER_AUTO_BLOCK_DEFINE("ChatInterface - OnSendPersistentMessage");
	if (result == CHATRESULT_SUCCESS && (srcAvatar == nullptr))
	{
		DEBUG_WARNING(true, ("We received an OnSendPersistentMessage with a success result code but nullptr data.  This is an error that the API should never give."));
		return;
	}
	unsigned sequence = (unsigned)user;
	UNREF(srcAvatar);
	ChatOnSendPersistentMessage chat(sequence, result);
	IGNORE_RETURN(ChatServer::sendResponseForTrackId(track, chat));
}

//-----------------------------------------------------------------------

const NetworkId & ChatInterface::getNetworkIdByAvatarId(const ChatAvatarId &id)
{
	std::map<ChatAvatarId, ChatServerAvatarOwner *>::iterator f = avatarMap.find(id);
	if(f != avatarMap.end())
	{
		ChatServerAvatarOwner * o = (*f).second;
		if (o)
		{
			return o->getNetworkId();
		}
	}
	else
	{
		//tmp hack because of the tolower thing
		ChatAvatarId lowerId = id;
		lowerId.name = toLower(id.name);
		f = avatarMap.find(lowerId);
		if(f != avatarMap.end())
		{
			ChatServerAvatarOwner * o = (*f).second;
			if (o)
			{
				return o->getNetworkId();
			}
		}
	}
	static NetworkId badId;
	return badId;
}

//-----------------------------------------------------------------------

void  ChatInterface::disconnectPlayer  (const ChatAvatarId & id)
{
	ChatServer::fileLog(false, "ChatInterface", "disconnectPlayer() id(%s)", id.getFullName().c_str());

	std::map<ChatAvatarId, ChatServerAvatarOwner *>::iterator f = avatarMap.find(id);
	if(f != avatarMap.end())
	{
		ChatServerAvatarOwner * o = (*f).second;
		delete o;
		avatarMap.erase(f);
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnUpdatePersistentMessages(unsigned track, unsigned result, const ChatAvatar *targetAvatar, void *user)
{
	UNREF(track);

	// Get the target chat avatar id

	ChatAvatarId targetChatAvatarId;

	if (targetAvatar != nullptr)
	{
		makeAvatarId(*targetAvatar, targetChatAvatarId);
	}

	// Get the source chat avatar id

	ChatAvatarId sourceChatAvatarId;
	NetworkId const *tmpNetworkId = reinterpret_cast<NetworkId const *>(user);

	if (tmpNetworkId != nullptr)
	{
		ChatAvatar const *sourceAvatar = ChatServer::getAvatarByNetworkId(*tmpNetworkId);

		if (sourceAvatar != nullptr)
		{
			makeAvatarId(*sourceAvatar, sourceChatAvatarId);
		}

		delete tmpNetworkId;
		tmpNetworkId = nullptr;
	}

	ChatServer::fileLog(false, "ChatInterface", "OnUpdatePersistentMessage() track(%u) result(%u) sourceAvatar(%s) targetAvatar(%s)", track, result, sourceChatAvatarId.getFullName().c_str(), targetChatAvatarId.getFullName().c_str());

	// Send a response to the person (source) who issued this message

	if (   !targetChatAvatarId.getFullName().empty()
	    && !sourceChatAvatarId.getFullName().empty())
	{
		std::string const &targetName = targetChatAvatarId.name;
		bool const success = (result == CHATRESULT_SUCCESS);

		ChatOnDeleteAllPersistentMessages msg(targetName, success);
		sendMessageToAvatar(sourceChatAvatarId, msg);
	}
	else
	{
		ChatServer::fileLog(false, "ChatInterface", "OnUpdatePersistentMessage() Unable to resolve source(%s) and target(%s).", sourceChatAvatarId.getFullName().c_str(), targetChatAvatarId.getFullName().c_str());
	}
}

//-----------------------------------------------------------------------

void ChatInterface::OnReceiveUnregisterRoomReady(const ChatRoom *destRoom)
{
	UNREF(destRoom);

	// Overloading default so we don't get a warning messsage
}

//-----------------------------------------------------------------------

void ChatInterface::OnTransferAvatar(unsigned track, unsigned result, unsigned oldUserID, unsigned newUserID, const ChatUnicodeString &oldName, const ChatUnicodeString &newName, const ChatUnicodeString &oldAddress, const ChatUnicodeString &newAddress, void *user)
{
	UNREF(user);
	LOG("CustomerService", ("CharacterTransfer: Received OnTransferAvatar(%d, %d, %d, %d, %s, %s, %s, %s)", track, result, oldUserID, newUserID, oldName.c_str(), newName.c_str(), oldAddress.c_str(), newAddress.c_str()));
	LOG("CustomerService", ("Player: Received OnTransferAvatar(%d, %d, %d, %d, %s, %s, %s, %s)", track, result, oldUserID, newUserID, oldName.c_str(), newName.c_str(), oldAddress.c_str(), newAddress.c_str()));

	// if the chat avatar rename was successful, tell each game server that has
	// a connected character that has the chat avatar on his friends list or ignore
	// list to refresh the friends list and ignore list to update the friends list
	// and ignore list with the new chat avatar name
	if ((result == CHATRESULT_SUCCESS) && (oldAddress == newAddress) && (oldName != newName))
	{
		ChatAvatarId const dummyAvatar("SWG", ConfigChatServer::getClusterName(), "dummy");
		if (oldAddress == dummyAvatar.getAPIAddress())
		{
			GenericValueTypeMessage<std::pair<std::string, std::string> > const chatAvatarRenamed("ChatAvatarRenamed", std::make_pair(std::string(oldName.c_str()), std::string(newName.c_str())));
			ChatServer::broadcastToGameServers(chatAvatarRenamed);
		}
	}
}

//-----------------------------------------------------------------------
