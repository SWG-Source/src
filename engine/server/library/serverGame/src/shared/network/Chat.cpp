// Chat.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/Chat.h"

#include "UnicodeUtils.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "serverGame/ChatServerConnection.h"
#include "serverGame/CityInfo.h"
#include "serverGame/CityInterface.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/GroupObject.h"
#include "serverGame/GuildInfo.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/ServerObject.h"
#include "sharedGame/OutOfBandPackager.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/TextManager.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include "sharedNetworkMessages/ChatChangeFriendStatus.h"
#include "sharedNetworkMessages/ChatChangeIgnoreStatus.h"
#include "sharedNetworkMessages/ChatCreateRoom.h"
#include "sharedNetworkMessages/ChatDestroyRoomByName.h"
#include "sharedNetworkMessages/ChatDeleteAllPersistentMessages.h"
#include "sharedNetworkMessages/ChatEnum.h"
#include "sharedNetworkMessages/ChatGetFriendsList.h"
#include "sharedNetworkMessages/ChatGetIgnoreList.h"
#include "sharedNetworkMessages/ChatInviteAvatarToRoom.h"
#include "sharedNetworkMessages/ChatInviteGroupMembersToRoom.h"
#include "sharedNetworkMessages/ChatMessageFromGame.h"
#include "sharedNetworkMessages/ChatPutAvatarInRoom.h"
#include "sharedNetworkMessages/ChatRemoveAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatRoomData.h"
#include "sharedNetworkMessages/ChatSystemMessage.h"
#include "sharedNetworkMessages/ChatUninviteFromRoom.h"


#include "sharedNetworkMessages/VoiceChatChannelInfo.h"
#include "sharedNetworkMessages/VoiceChatMiscMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

//Temporary include, remove if possible
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

namespace ChatNamespace
{
	std::string const cs_systemSenderName("SYSTEM");
}

using namespace ChatNamespace;

//-----------------------------------------------------------------------

Chat *Chat::m_instance = nullptr;


Chat::Chat () :
m_serverAvatar         (new ChatAvatarId),
m_chatServer           (0),
m_deferredChatMessages (),
m_gameCode             ("SWG")  //@todo this can not depend on swg
{
	m_serverAvatar->gameCode = m_gameCode;
	m_serverAvatar->cluster = GameServer::getInstance().getClusterName();
	m_serverAvatar->name = "System";
}

//-----------------------------------------------------------------------

Chat::~Chat()
{
	m_chatServer = 0;
	delete m_serverAvatar;
}

//-----------------------------------------------------------------------

void Chat::addModeratorToRoom(const std::string & who, const std::string & roomPath)
{
	UNREF(who);
	UNREF(roomPath);
}

//-----------------------------------------------------------------------
void Chat::emptyMail(const NetworkId &sourceNetworkId, const NetworkId &targetNetworkId)
{
	if (targetNetworkId != NetworkId::cms_invalid)
	{
		ChatDeleteAllPersistentMessages msg(sourceNetworkId, targetNetworkId);

		sendToChatServer(msg);
	}
}

//-----------------------------------------------------------------------


void Chat::addFriend(const std::string &player, const std::string & friendName)
{
	ChatAvatarId playerId = constructChatAvatarId (player);
	ChatAvatarId friendId = constructChatAvatarId (friendName);
	ChatChangeFriendStatus msg(0, playerId, friendId, true);
	sendToChatServer(msg);
}

//-----------------------------------------------------------------------

void Chat::removeFriend(const std::string &player, const std::string & friendName)
{
	ChatAvatarId playerId = constructChatAvatarId (player);
	ChatAvatarId friendId = constructChatAvatarId (friendName);
	ChatChangeFriendStatus msg(0, playerId, friendId, false);
	sendToChatServer(msg);
}

//-----------------------------------------------------------------------

void Chat::getFriendsList(const std::string &player)
{
	ChatAvatarId playerId = constructChatAvatarId (player);
	ChatGetFriendsList msg(playerId);
	sendToChatServer(msg);
}

//-----------------------------------------------------------------------


void Chat::addIgnore(const std::string &player, const std::string & ignoreName)
{
	ChatAvatarId playerId = constructChatAvatarId (player);
	ChatAvatarId ignoreId = constructChatAvatarId (ignoreName);
	ChatChangeIgnoreStatus msg(0, playerId, ignoreId, true);
	sendToChatServer(msg);
}

//-----------------------------------------------------------------------

void Chat::removeIgnore(const std::string &player, const std::string & ignoreName)
{
	ChatAvatarId playerId = constructChatAvatarId (player);
	ChatAvatarId ignoreId = constructChatAvatarId (ignoreName);
	ChatChangeIgnoreStatus msg(0, playerId, ignoreId, false);
	sendToChatServer(msg);
}

//-----------------------------------------------------------------------

void Chat::getIgnoreList(const std::string &player)
{
	ChatAvatarId playerId = constructChatAvatarId (player);
	ChatGetIgnoreList msg(playerId);
	sendToChatServer(msg);
}

//-----------------------------------------------------------------------

void Chat::createRoom(const std::string & ownerName, const bool isPublic, const std::string & roomPath, const std::string & roomTitle)
{
	if (ownerName != "System")
	{
		if (!TextManager::isAppropriateText(Unicode::narrowToWide(roomPath)))
		{
			sendSystemMessage(ownerName, StringId("ui_chatroom", "create_err_profane_name"), Unicode::emptyString);
			return;
		}
		
		if (!TextManager::isAppropriateText(Unicode::narrowToWide(roomTitle)))
		{
			sendSystemMessage(ownerName, StringId("ui_chatroom", "create_err_profane_title"), Unicode::emptyString);
			return;
		}
	}

	static int sequence = 1;
	ChatCreateRoom create(sequence++, ownerName, roomPath, false, isPublic, roomTitle);
	sendToChatServer(create);
}

//-----------------------------------------------------------------------

void Chat::createSystemRooms(const std::string & galaxyName, const std::string & planetName)
{
	NOT_NULL (instance ().m_serverAvatar);

	const std::string & avatarName = instance ().m_serverAvatar->getFullName ();
	const std::string & gamePrefix = getGameCode () + ".";

	createRoom(avatarName, true, gamePrefix + ChatRoomTypes::ROOM_UNIVERSE, "public chat for all galaxies, cannot create rooms here");
	createRoom(avatarName, true, gamePrefix + ChatRoomTypes::ROOM_SYSTEM,   "system messages for all galaxies");
	createRoom(avatarName, true, gamePrefix + ChatRoomTypes::ROOM_CHAT,     "public chat for all galaxies, can create rooms here");
	

	std::string galaxyRoom = gamePrefix + galaxyName + ".";
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_GALAXY, "public chat for the whole galaxy, cannot create rooms here");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_SYSTEM, "system messages for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_CHAT,   "public chat for this galaxy, can create rooms here");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_IMPERIAL, "Imperial chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_IMPERIAL_WAR_ROOM, "Imperial war room chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_REBEL,   "Rebel chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_REBEL_WAR_ROOM, "Rebel war room chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_BOUNTY_HUNTER  , "Bounty Hunter chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_COMMANDO       , "Commando chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_ENTERTAINER    , "Entertainer chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_FORCE_SENSITIVE, "Force Sensitive chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_MEDIC          , "Medic chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_OFFICER        , "Officer chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_PILOT          , "Pilot chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_POLITICIAN     , "Politician chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_SMUGGLER       , "Smuggler chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_SPY            , "Spy chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_TRADER         , "Trader chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_BEAST_MASTERY  , "Beast Mastery chat for this galaxy");
	createRoom(avatarName, true, galaxyRoom + ChatRoomTypes::ROOM_WARDEN         , "Warden chat for this galaxy");

	std::string planetRoom = gamePrefix + galaxyName + "." + planetName + ".";
	createRoom(avatarName, true, planetRoom + ChatRoomTypes::ROOM_PLANET, "public chat for this planet, cannot create rooms here");
	createRoom(avatarName, true, planetRoom + ChatRoomTypes::ROOM_SYSTEM, "system messages for this planet, cannot create rooms here");
	createRoom(avatarName, true, planetRoom + ChatRoomTypes::ROOM_CHAT,   "public chat for this planet, can create rooms here");

	GroupObject::createAllGroupChatRooms();
	GuildInterface::createAllGuildChatRooms();
	CityInterface::createAllCityChatRooms();
}

//-----------------------------------------------------------------------

void Chat::destroyRoom(const std::string & roomPath)
{
	ChatDestroyRoomByName destroy(roomPath);
	sendToChatServer(destroy);
}

//-----------------------------------------------------------------------

void Chat::enterRoom(const std::string & who, const std::string & roomPath, bool forceCreate, bool createPrivate)
{
	ChatPutAvatarInRoom enter(who, roomPath, forceCreate, createPrivate);
	sendToChatServer(enter);
}

//-----------------------------------------------------------------------

void Chat::exitRoom(const std::string & who, const std::string & roomPath)
{
	ChatAvatarId id(getGameCode (), GameServer::getInstance().getClusterName(), who);
	ChatRemoveAvatarFromRoom remove(id, roomPath);
	sendToChatServer(remove);
}

//-----------------------------------------------------------------------

Chat & Chat::instance()
{
	static Chat c;
	m_instance = &c;
	return *m_instance;
}

//-----------------------------------------------------------------------

void Chat::removeModeratorFromRoom(const std::string & who, const std::string & roomPath)
{
	UNREF(who);
	UNREF(roomPath);
}

//-----------------------------------------------------------------------

void Chat::sendInstantMessage(const std::string & from, const std::string & to, const Unicode::String & message, const Unicode::String & oob)
{
	ChatMessageFromGame m(ChatMessageFromGame::INSTANT, from, to, message, oob);
	sendToChatServer(m);
}

//----------------------------------------------------------------------

void Chat::sendPersistentMessage(const std::string & from, const std::string & to, const Unicode::String & subject, const Unicode::String & message, const Unicode::String & oob)
{
	DEBUG_REPORT_LOG (true, ("Chat::sendPersistentMessage oob size=%d\n", static_cast<int>(oob.size ())));
	ChatMessageFromGame m(ChatMessageFromGame::PERSISTENT, from, to, message, oob, subject);
	sendToChatServer(m);
}

//----------------------------------------------------------------------

void Chat::sendPersistentMessage    (const ServerObject & from, const std::string & to, const StringId & subject,        const Unicode::String & message, const Unicode::String & oob)
{
	Unicode::String subjectString;
	subjectString.push_back ('@');
	subjectString += Unicode::narrowToWide (subject.getCanonicalRepresentation ());

	Chat::sendPersistentMessage (Unicode::wideToNarrow (from.getEncodedObjectName ()), to, subjectString, message, oob);
}

//----------------------------------------------------------------------

void Chat::sendPersistentMessage    (const ServerObject & from, const std::string & to, const StringId & subject,        const StringId & message,        const Unicode::String & oob)
{
	Unicode::String messageString;
	messageString.push_back ('@');
	messageString += Unicode::narrowToWide (message.getCanonicalRepresentation ());

	Unicode::String subjectString;
	subjectString.push_back ('@');
	subjectString += Unicode::narrowToWide (subject.getCanonicalRepresentation ());

	Chat::sendPersistentMessage (Unicode::wideToNarrow (from.getEncodedObjectName ()), to, subjectString, messageString, oob);
}

//----------------------------------------------------------------------

Unicode::String & Chat::makePersistentMessageBody     (Unicode::String & target, const ProsePackage & pp)
{
	OutOfBandPackager::pack (pp, -1, target);
	return target;
}

//-----------------------------------------------------------------------

void Chat::sendToRoom(const std::string & from, const std::string & roomName, const Unicode::String & message, const Unicode::String & oob)
{
	ChatMessageFromGame m(ChatMessageFromGame::ROOM, from, message, oob, roomName);
	sendToChatServer(m);
}

//-----------------------------------------------------------------------

void Chat::broadcastSystemMessageToCluster (const Unicode::String & message, const Unicode::String & oob)
{
	std::string const roomName = std::string("SWG.") + GameServer::getInstance().getClusterName() + std::string(".SYSTEM");
	sendToRoom(cs_systemSenderName, roomName, message, oob);
}

//-----------------------------------------------------------------------

void Chat::sendToChatServer(const GameNetworkMessage & message)
{
	if(instance().m_chatServer)
	{
		instance().m_chatServer->send(message, true);
	}
	else
	{
		Archive::ByteStream bs;
		message.pack(bs);
		instance().m_deferredChatMessages.push_back(bs);
	}
}

//----------------------------------------------------------------------

const ChatServerConnection *Chat::getChatServer()
{
	return instance().m_chatServer;
}

//-----------------------------------------------------------------------

void Chat::setChatServer(ChatServerConnection * conn)
{
	if(! instance().m_chatServer && conn)
	{
		// flush deferred messages
		std::vector<Archive::ByteStream>::const_iterator iter;
		for(iter = instance().m_deferredChatMessages.begin(); iter != instance().m_deferredChatMessages.end(); ++iter)
		{
			// this cast seems unnecessary, but is required with VC6 for some reason.
			// it should resolve to 
			// Connection::send(const Archive::ByteStream &, const bool) const
			static_cast<Connection *>(conn)->send((*iter), true);
		}
	}
	instance().m_chatServer = conn;
}

//-----------------------------------------------------------------------

/**
 * Sends a system-specific message to a player.
 *
 * @param to		player to send to
 * @param message	the message to send
 * @param oob		thingy
 */
void Chat::sendSystemMessage(const ServerObject & to, const StringId & message, const Unicode::String & oob)
{
	ChatSystemMessage msg(ChatSystemMessage::PERSONAL, 
		Unicode::narrowToWide("@" + message.getCanonicalRepresentation()), oob);
	Client *client = to.getClient();
	if (client)
	{
		client->send(msg, true);
	}
	else
	{
		sendSystemMessage(Unicode::wideToNarrow(to.getAssignedObjectFirstName()), message, oob);
	}
}	// Chat::sendSystemMessage

//-----------------------------------------------------------------------

/**
 * Sends a system-specific message to a player.
 *
 * @param to		player to send to
 * @param message	the message to send
 * @param oob		thingy
 */
void Chat::sendSystemMessage(const ServerObject & to, const Unicode::String & message, const Unicode::String & oob)
{
	ChatSystemMessage msg(ChatSystemMessage::PERSONAL, message, oob);
	Client *client = to.getClient();
	if (client)
	{
		client->send(msg, true);
	}
	else
	{
		sendSystemMessage(Unicode::wideToNarrow(to.getAssignedObjectFirstName()), message, oob);
	}
}	// Chat::sendSystemMessage

//-----------------------------------------------------------------------

/**
 * Sends a system-specific message to a player.
 *
 * @param to		player to send to's name
 * @param message	the message to send
 * @param oob		thingy
 */
void Chat::sendSystemMessage(const std::string & to, const StringId & message, const Unicode::String & oob)
{
	// @todo: we need to be able to send the message string id to the player
	// and localize it client-side
	sendSystemMessage(to, Unicode::narrowToWide("@" + message.getCanonicalRepresentation()), oob);
}	// Chat::sendSystemMessage

//-----------------------------------------------------------------------

/**
 * Sends a system-specific message to a player.
 *
 * @param to		player to send to's n
 * @param message	the message to send
 * @param oob		thingy
 */
void Chat::sendSystemMessage(const std::string & to, const Unicode::String & message,  const Unicode::String & oob)
{
	NOT_NULL(instance().m_serverAvatar);
	
	std::string toName(to);
	// split off the last name from toName
	std::string::size_type spacePos = toName.find(' ');
	if (spacePos != std::string::npos)
		toName = toName.substr(0, spacePos);

	sendInstantMessage(instance().m_serverAvatar->getFullName(), to, message, oob);
}	// Chat::sendSystemMessage

//----------------------------------------------------------------------

void Chat::sendSystemMessage   (const std::string & to, const ProsePackage & pp)
{
	Unicode::String oob;
	OutOfBandPackager::pack (pp, -1, oob);
	sendSystemMessage (to, Unicode::String (), oob);
}

//----------------------------------------------------------------------

void Chat::sendSystemMessage   (const ServerObject & to, const ProsePackage & pp)
{
	//MLS shortcut me
	//Chat::sendSystemMessage(Unicode::wideToNarrow(to.getAssignedObjectFirstName()), pp);
	Unicode::String oob;
	OutOfBandPackager::pack (pp, -1, oob);
	ChatSystemMessage msg(ChatSystemMessage::PERSONAL, 
		Unicode::String(), oob);
	Client *client = to.getClient();
	if (client)
	{
		client->send(msg, true);
	}
}

//----------------------------------------------------------------------

void Chat::sendSystemMessageSimple  (const ServerObject & to, const StringId & stringId, const ServerObject * target)
{
	ProsePackage pp;
	pp.stringId = stringId;
	if (target)
	{
		pp.target.id       = target->getNetworkId ();
		pp.target.str      = target->getAssignedObjectName ();
		if (pp.target.str.empty ())
			pp.target.stringId = target->getObjectNameStringId ();
	}
	pp.actor.id        = to.getNetworkId ();

	Chat::sendSystemMessage (to, pp);
}

//----------------------------------------------------------------------

const std::string & Chat::getGameCode ()
{
	NOT_NULL(instance().m_serverAvatar);
	return instance ().m_gameCode;
}

//----------------------------------------------------------------------

const ChatAvatarId Chat::constructChatAvatarId (const ServerObject & to)
{
	return Chat::constructChatAvatarId (Unicode::wideToNarrow (to.getAssignedObjectFirstName ()));
}

//----------------------------------------------------------------------

const ChatAvatarId Chat::constructChatAvatarId (const std::string & to)
{
	NOT_NULL(instance().m_serverAvatar);

	ChatAvatarId to_id (to);

	if (to_id.gameCode.empty ())
		to_id.gameCode = instance().m_serverAvatar->gameCode;

	if (to_id.cluster.empty ())
		to_id.cluster = instance().m_serverAvatar->cluster;

	return to_id;
}

//----------------------------------------------------------------------

void Chat::invite(const std::string & to, const std::string & room) 
{
	ChatAvatarId id = constructChatAvatarId(to);
	ChatInviteAvatarToRoom inviteMessage(id, room);
	sendToChatServer(inviteMessage);
}

//----------------------------------------------------------------------

void Chat::uninvite(const std::string & to, const std::string & room) 
{
	ChatAvatarId id = constructChatAvatarId(to);
	ChatUninviteFromRoom uninviteMessage(0, id, room);
	sendToChatServer(uninviteMessage);
}

//----------------------------------------------------------------------

void Chat::inviteGroupMembers(const NetworkId & invitor, const ChatAvatarId & groupLeader, const std::string & room, const stdvector<NetworkId>::fwd & members) 
{
	ChatInviteGroupMembersToRoom inviteMessage(invitor, groupLeader, room, members);
	sendToChatServer(inviteMessage);
}

//----------------------------------------------------------------------

unsigned int Chat::isAllowedToEnterRoom(const CreatureObject & who, const std::string & room)
{
	NOT_NULL(instance().m_serverAvatar);

	// god is allowed to enter all rooms; don't have to actually
	// be in god mode; just secured login is adequate
	if (who.getClient() && who.getClient()->isGodValidated())
		return CHATRESULT_SUCCESS;

	// player must be of the correct faction to enter the factional chat rooms
	static std::string s_imperialChatRoom = getGameCode() + "." + instance().m_serverAvatar->cluster + "." + ChatRoomTypes::ROOM_IMPERIAL;
	static std::string s_rebelChatRoom = getGameCode() + "." + instance().m_serverAvatar->cluster + "." + ChatRoomTypes::ROOM_REBEL;

	if (((_stricmp(room.c_str(), s_imperialChatRoom.c_str()) == 0) && !PvpData::isImperialFactionId(who.getPvpFaction())) ||
		((_stricmp(room.c_str(), s_rebelChatRoom.c_str()) == 0) && !PvpData::isRebelFactionId(who.getPvpFaction())))
	{
		return SWG_CHAT_ERR_WRONG_FACTION;
	}

	// factional war room chat rooms only available to mayor and guild leader of the same faction of city and guild aligned with the same faction
	static std::string s_imperialWarRoomChatRoom = getGameCode() + "." + instance().m_serverAvatar->cluster + "." + ChatRoomTypes::ROOM_IMPERIAL_WAR_ROOM;
	static std::string s_rebelWarRoomChatRoom = getGameCode() + "." + instance().m_serverAvatar->cluster + "." + ChatRoomTypes::ROOM_REBEL_WAR_ROOM;

	if (_stricmp(room.c_str(), s_imperialWarRoomChatRoom.c_str()) == 0)
	{
		bool allowed = false;

		std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(who.getNetworkId());
		if (!cityIds.empty())
		{
			CityInfo const & cityInfo = CityInterface::getCityInfo(cityIds.front());
			uint32 const cityFaction = cityInfo.getFaction();
			if (PvpData::isImperialFactionId(cityFaction) && (cityFaction == who.getPvpFaction()) && (cityInfo.getLeaderId() == who.getNetworkId()))
				allowed = true;
		}

		if (!allowed)
		{
			int const guildId = who.getGuildId();
			if (guildId > 0)
			{
				GuildInfo const * const gi = GuildInterface::getGuildInfo(guildId);
				if (gi)
				{
					uint32 const guildFaction = GuildInterface::getGuildCurrentFaction(*gi);
					if (PvpData::isImperialFactionId(guildFaction) && (guildFaction == who.getPvpFaction()) && (gi->m_leaderId == who.getNetworkId()) && (gi->getCountMembersOnly() >= ConfigServerGame::getGcwGuildMinMembersForGcwRegionDefender()))
						allowed = true;
				}
			}
		}

		if (!allowed)
			return SWG_CHAT_ERR_WRONG_GCW_REGION_DEFENDER_FACTION;
	}

	if (_stricmp(room.c_str(), s_rebelWarRoomChatRoom.c_str()) == 0)
	{
		bool allowed = false;

		std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(who.getNetworkId());
		if (!cityIds.empty())
		{
			CityInfo const & cityInfo = CityInterface::getCityInfo(cityIds.front());
			uint32 const cityFaction = cityInfo.getFaction();
			if (PvpData::isRebelFactionId(cityFaction) && (cityFaction == who.getPvpFaction()) && (cityInfo.getLeaderId() == who.getNetworkId()))
				allowed = true;
		}

		if (!allowed)
		{
			int const guildId = who.getGuildId();
			if (guildId > 0)
			{
				GuildInfo const * const gi = GuildInterface::getGuildInfo(guildId);
				if (gi)
				{
					uint32 const guildFaction = GuildInterface::getGuildCurrentFaction(*gi);
					if (PvpData::isRebelFactionId(guildFaction) && (guildFaction == who.getPvpFaction()) && (gi->m_leaderId == who.getNetworkId()) && (gi->getCountMembersOnly() >= ConfigServerGame::getGcwGuildMinMembersForGcwRegionDefender()))
						allowed = true;
				}
			}
		}

		if (!allowed)
			return SWG_CHAT_ERR_WRONG_GCW_REGION_DEFENDER_FACTION;
	}

	// player must be a warden to enter the warden room
	static std::string s_wardenChatRoom = getGameCode() + "." + instance().m_serverAvatar->cluster + "." + ChatRoomTypes::ROOM_WARDEN;
	if (_stricmp(room.c_str(), s_wardenChatRoom.c_str()) == 0)
	{
		PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(&who);
		if (!playerObject || !playerObject->isWarden())
		{
			return SWG_CHAT_ERR_NOT_WARDEN;
		}
	}

	return CHATRESULT_SUCCESS;
}

//----------------------------------------------------------------------

void Chat::sendQuestSystemMessage(ServerObject const & to, Unicode::String const & message, Unicode::String const & oob)
{
	ChatSystemMessage const msg(ChatSystemMessage::F_quest, message, oob);
	Client * const client = to.getClient();
	if (client)
		client->send(msg, true);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//TODO: integrate these properly
void Chat::requestGetChannel(std::string const & roomName, std::string const & displayName, std::string const & password, bool persistant, uint32 limit)
{
	VoiceChatGetChannel msg(roomName, password, true, persistant, limit);
	sendToChatServer(msg);
}

void Chat::requestGetChannel(std::string const & roomName, bool isPublic, bool persistant, uint32 limit)
{
	VoiceChatGetChannel msg(roomName, "", isPublic, persistant, limit);
	sendToChatServer(msg);
}

void Chat::requestDestroyChannel(std::string const & roomName)
{
	VoiceChatDeleteChannel msg(roomName);
	sendToChatServer(msg);
}

//----------------------------------------------------------------------

void Chat::requestAddClientToChannel(NetworkId const & clientId, std::string const & playerName, std::string const & roomName, bool forceShortlist)
{
	VoiceChatAddClientToChannel msg (clientId, playerName, roomName, forceShortlist);
	sendToChatServer(msg);
}

//----------------------------------------------------------------------

void Chat::requestRemoveClientFromChannel(NetworkId const & clientId, std::string const & playerName, std::string const & roomName)
{
	VoiceChatRemoveClientFromChannel msg (clientId, playerName, roomName);
	sendToChatServer(msg);
}

//----------------------------------------------------------------------

void Chat::requestChannelCommand(std::string const & srcUser, std::string const & destUser, std::string const & destChan, uint32 commandType, uint32 banTimeout)
{
	VoiceChatChannelCommand msg(srcUser,destUser,destChan,commandType, banTimeout);
	sendToChatServer(msg);
}

//----------------------------------------------------------------------

void Chat::requestBroadcastGlobalChannelMessage(std::string const & channelName, std::string const & message, bool isRemove, bool sendToAllClusters)
{
	typedef std::pair<std::pair<std::string,std::string>, bool> PayloadType;

	LOG("CustomerService", ("BroadcastVoiceChannel: GameServer::Chat::requestBroadcastGlobalChannel chan(%s) text(%s) remove(%d) sendToAllClusters(%d)", 
		channelName.c_str(), message.c_str(), (isRemove ? 1 : 0), (sendToAllClusters?1:0)));

	if(sendToAllClusters)
	{
		GenericValueTypeMessage<PayloadType> msg("AllCluserGlobalChannel", std::make_pair(std::make_pair(channelName, message),isRemove));
		GameServer::getInstance().sendToCentralServer(msg);
	}
	else
	{
		GenericValueTypeMessage<PayloadType> msg("BroadcastGlobalChannel", std::make_pair(std::make_pair(channelName, message),isRemove));
		sendToChatServer(msg);
	}

}

//----------------------------------------------------------------------

bool Chat::debugTellClientAboutRoom(const NetworkId & clientCreatureId, const std::string &roomName, std::string const & uri, std::string const & password, bool autoJoin, bool leaveCommand)
{
	Object *o = NetworkIdManager::getObjectById(clientCreatureId);
	if (o)
	{
		ServerObject *so = o->asServerObject();
		if (so && so->isAuthoritative())
		{
			CreatureObject *creature = so->asCreatureObject();
			if (creature)
			{
				Client *client = creature->getClient();
				if (client)
				{
					uint32 flags = VoiceChatChannelInfo::CIF_None;
					if(autoJoin) flags |= VoiceChatChannelInfo::CIF_AutoJoin;
					if(leaveCommand) flags |= VoiceChatChannelInfo::CIF_LeaveChannel;

					VoiceChatChannelInfo msg(roomName, roomName, uri, password, "", flags);
					client->send(msg, true);
					return true;
				}
			}
		}
	}
	return false;
}
//END TODO
