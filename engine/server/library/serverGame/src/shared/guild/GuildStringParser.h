// ======================================================================
//
// GuildStringParser.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _GuildStringParser_H_
#define _GuildStringParser_H_

// ======================================================================

class BitArray;
class NetworkId;

// ======================================================================

class GuildStringParser // static
{
public:
	static bool parseGuildName   (std::string const &source, int &guildId, std::string &guildName, int &guildElectionPreviousEndTime, int &guildElectionNextEndTime, uint32 &guildFaction, int &timeLeftGuildFaction, std::string &guildGcwDefenderRegion, int &timeJoinedGuildGcwDefenderRegion, int &timeLeftGuildGcwDefenderRegion);
	static bool parseGuildAbbrev (std::string const &source, int &guildId, std::string &guildAbbrev);
	static bool parseGuildLeader (std::string const &source, int &guildId, NetworkId &leaderId);
	static bool parseMemberInfo  (std::string const &source, int &guildId, NetworkId &memberId, std::string &memberName, std::string &memberProfessionSkillTemplate, int &memberLevel, int &permissions, BitArray &rank, std::string &title, NetworkId &allegiance);
	static bool parseEnemyInfo   (std::string const &source, int &guildId, int &enemyId, int &killCount, int &lastUpdateTime);

	static void buildNameSpec    (int guildId, std::string const &guildName, int guildElectionPreviousEndTime, int guildElectionNextEndTime, uint32 guildFaction, int timeLeftGuildFaction, std::string const & guildGcwDefenderRegion, int timeJoinedGuildGcwDefenderRegion, int timeLeftGuildGcwDefenderRegion, std::string &result);
	static void buildAbbrevSpec  (int guildId, std::string const &guildAbbrev, std::string &result);
	static void buildLeaderSpec  (int guildId, NetworkId const &leaderId, std::string &result);
	static void buildMemberSpec  (int guildId, NetworkId const &memberId, std::string const &memberName, std::string const &memberProfessionSkillTemplate, int memberLevel, int permissions, BitArray const &rank, std::string const &title, NetworkId const &allegiance, std::string &result);
	static void buildEnemySpec   (int guildId, int enemyId, int killCount, int lastUpdateTime, std::string &result);

private:
	GuildStringParser();
};

// ======================================================================

#endif // _GuildStringParser_H_

