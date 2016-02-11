// ======================================================================
//
// GuildInterface.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/GuildInterface.h"

#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/GuildObject.h"
#include "serverGame/GuildStringParser.h"
#include "serverGame/ServerUniverse.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedNetworkMessages/ChatRoomData.h"

#include <unordered_map>

// ======================================================================

namespace GuildInterfaceNamespace
{
	// ----------------------------------------------------------------------
	const std::string s_empty;
	uint32 const s_guildChannelLimit = 400;

	int const s_maxInactiveGuildWarsTracked = 100;
	std::string const s_objvarInactiveGuildWarsPrefix("guildWarsEnded");
	std::string const s_objvarInactiveGuildWarsMostRecentIndex = s_objvarInactiveGuildWarsPrefix + std::string(".mostRecentIndex");

	std::map<int, std::vector<int> > s_guildEnemiesAToB;
	std::map<int, std::map<int, std::pair<int, time_t> > > s_guildEnemiesAToBKillInfo;
	std::map<int, std::vector<int> > s_guildEnemiesBToA;
	bool s_needEnemiesRebuild;

	// dictionary containing the table showing all active guild wars with at least 1 kill
	ScriptParams * s_activeGuildWars = nullptr;
	bool s_activeGuildWarsNeedRebuild = true;

	// dictionary containing the table showing the 100 most recently ended guild wars with at least 1 kill
	ScriptParams * s_inactiveGuildWars = nullptr;
	int s_inactiveGuildWarsMostRecentUpdateIndex = 0;

	typedef std::map<std::string, NetworkId> PendingChannelAddList; //using a map to make it easier to prevent redundant entries
	typedef std::map<std::string,PendingChannelAddList> PendingChannelAddMap;
	PendingChannelAddMap s_pendingChannelAdds;

	// ----------------------------------------------------------------------

	std::string const &getChatRoomPrefix()
	{ //lint !e1929 // returning a reference makes sense here.
		static std::string const prefix = std::string("SWG.") + GameServer::getInstance().getClusterName().c_str() + ".guild.";
		return prefix;
	}

	// ----------------------------------------------------------------------

	std::string const &getChatRoomSuffix()
	{ //lint !e1929 // returning a reference makes sense here.
		static std::string const suffix("." + ChatRoomTypes::ROOM_GUILD);
		return suffix;
	}

	// ----------------------------------------------------------------------

	std::string getChatRoomPath(int guildId)
	{
		char buf[256];
		snprintf(buf, 256, "%s%d%s", getChatRoomPrefix().c_str(), guildId, getChatRoomSuffix().c_str());
		return std::string(buf);
	}

	// ----------------------------------------------------------------------

	std::string getChatRoomTitle(int guildId)
	{
		char buf[32];
		snprintf(buf, 32, "%d", guildId);
		return std::string(buf);
	}

	// ----------------------------------------------------------------------

	std::string const & getVoiceChatDisplayName()
	{
		return ChatRoomTypes::ROOM_GUILD;
	}

	// ----------------------------------------------------------------------

	const char * makeCopyOfString(const char * rhs)
	{
		char * lhs = nullptr;
		if (rhs)
		{
			lhs = new char[strlen(rhs) + 1];
			strcpy(lhs, rhs);
		}
		else
		{
			lhs = new char[1];
			lhs[0] = '\0';
		}

		return lhs;
	}
}
using namespace GuildInterfaceNamespace;

// ======================================================================

void GuildInterface::update()
{
	if (ServerUniverse::getInstance().isUniverseLoaded())
	{
		GuildObject *masterGuildObject = ServerUniverse::getInstance().getMasterGuildObject();
		if (masterGuildObject)
		{
			if (s_needEnemiesRebuild)
				buildEnemies(*masterGuildObject);
		}
	}
}

// ----------------------------------------------------------------------

void GuildInterface::touchEnemies()
{
	s_needEnemiesRebuild = true;
}

// ----------------------------------------------------------------------

void GuildInterface::buildEnemies(GuildObject const &guildObject)
{
	PROFILER_AUTO_BLOCK_DEFINE("GuildInterface::buildEnemies");

	s_needEnemiesRebuild = false;
	s_activeGuildWarsNeedRebuild = true;

	// potential pvp updates for every guilded creature
	std::vector<PvpUpdateObserver*> pvpUpdateObservers;
	{
		CreatureObject::AllCreaturesSet const &creatureList = CreatureObject::getAllCreatures();
		for (CreatureObject::AllCreaturesSet::const_iterator i = creatureList.begin(); i != creatureList.end(); ++i)
			if ((*i)->getGuildId())
				pvpUpdateObservers.push_back(new PvpUpdateObserver(*i, Archive::ADOO_generic));
	}

	s_guildEnemiesAToB.clear();
	s_guildEnemiesAToBKillInfo.clear();
	s_guildEnemiesBToA.clear();

	std::set<std::string> const &enemies = guildObject.getGuildEnemies();
	for (std::set<std::string>::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
	{
		int guildId, enemyId, killCount, lastUpdateTime;
		if (GuildStringParser::parseEnemyInfo(*i, guildId, enemyId, killCount, lastUpdateTime))
		{
			s_guildEnemiesAToB[guildId].push_back(enemyId);

			std::pair<std::map<int, std::pair<int, time_t> >::iterator, bool> result = s_guildEnemiesAToBKillInfo[guildId].insert(std::make_pair(enemyId, std::make_pair(killCount, static_cast<time_t>(lastUpdateTime))));
			if (!result.second)
			{
				// multiple A->B guild enemy entries, so consolidate into one entry
				result.first->second.first += killCount;
				result.first->second.second = std::max(result.first->second.second, static_cast<time_t>(lastUpdateTime));
			}

			s_guildEnemiesBToA[enemyId].push_back(guildId);
		}
	}

	// destroy pvp update observers
	{
		for (std::vector<PvpUpdateObserver *>::iterator i = pvpUpdateObservers.begin(); i != pvpUpdateObservers.end(); ++i)
			delete (*i);
	}
}

// ----------------------------------------------------------------------

int GuildInterface::getGuildId(NetworkId const &memberId)
{
	return ServerUniverse::getInstance().getMasterGuildObject()->getGuildId(memberId);
}

// ----------------------------------------------------------------------

int GuildInterface::getGuildIdForSponsored(NetworkId const &memberId)
{
	return ServerUniverse::getInstance().getMasterGuildObject()->getGuildIdForSponsored(memberId);
}

// ----------------------------------------------------------------------

int GuildInterface::getGuildIdForGuildLeader(NetworkId const &leaderId)
{
	return ServerUniverse::getInstance().getMasterGuildObject()->getGuildIdForGuildLeader(leaderId);
}

// ----------------------------------------------------------------------

void GuildInterface::getAllGuildIds(std::vector<int> &results)
{
	ServerUniverse::getInstance().getMasterGuildObject()->getAllGuildIds(results);
}

// ----------------------------------------------------------------------


int GuildInterface::findGuild(std::string const &name)
{
	return ServerUniverse::getInstance().getMasterGuildObject()->findGuild(name);
}

// ----------------------------------------------------------------------

bool GuildInterface::guildExists(int guildId)
{
	return ServerUniverse::getInstance().getMasterGuildObject()->guildExists(guildId);
}

// ----------------------------------------------------------------------

std::pair<int, time_t> const *GuildInterface::hasDeclaredWarAgainst(int actorGuildId, int targetGuildId)
{
	if (actorGuildId && targetGuildId)
	{
		std::map<int, std::map<int, std::pair<int, time_t> > >::const_iterator const iterFindA = s_guildEnemiesAToBKillInfo.find(actorGuildId);
		if (iterFindA != s_guildEnemiesAToBKillInfo.end())
		{
			std::map<int, std::pair<int, time_t> >::const_iterator const iterFindB = iterFindA->second.find(targetGuildId);
			if (iterFindB != iterFindA->second.end())
				return &(iterFindB->second);
		}
	}

	return nullptr;
}

// ----------------------------------------------------------------------

GuildInfo const * GuildInterface::getGuildInfo(int guildId)
{
	return ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
}

// ----------------------------------------------------------------------

std::string const &GuildInterface::getGuildName(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return gi->m_name;

	return s_empty;
}

// ----------------------------------------------------------------------

std::string const &GuildInterface::getGuildAbbrev(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return gi->m_abbrev;
	
	return s_empty;
}

// ----------------------------------------------------------------------

NetworkId const &GuildInterface::getGuildLeaderId(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return gi->m_leaderId;

	return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

int GuildInterface::getGuildElectionPreviousEndTime(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return gi->m_guildElectionPreviousEndTime;

	return 0;
}

// ----------------------------------------------------------------------

int GuildInterface::getGuildElectionNextEndTime(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return gi->m_guildElectionNextEndTime;

	return 0;
}

// ----------------------------------------------------------------------

uint32 GuildInterface::getGuildCurrentFaction(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return getGuildCurrentFaction(*gi);

	return 0;
}

// ----------------------------------------------------------------------

uint32 GuildInterface::getGuildCurrentFaction(GuildInfo const & gi)
{
	if (gi.m_timeLeftGuildFaction <= 0)
		return gi.m_guildFaction;

	return 0;
}

// ----------------------------------------------------------------------

uint32 GuildInterface::getGuildPreviousFaction(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return getGuildPreviousFaction(*gi);

	return 0;
}

// ----------------------------------------------------------------------

uint32 GuildInterface::getGuildPreviousFaction(GuildInfo const & gi)
{
	if (gi.m_timeLeftGuildFaction > 0)
		return gi.m_guildFaction;

	return 0;
}

// ----------------------------------------------------------------------

int GuildInterface::getTimeLeftGuildPreviousFaction(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return getTimeLeftGuildPreviousFaction(*gi);

	return 0;
}

// ----------------------------------------------------------------------

int GuildInterface::getTimeLeftGuildPreviousFaction(GuildInfo const & gi)
{
	if (gi.m_timeLeftGuildFaction > 0)
		return gi.m_timeLeftGuildFaction;

	return 0;
}

// ----------------------------------------------------------------------

std::string const & GuildInterface::getGuildCurrentGcwDefenderRegion(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return getGuildCurrentGcwDefenderRegion(*gi);

	return s_empty;
}

// ----------------------------------------------------------------------

std::string const &GuildInterface::getGuildCurrentGcwDefenderRegion(GuildInfo const & gi)
{
	if (gi.m_timeLeftGuildGcwDefenderRegion <= 0)
		return gi.m_guildGcwDefenderRegion;

	return s_empty;
}

// ----------------------------------------------------------------------

int GuildInterface::getTimeJoinedGuildCurrentGcwDefenderRegion(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return getTimeJoinedGuildCurrentGcwDefenderRegion(*gi);

	return 0;
}

// ----------------------------------------------------------------------

int GuildInterface::getTimeJoinedGuildCurrentGcwDefenderRegion(GuildInfo const & gi)
{
	if (gi.m_timeLeftGuildGcwDefenderRegion <= 0)
		return gi.m_timeJoinedGuildGcwDefenderRegion;

	return 0;
}

// ----------------------------------------------------------------------

std::string const & GuildInterface::getGuildPreviousGcwDefenderRegion(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return getGuildPreviousGcwDefenderRegion(*gi);

	return s_empty;
}

// ----------------------------------------------------------------------

std::string const & GuildInterface::getGuildPreviousGcwDefenderRegion(GuildInfo const & gi)
{
	if (gi.m_timeLeftGuildGcwDefenderRegion > 0)
		return gi.m_guildGcwDefenderRegion;

	return s_empty;
}

// ----------------------------------------------------------------------

int GuildInterface::getTimeLeftGuildPreviousGcwDefenderRegion(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return getTimeLeftGuildPreviousGcwDefenderRegion(*gi);

	return 0;
}

// ----------------------------------------------------------------------

int GuildInterface::getTimeLeftGuildPreviousGcwDefenderRegion(GuildInfo const & gi)
{
	if (gi.m_timeLeftGuildGcwDefenderRegion > 0)
		return gi.m_timeLeftGuildGcwDefenderRegion;

	return 0;
}

// ----------------------------------------------------------------------

std::map<std::pair<std::string, int>, uint32> const & GuildInterface::getGcwRegionDefenderGuilds()
{
	return ServerUniverse::getInstance().getMasterGuildObject()->getGcwRegionDefenderGuilds();
}

// ----------------------------------------------------------------------

std::map<std::string, std::pair<int, int> > const & GuildInterface::getGcwRegionDefenderGuildsCount()
{
	return ServerUniverse::getInstance().getMasterGuildObject()->getGcwRegionDefenderGuildsCount();
}

// ----------------------------------------------------------------------

int GuildInterface::getGcwRegionDefenderGuildsVersion()
{
	return ServerUniverse::getInstance().getMasterGuildObject()->getGcwRegionDefenderGuildsVersion();
}

// ----------------------------------------------------------------------

void GuildInterface::getGuildMemberIds(int guildId, std::vector<NetworkId> &results)
{
	ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberIds(guildId, results);
}

// ----------------------------------------------------------------------

void GuildInterface::getGuildMemberIdsWithPermissions(int guildId, int permissions, std::vector<NetworkId> &results)
{
	ServerUniverse::getInstance().getMasterGuildObject()->getMemberIdsWithPermissions(guildId, permissions, results);
}

// ----------------------------------------------------------------------

int GuildInterface::getGuildCountMembersOnly(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return gi->getCountMembersOnly();

	return 0;
}

// ----------------------------------------------------------------------

int GuildInterface::getGuildCountSponsoredOnly(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return gi->getCountSponsoredOnly();

	return 0;
}

// ----------------------------------------------------------------------

int GuildInterface::getGuildCountMembersAndSponsored(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return gi->getCountMembersAndSponsored();

	return 0;
}

// ----------------------------------------------------------------------

int GuildInterface::getGuildCountMembersGuildWarPvPEnabled(int guildId)
{
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
		return gi->getCountMembersGuildWarPvPEnabled();

	return 0;
}

// ----------------------------------------------------------------------

GuildMemberInfo const * GuildInterface::getGuildMemberInfo(int guildId, NetworkId const &memberId)
{
	return ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberInfo(guildId, memberId);
}

// ----------------------------------------------------------------------

bool GuildInterface::getGuildMemberGuildWarEnabled(int guildId, NetworkId const &memberId)
{
	if (guildId == 0)
		return false;

	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (!gi)
		return false;

	GuildMemberInfo const * const gmi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberInfo(guildId, memberId);
	if (!gmi)
		return false;

	// if guild is using guild war inclusion list, only members
	// with GuildInterface::WarInclusion is enabled for guild war
	if (gi->m_guildWarInclusionCount)
		return (gmi->m_permissions & GuildInterface::WarInclusion);

	// if guild is not using guild war inclusion list, only members
	// with GuildInterface::WarExclusion is disabled for guild war
	return (!(gmi->m_permissions & GuildInterface::WarExclusion));
}

// ----------------------------------------------------------------------

std::vector<int> const &GuildInterface::getGuildEnemies(int guildId)
{
	static std::vector<int> empty;

	std::map<int, std::vector<int> >::const_iterator iterFind =  s_guildEnemiesAToB.find(guildId);
	if (iterFind != s_guildEnemiesAToB.end())
		return iterFind->second;

	return empty;
}

// ----------------------------------------------------------------------

std::map<int, std::pair<int, time_t> > const &GuildInterface::getGuildEnemiesKillInfo(int guildId)
{
	static std::map<int, std::pair<int, time_t> > empty;

	std::map<int, std::map<int, std::pair<int, time_t> > >::const_iterator iterFind = s_guildEnemiesAToBKillInfo.find(guildId);
	if (iterFind != s_guildEnemiesAToBKillInfo.end())
		return iterFind->second;

	return empty;
}

// ----------------------------------------------------------------------

std::pair<int, time_t> const &GuildInterface::getGuildEnemyKillInfo(int guildIdA, int guildIdB)
{
	static std::pair<int, time_t> empty = std::make_pair(0, 0);

	std::map<int, std::map<int, std::pair<int, time_t> > >::const_iterator iterFindA = s_guildEnemiesAToBKillInfo.find(guildIdA);
	if (iterFindA != s_guildEnemiesAToBKillInfo.end())
	{
		std::map<int, std::pair<int, time_t> >::const_iterator iterFindB = iterFindA->second.find(guildIdB);
		if (iterFindB != iterFindA->second.end())
			return iterFindB->second;
	}

	return empty;
}

// ----------------------------------------------------------------------

std::vector<int> const &GuildInterface::getGuildsAtWarWith(int guildId)
{
	static std::vector<int> empty;

	std::map<int, std::vector<int> >::const_iterator iterFind =  s_guildEnemiesBToA.find(guildId);
	if (iterFind != s_guildEnemiesBToA.end())
		return iterFind->second;

	return empty;
}

// ----------------------------------------------------------------------

ScriptParams const *GuildInterface::getMasterGuildWarTableDictionary()
{
	if (s_activeGuildWarsNeedRebuild)
	{
		// build list of guild wars with at least 1 kill, ordered by most recent update time, with the guild with more kills appearing first
		std::multimap<time_t, std::pair<std::pair<int, int>, std::pair<int, int> > > guildMutuallyAtWarWithKillSummary;

		{
			for (std::map<int, std::map<int, std::pair<int, time_t> > >::const_iterator iter1 = s_guildEnemiesAToBKillInfo.begin(); iter1 != s_guildEnemiesAToBKillInfo.end(); ++iter1)
			{
				for (std::map<int, std::pair<int, time_t> >::const_iterator iter2 = iter1->second.upper_bound(iter1->first); iter2 != iter1->second.end(); ++iter2)
				{
					// we know A has declared war on B, so see if B has declared on A to make it a guild war
					std::pair<int, time_t> const * killInfoBtoA = hasDeclaredWarAgainst(iter2->first, iter1->first);
					if (killInfoBtoA)
					{
						// we have a guild war, store it if there is at least one kill
						if ((iter2->second.first <= 0) && (killInfoBtoA->first <= 0))
							continue;

						// guild with more kills appears first, and if guilds have the same
						// number of kills, guild with more recent kill appears first
						if (iter2->second.first > killInfoBtoA->first)
							guildMutuallyAtWarWithKillSummary.insert(std::make_pair(-std::max(iter2->second.second, killInfoBtoA->second), std::make_pair(std::make_pair(iter1->first, iter2->second.first), std::make_pair(iter2->first, killInfoBtoA->first))));
						else if (killInfoBtoA->first > iter2->second.first)
							guildMutuallyAtWarWithKillSummary.insert(std::make_pair(-std::max(iter2->second.second, killInfoBtoA->second), std::make_pair(std::make_pair(iter2->first, killInfoBtoA->first), std::make_pair(iter1->first, iter2->second.first))));
						else if (iter2->second.second > killInfoBtoA->second)
							guildMutuallyAtWarWithKillSummary.insert(std::make_pair(-std::max(iter2->second.second, killInfoBtoA->second), std::make_pair(std::make_pair(iter1->first, iter2->second.first), std::make_pair(iter2->first, killInfoBtoA->first))));
						else
							guildMutuallyAtWarWithKillSummary.insert(std::make_pair(-std::max(iter2->second.second, killInfoBtoA->second), std::make_pair(std::make_pair(iter2->first, killInfoBtoA->first), std::make_pair(iter1->first, iter2->second.first))));
					}
				}
			}
		}

		delete s_activeGuildWars;
		s_activeGuildWars = nullptr;

		// build the table
		if (!guildMutuallyAtWarWithKillSummary.empty())
		{
			// column header
			static const char * s_scriptParamsColumnHeadersText[9] = 
			{
				"Name - Guild A",
				"Abbrev - Guild A",
				"Kill Count - Guild A",
				"Guild War PvP Enabled Count - Guild A",
				"Name - Guild B",
				"Abbrev - Guild B",
				"Kill Count - Guild B",
				"Guild War PvP Enabled Count - Guild B",
				"Last Updated"
			};
			static std::vector<const char *> s_scriptParamsColumnHeaders(s_scriptParamsColumnHeadersText, s_scriptParamsColumnHeadersText + (sizeof(s_scriptParamsColumnHeadersText) / sizeof(const char *)));

			// column type
			static const char * s_scriptParamsColumnTypeText[9] = 
			{
				"text",
				"text",
				"integer",
				"integer",
				"text",
				"text",
				"integer",
				"integer",
				"text"
			};
			static std::vector<const char *> s_scriptParamsColumnType(s_scriptParamsColumnTypeText, s_scriptParamsColumnTypeText + (sizeof(s_scriptParamsColumnTypeText) / sizeof(const char *)));

			s_activeGuildWars = new ScriptParams();
			std::vector<const char *> * scriptParamsGuildAName = new std::vector<const char *>;
			std::vector<const char *> * scriptParamsGuildAAbbrev = new std::vector<const char *>;
			std::vector<const char *> * scriptParamsGuildAKillCount = new std::vector<const char *>;
			std::vector<const char *> * scriptParamsGuildAPvpEnabledCount = new std::vector<const char *>;
			std::vector<const char *> * scriptParamsGuildBName = new std::vector<const char *>;
			std::vector<const char *> * scriptParamsGuildBAbbrev = new std::vector<const char *>;
			std::vector<const char *> * scriptParamsGuildBKillCount = new std::vector<const char *>;
			std::vector<const char *> * scriptParamsGuildBPvpEnabledCount = new std::vector<const char *>;
			std::vector<const char *> * scriptParamsLastUpdated = new std::vector<const char *>;

			size_t const size = guildMutuallyAtWarWithKillSummary.size();
			scriptParamsGuildAName->reserve(size);
			scriptParamsGuildAAbbrev->reserve(size);
			scriptParamsGuildAKillCount->reserve(size);
			scriptParamsGuildAPvpEnabledCount->reserve(size);
			scriptParamsGuildBName->reserve(size);
			scriptParamsGuildBAbbrev->reserve(size);
			scriptParamsGuildBKillCount->reserve(size);
			scriptParamsGuildBPvpEnabledCount->reserve(size);
			scriptParamsLastUpdated->reserve(size);

			char buffer[128];
			for (std::multimap<time_t, std::pair<std::pair<int, int>, std::pair<int, int> > >::const_iterator iter = guildMutuallyAtWarWithKillSummary.begin(); iter != guildMutuallyAtWarWithKillSummary.end(); ++iter)
			{
				GuildInfo const * gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(iter->second.first.first);
				if (!gi)
					continue;

				GuildInfo const & guildA = *gi;

				gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(iter->second.second.first);
				if (!gi)
					continue;

				GuildInfo const & guildB = *gi;

				scriptParamsGuildAName->push_back(makeCopyOfString(guildA.m_name.c_str()));
				scriptParamsGuildAAbbrev->push_back(makeCopyOfString(guildA.m_abbrev.c_str()));

				snprintf(buffer, sizeof(buffer), "%d", iter->second.first.second);
				scriptParamsGuildAKillCount->push_back(makeCopyOfString(buffer));

				snprintf(buffer, sizeof(buffer), "%d", guildA.getCountMembersGuildWarPvPEnabled());
				scriptParamsGuildAPvpEnabledCount->push_back(makeCopyOfString(buffer));

				scriptParamsGuildBName->push_back(makeCopyOfString(guildB.m_name.c_str()));
				scriptParamsGuildBAbbrev->push_back(makeCopyOfString(guildB.m_abbrev.c_str()));

				snprintf(buffer, sizeof(buffer), "%d", iter->second.second.second);
				scriptParamsGuildBKillCount->push_back(makeCopyOfString(buffer));

				snprintf(buffer, sizeof(buffer), "%d", guildB.getCountMembersGuildWarPvPEnabled());
				scriptParamsGuildBPvpEnabledCount->push_back(makeCopyOfString(buffer));

				scriptParamsLastUpdated->push_back(makeCopyOfString(CalendarTime::convertEpochToTimeStringLocal_YYYYMMDDHHMMSS(-iter->first).c_str()));
			}

			s_activeGuildWars->addParam(s_scriptParamsColumnHeaders, "column", false);
			s_activeGuildWars->addParam(s_scriptParamsColumnType, "columnType", false);
			s_activeGuildWars->addParam(*scriptParamsGuildAName, "column0", true);
			s_activeGuildWars->addParam(*scriptParamsGuildAAbbrev, "column1", true);
			s_activeGuildWars->addParam(*scriptParamsGuildAKillCount, "column2", true);
			s_activeGuildWars->addParam(*scriptParamsGuildAPvpEnabledCount, "column3", true);
			s_activeGuildWars->addParam(*scriptParamsGuildBName, "column4", true);
			s_activeGuildWars->addParam(*scriptParamsGuildBAbbrev, "column5", true);
			s_activeGuildWars->addParam(*scriptParamsGuildBKillCount, "column6", true);
			s_activeGuildWars->addParam(*scriptParamsGuildBPvpEnabledCount, "column7", true);
			s_activeGuildWars->addParam(*scriptParamsLastUpdated, "column8", true);
		}

		s_activeGuildWarsNeedRebuild = false;
	}

	return s_activeGuildWars;
}

// ----------------------------------------------------------------------

void GuildInterface::updateInactiveGuildWarTrackingInfo(GuildObject &masterGuildObject, int guildAId, int guildAKillCount, int guildBId, int guildBKillCount)
{
	if (!masterGuildObject.isAuthoritative())
		return;

	if ((guildAKillCount <= 0) && (guildBKillCount <= 0))
		return;

	GuildInfo const * giA = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildAId);
	if (!giA)
		return;

	if (giA->m_name.empty() || giA->m_abbrev.empty())
		return;

	GuildInfo const * giB = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildBId);
	if (!giB)
		return;

	if (giB->m_name.empty() || giB->m_abbrev.empty())
		return;

	// the information is stored as objvars on the master guild object as a circular
	// list of the 100 most recently ended guild wars with at least 1 kill
	int nextIndex = 0;
	if (!masterGuildObject.getObjVars().getItem(s_objvarInactiveGuildWarsMostRecentIndex, nextIndex))
		nextIndex = 0;

	++nextIndex;
	if (nextIndex > s_maxInactiveGuildWarsTracked)
		nextIndex = 1;

	// guild with higher kill count appears first
	if (guildAKillCount > guildBKillCount)
		IGNORE_RETURN(masterGuildObject.setObjVarItem(FormattedString<32>().sprintf("%s.%d", s_objvarInactiveGuildWarsPrefix.c_str(), nextIndex), FormattedString<512>().sprintf("%s~%s~%d~%s~%s~%d~%ld", giA->m_name.c_str(), giA->m_abbrev.c_str(), guildAKillCount, giB->m_name.c_str(), giB->m_abbrev.c_str(), guildBKillCount, ::time(nullptr))));
	else
		IGNORE_RETURN(masterGuildObject.setObjVarItem(FormattedString<32>().sprintf("%s.%d", s_objvarInactiveGuildWarsPrefix.c_str(), nextIndex), FormattedString<512>().sprintf("%s~%s~%d~%s~%s~%d~%ld", giB->m_name.c_str(), giB->m_abbrev.c_str(), guildBKillCount, giA->m_name.c_str(), giA->m_abbrev.c_str(), guildAKillCount, ::time(nullptr))));

	IGNORE_RETURN(masterGuildObject.setObjVarItem(s_objvarInactiveGuildWarsMostRecentIndex, nextIndex));
}

// ----------------------------------------------------------------------

ScriptParams const *GuildInterface::getInactiveGuildWarTableDictionary()
{
	// update the table if necessary
	GuildObject const * const masterGuildObject = ServerUniverse::getInstance().getMasterGuildObject();
	if (!masterGuildObject)
		return s_inactiveGuildWars;

	DynamicVariableList const & objVars = masterGuildObject->getObjVars();
	int mostRecentIndex = 0;
	if (!objVars.getItem(s_objvarInactiveGuildWarsMostRecentIndex, mostRecentIndex))
		return s_inactiveGuildWars;

	if ((mostRecentIndex <= 0) || (mostRecentIndex > s_maxInactiveGuildWarsTracked))
		return s_inactiveGuildWars;

	// no new update since we last updated the table
	if (mostRecentIndex == s_inactiveGuildWarsMostRecentUpdateIndex)
		return s_inactiveGuildWars;

	// new update since we last updated the table, recreate the table
	s_inactiveGuildWarsMostRecentUpdateIndex = mostRecentIndex;

	std::vector<const Unicode::String *> * scriptParamsGuildAName = new std::vector<const Unicode::String *>;
	std::vector<const Unicode::String *> * scriptParamsGuildAAbbrev = new std::vector<const Unicode::String *>;
	std::vector<const Unicode::String *> * scriptParamsGuildAKillCount = new std::vector<const Unicode::String *>;
	std::vector<const Unicode::String *> * scriptParamsGuildBName = new std::vector<const Unicode::String *>;
	std::vector<const Unicode::String *> * scriptParamsGuildBAbbrev = new std::vector<const Unicode::String *>;
	std::vector<const Unicode::String *> * scriptParamsGuildBKillCount = new std::vector<const Unicode::String *>;
	std::vector<const char *> * scriptParamsLastUpdated = new std::vector<const char *>;

	scriptParamsGuildAName->reserve(s_maxInactiveGuildWarsTracked);
	scriptParamsGuildAAbbrev->reserve(s_maxInactiveGuildWarsTracked);
	scriptParamsGuildAKillCount->reserve(s_maxInactiveGuildWarsTracked);
	scriptParamsGuildBName->reserve(s_maxInactiveGuildWarsTracked);
	scriptParamsGuildBAbbrev->reserve(s_maxInactiveGuildWarsTracked);
	scriptParamsGuildBKillCount->reserve(s_maxInactiveGuildWarsTracked);
	scriptParamsLastUpdated->reserve(s_maxInactiveGuildWarsTracked);

	// the information is stored as objvars on the master guild object as a circular
	// list of the 100 most recently ended guild wars with at least 1 kill, so get
	// each one, ordered by the most recent one first
	static Unicode::String const delimiters(Unicode::narrowToWide("~"));
	Unicode::UnicodeStringVector tokens;
	Unicode::String guildWarData;
	int currentIndex = mostRecentIndex;
	while (true)
	{
		if (!objVars.getItem(FormattedString<32>().sprintf("%s.%d", s_objvarInactiveGuildWarsPrefix.c_str(), currentIndex), guildWarData))
			break;

		if (!Unicode::tokenize(guildWarData, tokens, &delimiters, nullptr) || (tokens.size() != 7))
			break;

		scriptParamsGuildAName->push_back(new Unicode::String(tokens[0]));
		scriptParamsGuildAAbbrev->push_back(new Unicode::String(tokens[1]));
		scriptParamsGuildAKillCount->push_back(new Unicode::String(tokens[2]));

		scriptParamsGuildBName->push_back(new Unicode::String(tokens[3]));
		scriptParamsGuildBAbbrev->push_back(new Unicode::String(tokens[4]));
		scriptParamsGuildBKillCount->push_back(new Unicode::String(tokens[5]));

		scriptParamsLastUpdated->push_back(makeCopyOfString(CalendarTime::convertEpochToTimeStringLocal_YYYYMMDDHHMMSS(static_cast<time_t>(::atol(Unicode::wideToNarrow(tokens[6]).c_str()))).c_str()));

		--currentIndex;
		if (currentIndex <= 0)
			currentIndex = s_maxInactiveGuildWarsTracked;

		if (currentIndex == mostRecentIndex)
			break;
	}

	delete s_inactiveGuildWars;
	s_inactiveGuildWars = nullptr;

	if (!scriptParamsGuildAName->empty())
	{
		// column header
		static const char * s_scriptParamsColumnHeadersText[7] = 
		{
			"Name - Guild A",
			"Abbrev - Guild A",
			"Kill Count - Guild A",
			"Name - Guild B",
			"Abbrev - Guild B",
			"Kill Count - Guild B",
			"Time Ended"
		};
		static std::vector<const char *> s_scriptParamsColumnHeaders(s_scriptParamsColumnHeadersText, s_scriptParamsColumnHeadersText + (sizeof(s_scriptParamsColumnHeadersText) / sizeof(const char *)));

		// column type
		static const char * s_scriptParamsColumnTypeText[7] = 
		{
			"text",
			"text",
			"integer",
			"text",
			"text",
			"integer",
			"text"
		};
		static std::vector<const char *> s_scriptParamsColumnType(s_scriptParamsColumnTypeText, s_scriptParamsColumnTypeText + (sizeof(s_scriptParamsColumnTypeText) / sizeof(const char *)));

		s_inactiveGuildWars = new ScriptParams();
		s_inactiveGuildWars->addParam(s_scriptParamsColumnHeaders, "column", false);
		s_inactiveGuildWars->addParam(s_scriptParamsColumnType, "columnType", false);
		s_inactiveGuildWars->addParam(*scriptParamsGuildAName, "column0", true);
		s_inactiveGuildWars->addParam(*scriptParamsGuildAAbbrev, "column1", true);
		s_inactiveGuildWars->addParam(*scriptParamsGuildAKillCount, "column2", true);
		s_inactiveGuildWars->addParam(*scriptParamsGuildBName, "column3", true);
		s_inactiveGuildWars->addParam(*scriptParamsGuildBAbbrev, "column4", true);
		s_inactiveGuildWars->addParam(*scriptParamsGuildBKillCount, "column5", true);
		s_inactiveGuildWars->addParam(*scriptParamsLastUpdated, "column6", true);
	}
	else
	{
		delete scriptParamsGuildAName;
		delete scriptParamsGuildAAbbrev;
		delete scriptParamsGuildAKillCount;
		delete scriptParamsGuildBName;
		delete scriptParamsGuildBAbbrev;
		delete scriptParamsGuildBKillCount;
		delete scriptParamsLastUpdated;
	}

	return s_inactiveGuildWars;
}

// ----------------------------------------------------------------------

bool GuildInterface::isValidGuildName(std::string const &guildName)
{
	if (guildName.empty() || guildName.length() > 25)
		return false;
	// TODO RAD: add obscenity checking here
	return true;
}

// ----------------------------------------------------------------------

bool GuildInterface::isValidGuildAbbrev(std::string const &guildAbbrev)
{
	if (guildAbbrev.length() > 5)
		return false;
	return isValidGuildName(guildAbbrev);
}

// ----------------------------------------------------------------------

int GuildInterface::createGuild(std::string const &guildName, std::string const &guildAbbrev)
{
	if (!isValidGuildName(guildName) || !isValidGuildAbbrev(guildAbbrev))
		return 0;
	int guildId = ServerUniverse::getInstance().getMasterGuildObject()->createGuild(guildName, guildAbbrev);
	Chat::requestGetChannel(getChatRoomPath(guildId), false, true, s_guildChannelLimit);
	return guildId;
}

// ----------------------------------------------------------------------
void GuildInterface::disbandGuild(int guildId)
{
	Chat::requestDestroyChannel(getChatRoomPath(guildId));
	ServerUniverse::getInstance().getMasterGuildObject()->disbandGuild(guildId);
}

// ----------------------------------------------------------------------

void GuildInterface::removeGuildMember(int guildId, NetworkId const &memberId)
{
	ServerUniverse::getInstance().getMasterGuildObject()->removeGuildMember(guildId, memberId);
}

// ----------------------------------------------------------------------

void GuildInterface::addGuildCreatorMember(int guildId, NetworkId const &memberId)
{
	ServerUniverse::getInstance().getMasterGuildObject()->addGuildCreatorMember(guildId, memberId, std::string(), std::string(), 0);
}

// ----------------------------------------------------------------------

void GuildInterface::addGuildSponsorMember(int guildId, NetworkId const &memberId)
{
	ServerUniverse::getInstance().getMasterGuildObject()->addGuildSponsorMember(guildId, memberId, std::string(), std::string(), 0);
}

// ----------------------------------------------------------------------

void GuildInterface::setGuildMemberPermission(int guildId, NetworkId const &memberId, int permissions)
{
	ServerUniverse::getInstance().getMasterGuildObject()->setGuildMemberPermission(guildId, memberId, permissions);
}

// ----------------------------------------------------------------------

void GuildInterface::setGuildMemberTitle(int guildId, NetworkId const &memberId, std::string const &title)
{
	ServerUniverse::getInstance().getMasterGuildObject()->setGuildMemberTitle(guildId, memberId, title);
}

// ----------------------------------------------------------------------

void GuildInterface::setGuildMemberAllegiance(int guildId, NetworkId const &memberId, NetworkId const &allegiance)
{
	ServerUniverse::getInstance().getMasterGuildObject()->setGuildMemberAllegiance(guildId, memberId, allegiance);
}

// ----------------------------------------------------------------------

void GuildInterface::setGuildMemberPermissionAndAllegiance(int guildId, NetworkId const &memberId, int permissions, NetworkId const &allegiance)
{
	ServerUniverse::getInstance().getMasterGuildObject()->setGuildMemberPermissionAndAllegiance(guildId, memberId, permissions, allegiance);
}

// ----------------------------------------------------------------------

void GuildInterface::setGuildMemberProfessionInfo(int guildId, NetworkId const &memberId, std::string const &memberProfessionSkillTemplate, int memberLevel)
{
	ServerUniverse::getInstance().getMasterGuildObject()->setGuildMemberProfessionInfo(guildId, memberId, memberProfessionSkillTemplate, memberLevel);
}

// ----------------------------------------------------------------------

void GuildInterface::addGuildMemberRank(int guildId, NetworkId const &memberId, std::string const &rankName)
{
	ServerUniverse::getInstance().getMasterGuildObject()->addGuildMemberRank(guildId, memberId, rankName);
}

// ----------------------------------------------------------------------

void GuildInterface::removeGuildMemberRank(int guildId, NetworkId const &memberId, std::string const &rankName)
{
	ServerUniverse::getInstance().getMasterGuildObject()->removeGuildMemberRank(guildId, memberId, rankName);
}

// ----------------------------------------------------------------------

bool GuildInterface::hasGuildMemberRank(int guildId, NetworkId const &memberId, std::string const &rankName)
{
	return ServerUniverse::getInstance().getMasterGuildObject()->hasGuildMemberRank(guildId, memberId, rankName);
}

// ----------------------------------------------------------------------

void GuildInterface::getGuildMemberRank(int guildId, NetworkId const &memberId, std::vector<std::string> &ranks)
{
	ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberRank(guildId, memberId, ranks);
}

// ----------------------------------------------------------------------

void GuildInterface::setGuildLeader(int guildId, NetworkId const &leaderId)
{
	ServerUniverse::getInstance().getMasterGuildObject()->setGuildLeader(guildId, leaderId);
}

// ----------------------------------------------------------------------

void GuildInterface::setGuildElectionEndTime(int guildId, int electionPreviousEndTime, int electionNextEndTime)
{
	ServerUniverse::getInstance().getMasterGuildObject()->setGuildElectionEndTime(guildId, electionPreviousEndTime, electionNextEndTime);
}

// ----------------------------------------------------------------------

void GuildInterface::setGuildFaction(int guildId, uint32 guildFaction)
{
	ServerUniverse::getInstance().getMasterGuildObject()->setGuildFaction(guildId, guildFaction);
}

// ----------------------------------------------------------------------

void GuildInterface::setGuildGcwDefenderRegion(int guildId, std::string const &guildGcwDefenderRegion)
{
	ServerUniverse::getInstance().getMasterGuildObject()->setGuildGcwDefenderRegion(guildId, guildGcwDefenderRegion);
}

// ----------------------------------------------------------------------

void GuildInterface::setGuildEnemy(int guildId, int enemyId)
{
	ServerUniverse::getInstance().getMasterGuildObject()->setGuildEnemy(guildId, enemyId);
}

// ----------------------------------------------------------------------

void GuildInterface::removeGuildEnemy(int guildId, int enemyId)
{
	ServerUniverse::getInstance().getMasterGuildObject()->removeGuildEnemy(guildId, enemyId);
}

// ----------------------------------------------------------------------

void GuildInterface::updateGuildWarKillTracking(CreatureObject const &killer, CreatureObject const &victim)
{
	if (killer.getGuildId()
	 && victim.getGuildId()
	 && killer.getGuildWarEnabled()
	 && victim.getGuildWarEnabled()
	 && hasDeclaredWarAgainst(killer.getGuildId(), victim.getGuildId())
	 && hasDeclaredWarAgainst(victim.getGuildId(), killer.getGuildId()))
	{
		ServerUniverse::getInstance().getMasterGuildObject()->modifyGuildWarKillTracking(killer.getGuildId(), victim.getGuildId(), 1, static_cast<int>(::time(nullptr)));
	}
}

// ----------------------------------------------------------------------

void GuildInterface::setGuildName(int guildId, std::string const &newName)
{
	ServerUniverse::getInstance().getMasterGuildObject()->setGuildName(guildId, newName);
}

// ----------------------------------------------------------------------

void GuildInterface::setGuildAbbrev(int guildId, std::string const &newAbbrev)
{
	ServerUniverse::getInstance().getMasterGuildObject()->setGuildAbbrev(guildId, newAbbrev);
}

// ----------------------------------------------------------------------

void GuildInterface::onGuildIdSet(CreatureObject const &who)
{
	int const guildId = who.getGuildId();
	NetworkId const & whoId = who.getNetworkId();
	std::string const objectName = Unicode::wideToNarrow(who.getAssignedObjectName());

	GuildMemberInfo const * const memberInfo = ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberInfo(guildId, whoId);
	if (memberInfo)
	{
		bool const fixName = (!objectName.empty() && (memberInfo->m_name != objectName));
		bool const fixGuildLeaderPermissions = ((getGuildLeaderId(guildId) == whoId) && ((memberInfo->m_permissions & GuildInterface::AllAdministrativePermissions) != GuildInterface::AllAdministrativePermissions));

		WARNING(fixName, ("Fixing guild (%d) member (%s) name (%s) which doesn't match object name (%s)", guildId, whoId.getValueString().c_str(), memberInfo->m_name.c_str(), objectName.c_str()));
		WARNING(fixGuildLeaderPermissions, ("Fixing guild (%d) leader (%s) permission from (%d) to (%d)", guildId, whoId.getValueString().c_str(), memberInfo->m_permissions, (memberInfo->m_permissions | GuildInterface::AllAdministrativePermissions)));

		if (fixName || fixGuildLeaderPermissions)
		{
			ServerUniverse::getInstance().getMasterGuildObject()->setGuildMemberNameAndPermision(
				guildId,
				whoId,
				(fixName ? objectName : memberInfo->m_name),
				(fixGuildLeaderPermissions ? (memberInfo->m_permissions | GuildInterface::AllAdministrativePermissions) : memberInfo->m_permissions));
		}
	}
	else if ((getGuildLeaderId(guildId) == whoId) && !objectName.empty())
	{
		// handle the case where a guild leader somehow
		// is no longer a member of the guild
		WARNING(true, ("Guild (%d) leader (%s, %s) is currently not a guild member, so making the guild leader a guild member", guildId, whoId.getValueString().c_str(), objectName.c_str()));

		addGuildCreatorMember(guildId, whoId);
	}

	enterGuildChatRoom(who);
	enterGuildVoiceChatRoom(who);
}

// ----------------------------------------------------------------------

void GuildInterface::onAboutToClearGuildId(CreatureObject const &who)
{
	leaveGuildChatRoom(who);
}

// ----------------------------------------------------------------------

void GuildInterface::verifyGuildMemberName(NetworkId const &memberId, std::string const &memberName)
{
	int guildId = getGuildId(memberId);
	if (guildId <= 0)
		guildId = getGuildIdForSponsored(memberId);

	if (guildId <= 0)
		return;

	GuildMemberInfo const * const memberInfo = getGuildMemberInfo(guildId, memberId);
	if (!memberInfo)	
		return;

	if (memberInfo->m_name == memberName)
		return;

	ServerUniverse::getInstance().getMasterGuildObject()->setGuildMemberNameAndPermision(
		guildId,
		memberId,
		memberName,
		memberInfo->m_permissions);
}

// ----------------------------------------------------------------------

void GuildInterface::enterGuildChatRoom(CreatureObject const &who)
{
	int guildId = who.getGuildId();
	if (guildId)
	{
		Chat::createRoom("System", false, getChatRoomPath(guildId), getChatRoomTitle(guildId));
		std::string firstName;
		size_t pos = 0;
		IGNORE_RETURN(Unicode::getFirstToken(Unicode::wideToNarrow(who.getObjectName()), 0, pos, firstName));
		Chat::invite(firstName, getChatRoomPath(guildId));
		Chat::enterRoom(firstName, getChatRoomPath(guildId), false, false);
	}
}

// ----------------------------------------------------------------------

void GuildInterface::reenterGuildChatRoom(CreatureObject const &who)
{
	int guildId = who.getGuildId();
	if (guildId)
	{
		std::string firstName;
		size_t pos = 0;
		IGNORE_RETURN(Unicode::getFirstToken(Unicode::wideToNarrow(who.getObjectName()), 0, pos, firstName));

		std::string const guildChatRoomPath = getChatRoomPath(guildId);
		Chat::exitRoom(firstName, guildChatRoomPath);
		Chat::uninvite(firstName, guildChatRoomPath);
		Chat::invite(firstName, guildChatRoomPath);
		Chat::enterRoom(firstName, guildChatRoomPath, false, false);
	}
}

// ----------------------------------------------------------------------

void GuildInterface::enterGuildVoiceChatRoom(CreatureObject const &who)
{
	int guildId = who.getGuildId();
	if (guildId)
	{
		std::string firstName;
		size_t pos = 0;
		IGNORE_RETURN(Unicode::getFirstToken(Unicode::wideToNarrow(who.getObjectName()), 0, pos, firstName));

		std::string roomPath = getChatRoomPath(guildId);

		PendingChannelAddMap::iterator entryItr = s_pendingChannelAdds.find(roomPath);
		if(entryItr == s_pendingChannelAdds.end()) //not currently pending
		{
			Chat::requestGetChannel(roomPath, false, true, s_guildChannelLimit);
			s_pendingChannelAdds[roomPath].insert(std::make_pair(firstName, who.getNetworkId()));
			//DEBUG_WARNING(true,("Created channel and differed add for %s %s %s", roomPath.c_str(), who.getNetworkId().getValueString().c_str(), firstName.c_str()));

		}
		else //already have a request out
		{
			entryItr->second[firstName] = who.getNetworkId();
			//DEBUG_WARNING(true,("added differed add for %s %s %s", roomPath.c_str(), who.getNetworkId().getValueString().c_str(), firstName.c_str()));
		}
	}
}


// ----------------------------------------------------------------------

void GuildInterface::leaveGuildChatRoom(CreatureObject const &who)
{
	int guildId = who.getGuildId();
	if (guildId)
	{
		std::string firstName;
		size_t pos = 0;
		IGNORE_RETURN(Unicode::getFirstToken(Unicode::wideToNarrow(who.getObjectName()), 0, pos, firstName));
		Chat::exitRoom(firstName, getChatRoomPath(guildId));
		Chat::uninvite(firstName, getChatRoomPath(guildId));
	}
}

// ----------------------------------------------------------------------

void GuildInterface::leaveGuildVoiceChatRoom(NetworkId const & playerNetworkId, std::string const & firstName, int guildId)
{
	PendingChannelAddMap::iterator channelAddList = s_pendingChannelAdds.find(getChatRoomPath(guildId));
	if(channelAddList != s_pendingChannelAdds.end())
	{
		PendingChannelAddList::iterator creatureEntry = channelAddList->second.find(firstName);
		if(creatureEntry != channelAddList->second.end())
		{
			channelAddList->second.erase(creatureEntry);
		}
	}

	Chat::requestRemoveClientFromChannel(playerNetworkId, firstName, getChatRoomPath(guildId));
}

// ----------------------------------------------------------------------

void GuildInterface::sendGuildChat(CreatureObject const &who, Unicode::String const &text)
{
	int guildId = who.getGuildId();
	if (guildId)
	{
		std::string firstName;
		size_t pos = 0;
		IGNORE_RETURN(Unicode::getFirstToken(Unicode::wideToNarrow(who.getObjectName()), 0, pos, firstName));
		Chat::sendToRoom(firstName, getChatRoomPath(guildId), text, Unicode::String());
	}
}

// ---------------------------------------------------------------------

void GuildInterface::onChatRoomCreate(std::string const &path)
{
	std::string const &prefix = getChatRoomPrefix();
	if (!path.compare(0, prefix.length(), prefix))
	{
		int guildId = atoi(path.c_str()+prefix.length());
		if (guildId)
		{
			std::vector<ServerObject *> players;
			GameServer::getInstance().getObjectsWithClients(players);
			for (std::vector<ServerObject *>::const_iterator i = players.begin(); i != players.end(); ++i)
			{
				if ((*i)->isAuthoritative() && (*i)->isPlayerControlled())
				{
					CreatureObject const *playerCreature = (*i)->asCreatureObject();
					if (playerCreature && playerCreature->getGuildId() == guildId)
						enterGuildChatRoom(*playerCreature);
				}
			}
		}
	}
}

void GuildInterface::onVoiceChatRoomCreate(std::string const &roomPath)
{
	//DEBUG_WARNING(true, ("onVoiceChatRoomCreate path %s", roomPath.c_str()));
	PendingChannelAddMap::iterator entry = s_pendingChannelAdds.find(roomPath);
	if(entry != s_pendingChannelAdds.end())
	{
		std::string const & path = entry->first;
		PendingChannelAddList const & pendingAdds = entry->second;
		for(PendingChannelAddList::const_iterator i = pendingAdds.begin(); i != pendingAdds.end(); ++i)
		{
			//DEBUG_WARNING(true,("Requesting add for %s %s %s", path.c_str(), i->second.getValueString().c_str(), i->first.c_str()));
			Chat::requestAddClientToChannel(i->second, i->first, path, true);
		}
		s_pendingChannelAdds.erase(entry);
	}
}

// ----------------------------------------------------------------------

void GuildInterface::createAllGuildChatRooms()
{
	std::set<int> guildIds;
	std::vector<ServerObject *> players;
	GameServer::getInstance().getObjectsWithClients(players);
	for (std::vector<ServerObject *>::const_iterator i = players.begin(); i != players.end(); ++i)
	{
		if ((*i)->isAuthoritative() && (*i)->isPlayerControlled())
		{
			CreatureObject const *playerCreature = (*i)->asCreatureObject();
			if (playerCreature && playerCreature->getGuildId())
				IGNORE_RETURN(guildIds.insert(playerCreature->getGuildId()));
		}
	}
	for (std::set<int>::const_iterator j = guildIds.begin(); j != guildIds.end(); ++j)
	{
		Chat::createRoom("System", false, getChatRoomPath(*j), getChatRoomTitle(*j));

		//TODO: temporary separation of voice and text, this should be rolled into the createRoom call above
		//We create the chat room when a guild member logs in, it will save us $$ to not
		//create it here.
		//Chat::requestGetChannel(getVoiceChatRoomPath(*j), false, false);
	}
}

// ----------------------------------------------------------------------
// returns the number of guild members, guild name, and guild abbrev that the
// mail was sent to or -1 if insufficient permission to send mail to guild members
std::pair<int, std::pair<std::string, std::string> > GuildInterface::mailToGuild(CreatureObject const &who, Unicode::String const &subject, Unicode::String const &message, Unicode::String const &oob)
{
	int count = -1;
	std::string guildName, guildAbbrev;
	int const guildId = who.getGuildId();
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
	{
		GuildMemberInfo const * const mailerInfo = ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberInfo(guildId, who.getNetworkId());
		if (mailerInfo && ((mailerInfo->m_permissions & GuildInterface::Mail) || (gi->m_leaderId == who.getNetworkId())))
		{
			guildName = gi->m_name;
			guildAbbrev = gi->m_abbrev;
			count = 0;
			std::vector<NetworkId> memberIds;
			ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberIds(guildId, memberIds);
			for (std::vector<NetworkId>::const_iterator i = memberIds.begin(); i != memberIds.end(); ++i)
			{
				GuildMemberInfo const * const memberInfo = ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberInfo(guildId, *i);
				if (memberInfo && (memberInfo->m_permissions & GuildInterface::Member))
				{
					Chat::sendPersistentMessage(Unicode::wideToNarrow(who.getEncodedObjectName()), memberInfo->m_name, subject, message, oob);
					++count;
				}
			}
		}
	}

	return std::make_pair(count, std::make_pair(guildName, guildAbbrev));
}

// ----------------------------------------------------------------------
// returns the number of guild members, guild name, and guild abbrev that the
// mail was sent to or -1 if insufficient permission to send mail to guild members
std::pair<int, std::pair<std::string, std::string> > GuildInterface::mailToGuild(int guildId, Unicode::String const &subject, Unicode::String const &message, Unicode::String const &oob)
{
	int count = -1;
	std::string guildName, guildAbbrev;
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
	{
		guildName = gi->m_name;
		guildAbbrev = gi->m_abbrev;
		count = 0;
		std::vector<NetworkId> memberIds;
		ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberIds(guildId, memberIds);
		for (std::vector<NetworkId>::const_iterator i = memberIds.begin(); i != memberIds.end(); ++i)
		{
			GuildMemberInfo const * const memberInfo = ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberInfo(guildId, *i);
			if (memberInfo && (memberInfo->m_permissions & GuildInterface::Member))
			{
				Chat::sendPersistentMessage(guildName, memberInfo->m_name, subject, message, oob);
				++count;
			}
		}
	}

	return std::make_pair(count, std::make_pair(guildName, guildAbbrev));
}

// ----------------------------------------------------------------------
// returns the number of guild members, guild name, and guild abbrev that the
// mail was sent to or -1 if insufficient permission to send mail to guild members
std::pair<int, std::pair<std::string, std::string> > GuildInterface::mailToGuildRank(CreatureObject const &who, int guildRankSlotId, Unicode::String const &subject, Unicode::String const &message, Unicode::String const &oob)
{
	int count = -1;
	std::string guildName, guildAbbrev;
	int const guildId = who.getGuildId();
	GuildInfo const * const gi = ServerUniverse::getInstance().getMasterGuildObject()->getGuildInfo(guildId);
	if (gi)
	{
		GuildMemberInfo const * const mailerInfo = ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberInfo(guildId, who.getNetworkId());
		if (mailerInfo && ((mailerInfo->m_permissions & GuildInterface::Mail) || (gi->m_leaderId == who.getNetworkId())))
		{
			guildName = gi->m_name;
			guildAbbrev = gi->m_abbrev;
			count = 0;
			std::vector<NetworkId> memberIds;
			ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberIds(guildId, memberIds);
			for (std::vector<NetworkId>::const_iterator i = memberIds.begin(); i != memberIds.end(); ++i)
			{
				GuildMemberInfo const * const memberInfo = ServerUniverse::getInstance().getMasterGuildObject()->getGuildMemberInfo(guildId, *i);
				if (memberInfo && (memberInfo->m_permissions & GuildInterface::Member) && memberInfo->m_rank.testBit(guildRankSlotId))
				{
					Chat::sendPersistentMessage(Unicode::wideToNarrow(who.getEncodedObjectName()), memberInfo->m_name, subject, message, oob);
					++count;
				}
			}
		}
	}

	return std::make_pair(count, std::make_pair(guildName, guildAbbrev));
}

// ======================================================================
