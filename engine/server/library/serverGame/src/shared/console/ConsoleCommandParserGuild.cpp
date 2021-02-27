// ======================================================================
//
// ConsoleCommandParserGuild.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserGuild.h"

#include "UnicodeUtils.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GuildInfo.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/GuildMemberInfo.h"
#include "serverGame/NameManager.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/CalendarTime.h"

// ======================================================================

namespace ConsoleCommandParserGuildNamespace
{
	void buildGuildInfoSpec(int guildId, GuildInfo const & gi, std::string & giSpec, int & totalSize, int & totalSizeActive, int & totalSizeInactive, int & memberSize, int & memberSizeActive, int & memberSizeInactive, int & sponsoredSize, int & sponsoredSizeActive, int & sponsoredSizeInactive);
	void getGuildSize(int guildId, GuildInfo const & gi, int & total, int & totalActive, int & totalInactive, int & member, int & memberActive, int & memberInactive, int & sponsored, int & sponsoredActive, int & sponsoredInactive);
	std::string const getGuildPermissionsText(int permissions);
	std::string const getFactionString(uint32 factionId);
}
using namespace ConsoleCommandParserGuildNamespace;
 
static const CommandParser::CmdInfo cmds[] =
{
	{"listById",                          0, "", "List all guilds by guild id."},
	{"listByName",                        0, "", "List all guilds by guild name."},
	{"listByAbbrev",                      0, "", "List all guilds by guild abbreviation."},
	{"listByLeaderId",                    0, "", "List all guilds by guild leader id."},
	{"listByLeaderName",                  0, "", "List all guilds by guild leader name."},
	{"listByTotalSize",                   0, "", "List all guilds by guild total (member + sponsored) size."},
	{"listByTotalActiveSize",             0, "", "List all guilds by guild total (member + sponsored) active size."},
	{"listByTotalInactiveSize",           0, "", "List all guilds by guild total (member + sponsored) inactive size."},
	{"listByMemberSize",                  0, "", "List all guilds by guild member size."},
	{"listByMemberActiveSize",            0, "", "List all guilds by guild member active size."},
	{"listByMemberInactiveSize",          0, "", "List all guilds by guild member inactive size."},
	{"listBySponsoredSize",               0, "", "List all guilds by guild sponsored size."},
	{"listBySponsoredActiveSize",         0, "", "List all guilds by guild sponsored active size."},
	{"listBySponsoredInactiveSize",       0, "", "List all guilds by guild sponsored inactive size."},
	{"showGuildGcwRegionDefender",        0, "", "Display GCW region defender data for guilds."},
	{"showGuildDetails",                  1,"<guild id>","Display guild details for a guild."},
	{"setGuildName",                      2,"<guild id> <guild name> [override]","Set guild name (if the guild name has embedded space(s), enclose it between double quotes). If the optional third parameter is the word 'override' then override the reserved name check."},
	{"setGuildAbbrev",                    2,"<guild id> <guild abbrev>","Set guild abbreviation (if the guild abbreviation has embedded space(s), enclose it between double quotes)."},
	{"setGuildLeader",                    2,"<guild id> <guild leader oid>","Set guild leader."},
	{"findGuildMemberNameMismatch",       0, "", "(POTENTIALLY EXPENSIVE OPERATION!!!) Find all guild member whose guild member name doesn't match the character name."},
	{"findGuildLeaderPermissionMismatch", 0, "", "Find all guild leader whose permission is not \"all\"."},
	{"findGuildLeaderMismatch",           0, "", "Find all guild where the guild leader is not a guild member."},
	{"findLeaderlessGuild",               0, "", "Find all guild where the guild leader no longer exists in the DB (i.e. character has been deleted and purged)."},
	{"findOrphanedGuild",                 0, "", "Find all guild where there is no guild leader."},
	{"",                                  0, "", ""} // this must be last
};

//-----------------------------------------------------------------

ConsoleCommandParserGuild::ConsoleCommandParserGuild (void) :
CommandParser ("guild", 0, "...", "Guild related commands.", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------

bool ConsoleCommandParserGuild::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	NOT_NULL (node);
	UNREF (userId);

	UNREF(originalCommand);

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	//-----------------------------------------------------------------

	if (isCommand(argv [0], "listById"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::map<int, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo[*iter] = giSpec;
			}
		}

		for (std::map<int, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByName"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<std::string, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(Unicode::toLower(gi->m_name), giSpec));
			}
		}

		for (std::multimap<std::string, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByAbbrev"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<std::string, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(Unicode::toLower(gi->m_abbrev), giSpec));
			}
		}

		for (std::multimap<std::string, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByLeaderId"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<NetworkId, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(gi->m_leaderId, giSpec));
			}
		}

		for (std::multimap<NetworkId, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByLeaderName"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<std::string, std::string> guildInfo;
		std::string leaderName;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			GuildMemberInfo const * const gmi = GuildInterface::getGuildMemberInfo(*iter, gi->m_leaderId);
			if (gmi)
				leaderName = gmi->m_name;
			else
				leaderName.clear();

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(Unicode::toLower(leaderName), giSpec));
			}
		}

		for (std::multimap<std::string, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByTotalSize"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<int, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(totalSize, giSpec));
			}
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByTotalActiveSize"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<int, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(totalSizeActive, giSpec));
			}
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByTotalInactiveSize"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<int, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(totalSizeInactive, giSpec));
			}
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByMemberSize"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<int, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(memberSize, giSpec));
			}
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByMemberActiveSize"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<int, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(memberSizeActive, giSpec));
			}
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByMemberInactiveSize"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<int, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(memberSizeInactive, giSpec));
			}
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listBySponsoredSize"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<int, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(sponsoredSize, giSpec));
			}
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listBySponsoredActiveSize"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<int, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(sponsoredSizeActive, giSpec));
			}
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listBySponsoredInactiveSize"))
	{
		std::vector<int> guildIds;
		GuildInterface::getAllGuildIds(guildIds);

		std::multimap<int, std::string> guildInfo;
		std::string giSpec;
		int totalSize;
		int totalSizeActive;
		int totalSizeInactive;
		int memberSize;
		int memberSizeActive;
		int memberSizeInactive;
		int sponsoredSize;
		int sponsoredSizeActive;
		int sponsoredSizeInactive;
		for (std::vector<int>::const_iterator iter = guildIds.begin(); iter != guildIds.end(); ++iter)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iter);
			if (!gi || gi->m_name.empty())
				continue;

			buildGuildInfoSpec(*iter, *gi, giSpec, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			if (!giSpec.empty())
			{
				giSpec += "\n";
				guildInfo.insert(std::make_pair(sponsoredSizeInactive, giSpec));
			}
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = guildInfo.begin(); iter2 != guildInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%d guilds listed\n", guildInfo.size());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		if (!guildInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, abbrev, leaderId, leaderName, total size (active/inactive), member size (active/inactive), sponsored size (active/inactive)\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "showGuildGcwRegionDefender"))
	{
		char buffer[1024];

		snprintf(buffer, sizeof(buffer)-1, "Version (%d)\n\n", GuildInterface::getGcwRegionDefenderGuildsVersion());
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		std::map<std::pair<std::string, int>, uint32> const & gcwRegionDefenderGuilds = GuildInterface::getGcwRegionDefenderGuilds();
		for (std::map<std::pair<std::string, int>, uint32>::const_iterator iterGuild = gcwRegionDefenderGuilds.begin(); iterGuild != gcwRegionDefenderGuilds.end(); ++iterGuild)
		{
			snprintf(buffer, sizeof(buffer)-1, "(%s) (%d, %s) (%lu) (%s)\n", iterGuild->first.first.c_str(), iterGuild->first.second, GuildInterface::getGuildName(iterGuild->first.second).c_str(), iterGuild->second, getFactionString(iterGuild->second).c_str());
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);
		}

		result += Unicode::narrowToWide("\n");

		std::map<std::string, std::pair<int, int> > const & gcwRegionDefenderGuildsCount = GuildInterface::getGcwRegionDefenderGuildsCount();
		for (std::map<std::string, std::pair<int, int> >::const_iterator iterCount = gcwRegionDefenderGuildsCount.begin(); iterCount != gcwRegionDefenderGuildsCount.end(); ++iterCount)
		{
			snprintf(buffer, sizeof(buffer)-1, "(%s) (Imperial=%d) (Rebel=%d)\n", iterCount->first.c_str(), iterCount->second.first, iterCount->second.second);
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "showGuildDetails"))
	{
		int const guildId = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		GuildInfo const * const gi = GuildInterface::getGuildInfo(guildId);

		if (!gi || gi->m_name.empty())
		{
			char buffer[1024];
			snprintf(buffer, sizeof(buffer)-1, "no guild with guild id %d\n", guildId);
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			// guild id
			char buffer[1024];
			snprintf(buffer, sizeof(buffer)-1, "id: %d\n", guildId);
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			// guild name
			result += Unicode::narrowToWide("name: ");
			result += Unicode::narrowToWide(gi->m_name);
			result += Unicode::narrowToWide("\n");

			// guild abbreviation
			result += Unicode::narrowToWide("abbrev: ");
			result += Unicode::narrowToWide(gi->m_abbrev);
			result += Unicode::narrowToWide("\n");

			// guild leader
			result += Unicode::narrowToWide("leader: ");
			result += Unicode::narrowToWide(gi->m_leaderId.getValueString());
			result += Unicode::narrowToWide(" (");

			GuildMemberInfo const * const gmi = GuildInterface::getGuildMemberInfo(guildId, gi->m_leaderId);
			if (gmi)
				result += Unicode::narrowToWide(gmi->m_name);

			result += Unicode::narrowToWide(")\n");

			// previous election end time
			result += Unicode::narrowToWide("guild election previous end time: ");

			if (gi->m_guildElectionPreviousEndTime > 0)
				result += Unicode::narrowToWide(CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(gi->m_guildElectionPreviousEndTime)));
			else
				result += Unicode::narrowToWide("NONE");

			result += Unicode::narrowToWide("\n");

			// next election end time
			result += Unicode::narrowToWide("guild election next end time: ");

			if (gi->m_guildElectionNextEndTime > 0)
				result += Unicode::narrowToWide(CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(gi->m_guildElectionNextEndTime)));
			else
				result += Unicode::narrowToWide("NONE");

			result += Unicode::narrowToWide("\n");

			// guild faction
			uint32 const currentFaction = GuildInterface::getGuildCurrentFaction(guildId);			
			snprintf(buffer, sizeof(buffer)-1, "current faction: %lu (%s)\n", currentFaction, getFactionString(currentFaction).c_str());
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			uint32 const previousFaction = GuildInterface::getGuildPreviousFaction(guildId);
			if (!PvpData::isNeutralFactionId(previousFaction))
			{
				int const timeLeftPreviousFaction = GuildInterface::getTimeLeftGuildPreviousFaction(guildId);
				snprintf(buffer, sizeof(buffer)-1, "previous faction: %lu (%s) [left at %d (%s)]\n", previousFaction, getFactionString(previousFaction).c_str(), timeLeftPreviousFaction, CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(timeLeftPreviousFaction)).c_str());
				buffer[sizeof(buffer)-1] = '\0';
				result += Unicode::narrowToWide(buffer);
			}

			// guild GCW region defender
			std::string const & currentGcwDefenderRegion = GuildInterface::getGuildCurrentGcwDefenderRegion(guildId);
			result += Unicode::narrowToWide("current GCW defender region: ");
			if (currentGcwDefenderRegion.empty())
			{
				result += Unicode::narrowToWide("(NONE)\n");
			}
			else
			{
				int const timeJoinedCurrentGcwDefenderRegion = GuildInterface::getTimeJoinedGuildCurrentGcwDefenderRegion(guildId);
				snprintf(buffer, sizeof(buffer)-1, "%s [joined at %d (%s)]\n", currentGcwDefenderRegion.c_str(), timeJoinedCurrentGcwDefenderRegion, CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(timeJoinedCurrentGcwDefenderRegion)).c_str());
				buffer[sizeof(buffer)-1] = '\0';
				result += Unicode::narrowToWide(buffer);
			}

			std::string const & previousGcwDefenderRegion = GuildInterface::getGuildPreviousGcwDefenderRegion(guildId);
			if (!previousGcwDefenderRegion.empty())
			{
				int const timeLeftPreviousGcwDefenderRegion = GuildInterface::getTimeLeftGuildPreviousGcwDefenderRegion(guildId);
				snprintf(buffer, sizeof(buffer)-1, "previous GCW defender region: %s [joined at %d (%s), left at %d (%s)]\n", previousGcwDefenderRegion.c_str(), gi->m_timeJoinedGuildGcwDefenderRegion, CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(gi->m_timeJoinedGuildGcwDefenderRegion)).c_str(), timeLeftPreviousGcwDefenderRegion, CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(timeLeftPreviousGcwDefenderRegion)).c_str());
				buffer[sizeof(buffer)-1] = '\0';
				result += Unicode::narrowToWide(buffer);
			}

			// sponsored count
			snprintf(buffer, sizeof(buffer)-1, "sponsored count: %d\n", GuildInterface::getGuildCountSponsoredOnly(guildId));
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			// members only count
			snprintf(buffer, sizeof(buffer)-1, "members only count: %d\n", GuildInterface::getGuildCountMembersOnly(guildId));
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			// sponsored + members
			snprintf(buffer, sizeof(buffer)-1, "members and sponsored count: %d\n", GuildInterface::getGuildCountMembersAndSponsored(guildId));
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			// guild war pvp enabled
			snprintf(buffer, sizeof(buffer)-1, "guild pvp enabled count: %d\n", GuildInterface::getGuildCountMembersGuildWarPvPEnabled(guildId));
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			// guild size
			int totalSize;
			int totalSizeActive;
			int totalSizeInactive;
			int memberSize;
			int memberSizeActive;
			int memberSizeInactive;
			int sponsoredSize;
			int sponsoredSizeActive;
			int sponsoredSizeInactive;
			getGuildSize(guildId, gi, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

			snprintf(buffer, sizeof(buffer)-1, "size: %d (%d/%d) total, %d (%d/%d) member, %d (%d/%d) sponsored\n", totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			// members
			result += Unicode::narrowToWide("\n");
			result += Unicode::narrowToWide("members:\n");

			std::vector<NetworkId> memberIds;
			GuildInterface::getGuildMemberIds(guildId, memberIds);
			std::string gmiSpec;
			std::string rankStr;
			std::vector<NetworkId>::const_iterator iter;
			for (iter = memberIds.begin(); iter != memberIds.end(); ++iter)
			{
				GuildMemberInfo const * const gmi = GuildInterface::getGuildMemberInfo(guildId, *iter);
				if (!gmi)
					continue;

				if (gmi->m_permissions != static_cast<int>(GuildInterface::None))
				{
					// member id and name
					gmiSpec = iter->getValueString();
					gmiSpec += " (";
					gmiSpec += gmi->m_name;

					// inactive
					if (!NameManager::getInstance().isPlayer(*iter))
						gmiSpec += ", INACTIVE";

					gmiSpec += "), ";

					// member profession
					gmiSpec += gmi->m_professionSkillTemplate;
					gmiSpec += ", ";

					// member level
					snprintf(buffer, sizeof(buffer)-1, "%d", gmi->m_level);
					buffer[sizeof(buffer)-1] = '\0';
					gmiSpec += buffer;
					gmiSpec += ", ";

					// member title
					gmiSpec += gmi->m_title;
					gmiSpec += ", ";

					// member permission
					snprintf(buffer, sizeof(buffer)-1, "%d", gmi->m_permissions);
					buffer[sizeof(buffer)-1] = '\0';
					gmiSpec += buffer;

					gmiSpec += " (";
					gmiSpec += getGuildPermissionsText(gmi->m_permissions);
					gmiSpec += "), ";

					// member allegiance
					gmiSpec += gmi->m_allegiance.getValueString();
					gmiSpec += " (";

					{
						GuildMemberInfo const * const gmiAllegiance = GuildInterface::getGuildMemberInfo(guildId, gmi->m_allegiance);
						if (gmiAllegiance)
							gmiSpec += gmiAllegiance->m_name;
					}

					gmiSpec += "), ";

					// member rank
					gmi->m_rank.getAsDbTextString(rankStr);
					gmiSpec += rankStr;

					std::string rankDesc;
					std::vector<std::string> ranks;
					GuildInterface::getGuildMemberRank(guildId, *iter, ranks);
					for (std::vector<std::string>::const_iterator iterRank = ranks.begin(); iterRank != ranks.end(); ++iterRank)
					{
						if (!rankDesc.empty())
							rankDesc += ", ";

						rankDesc += *iterRank;
					}

					gmiSpec += " (";
					gmiSpec += rankDesc;
					gmiSpec += ")\n";

					result += Unicode::narrowToWide(gmiSpec);
				}
			}

			if (memberSize > 0)
				result += Unicode::narrowToWide("Output format is: \"id (name), profession, level, title, permissions (description), allegiance id (name), rank (description)\"\n");

			// sponsored
			result += Unicode::narrowToWide("\n");
			result += Unicode::narrowToWide("sponsored:\n");

			for (iter = memberIds.begin(); iter != memberIds.end(); ++iter)
			{
				GuildMemberInfo const * const gmi = GuildInterface::getGuildMemberInfo(guildId, *iter);
				if (!gmi)
					continue;

				if (gmi->m_permissions == static_cast<int>(GuildInterface::None))
				{
					// member id and name
					gmiSpec = iter->getValueString();
					gmiSpec += " (";
					gmiSpec += gmi->m_name;

					// inactive
					if (!NameManager::getInstance().isPlayer(*iter))
						gmiSpec += ", INACTIVE";

					gmiSpec += "), ";

					// member profession
					gmiSpec += gmi->m_professionSkillTemplate;
					gmiSpec += ", ";

					// member level
					snprintf(buffer, sizeof(buffer)-1, "%d", gmi->m_level);
					buffer[sizeof(buffer)-1] = '\0';
					gmiSpec += buffer;
					gmiSpec += "\n";

					result += Unicode::narrowToWide(gmiSpec);
				}
			}

			if (sponsoredSize > 0)
				result += Unicode::narrowToWide("Output format is: \"id (name), profession, level\"\n");

			// enemies
			result += Unicode::narrowToWide("\n");
			result += Unicode::narrowToWide("enemies:\n");

			std::vector<int> const & enemiesAToB = GuildInterface::getGuildEnemies(guildId);
			std::vector<int> const & enemiesBToA = GuildInterface::getGuildsAtWarWith(guildId);

			std::vector<int>::const_iterator iter2;
			for (iter2 = enemiesAToB.begin(); iter2 != enemiesAToB.end(); ++iter2)
			{
				GuildInfo const * const geGuildInfo = GuildInterface::getGuildInfo(*iter2);
				if (!geGuildInfo || geGuildInfo->m_name.empty())
					continue;

				if (std::find(enemiesBToA.begin(), enemiesBToA.end(), *iter2) == enemiesBToA.end())
				{
					snprintf(buffer, sizeof(buffer)-1, "this guild has declared war on guild %d (%s, %s)\n", *iter2, geGuildInfo->m_name.c_str(), geGuildInfo->m_abbrev.c_str());
					buffer[sizeof(buffer)-1] = '\0';
					result += Unicode::narrowToWide(buffer);
				}
				else
				{
					std::pair<int, time_t> const & killInfoAtoB = GuildInterface::getGuildEnemyKillInfo(guildId, *iter2);
					std::pair<int, time_t> const & killInfoBtoA = GuildInterface::getGuildEnemyKillInfo(*iter2, guildId);
					time_t const killInfoTime = std::max(killInfoAtoB.second, killInfoBtoA.second);

					snprintf(buffer, sizeof(buffer)-1, "this guild is at war with guild %d (%s, %s) (%d kills vs %d kills as of %s)\n", *iter2, geGuildInfo->m_name.c_str(), geGuildInfo->m_abbrev.c_str(), killInfoAtoB.first, killInfoBtoA.first, ((killInfoTime > 0) ? CalendarTime::convertEpochToTimeStringLocal(killInfoTime).c_str() : "<no kills yet>"));
					buffer[sizeof(buffer)-1] = '\0';
					result += Unicode::narrowToWide(buffer);
				}
			}

			for (iter2 = enemiesBToA.begin(); iter2 != enemiesBToA.end(); ++iter2)
			{
				GuildInfo const * const geGuildInfo = GuildInterface::getGuildInfo(*iter2);
				if (!geGuildInfo || geGuildInfo->m_name.empty())
					continue;

				if (std::find(enemiesAToB.begin(), enemiesAToB.end(), *iter2) == enemiesAToB.end())
				{
					snprintf(buffer, sizeof(buffer)-1, "guild %d (%s, %s) has declared war on this guild\n", *iter2, geGuildInfo->m_name.c_str(), geGuildInfo->m_abbrev.c_str());
					buffer[sizeof(buffer)-1] = '\0';
					result += Unicode::narrowToWide(buffer);
				}
			}

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
	}
	else if (isCommand(argv [0], "setGuildName"))
	{
		int const guildId = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		std::string const guildName = Unicode::getTrim(Unicode::wideToNarrow(argv[2]), " \t");
		bool const overrideReservedNameCheck = argv.size() == 4 && argv[3] == Unicode::narrowToWide("override");

		std::string reasonReserved;
		int existingGuildId;
		char buffer[1024];
		GuildInfo const * const gi = GuildInterface::getGuildInfo(guildId);

		if (!gi || gi->m_name.empty())
		{
			snprintf(buffer, sizeof(buffer)-1, "no guild with guild id %d\n", guildId);
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else if ((guildName.size() < 1) || (guildName.size() > 25))
		{
			snprintf(buffer, sizeof(buffer)-1, "guild name (%s) must be between 1 and 25 characters in length\n", guildName.c_str());
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else if (!overrideReservedNameCheck && NameManager::getInstance().isNameReserved(Unicode::narrowToWide(guildName), reasonReserved))
		{
			snprintf(buffer, sizeof(buffer)-1, "guild name (%s) is reserved (", guildName.c_str());
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);
			result += Unicode::narrowToWide(reasonReserved);
			result += Unicode::narrowToWide(") - use 'override' as the last parameter to bypass this check\n");

			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else if ((existingGuildId = GuildInterface::findGuild(guildName)) != 0)
		{
			snprintf(buffer, sizeof(buffer)-1, "guild name (%s) is already used as the name or abbreviation of guild %d\n", guildName.c_str(), existingGuildId);
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			snprintf(buffer, sizeof(buffer)-1, "changing guild name for guild %d from (%s) to (%s)\n", guildId, gi->m_name.c_str(), guildName.c_str());
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			GuildInterface::setGuildName(guildId, guildName);

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
	}
	else if (isCommand(argv [0], "setGuildAbbrev"))
	{
		int const guildId = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		std::string const guildAbbrev = Unicode::getTrim(Unicode::wideToNarrow(argv[2]), " \t");
		std::string reasonReserved;
		int existingGuildId;
		char buffer[1024];
		GuildInfo const * const gi = GuildInterface::getGuildInfo(guildId);

		if (!gi || gi->m_name.empty())
		{
			snprintf(buffer, sizeof(buffer)-1, "no guild with guild id %d\n", guildId);
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else if ((guildAbbrev.size() < 1) || (guildAbbrev.size() > 5))
		{
			snprintf(buffer, sizeof(buffer)-1, "guild abbreviation (%s) must be between 1 and 5 characters in length\n", guildAbbrev.c_str());
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else if (NameManager::getInstance().isNameReserved(Unicode::narrowToWide(guildAbbrev), reasonReserved))
		{
			snprintf(buffer, sizeof(buffer)-1, "guild abbreviation (%s) is reserved (", guildAbbrev.c_str());
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);
			result += Unicode::narrowToWide(reasonReserved);
			result += Unicode::narrowToWide(")\n");

			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else if ((existingGuildId = GuildInterface::findGuild(guildAbbrev)) != 0)
		{
			snprintf(buffer, sizeof(buffer)-1, "guild abbreviation (%s) is already used as the name or abbreviation of guild %d\n", guildAbbrev.c_str(), existingGuildId);
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			snprintf(buffer, sizeof(buffer)-1, "changing guild abbreviation for guild %d from (%s) to (%s)\n", guildId, gi->m_abbrev.c_str(), guildAbbrev.c_str());
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			GuildInterface::setGuildAbbrev(guildId, guildAbbrev);

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
	}
	else if (isCommand(argv [0], "setGuildLeader"))
	{
		int const guildId = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		NetworkId const leaderOid(Unicode::wideToNarrow(argv[2]));

		char buffer[1024];
		GuildInfo const * const gi = GuildInterface::getGuildInfo(guildId);

		if (!gi || gi->m_name.empty())
		{
			snprintf(buffer, sizeof(buffer)-1, "no guild with guild id %d\n", guildId);
			buffer[sizeof(buffer)-1] = '\0';
			result += Unicode::narrowToWide(buffer);

			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			// is new guild leader already current leader?
			NetworkId const currentLeaderOid = GuildInterface::getGuildLeaderId(guildId);
			if (currentLeaderOid == leaderOid)
			{
				snprintf(buffer, sizeof(buffer)-1, "%s is already the guild leader for guild %d\n", currentLeaderOid.getValueString().c_str(), guildId);
				buffer[sizeof(buffer)-1] = '\0';
				result += Unicode::narrowToWide(buffer);

				result += getErrorMessage(argv[0], ERR_FAIL);
			}
			else
			{
				// is new guild leader an existing guild member or existing sponsored member?
				GuildMemberInfo const * gmi = GuildInterface::getGuildMemberInfo(guildId, leaderOid);
				if (gmi)
				{
					snprintf(buffer, sizeof(buffer)-1, "making existing guild %s %s (%s) the guild leader for guild %d\n", ((gmi->m_permissions == 0) ? "sponsored member" : "member"), leaderOid.getValueString().c_str(), gmi->m_name.c_str(), guildId);
					buffer[sizeof(buffer)-1] = '\0';
					result += Unicode::narrowToWide(buffer);

					// make the existing guild member or existing sponsored member the guild leader
					GuildInterface::setGuildLeader(guildId, leaderOid);

					// set the new guild leader's permission to All
					GuildInterface::setGuildMemberPermission(
							guildId,
							leaderOid,
							(gmi->m_permissions | static_cast<int>(GuildInterface::AllAdministrativePermissions)));

					// reset the previous guild leader's permission
					GuildMemberInfo const * gmiPreviousGuildLeader = GuildInterface::getGuildMemberInfo(guildId, currentLeaderOid);
					if (gmiPreviousGuildLeader)
					{
						GuildInterface::setGuildMemberPermission(
							guildId,
							currentLeaderOid,
							(gmiPreviousGuildLeader->m_permissions & static_cast<int>(~(static_cast<unsigned int>(GuildInterface::AllAdministrativePermissions)))) | static_cast<int>(GuildInterface::Member));
					}

					result += getErrorMessage(argv[0], ERR_SUCCESS);
				}
				else
				{
					// new guild leader is not a guild member, so make a guild member first, then make guild leader
					ServerObject const* o = dynamic_cast<ServerObject const*>(ServerWorld::findObjectByNetworkId(leaderOid));
					CreatureObject const* c = (o ? o->asCreatureObject() : nullptr);
					PlayerObject const* p = (c ? PlayerCreatureController::getPlayerObject(c) : nullptr);
					if (o == nullptr)
					{
						result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
					}
					else if (c == nullptr)
					{
						result += Unicode::narrowToWide("specified object is not a creature object\n");
					}
					else if (p == nullptr)
					{
						result += Unicode::narrowToWide("specified object is not a character object\n");
					}
					else
					{
						std::string const objectName = Unicode::wideToNarrow(o->getAssignedObjectName());
						if (objectName.empty())
						{
							result += Unicode::narrowToWide("could not determine character name for specified character object\n");
						}
						else
						{
							snprintf(buffer, sizeof(buffer)-1, "making %s (%s) the guild leader for guild %d\n", leaderOid.getValueString().c_str(), objectName.c_str(), guildId);
							buffer[sizeof(buffer)-1] = '\0';
							result += Unicode::narrowToWide(buffer);

							// add the new guild leader
							GuildInterface::addGuildCreatorMember(guildId, leaderOid);

							// make a new guild leader
							GuildInterface::setGuildLeader(guildId, leaderOid);

							// reset the previous guild leader's permission
							GuildMemberInfo const * gmiPreviousGuildLeader = GuildInterface::getGuildMemberInfo(guildId, currentLeaderOid);
							if (gmiPreviousGuildLeader)
							{
								GuildInterface::setGuildMemberPermission(
									guildId,
									currentLeaderOid,
									(gmiPreviousGuildLeader->m_permissions & static_cast<int>(~(static_cast<unsigned int>(GuildInterface::AllAdministrativePermissions)))) | static_cast<int>(GuildInterface::Member));
							}

							result += getErrorMessage(argv[0], ERR_SUCCESS);
						}
					}
				}
			}
		}
	}
	else if (isCommand(argv [0], "findGuildMemberNameMismatch"))
	{
		char buffer[1024];
		std::vector<int> guildIds;
		std::string memberName;
		int memberCount = 0;
		int mismatchCount = 0;

		GuildInterface::getAllGuildIds(guildIds);
		for (std::vector<int>::const_iterator iterGuild = guildIds.begin(); iterGuild != guildIds.end(); ++iterGuild)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iterGuild);
			if (gi && !gi->m_name.empty())
			{
				std::vector<NetworkId> memberIds;
				GuildInterface::getGuildMemberIds(*iterGuild, memberIds);
				for (std::vector<NetworkId>::const_iterator iterGuildMember = memberIds.begin(); iterGuildMember != memberIds.end(); ++iterGuildMember)
				{
					GuildMemberInfo const * const gmi = GuildInterface::getGuildMemberInfo(*iterGuild, *iterGuildMember);
					if (!gmi)
						continue;

					++memberCount;

					memberName = NameManager::getInstance().getPlayerFullName(*iterGuildMember);
					if (memberName.empty())
					{
						CachedNetworkId memberId(*iterGuildMember);
						ServerObject const * so = safe_cast<ServerObject const *>(memberId.getObject());
						if (so)
							memberName = Unicode::wideToNarrow(so->getAssignedObjectName());
					}

					if (!memberName.empty() && (memberName != gmi->m_name))
					{
						++mismatchCount;
						snprintf(buffer, sizeof(buffer)-1, "object name (%s), guild member (%s, %s), guild %d (%s, %s)\n", memberName.c_str(), iterGuildMember->getValueString().c_str(), gmi->m_name.c_str(), *iterGuild, gi->m_name.c_str(), gi->m_abbrev.c_str());
						buffer[sizeof(buffer)-1] = '\0';
						result += Unicode::narrowToWide(buffer);
					}
				}
			}
		}

		snprintf(buffer, sizeof(buffer)-1, "%d guild members searched, %d mismatches found\n", memberCount, mismatchCount);
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "findGuildLeaderPermissionMismatch"))
	{
		char buffer[1024];
		std::vector<int> guildIds;
		int leaderCount = 0;
		int mismatchCount = 0;

		GuildInterface::getAllGuildIds(guildIds);
		for (std::vector<int>::const_iterator iterGuild = guildIds.begin(); iterGuild != guildIds.end(); ++iterGuild)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iterGuild);
			if (gi && !gi->m_name.empty())
			{
				++leaderCount;

				GuildMemberInfo const * const gmi = GuildInterface::getGuildMemberInfo(*iterGuild, gi->m_leaderId);
				if (gmi)
				{
					if ((gmi->m_permissions & static_cast<int>(GuildInterface::AllAdministrativePermissions)) != static_cast<int>(GuildInterface::AllAdministrativePermissions))
					{
						++mismatchCount;
						snprintf(buffer, sizeof(buffer)-1, "guild leader (%s, %s), guild %d (%s, %s), guild permission %d (%s)\n", gi->m_leaderId.getValueString().c_str(), gmi->m_name.c_str(), *iterGuild, gi->m_name.c_str(), gi->m_abbrev.c_str(), gmi->m_permissions, getGuildPermissionsText(gmi->m_permissions).c_str());
						buffer[sizeof(buffer)-1] = '\0';
						result += Unicode::narrowToWide(buffer);
					}
				}
			}
		}

		snprintf(buffer, sizeof(buffer)-1, "%d guild leaders searched, %d mismatches found\n", leaderCount, mismatchCount);
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "findGuildLeaderMismatch"))
	{
		char buffer[1024];
		std::vector<int> guildIds;
		int guildCount = 0;
		int mismatchCount = 0;

		GuildInterface::getAllGuildIds(guildIds);
		for (std::vector<int>::const_iterator iterGuild = guildIds.begin(); iterGuild != guildIds.end(); ++iterGuild)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iterGuild);
			if (gi && !gi->m_name.empty())
			{
				++guildCount;

				if (gi->m_leaderId.isValid())
				{
					GuildMemberInfo const * const gmi = GuildInterface::getGuildMemberInfo(*iterGuild, gi->m_leaderId);
					if (!gmi)
					{
						++mismatchCount;
						snprintf(buffer, sizeof(buffer)-1, "guild leader (%s) is not guild member, guild %d (%s, %s)\n", gi->m_leaderId.getValueString().c_str(), *iterGuild, gi->m_name.c_str(), gi->m_abbrev.c_str());
						buffer[sizeof(buffer)-1] = '\0';
						result += Unicode::narrowToWide(buffer);
					}
				}
			}
		}

		snprintf(buffer, sizeof(buffer)-1, "%d guilds searched, %d guilds found where guild leader is not guild member\n", guildCount, mismatchCount);
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "findLeaderlessGuild"))
	{
		char buffer[1024];
		std::vector<int> guildIds;
		int guildCount = 0;
		int mismatchCount = 0;

		GuildInterface::getAllGuildIds(guildIds);
		for (std::vector<int>::const_iterator iterGuild = guildIds.begin(); iterGuild != guildIds.end(); ++iterGuild)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iterGuild);
			if (gi && !gi->m_name.empty())
			{
				++guildCount;

				if (gi->m_leaderId.isValid())
				{
					if (!NameManager::getInstance().isPlayer(gi->m_leaderId))
					{
						++mismatchCount;
						snprintf(buffer, sizeof(buffer)-1, "guild leader (%s) is not in DB, guild %d (%s, %s)\n", gi->m_leaderId.getValueString().c_str(), *iterGuild, gi->m_name.c_str(), gi->m_abbrev.c_str());
						buffer[sizeof(buffer)-1] = '\0';
						result += Unicode::narrowToWide(buffer);
					}
				}
			}
		}

		snprintf(buffer, sizeof(buffer)-1, "%d guilds searched, %d guilds found where guild leader is not in DB\n", guildCount, mismatchCount);
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "findOrphanedGuild"))
	{
		char buffer[1024];
		std::vector<int> guildIds;
		int guildCount = 0;
		int mismatchCount = 0;

		GuildInterface::getAllGuildIds(guildIds);
		for (std::vector<int>::const_iterator iterGuild = guildIds.begin(); iterGuild != guildIds.end(); ++iterGuild)
		{
			GuildInfo const * const gi = GuildInterface::getGuildInfo(*iterGuild);
			if (gi && !gi->m_name.empty())
			{
				++guildCount;

				if (!gi->m_leaderId.isValid())
				{
					++mismatchCount;
					snprintf(buffer, sizeof(buffer)-1, "guild %d (%s, %s) has no leader\n", *iterGuild, gi->m_name.c_str(), gi->m_abbrev.c_str());
					buffer[sizeof(buffer)-1] = '\0';
					result += Unicode::narrowToWide(buffer);
				}
			}
		}

		snprintf(buffer, sizeof(buffer)-1, "%d guilds searched, %d guilds found with no guild leader\n", guildCount, mismatchCount);
		buffer[sizeof(buffer)-1] = '\0';
		result += Unicode::narrowToWide(buffer);

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	return true;
}	// ConsoleCommandParserGuild::performParsing

//-----------------------------------------------------------------

void ConsoleCommandParserGuildNamespace::buildGuildInfoSpec(int guildId, GuildInfo const & gi, std::string & giSpec, int & totalSize, int & totalSizeActive, int & totalSizeInactive, int & memberSize, int & memberSizeActive, int & memberSizeInactive, int & sponsoredSize, int & sponsoredSizeActive, int & sponsoredSizeInactive)
{
	giSpec.clear();
	totalSize = 0;
	memberSize = 0;
	sponsoredSize = 0;

	if (gi.m_name.empty())
		return;

	char buffer[1024];

	// guild id
	snprintf(buffer, sizeof(buffer)-1, "%d", guildId);
	buffer[sizeof(buffer)-1] = '\0';

	giSpec = buffer;
	giSpec += ", ";

	// guild name
	giSpec += gi.m_name;
	giSpec += ", ";

	// guild abbreviation
	giSpec += gi.m_abbrev;
	giSpec += ", ";

	// guild leader id
	giSpec += gi.m_leaderId.getValueString();
	giSpec += ", ";

	// guild leader name
	GuildMemberInfo const * const gmi = GuildInterface::getGuildMemberInfo(guildId, gi.m_leaderId);
	if (gmi)
		giSpec += gmi->m_name;

	giSpec += ", ";

	// guild size
	getGuildSize(guildId, gi, totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);

	snprintf(buffer, sizeof(buffer)-1, "%d (%d/%d), %d (%d/%d), %d (%d/%d)", totalSize, totalSizeActive, totalSizeInactive, memberSize, memberSizeActive, memberSizeInactive, sponsoredSize, sponsoredSizeActive, sponsoredSizeInactive);
	buffer[sizeof(buffer)-1] = '\0';

	giSpec += buffer;
}

// ----------------------------------------------------------------------

void ConsoleCommandParserGuildNamespace::getGuildSize(int guildId, GuildInfo const & gi, int & total, int & totalActive, int & totalInactive, int & member, int & memberActive, int & memberInactive, int & sponsored, int & sponsoredActive, int & sponsoredInactive)
{
	total = 0;
	totalActive = 0;
	totalInactive = 0;

	member = 0;
	memberActive = 0;
	memberInactive = 0;

	sponsored = 0;
	sponsoredActive = 0;
	sponsoredInactive = 0;

	if (gi.m_name.empty())
		return;

	std::vector<NetworkId> memberIds;
	GuildInterface::getGuildMemberIds(guildId, memberIds);
	if (memberIds.empty())
		return;

	bool active;
	for (std::vector<NetworkId>::const_iterator iter = memberIds.begin(); iter != memberIds.end(); ++iter)
	{
		GuildMemberInfo const * const gmi = GuildInterface::getGuildMemberInfo(guildId, *iter);
		if (!gmi)
			continue;

		active = NameManager::getInstance().isPlayer(*iter);

		++total;
		if (active)
			++totalActive;
		else
			++totalInactive;

		if (gmi->m_permissions == static_cast<int>(GuildInterface::None))
		{
			++sponsored;
			if (active)
				++sponsoredActive;
			else
				++sponsoredInactive;
		}
		else
		{
			++member;
			if (active)
				++memberActive;
			else
				++memberInactive;
		}
	}
}

// ----------------------------------------------------------------------

std::string const ConsoleCommandParserGuildNamespace::getGuildPermissionsText(int permissions)
{
	std::string result;
	if (permissions == static_cast<int>(GuildInterface::None))
	{
		result = "None";
	}
	else
	{
		if ((permissions & static_cast<int>(GuildInterface::AllAdministrativePermissions)) == static_cast<int>(GuildInterface::AllAdministrativePermissions))
		{
			result = "All ";
		}
		else
		{
			if (permissions & static_cast<int>(GuildInterface::Member))
				result += "Member ";
			if (permissions & static_cast<int>(GuildInterface::Sponsor))
				result += "Sponsor ";
			if (permissions & static_cast<int>(GuildInterface::Disband))
				result += "Disband ";
			if (permissions & static_cast<int>(GuildInterface::Accept))
				result += "Accept ";
			if (permissions & static_cast<int>(GuildInterface::Kick))
				result += "Kick ";
			if (permissions & static_cast<int>(GuildInterface::Mail))
				result += "Mail ";
			if (permissions & static_cast<int>(GuildInterface::Title))
				result += "Title ";
			if (permissions & static_cast<int>(GuildInterface::Namechange))
				result += "Namechange ";
			if (permissions & static_cast<int>(GuildInterface::War))
				result += "War ";
			if (permissions & static_cast<int>(GuildInterface::Rank))
				result += "Rank ";
		}

		if (permissions & static_cast<int>(GuildInterface::ElectionCandidate))
			result += "ElectionCandidate ";

		if (permissions & static_cast<int>(GuildInterface::WarInclusion))
			result += "WarInclusion ";

		if (permissions & static_cast<int>(GuildInterface::WarExclusion))
			result += "WarExclusion ";
	}

	return result;
}

// ----------------------------------------------------------------------

std::string const ConsoleCommandParserGuildNamespace::getFactionString(uint32 factionId)
{
	if (PvpData::isNeutralFactionId(factionId))
		return "Neutral";
	else if (PvpData::isImperialFactionId(factionId))
		return "Imperial";
	else if (PvpData::isRebelFactionId(factionId))
		return "Rebel";

	return "Unknown";
}

// ======================================================================
