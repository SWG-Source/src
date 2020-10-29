//======================================================================
//
// ChatRoomData.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ChatRoomData_H
#define INCLUDED_ChatRoomData_H

//======================================================================

#include "sharedNetworkMessages/ChatAvatarId.h"
#include "unicodeArchive/UnicodeArchive.h"

#include <vector>

//-----------------------------------------------------------------------

namespace ChatRoomTypes 
{
	static const std::string ROOM_SYSTEM    = "system";
	static const std::string ROOM_UNIVERSE  = "Universe";
	static const std::string ROOM_GALAXY    = "Galaxy";
	static const std::string ROOM_PLANET    = "Planet";
	static const std::string ROOM_CHAT      = "Chat";
	static const std::string ROOM_GROUP     = "GroupChat";
	static const std::string ROOM_GUILD     = "GuildChat";
	static const std::string ROOM_CITY      = "CityChat";
	static const std::string ROOM_IMPERIAL  = "Imperial";
	static const std::string ROOM_IMPERIAL_WAR_ROOM = "ImperialWarRoom";
	static const std::string ROOM_REBEL     = "Rebel";
	static const std::string ROOM_REBEL_WAR_ROOM = "RebelWarRoom";
	static const std::string ROOM_BOUNTY_HUNTER   = "BountyHunter";
	static const std::string ROOM_COMMANDO        = "Commando";
	static const std::string ROOM_ENTERTAINER     = "Entertainer";
	static const std::string ROOM_FORCE_SENSITIVE = "ForceSensitive";
	static const std::string ROOM_MEDIC           = "Medic";
	static const std::string ROOM_OFFICER         = "Officer";
	static const std::string ROOM_PILOT           = "Pilot";
	static const std::string ROOM_POLITICIAN      = "Politician";
	static const std::string ROOM_SMUGGLER        = "Smuggler";
	static const std::string ROOM_SPY             = "Spy";
	static const std::string ROOM_TRADER          = "Trader";
	static const std::string ROOM_BEAST_MASTERY   = "BeastMastery";
	static const std::string ROOM_WARDEN          = "Warden";
	static const std::string ROOM_GUILD_LEADER    = "GuildLeaders";
	static const std::string ROOM_CITY_MAYOR      = "CityMayors";
}//namespace ChatRoomTypes

enum 
{
	CHAT_ROOM_PUBLIC,
	CHAT_ROOM_PRIVATE
};

//----------------------------------------------------------------------

struct ChatRoomData
{
	uint32                     id;
	unsigned int               roomType;
	std::string                path;
	ChatAvatarId               owner;
	ChatAvatarId               creator;
	Unicode::String            title;
	std::vector<ChatAvatarId>  moderators;
	std::vector<ChatAvatarId>  invitees;
	unsigned char              moderated;

	ChatRoomData ();
};

//======================================================================

#endif
