// ======================================================================
//
// GuildStringParser.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/GuildStringParser.h"

#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/PvpData.h"

// ======================================================================

static void skipField(std::string const &source, unsigned int &pos)
{
	while (source[pos] && source[pos] != ':')
		++pos;
	if (source[pos] == ':')
		++pos;
}

// ----------------------------------------------------------------------

static void nextInt(std::string const &source, unsigned int &pos, int &ret)
{
	ret = atoi(source.c_str()+pos);
	skipField(source, pos);
}

// ----------------------------------------------------------------------

static void nextString(std::string const &source, unsigned int &pos, std::string &ret)
{
	unsigned int oldPos = pos;
	while (source[pos] && source[pos] != ':')
		++pos;
	ret = source.substr(oldPos, pos-oldPos);
	if (source[pos] == ':')
		++pos;
}

// ----------------------------------------------------------------------

static void nextOid(std::string const &source, unsigned int &pos, NetworkId &ret)
{
	unsigned int oldPos = pos;
	skipField(source, pos);
	ret = NetworkId(source.substr(oldPos, pos-oldPos));
}

// ======================================================================

bool GuildStringParser::parseGuildName(std::string const &source, int &guildId, std::string &guildName, int &guildElectionPreviousEndTime, int &guildElectionNextEndTime, uint32 &guildFaction, int &timeLeftGuildFaction, std::string &guildGcwDefenderRegion, int &timeJoinedGuildGcwDefenderRegion, int &timeLeftGuildGcwDefenderRegion)
{
	// need to parse different versions
	if (source.find("v3:") == 0) // version 3
	{
		unsigned int pos = 3;
		nextInt(source, pos, guildId);
		nextString(source, pos, guildName);
		nextInt(source, pos, guildElectionPreviousEndTime);
		nextInt(source, pos, guildElectionNextEndTime);

		int temp;
		nextInt(source, pos, temp);
		guildFaction = static_cast<uint32>(temp);

		nextInt(source, pos, timeLeftGuildFaction);
		nextString(source, pos, guildGcwDefenderRegion);
		nextInt(source, pos, timeJoinedGuildGcwDefenderRegion);
		nextInt(source, pos, timeLeftGuildGcwDefenderRegion);
	}
	else if (source.find("v2:") == 0) // version 2
	{
		unsigned int pos = 3;
		nextInt(source, pos, guildId);
		nextString(source, pos, guildName);
		nextInt(source, pos, guildElectionPreviousEndTime);
		nextInt(source, pos, guildElectionNextEndTime);
		guildFaction = 0;
		timeLeftGuildFaction = 0;
		guildGcwDefenderRegion.clear();
		timeJoinedGuildGcwDefenderRegion = 0;
		timeLeftGuildGcwDefenderRegion = 0;
	}
	else // version 1
	{
		unsigned int pos = 0;
		nextInt(source, pos, guildId);
		nextString(source, pos, guildName);
		guildElectionPreviousEndTime = 0;
		guildElectionNextEndTime = 0;
		guildFaction = 0;
		timeLeftGuildFaction = 0;
		guildGcwDefenderRegion.clear();
		timeJoinedGuildGcwDefenderRegion = 0;
		timeLeftGuildGcwDefenderRegion = 0;
	}

	bool const ok = guildId && !guildName.empty();
	WARNING(!ok, ("GuildStringParser::parseGuildName: bad string [%s]", source.c_str()));
	return ok;
}

// ----------------------------------------------------------------------

bool GuildStringParser::parseGuildAbbrev(std::string const &source, int &guildId, std::string &guildAbbrev)
{
	unsigned int pos = 0;
	nextInt(source, pos, guildId);
	nextString(source, pos, guildAbbrev);
	bool ok = guildId && !guildAbbrev.empty();
	WARNING(!ok, ("GuildStringParser::parseGuildAbbrev: bad string [%s]", source.c_str()));
	return ok;
}

// ----------------------------------------------------------------------

bool GuildStringParser::parseGuildLeader(std::string const &source, int &guildId, NetworkId &leaderId)
{
	unsigned int pos = 0;
	nextInt(source, pos, guildId);
	nextOid(source, pos, leaderId);
	bool ok = guildId && leaderId != NetworkId::cms_invalid;
	WARNING(!ok, ("GuildStringParser::parseGuildAbbrev: bad string [%s]", source.c_str()));
	return ok;
}

// ----------------------------------------------------------------------

bool GuildStringParser::parseMemberInfo(std::string const &source, int &guildId, NetworkId &memberId, std::string &memberName, std::string &memberProfessionSkillTemplate, int &memberLevel, int &permissions, BitArray &rank, std::string &title, NetworkId &allegiance)
{
	// need to parse different versions
	if (source.find("v2:") == 0) // version 2
	{
		unsigned int pos = 3;
		nextInt(source, pos, guildId);
		nextOid(source, pos, memberId);
		nextString(source, pos, memberName);
		nextString(source, pos, memberProfessionSkillTemplate);
		nextInt(source, pos, memberLevel);
		nextInt(source, pos, permissions);
		nextString(source, pos, title);
		nextOid(source, pos, allegiance);

		std::string rankStr;
		nextString(source, pos, rankStr);
		rank.setFromDbTextString(rankStr.c_str());
	}
	else // version 1
	{
		unsigned int pos = 0;
		nextInt(source, pos, guildId);
		nextOid(source, pos, memberId);
		nextString(source, pos, memberName);
		memberProfessionSkillTemplate.clear();
		memberLevel = 0;
		nextInt(source, pos, permissions);
		nextString(source, pos, title);
		nextOid(source, pos, allegiance);
		rank.clear();
	}

	bool const ok = guildId && memberId != NetworkId::cms_invalid && !memberName.empty();
	WARNING(!ok, ("GuildStringParser::parseMemberInfo: bad string [%s]", source.c_str()));
	return ok;
}

// ----------------------------------------------------------------------

bool GuildStringParser::parseEnemyInfo(std::string const &source, int &guildId, int &enemyId, int &killCount, int &lastUpdateTime)
{
	// need to parse different versions
	if (source.find("v2:") == 0) // version 2
	{
		unsigned int pos = 3;
		nextInt(source, pos, guildId);
		nextInt(source, pos, enemyId);
		nextInt(source, pos, killCount);
		nextInt(source, pos, lastUpdateTime);
	}
	else // version 1
	{
		unsigned int pos = 0;
		nextInt(source, pos, guildId);
		nextInt(source, pos, enemyId);
		killCount = 0;
		lastUpdateTime = 0;
	}

	bool const ok = guildId && enemyId;
	WARNING(!ok, ("GuildStringParser::parseEnemyInfo: bad string [%s]", source.c_str()));
	return ok;
}

// ----------------------------------------------------------------------

void GuildStringParser::buildNameSpec(int guildId, std::string const &guildName, int guildElectionPreviousEndTime, int guildElectionNextEndTime, uint32 guildFaction, int timeLeftGuildFaction, std::string const & guildGcwDefenderRegion, int timeJoinedGuildGcwDefenderRegion, int timeLeftGuildGcwDefenderRegion, std::string &result)
{
	FATAL(guildName.length() > 25, ("GuildStringParser::buildNameSpec: bad guild name"));

	// DB column to store information is VARCHAR2(500)
	char buffer[501]; // 500 chars + terminating \0

	// only write out newer version if there is actually new version information
	if (!PvpData::isNeutralFactionId(guildFaction) || (timeLeftGuildFaction > 0) || (timeJoinedGuildGcwDefenderRegion > 0) || (timeLeftGuildGcwDefenderRegion > 0)) // version 3
	{
		snprintf(buffer, sizeof(buffer), "v3:%d:%s:%d:%d:%d:%d:%s:%d:%d", guildId, guildName.c_str(), guildElectionPreviousEndTime, guildElectionNextEndTime, static_cast<int>(guildFaction), timeLeftGuildFaction, guildGcwDefenderRegion.c_str(), timeJoinedGuildGcwDefenderRegion, timeLeftGuildGcwDefenderRegion);
	}
	else if ((guildElectionPreviousEndTime > 0) || (guildElectionNextEndTime > 0)) // version 2
	{
		snprintf(buffer, sizeof(buffer), "v2:%d:%s:%d:%d", guildId, guildName.c_str(), guildElectionPreviousEndTime, guildElectionNextEndTime);
	}
	else // version 1
	{
		snprintf(buffer, sizeof(buffer), "%d:%s", guildId, guildName.c_str());
	}

	// absolutely, positively make sure that there is no more than 500
	// characters in buffer (not including the terminating \0) or the DB
	// will ***CRASH*** because the column is defined as VARCHAR2(500)
	buffer[500] = '\0';
	result = buffer;
}

// ----------------------------------------------------------------------

void GuildStringParser::buildAbbrevSpec(int guildId, std::string const &guildAbbrev, std::string &result)
{
	FATAL(guildAbbrev.length() > 5, ("GuildStringParser::buildAbbrevSpec: bad guild abbrev"));
	char buffer[32];
	// max field lengths - 11:5 -> 16
	snprintf(buffer, sizeof(buffer)-1, "%d:%s", guildId, guildAbbrev.c_str());
	buffer[sizeof(buffer)-1] = '\0';
	result = buffer;
}

// ----------------------------------------------------------------------

void GuildStringParser::buildLeaderSpec(int guildId, NetworkId const &leaderId, std::string &result)
{
	char buffer[64];
	// max field lengths - 11:21 -> 33
	snprintf(buffer, sizeof(buffer)-1, "%d:%s", guildId, leaderId.getValueString().c_str());
	buffer[sizeof(buffer)-1] = '\0';
	result = buffer;
}

// ----------------------------------------------------------------------

void GuildStringParser::buildMemberSpec(int const guildId, NetworkId const &memberId, std::string const &memberName, std::string const &memberProfessionSkillTemplate, int const memberLevel, int const permissions, BitArray const &rank, std::string const &title, NetworkId const &allegiance, std::string &result)
{
	FATAL(title.length() > 25, ("GuildStringParser::buildMemberSpec: bad title"));

	std::string rankStr;
	rank.getAsDbTextString(rankStr);

	// DB column to store information is VARCHAR2(500)
	char buffer[501]; // 500 chars + terminating \0

	// only write out newer version if there is actually new version information
	if ((memberLevel > 0) || !rankStr.empty() || !memberProfessionSkillTemplate.empty()) // version 2
	{
		snprintf(buffer, sizeof(buffer), "v2:%d:%s:%s:%s:%d:%d:%s:%s:%s",
			guildId,
			memberId.getValueString().c_str(),
			memberName.c_str(),
			memberProfessionSkillTemplate.c_str(),
			memberLevel,
			permissions,
			title.c_str(),
			allegiance.getValueString().c_str(),
			rankStr.c_str());
	}
	else // version 1
	{
		snprintf(buffer, sizeof(buffer), "%d:%s:%s:%d:%s:%s",
			guildId,
			memberId.getValueString().c_str(),
			memberName.c_str(),
			permissions,
			title.c_str(),
			allegiance.getValueString().c_str());
	}

	// absolutely, positively make sure that there is no more than 500
	// characters in buffer (not including the terminating \0) or the DB
	// will ***CRASH*** because the column is defined as VARCHAR2(500)
	buffer[500] = '\0';
	result = buffer;
}

// ----------------------------------------------------------------------

void GuildStringParser::buildEnemySpec(int guildId, int enemyId, int killCount, int lastUpdateTime, std::string &result)
{
	// DB column to store information is VARCHAR2(500)
	char buffer[501]; // 500 chars + terminating \0

	// only write out newer version if there is actually new version information
	if (killCount > 0) // version 2
	{
		snprintf(buffer, sizeof(buffer), "v2:%d:%d:%d:%d", guildId, enemyId, killCount, lastUpdateTime);
	}
	else // version 1
	{
		snprintf(buffer, sizeof(buffer), "%d:%d", guildId, enemyId);
	}

	// absolutely, positively make sure that there is no more than 500
	// characters in buffer (not including the terminating \0) or the DB
	// will ***CRASH*** because the column is defined as VARCHAR2(500)
	buffer[500] = '\0';
	result = buffer;
}

// ======================================================================

