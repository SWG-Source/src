// ======================================================================
//
// GuildObject.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/GuildObject.h"

#include "UnicodeUtils.h"
#include "serverGame/CityObject.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/GuildController.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/GuildStringParser.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerGuildObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedGame/GuildRankDataTable.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/Controller.h"

// ======================================================================

namespace GuildObjectNamespace
{
	void replaceSetIfNeeded(char const *label, Archive::AutoDeltaSet<std::string> &oldSet, std::set<std::string> const &newSet, bool force);
	void replaceSetIfNeeded(char const *label, Archive::AutoDeltaSet<std::string, GuildObject> &oldSet, std::set<std::string> const &newSet, bool force);

	void updateGuildInfoMembersCount(GuildInfo & gi, int const * oldPermissions, int const * newPermissions);

	void updateGcwPercentileHistory(Archive::AutoDeltaMap<std::pair<std::string, int>, int> & history, Archive::AutoDeltaMap<std::string, int> & historyCount, std::string const & scoreName, int score);

	std::string const getFactionString(uint32 factionId)
	{
		if (PvpData::isNeutralFactionId(factionId))
			return "Neutral";
		else if (PvpData::isImperialFactionId(factionId))
			return "Imperial";
		else if (PvpData::isRebelFactionId(factionId))
			return "Rebel";

		return "Unknown";
	}
}
using namespace GuildObjectNamespace;

// ======================================================================

GuildObject::GuildObject(ServerGuildObjectTemplate const *newTemplate) :
	UniverseObject(newTemplate),
	m_names(),
	m_abbrevs(),
	m_leaders(),
	m_members(),
	m_enemies(),
	m_guildsInfo(),
	m_membersInfo(),
	m_fullMembers(),
	m_sponsoredMembers(),
	m_guildLeaders(),
	m_gcwImperialScorePercentileThisGalaxy(),
	m_gcwRegionDefenderBonus(),
	m_gcwGroupImperialScorePercentileThisGalaxy(),
	m_gcwImperialScorePercentileHistoryThisGalaxy(),
	m_gcwGroupImperialScorePercentileHistoryThisGalaxy(),
	m_gcwImperialScorePercentileHistoryCountThisGalaxy(),
	m_gcwGroupImperialScorePercentileHistoryCountThisGalaxy(),
	m_gcwImperialScorePercentileOtherGalaxies(),
	m_gcwGroupImperialScorePercentileOtherGalaxies(),
	m_gcwGroupCategoryImperialScoreRawThisGalaxy(),
	m_gcwGroupImperialScoreRawThisGalaxy(),
	m_gcwImperialScoreOtherGalaxies(),
	m_gcwRebelScoreOtherGalaxies(),
	m_lowFreeGuildId(1000*GameServer::getInstance().getProcessId()),
	m_nextGuildWarKillTrackingAdjustmentUpdate(0),
	m_guildWarKillTrackingAdjustment(),
	m_gcwRegionDefenderGuilds(),
	m_gcwRegionDefenderGuildsCount(),
	m_gcwRegionDefenderGuildsVersion(0)
{
	addMembersToPackages();

	m_enemies.setOnChanged(this, &GuildObject::onGuildEnemiesChanged);

	m_membersInfo.setOnErase(this, &GuildObject::onMembersInfoErase);
	m_membersInfo.setOnInsert(this, &GuildObject::onMembersInfoInsert);
	m_membersInfo.setOnSet(this, &GuildObject::onMembersInfoSet);
}

// ----------------------------------------------------------------------

GuildObject::~GuildObject()
{
	if (isInWorld())
		removeFromWorld();
}

// ----------------------------------------------------------------------

Controller* GuildObject::createDefaultController()
{
	Controller* controller = new GuildController(this);

	setController(controller);
	return controller;
}

// ----------------------------------------------------------------------

bool GuildObject::isVisibleOnClient(const Client &) const
{
	// The GuildObject is visible to all clients and is explicitly observed on login.
	return true;
}

// ----------------------------------------------------------------------

void GuildObject::setupUniverse()
{
	ServerUniverse::getInstance().registerMasterGuildObject(*this);
	// force update of the guild interface's structures so we have current data for sanity checking
	GuildInterface::update();

	if (isAuthoritative())
	{
		// build the guild and guild member info from data read in from DB

		// disable notification while building the initial list
		m_membersInfo.setOnErase(nullptr, nullptr);
		m_membersInfo.setOnInsert(nullptr, nullptr);
		m_membersInfo.setOnSet(nullptr, nullptr);

		// build names
		{
			std::set<std::string> const &names = m_names.get();
			for (std::set<std::string>::const_iterator i = names.begin(); i != names.end(); ++i)
			{
				int guildId;
				std::string guildName;
				int guildElectionPreviousEndTime;
				int guildElectionNextEndTime;
				uint32 guildFaction;
				int timeLeftGuildFaction;
				std::string guildGcwDefenderRegion;
				int timeJoinedGuildGcwDefenderRegion;
				int timeLeftGuildGcwDefenderRegion;
				if (GuildStringParser::parseGuildName(*i, guildId, guildName, guildElectionPreviousEndTime, guildElectionNextEndTime, guildFaction, timeLeftGuildFaction, guildGcwDefenderRegion, timeJoinedGuildGcwDefenderRegion, timeLeftGuildGcwDefenderRegion))
				{
					GuildInfo gi(getGuildInfo(guildId));
					gi.m_name = guildName;
					gi.m_guildElectionPreviousEndTime = guildElectionPreviousEndTime;
					gi.m_guildElectionNextEndTime = guildElectionNextEndTime;
					gi.m_guildFaction = guildFaction;
					gi.m_timeLeftGuildFaction = timeLeftGuildFaction;
					gi.m_guildGcwDefenderRegion = guildGcwDefenderRegion;
					gi.m_timeJoinedGuildGcwDefenderRegion = timeJoinedGuildGcwDefenderRegion;
					gi.m_timeLeftGuildGcwDefenderRegion = timeLeftGuildGcwDefenderRegion;

					m_guildsInfo.set(guildId, gi);

					if (!gi.m_guildGcwDefenderRegion.empty() && (gi.m_timeLeftGuildGcwDefenderRegion <= 0))
						addGcwRegionDefenderGuild(gi.m_guildGcwDefenderRegion, guildId, (((gi.m_timeLeftGuildFaction > 0) || PvpData::isNeutralFactionId(gi.m_guildFaction)) ? PvpData::getNeutralFactionId() : gi.m_guildFaction));
				}
			}
		}

		// build abbrevs
		{
			std::set<std::string> const &abbrevs = m_abbrevs.get();
			for (std::set<std::string>::const_iterator i = abbrevs.begin(); i != abbrevs.end(); ++i)
			{
				int guildId;
				std::string guildAbbrev;
				if (GuildStringParser::parseGuildAbbrev(*i, guildId, guildAbbrev))
				{
					GuildInfo gi(getGuildInfo(guildId));
					gi.m_abbrev = guildAbbrev;
					m_guildsInfo.set(guildId, gi);
				}
			}
		}

		// build leaderIds
		{
			std::set<std::string> const &leaders = m_leaders.get();
			for (std::set<std::string>::const_iterator i = leaders.begin(); i != leaders.end(); ++i)
			{
				int guildId;
				NetworkId leaderId;
				if (GuildStringParser::parseGuildLeader(*i, guildId, leaderId))
				{
					m_guildLeaders.set(leaderId, guildId);

					GuildInfo gi(getGuildInfo(guildId));
					gi.m_leaderId = leaderId;
					m_guildsInfo.set(guildId, gi);
				}
			}
		}

		// build members
		{
			// make temporary list of guilds so that guild info members count can be updated
			// in the temporary list rather than in the master list, because there will be lots
			// of update, and we don't want to generated unnecessary deltas; by doing it using
			// the temporary list, we'll generate only 1 delta when we write the information back
			std::map<int, GuildInfo> guildInfoMembersCount = m_guildsInfo.getMap();

			int const allAdministrativePermissions = GuildInterface::AllAdministrativePermissions;
			std::set<std::string> const &members = m_members.get();
			for (std::set<std::string>::const_iterator i = members.begin(); i != members.end(); ++i)
			{
				int guildId;
				NetworkId memberId;
				std::string name;
				std::string professionSkillTemplate;
				int level;
				int permissions;
				BitArray rank;
				std::string title;
				NetworkId allegiance;
				if (GuildStringParser::parseMemberInfo(*i, guildId, memberId, name, professionSkillTemplate, level, permissions, rank, title, allegiance))
				{
					// update guild info members count
					GuildInfo & gi = guildInfoMembersCount[guildId];
					GuildMemberInfo const * const existingGmi = getGuildMemberInfo(guildId, memberId);
					updateGuildInfoMembersCount(gi, (existingGmi ? ((existingGmi->m_permissions == -1) ? &allAdministrativePermissions : &(existingGmi->m_permissions)) : nullptr), ((permissions == -1) ? &allAdministrativePermissions : &permissions));

					if (existingGmi)
					{
						if (existingGmi->m_permissions == GuildInterface::None)
							m_sponsoredMembers.erase(memberId);
						else
							m_fullMembers.erase(memberId);
					}

					if (permissions == GuildInterface::None)
						m_sponsoredMembers.set(memberId, guildId);
					else
						m_fullMembers.set(memberId, guildId);

					GuildMemberInfo gmi(existingGmi);
					gmi.m_name = name;
					gmi.m_professionSkillTemplate = professionSkillTemplate;
					gmi.m_level = level;
					gmi.m_permissions = permissions;
					gmi.m_rank = rank;
					gmi.m_title = title;
					gmi.m_allegiance = allegiance;
					m_membersInfo.set(std::make_pair(guildId, memberId), gmi);
				}
			}

			// save off final guild member count info
			{
				for (std::map<int, GuildInfo>::const_iterator iterGuild = guildInfoMembersCount.begin(); iterGuild != guildInfoMembersCount.end(); ++iterGuild)
					m_guildsInfo.set(iterGuild->first, iterGuild->second);
			}
		}

		// rebuild names, abbrevs, leaders
		{
			std::set<std::string> names;
			std::set<std::string> abbrevs;
			std::set<std::string> leaders;
			std::string gi;
			for (std::map<int, GuildInfo>::const_iterator iterGuild = m_guildsInfo.begin(); iterGuild != m_guildsInfo.end(); ++iterGuild)
			{
				if (!iterGuild->second.m_name.empty())
				{
					GuildStringParser::buildNameSpec(iterGuild->first, iterGuild->second.m_name, iterGuild->second.m_guildElectionPreviousEndTime, iterGuild->second.m_guildElectionNextEndTime, iterGuild->second.m_guildFaction, iterGuild->second.m_timeLeftGuildFaction, iterGuild->second.m_guildGcwDefenderRegion, iterGuild->second.m_timeJoinedGuildGcwDefenderRegion, iterGuild->second.m_timeLeftGuildGcwDefenderRegion, gi);
					names.insert(gi);
				}

				if (!iterGuild->second.m_abbrev.empty())
				{
					GuildStringParser::buildAbbrevSpec(iterGuild->first, iterGuild->second.m_abbrev, gi);
					abbrevs.insert(gi);
				}

				GuildStringParser::buildLeaderSpec(iterGuild->first, iterGuild->second.m_leaderId, gi);
				leaders.insert(gi);
			}

			replaceSetIfNeeded("names", m_names, names, false);
			replaceSetIfNeeded("abbrevs", m_abbrevs, abbrevs, false);
			replaceSetIfNeeded("leaders", m_leaders, leaders, false);
		}

		// rebuild members
		{
			std::set<std::string> members;
			std::list<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::string> > > membersToConvertPermissions;
			std::string gmi;
			std::string gmiConverted;
			for (std::map<std::pair<int, NetworkId>, GuildMemberInfo>::const_iterator iter = m_membersInfo.begin(); iter != m_membersInfo.end(); ++iter)
			{
				GuildStringParser::buildMemberSpec(iter->first.first, iter->first.second, iter->second.m_name, iter->second.m_professionSkillTemplate, iter->second.m_level, iter->second.m_permissions, iter->second.m_rank, iter->second.m_title, iter->second.m_allegiance, gmi);
				members.insert(gmi);

				// construct list of members with permission of -1, which will need to be converted
				if (iter->second.m_permissions == -1)
				{
					GuildStringParser::buildMemberSpec(iter->first.first, iter->first.second, iter->second.m_name, iter->second.m_professionSkillTemplate, iter->second.m_level, static_cast<int>(GuildInterface::AllAdministrativePermissions), iter->second.m_rank, iter->second.m_title, iter->second.m_allegiance, gmiConverted);
					membersToConvertPermissions.push_back(std::make_pair(iter->first, std::make_pair(gmi, gmiConverted)));
				}
			}

			// convert members with permissions of -1
			{
				for (std::list<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::string> > >::const_iterator iter = membersToConvertPermissions.begin(); iter != membersToConvertPermissions.end(); ++iter)
				{
					GuildMemberInfo gmi(getGuildMemberInfo(iter->first.first, iter->first.second));
					gmi.m_permissions = GuildInterface::AllAdministrativePermissions;
					m_membersInfo.set(iter->first, gmi);
				}
			}

			if (m_members.get() != members)
			{
				// since we're going to be replacing everything, take the opportunity
				// to convert over those guild members with permissions of -1
				for (std::list<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::string> > >::const_iterator iterConvert = membersToConvertPermissions.begin(); iterConvert != membersToConvertPermissions.end(); ++iterConvert)
				{
					members.erase(iterConvert->second.first);
					members.insert(iterConvert->second.second);
				}

				replaceSetIfNeeded("members", m_members, members, true);
			}
			else
			{
				// convert over those guild members with permissions of -1
				for (std::list<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::string> > >::const_iterator iterConvert = membersToConvertPermissions.begin(); iterConvert != membersToConvertPermissions.end(); ++iterConvert)
				{
					m_members.erase(iterConvert->second.first);
					m_members.insert(iterConvert->second.second);
				}
			}
		}

		// rebuild enemies
		{
			std::set<std::string> enemies;
			for (std::map<int, GuildInfo>::const_iterator i = m_guildsInfo.begin(); i != m_guildsInfo.end(); ++i)
			{
				std::map<int, std::pair<int, time_t> > const &enemyGuildIds = GuildInterface::getGuildEnemiesKillInfo(i->first);
				for (std::map<int, std::pair<int, time_t> >::const_iterator j = enemyGuildIds.begin(); j != enemyGuildIds.end(); ++j)
				{
					std::string enemySpec;
					GuildStringParser::buildEnemySpec(i->first, j->first, j->second.first, static_cast<int>(j->second.second), enemySpec);
					enemies.insert(enemySpec);
				}
			}
			replaceSetIfNeeded("enemies", m_enemies, enemies, false);
		}

		// re-enable notification
		m_membersInfo.setOnErase(this, &GuildObject::onMembersInfoErase);
		m_membersInfo.setOnInsert(this, &GuildObject::onMembersInfoInsert);
		m_membersInfo.setOnSet(this, &GuildObject::onMembersInfoSet);
	}
}

// ----------------------------------------------------------------------

void GuildObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	UniverseObject::getAttributes(data);
}

// ----------------------------------------------------------------------

GuildInfo const * GuildObject::getGuildInfo(int guildId) const
{
	std::map<int, GuildInfo>::const_iterator const iterFind = m_guildsInfo.find(guildId);
	if (iterFind != m_guildsInfo.end())
		return &(iterFind->second);

	return nullptr;
}

// ----------------------------------------------------------------------

GuildMemberInfo const * GuildObject::getGuildMemberInfo(int guildId, NetworkId const &memberId) const
{
	const auto iterFind = m_membersInfo.find(std::make_pair(guildId, memberId));
	if (iterFind != m_membersInfo.end())
		return &(iterFind->second);

	return nullptr;
}

// ----------------------------------------------------------------------

std::set<std::string> const &GuildObject::getGuildEnemies() const
{
	return m_enemies.get();
}

// ----------------------------------------------------------------------

void GuildObject::getAllGuildIds(std::vector<int> &results) const
{
	results.clear();

	for (std::map<int, GuildInfo>::const_iterator iter = m_guildsInfo.begin(); iter != m_guildsInfo.end(); ++iter)
	{
		results.push_back(iter->first);
	}
}

// ----------------------------------------------------------------------

int GuildObject::findGuild(std::string const &name) const
{
	// Name may be either an abbreviation or a name, and we should match in either case.
	char const *nameText = name.c_str();
	for (std::map<int, GuildInfo>::const_iterator i = m_guildsInfo.begin(); i != m_guildsInfo.end(); ++i)
	{
		if (   !_stricmp(i->second.m_name.c_str(), nameText)
			|| !_stricmp(i->second.m_abbrev.c_str(), nameText))
			return i->first;
	}

	return 0;
}

// ----------------------------------------------------------------------

bool GuildObject::guildExists(int guildId) const
{
	return (m_guildsInfo.find(guildId) != m_guildsInfo.end());
}

// ----------------------------------------------------------------------

int GuildObject::getGuildId(NetworkId const &memberId) const
{
	std::map<NetworkId, int>::const_iterator const iterFind = m_fullMembers.find(memberId);
	if (iterFind != m_fullMembers.end())
		return iterFind->second;

	return 0;
}

// ----------------------------------------------------------------------

int GuildObject::getGuildIdForSponsored(NetworkId const &memberId) const
{
	std::map<NetworkId, int>::const_iterator const iterFind = m_sponsoredMembers.find(memberId);
	if (iterFind != m_sponsoredMembers.end())
		return iterFind->second;

	return 0;
}

// ----------------------------------------------------------------------

int GuildObject::getGuildIdForGuildLeader(NetworkId const &leaderId) const
{
	std::map<NetworkId, int>::const_iterator const iterFind = m_guildLeaders.find(leaderId);
	if (iterFind != m_guildLeaders.end())
		return iterFind->second;

	return 0;
}

// ----------------------------------------------------------------------

void GuildObject::getGuildMemberIds(int guildId, std::vector<NetworkId> &results) const
{
	results.clear();

	for (auto iter = m_membersInfo.lower_bound(std::make_pair(guildId, NetworkId::cms_invalid)); iter != m_membersInfo.end(); ++iter)
	{
		if (iter->first.first != guildId)
			break;

		results.push_back(iter->first.second);
	}
}

// ----------------------------------------------------------------------

void GuildObject::getMemberIdsWithPermissions(int guildId, int permissions, std::vector<NetworkId> &results) const
{
	results.clear();

	for (auto iter = m_membersInfo.lower_bound(std::make_pair(guildId, NetworkId::cms_invalid)); iter != m_membersInfo.end(); ++iter)
	{
		if (iter->first.first != guildId)
			break;

		if (permissions == GuildInterface::None)
		{
			if (iter->second.m_permissions == GuildInterface::None)
				results.push_back(iter->first.second);
		}
		else if ((iter->second.m_permissions & permissions) == permissions)
		{
			results.push_back(iter->first.second);
		}
	}
}

// ----------------------------------------------------------------------

int GuildObject::nextFreeGuildId()
{
	// Note: this must return a guild id which is not only unused, but also one
	// that cannot have collisions with a free guildId generated by another
	// server, even during the same frame.  This is because we need immediate
	// feedback of what the newly created guild's ID is, regardless of whether
	// the master guild object is authoritative on this server.
	// In order to do this, we find the lowest unused id starting from
	// 1000 * our process id + 1.  We also need to assure that if multiple guilds
	// are created from the same process, they do not collide in the time before
	// the creation actually takes place, since we may not have up to date
	// guild info if we are not authoritative.

	do
	{
		++m_lowFreeGuildId;
	}
	while (guildExists(m_lowFreeGuildId));
	return m_lowFreeGuildId;
}

// ----------------------------------------------------------------------

int GuildObject::createGuild(std::string const &guildName, std::string const &guildAbbrev, int guildId)
{
	if (guildId == 0)
		guildId = nextFreeGuildId();

	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_guildCreate,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, std::pair<std::string, std::string> > >(std::make_pair(guildId, std::make_pair(guildName, guildAbbrev))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (!guildExists(guildId))
	{
		{
			std::string nameSpec;
			GuildStringParser::buildNameSpec(guildId, guildName, 0, 0, 0, 0, std::string(), 0, 0, nameSpec);
			m_names.insert(nameSpec);
		}
		{
			std::string abbrevSpec;
			GuildStringParser::buildAbbrevSpec(guildId, guildAbbrev, abbrevSpec);
			m_abbrevs.insert(abbrevSpec);
		}

		GuildInfo gi;
		gi.m_name = guildName;
		gi.m_abbrev = guildAbbrev;
		m_guildsInfo.set(guildId, gi);
	}

	return guildId;
}

// ----------------------------------------------------------------------

void GuildObject::disbandGuild(int guildId)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_guildDisband,
				0.0f,
				new MessageQueueGenericValueType<int>(guildId),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (guildExists(guildId))
	{
		// remove guild
		{
			GuildInfo const * const gi = getGuildInfo(guildId);
			if (gi)
			{
				std::string nameSpec;
				GuildStringParser::buildNameSpec(guildId, gi->m_name, gi->m_guildElectionPreviousEndTime, gi->m_guildElectionNextEndTime, gi->m_guildFaction, gi->m_timeLeftGuildFaction, gi->m_guildGcwDefenderRegion, gi->m_timeJoinedGuildGcwDefenderRegion, gi->m_timeLeftGuildGcwDefenderRegion, nameSpec);
				m_names.erase(nameSpec);

				std::string abbrevSpec;
				GuildStringParser::buildAbbrevSpec(guildId, gi->m_abbrev, abbrevSpec);
				m_abbrevs.erase(abbrevSpec);

				std::string leaderSpec;
				GuildStringParser::buildLeaderSpec(guildId, gi->m_leaderId, leaderSpec);
				m_leaders.erase(leaderSpec);

				std::map<NetworkId, int>::const_iterator iterGuildLeader = m_guildLeaders.find(gi->m_leaderId);
				if ((iterGuildLeader != m_guildLeaders.end()) && (iterGuildLeader->second == guildId))
					m_guildLeaders.erase(gi->m_leaderId);

				if (!gi->m_guildGcwDefenderRegion.empty() && (gi->m_timeLeftGuildGcwDefenderRegion <= 0))
					removeGcwRegionDefenderGuild(gi->m_guildGcwDefenderRegion, guildId, (((gi->m_timeLeftGuildFaction > 0) || PvpData::isNeutralFactionId(gi->m_guildFaction)) ? PvpData::getNeutralFactionId() : gi->m_guildFaction));

				m_guildsInfo.erase(guildId);
			}
		}

		// remove all members
		{
			std::vector<NetworkId> memberIds;
			for (auto iter = m_membersInfo.lower_bound(std::make_pair(guildId, NetworkId::cms_invalid)); iter != m_membersInfo.end(); ++iter)
			{
				if (iter->first.first != guildId)
					break;

				memberIds.push_back(iter->first.second);

				std::string memberSpec;
				GuildStringParser::buildMemberSpec(iter->first.first, iter->first.second, iter->second.m_name, iter->second.m_professionSkillTemplate, iter->second.m_level, iter->second.m_permissions, iter->second.m_rank, iter->second.m_title, iter->second.m_allegiance, memberSpec);
				m_members.erase(memberSpec);
			}

			for (auto iterMemberId = memberIds.begin(); iterMemberId != memberIds.end(); ++iterMemberId)
			{
				std::map<NetworkId, int>::const_iterator iterFind = m_fullMembers.find(*iterMemberId);
				if ((iterFind != m_fullMembers.end()) && (iterFind->second == guildId))
					m_fullMembers.erase(*iterMemberId);

				iterFind = m_sponsoredMembers.find(*iterMemberId);
				if ((iterFind != m_sponsoredMembers.end()) && (iterFind->second == guildId))
					m_sponsoredMembers.erase(*iterMemberId);

				iterFind = m_guildLeaders.find(*iterMemberId);
				if ((iterFind != m_guildLeaders.end()) && (iterFind->second == guildId))
					m_guildLeaders.erase(*iterMemberId);

				m_membersInfo.erase(std::make_pair(guildId, *iterMemberId));
			}
		}

		// remove who we're at war with
		{
			std::vector<int> const &enemies = GuildInterface::getGuildEnemies(guildId);
			for (std::vector<int>::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
			{
				std::pair<int, time_t> const & killInfo = GuildInterface::getGuildEnemyKillInfo(guildId, *i);
				std::string enemySpec;
				GuildStringParser::buildEnemySpec(guildId, *i, killInfo.first, (int)killInfo.second, enemySpec);
				m_enemies.erase(enemySpec);

				// remove any pending kill count updates for the 2 guilds
				m_guildWarKillTrackingAdjustment.erase(std::make_pair(guildId, *i));

				// if it's a guild war that's ending and there's at least 1 kill, update ended guild war tracking data
				std::pair<int, time_t> const * killInfoBtoA = GuildInterface::hasDeclaredWarAgainst(*i, guildId);
				if (killInfoBtoA && ((killInfoBtoA->first > 0) || (killInfo.first > 0)))
					GuildInterface::updateInactiveGuildWarTrackingInfo(*this, guildId, killInfo.first, *i, killInfoBtoA->first);
			}
		}

		// remove who's at war with us
		{
			std::vector<int> const &enemies = GuildInterface::getGuildsAtWarWith(guildId);
			for (std::vector<int>::const_iterator i = enemies.begin(); i != enemies.end(); ++i)
			{
				std::pair<int, time_t> const & killInfo = GuildInterface::getGuildEnemyKillInfo(*i, guildId);
				std::string enemySpec;
				GuildStringParser::buildEnemySpec(*i, guildId, killInfo.first, (int)killInfo.second, enemySpec);
				m_enemies.erase(enemySpec);

				// remove any pending kill count updates for the 2 guilds
				m_guildWarKillTrackingAdjustment.erase(std::make_pair(*i, guildId));
			}
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::removeGuildMember(int guildId, NetworkId const &memberId)
{
	GuildInfo const * const gi = getGuildInfo(guildId);
	GuildMemberInfo const * const gmi = getGuildMemberInfo(guildId, memberId);
	if (gi && gmi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildRemoveMember,
					0.0f,
					new MessageQueueGenericValueType<std::pair<int, NetworkId> >(std::make_pair(guildId, memberId)),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else
		{
			{
				std::string firstName;
				size_t pos = 0;
				IGNORE_RETURN(Unicode::getFirstToken(gmi->m_name, 0, pos, firstName));
				GuildInterface::leaveGuildVoiceChatRoom(memberId, gmi->m_name, guildId);
			}

			std::string memberSpec;
			GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, gmi->m_rank, gmi->m_title, gmi->m_allegiance, memberSpec);
			m_members.erase(memberSpec);

			GuildInfo updatedGi(gi);
			updateGuildInfoMembersCount(updatedGi, &(gmi->m_permissions), nullptr);
			m_guildsInfo.set(guildId, updatedGi);

			std::map<NetworkId, int>::const_iterator iterFind = m_fullMembers.find(memberId);
			if ((iterFind != m_fullMembers.end()) && (iterFind->second == guildId))
				m_fullMembers.erase(memberId);

			iterFind = m_sponsoredMembers.find(memberId);
			if ((iterFind != m_sponsoredMembers.end()) && (iterFind->second == guildId))
				m_sponsoredMembers.erase(memberId);

			iterFind = m_guildLeaders.find(memberId);
			if ((iterFind != m_guildLeaders.end()) && (iterFind->second == guildId))
				m_guildLeaders.erase(memberId);

			m_membersInfo.erase(std::make_pair(guildId, memberId));

			// cannot be a GCW region defender if doesn't meet minimum guild member count requirement
			if (!updatedGi.m_guildGcwDefenderRegion.empty() && (updatedGi.m_timeLeftGuildGcwDefenderRegion <= 0) && (updatedGi.getCountMembersOnly() < ConfigServerGame::getGcwGuildMinMembersForGcwRegionDefender()))
				setGuildGcwDefenderRegion(guildId, std::string());
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::addGuildCreatorMember(int guildId, NetworkId const &memberId, std::string const &memberName, std::string const &memberProfessionSkillTemplate, int const memberLevel)
{
	GuildMemberInfo const * const existingGmi = getGuildMemberInfo(guildId, memberId);
	std::string realMemberName = memberName;
	if (realMemberName.empty() && existingGmi)
		realMemberName = existingGmi->m_name;

	ServerObject const *so = nullptr;
	if (realMemberName.empty())
	{
		so = ServerWorld::findObjectByNetworkId(memberId);
		if (so)
			realMemberName = Unicode::wideToNarrow(so->getAssignedObjectName());
	}

	if (realMemberName.empty())
	{
		WARNING(true, ("Unable to find name for guild (%d) member (%s)", guildId, memberId.getValueString().c_str()));
		return;
	}

	std::string realMemberProfessionSkillTemplate = memberProfessionSkillTemplate;
	int realMemberLevel = memberLevel;
	if (realMemberProfessionSkillTemplate.empty() || (realMemberLevel <= 0))
	{
		if (existingGmi)
		{
			realMemberProfessionSkillTemplate = existingGmi->m_professionSkillTemplate;
			realMemberLevel = existingGmi->m_level;
		}
		else
		{
			if (!so)
				so = ServerWorld::findObjectByNetworkId(memberId);

			if (so)
			{
				CreatureObject const * const co = so->asCreatureObject();
				if (co)
				{
					realMemberLevel = co->getLevel();

					PlayerObject const * const player = PlayerCreatureController::getPlayerObject(co);
					if (player)
						realMemberProfessionSkillTemplate = player->getSkillTemplate();
				}
			}
		}
	}

	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_guildAddCreatorMember,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<std::string, int> > > >(std::make_pair(std::make_pair(guildId, memberId), std::make_pair(realMemberName, std::make_pair(realMemberProfessionSkillTemplate, realMemberLevel)))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		GuildInfo const * const gi = getGuildInfo(guildId);
		if (gi)
		{
			if (existingGmi)
			{
				std::string oldMemberSpec;
				GuildStringParser::buildMemberSpec(guildId, memberId, existingGmi->m_name, existingGmi->m_professionSkillTemplate, existingGmi->m_level, existingGmi->m_permissions, existingGmi->m_rank, existingGmi->m_title, existingGmi->m_allegiance, oldMemberSpec);
				m_members.erase(oldMemberSpec);
			}

			int const permissions = (existingGmi ? (existingGmi->m_permissions | GuildInterface::AllAdministrativePermissions) : GuildInterface::AllAdministrativePermissions);
			BitArray const rank = (existingGmi ? existingGmi->m_rank : BitArray());
			std::string const title = (existingGmi ? existingGmi->m_title : std::string());
			std::string memberSpec;
			GuildStringParser::buildMemberSpec(guildId, memberId, realMemberName, realMemberProfessionSkillTemplate, realMemberLevel, permissions, rank, title, memberId, memberSpec);
			m_members.insert(memberSpec);

			GuildInfo updatedGi(gi);
			updateGuildInfoMembersCount(updatedGi, (existingGmi ? &(existingGmi->m_permissions) : nullptr), &permissions);
			m_guildsInfo.set(guildId, updatedGi);

			if (existingGmi)
			{
				if (existingGmi->m_permissions == GuildInterface::None)
					m_sponsoredMembers.erase(memberId);
				else
					m_fullMembers.erase(memberId);
			}

			m_fullMembers.set(memberId, guildId);

			GuildMemberInfo updatedGmi(existingGmi);
			updatedGmi.m_name = realMemberName;
			updatedGmi.m_professionSkillTemplate = realMemberProfessionSkillTemplate;
			updatedGmi.m_level = realMemberLevel;
			updatedGmi.m_permissions = permissions;
			updatedGmi.m_rank = rank;
			updatedGmi.m_title = title;
			updatedGmi.m_allegiance = memberId;
			m_membersInfo.set(std::make_pair(guildId, memberId), updatedGmi);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::addGuildSponsorMember(int guildId, NetworkId const &memberId, std::string const &memberName, std::string const &memberProfessionSkillTemplate, int const memberLevel)
{
	GuildMemberInfo const * const existingGmi = getGuildMemberInfo(guildId, memberId);
	std::string realMemberName = memberName;
	if (realMemberName.empty() && existingGmi)
		realMemberName = existingGmi->m_name;

	ServerObject const *so = nullptr;
	if (realMemberName.empty())
	{
		so = ServerWorld::findObjectByNetworkId(memberId);
		if (so)
			realMemberName = Unicode::wideToNarrow(so->getAssignedObjectName());
	}

	if (realMemberName.empty())
	{
		WARNING(true, ("Unable to find name for guild (%d) member (%s)", guildId, memberId.getValueString().c_str()));
		return;
	}

	std::string realMemberProfessionSkillTemplate = memberProfessionSkillTemplate;
	int realMemberLevel = memberLevel;
	if (realMemberProfessionSkillTemplate.empty() || (realMemberLevel <= 0))
	{
		if (existingGmi)
		{
			realMemberProfessionSkillTemplate = existingGmi->m_professionSkillTemplate;
			realMemberLevel = existingGmi->m_level;
		}
		else
		{
			if (!so)
				so = ServerWorld::findObjectByNetworkId(memberId);

			if (so)
			{
				CreatureObject const * const co = so->asCreatureObject();
				if (co)
				{
					realMemberLevel = co->getLevel();

					PlayerObject const * const player = PlayerCreatureController::getPlayerObject(co);
					if (player)
						realMemberProfessionSkillTemplate = player->getSkillTemplate();
				}
			}
		}
	}

	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_guildAddSponsorMember,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<std::string, int> > > >(std::make_pair(std::make_pair(guildId, memberId), std::make_pair(realMemberName, std::make_pair(realMemberProfessionSkillTemplate, realMemberLevel)))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		GuildInfo const * const gi = getGuildInfo(guildId);
		if (gi)
		{
			if (existingGmi)
			{
				std::string oldMemberSpec;
				GuildStringParser::buildMemberSpec(guildId, memberId, existingGmi->m_name, existingGmi->m_professionSkillTemplate, existingGmi->m_level, existingGmi->m_permissions, existingGmi->m_rank, existingGmi->m_title, existingGmi->m_allegiance, oldMemberSpec);
				m_members.erase(oldMemberSpec);
			}

			int const permissions = GuildInterface::None;
			std::string memberSpec;
			GuildStringParser::buildMemberSpec(guildId, memberId, realMemberName, realMemberProfessionSkillTemplate, realMemberLevel, permissions, BitArray(), std::string(), NetworkId::cms_invalid, memberSpec);
			m_members.insert(memberSpec);

			GuildInfo updatedGi(gi);
			updateGuildInfoMembersCount(updatedGi, (existingGmi ? &(existingGmi->m_permissions) : nullptr), &permissions);
			m_guildsInfo.set(guildId, updatedGi);

			if (existingGmi)
			{
				if (existingGmi->m_permissions == GuildInterface::None)
					m_sponsoredMembers.erase(memberId);
				else
					m_fullMembers.erase(memberId);
			}

			m_sponsoredMembers.set(memberId, guildId);

			GuildMemberInfo updatedGmi(existingGmi);
			updatedGmi.m_name = realMemberName;
			updatedGmi.m_professionSkillTemplate = realMemberProfessionSkillTemplate;
			updatedGmi.m_level = realMemberLevel;
			updatedGmi.m_permissions = permissions;
			updatedGmi.m_rank.clear();
			updatedGmi.m_title.clear();
			updatedGmi.m_allegiance = NetworkId::cms_invalid;
			m_membersInfo.set(std::make_pair(guildId, memberId), updatedGmi);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGuildMemberPermission(int guildId, NetworkId const &memberId, int permissions)
{
	GuildInfo const * const gi = getGuildInfo(guildId);
	GuildMemberInfo const * const gmi = getGuildMemberInfo(guildId, memberId);
	if (gi && gmi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetMemberPermission,
					0.0f,
					new MessageQueueGenericValueType<std::pair<std::pair<int, int>, NetworkId> >(std::make_pair(std::make_pair(guildId, permissions), memberId)),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else if (gmi->m_permissions != permissions)
		{	
			std::string memberSpec;
			GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, gmi->m_rank, gmi->m_title, gmi->m_allegiance, memberSpec);
			m_members.erase(memberSpec);

			// if the existing guild member is already a guild member (i.e. not sponsored),
			// absolutely, positively make sure  that the new permissions contain
			// GuildPermissions::Member; once this permission has been set, it can
			// never be removed; the guild member should be removed from the guild
			// if you really want to remove the GuildPermissions::Member permission
			if (gmi->m_permissions & GuildInterface::Member)
				permissions |= GuildInterface::Member;

			GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, permissions, gmi->m_rank, gmi->m_title, gmi->m_allegiance, memberSpec);
			m_members.insert(memberSpec);

			GuildInfo updatedGi(gi);
			updateGuildInfoMembersCount(updatedGi, &gmi->m_permissions, &permissions);
			m_guildsInfo.set(guildId, updatedGi);

			// handle moving from the member from the sponsored list to the member
			// list if the permission has been changed from sponsored to member
			if ((gmi->m_permissions == GuildInterface::None) && (permissions & GuildInterface::Member))
			{
				std::map<NetworkId, int>::const_iterator const iterFind = m_sponsoredMembers.find(memberId);
				if ((iterFind != m_sponsoredMembers.end()) && (iterFind->second == guildId))
					m_sponsoredMembers.erase(memberId);

				m_fullMembers.set(memberId, guildId);
			}

			GuildMemberInfo updatedGmi(gmi);
			updatedGmi.m_permissions = permissions;
			m_membersInfo.set(std::make_pair(guildId, memberId), updatedGmi);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGuildMemberTitle(int guildId, NetworkId const &memberId, std::string const &title)
{
	GuildMemberInfo const * const gmi = getGuildMemberInfo(guildId, memberId);
	if (gmi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetMemberTitle,
					0.0f,
					new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > >(std::make_pair(title, std::make_pair(memberId, guildId))),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else if (gmi->m_title != title)
		{
			std::string memberSpec;
			GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, gmi->m_rank, gmi->m_title, gmi->m_allegiance, memberSpec);
			m_members.erase(memberSpec);

			GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, gmi->m_rank, title, gmi->m_allegiance, memberSpec);
			m_members.insert(memberSpec);

			GuildMemberInfo updatedGmi(gmi);
			updatedGmi.m_title = title;
			m_membersInfo.set(std::make_pair(guildId, memberId), updatedGmi);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGuildMemberAllegiance(int guildId, NetworkId const &memberId, NetworkId const &allegiance)
{
	GuildMemberInfo const * const gmi = getGuildMemberInfo(guildId, memberId);
	if (gmi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetMemberAllegiance,
					0.0f,
					new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, NetworkId> >(std::make_pair(std::make_pair(guildId, memberId), allegiance)),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else if (gmi->m_allegiance != allegiance)
		{
			std::string memberSpec;
			GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, gmi->m_rank, gmi->m_title, gmi->m_allegiance, memberSpec);
			m_members.erase(memberSpec);

			GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, gmi->m_rank, gmi->m_title, allegiance, memberSpec);
			m_members.insert(memberSpec);

			GuildMemberInfo updatedGmi(gmi);
			updatedGmi.m_allegiance = allegiance;
			m_membersInfo.set(std::make_pair(guildId, memberId), updatedGmi);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGuildMemberPermissionAndAllegiance(int guildId, NetworkId const &memberId, int permissions, NetworkId const &allegiance)
{
	GuildInfo const * const gi = getGuildInfo(guildId);
	GuildMemberInfo const * const gmi = getGuildMemberInfo(guildId, memberId);
	if (gi && gmi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetMemberPermissionAndAllegiance,
					0.0f,
					new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<int, NetworkId> > >(std::make_pair(std::make_pair(guildId, memberId), std::make_pair(permissions, allegiance))),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else if ((gmi->m_permissions != permissions) || (gmi->m_allegiance != allegiance))
		{
			std::string memberSpec;
			GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, gmi->m_rank, gmi->m_title, gmi->m_allegiance, memberSpec);
			m_members.erase(memberSpec);

			// if the existing guild member is already a guild member (i.e. not sponsored),
			// absolutely, positively make sure  that the new permissions contain
			// GuildPermissions::Member; once this permission has been set, it can
			// never be removed; the guild member should be removed from the guild
			// if you really want to remove the GuildPermissions::Member permission
			if (gmi->m_permissions & GuildInterface::Member)
				permissions |= GuildInterface::Member;

			memberSpec.clear();
			GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, permissions, gmi->m_rank, gmi->m_title, allegiance, memberSpec);
			m_members.insert(memberSpec);

			if (gmi->m_permissions != permissions)
			{
				GuildInfo updatedGi(gi);
				updateGuildInfoMembersCount(updatedGi, &gmi->m_permissions, &permissions);
				m_guildsInfo.set(guildId, updatedGi);

				// handle moving from the member from the sponsored list to the member
				// list if the permission has been changed from sponsored to member
				if ((gmi->m_permissions == GuildInterface::None) && (permissions & GuildInterface::Member))
				{
					std::map<NetworkId, int>::const_iterator const iterFind = m_sponsoredMembers.find(memberId);
					if ((iterFind != m_sponsoredMembers.end()) && (iterFind->second == guildId))
						m_sponsoredMembers.erase(memberId);

					m_fullMembers.set(memberId, guildId);
				}
			}

			GuildMemberInfo updatedGmi(gmi);
			updatedGmi.m_permissions = permissions;
			updatedGmi.m_allegiance = allegiance;
			m_membersInfo.set(std::make_pair(guildId, memberId), updatedGmi);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGuildMemberNameAndPermision(int guildId, NetworkId const &memberId, std::string const &memberName, int permissions)
{
	GuildInfo const * const gi = getGuildInfo(guildId);
	GuildMemberInfo const * const gmi = getGuildMemberInfo(guildId, memberId);
	if (gi && gmi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetMemberNameAndPermission,
					0.0f,
					new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > >(std::make_pair(std::make_pair(guildId, memberId), std::make_pair(memberName, permissions))),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else if ((gmi->m_name != memberName) || (gmi->m_permissions != permissions))
		{
			std::string memberSpec;
			GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, gmi->m_rank, gmi->m_title, gmi->m_allegiance, memberSpec);
			m_members.erase(memberSpec);

			// if the existing guild member is already a guild member (i.e. not sponsored),
			// absolutely, positively make sure  that the new permissions contain
			// GuildPermissions::Member; once this permission has been set, it can
			// never be removed; the guild member should be removed from the guild
			// if you really want to remove the GuildPermissions::Member permission
			if (gmi->m_permissions & GuildInterface::Member)
				permissions |= GuildInterface::Member;

			GuildStringParser::buildMemberSpec(guildId, memberId, memberName, gmi->m_professionSkillTemplate, gmi->m_level, permissions, gmi->m_rank, gmi->m_title, gmi->m_allegiance, memberSpec);
			m_members.insert(memberSpec);

			if (gmi->m_permissions != permissions)
			{
				GuildInfo updatedGi(gi);
				updateGuildInfoMembersCount(updatedGi, &gmi->m_permissions, &permissions);
				m_guildsInfo.set(guildId, updatedGi);

				// handle moving from the member from the sponsored list to the member
				// list if the permission has been changed from sponsored to member
				if ((gmi->m_permissions == GuildInterface::None) && (permissions & GuildInterface::Member))
				{
					std::map<NetworkId, int>::const_iterator const iterFind = m_sponsoredMembers.find(memberId);
					if ((iterFind != m_sponsoredMembers.end()) && (iterFind->second == guildId))
						m_sponsoredMembers.erase(memberId);

					m_fullMembers.set(memberId, guildId);
				}
			}

			GuildMemberInfo updatedGmi(gmi);
			updatedGmi.m_name = memberName;
			updatedGmi.m_permissions = permissions;
			m_membersInfo.set(std::make_pair(guildId, memberId), updatedGmi);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGuildMemberProfessionInfo(int guildId, NetworkId const &memberId, std::string const &memberProfessionSkillTemplate, int memberLevel)
{
	GuildMemberInfo const * const gmi = getGuildMemberInfo(guildId, memberId);
	if (gmi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetMemberProfessionInfo,
					0.0f,
					new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > >(std::make_pair(std::make_pair(guildId, memberId), std::make_pair(memberProfessionSkillTemplate, memberLevel))),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else if ((gmi->m_professionSkillTemplate != memberProfessionSkillTemplate) || (gmi->m_level != memberLevel))
		{			
			std::string memberSpec;
			GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, gmi->m_rank, gmi->m_title, gmi->m_allegiance, memberSpec);
			m_members.erase(memberSpec);

			GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, memberProfessionSkillTemplate, memberLevel, gmi->m_permissions, gmi->m_rank, gmi->m_title, gmi->m_allegiance, memberSpec);
			m_members.insert(memberSpec);

			GuildMemberInfo updatedGmi(gmi);
			updatedGmi.m_professionSkillTemplate = memberProfessionSkillTemplate;
			updatedGmi.m_level = memberLevel;
			m_membersInfo.set(std::make_pair(guildId, memberId), updatedGmi);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::addGuildMemberRank(int guildId, NetworkId const &memberId, std::string const &rankName)
{
	GuildMemberInfo const * const gmi = getGuildMemberInfo(guildId, memberId);
	if (gmi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildAddMemberRank,
					0.0f,
					new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > >(std::make_pair(rankName, std::make_pair(memberId, guildId))),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else
		{
			GuildRankDataTable::GuildRank const * rankData = GuildRankDataTable::getRank(rankName);
			if (rankData && !gmi->m_rank.testBit(rankData->slotId))
			{
				std::string memberSpec;
				GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, gmi->m_rank, gmi->m_title, gmi->m_allegiance, memberSpec);
				m_members.erase(memberSpec);

				BitArray rank = gmi->m_rank;
				rank.setBit(rankData->slotId);
				GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, rank, gmi->m_title, gmi->m_allegiance, memberSpec);
				m_members.insert(memberSpec);

				GuildMemberInfo updatedGmi(gmi);
				updatedGmi.m_rank = rank;
				m_membersInfo.set(std::make_pair(guildId, memberId), updatedGmi);
			}
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::removeGuildMemberRank(int guildId, NetworkId const &memberId, std::string const &rankName)
{
	GuildMemberInfo const * const gmi = getGuildMemberInfo(guildId, memberId);
	if (gmi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildRemoveMemberRank,
					0.0f,
					new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > >(std::make_pair(rankName, std::make_pair(memberId, guildId))),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else
		{
			GuildRankDataTable::GuildRank const * rankData = GuildRankDataTable::getRank(rankName);
			if (rankData && gmi->m_rank.testBit(rankData->slotId))
			{
				std::string memberSpec;
				GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, gmi->m_rank, gmi->m_title, gmi->m_allegiance, memberSpec);
				m_members.erase(memberSpec);

				BitArray rank = gmi->m_rank;
				rank.clearBit(rankData->slotId);
				GuildStringParser::buildMemberSpec(guildId, memberId, gmi->m_name, gmi->m_professionSkillTemplate, gmi->m_level, gmi->m_permissions, rank, gmi->m_title, gmi->m_allegiance, memberSpec);
				m_members.insert(memberSpec);

				GuildMemberInfo updatedGmi(gmi);
				updatedGmi.m_rank = rank;
				m_membersInfo.set(std::make_pair(guildId, memberId), updatedGmi);
			}
		}
	}
}

// ----------------------------------------------------------------------

bool GuildObject::hasGuildMemberRank(int guildId, NetworkId const &memberId, std::string const &rankName)
{
	GuildRankDataTable::GuildRank const * rankData = GuildRankDataTable::getRank(rankName);
	GuildMemberInfo const * const gmi = getGuildMemberInfo(guildId, memberId);
	return (rankData && gmi && gmi->m_rank.testBit(rankData->slotId));
}

// ----------------------------------------------------------------------

void GuildObject::getGuildMemberRank(int guildId, NetworkId const &memberId, std::vector<std::string> &ranks)
{
	ranks.clear();

	GuildMemberInfo const * const gmi = getGuildMemberInfo(guildId, memberId);
	if (!gmi)
		return;

	std::vector<GuildRankDataTable::GuildRank const *> const & allRanks = GuildRankDataTable::getAllRanks();
	for (std::vector<GuildRankDataTable::GuildRank const *>::const_iterator iter = allRanks.begin(); iter != allRanks.end(); ++iter)
	{
		if (gmi->m_rank.testBit((*iter)->slotId))
			ranks.push_back((*iter)->name);
	}
}

// ----------------------------------------------------------------------

void GuildObject::removeGuildEnemy(int guildId, int enemyId)
{
	if (GuildInterface::hasDeclaredWarAgainst(guildId, enemyId))
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildRemoveEnemy,
					0.0f,
					new MessageQueueGenericValueType<std::pair<int, int> >(std::make_pair(guildId, enemyId)),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else
		{
			std::pair<int, time_t> const & killInfo = GuildInterface::getGuildEnemyKillInfo(guildId, enemyId);
			std::string enemySpec;
			GuildStringParser::buildEnemySpec(guildId, enemyId, killInfo.first, (int)killInfo.second, enemySpec);
			m_enemies.erase(enemySpec);

			// if the other build B has declared war on us, clear the
			// B -> A kill count as we are no longer at war with guild B
			std::pair<int, time_t> const * killInfoBtoA = GuildInterface::hasDeclaredWarAgainst(enemyId, guildId);
			if (killInfoBtoA)
			{
				if (killInfoBtoA->first > 0)
				{
					GuildStringParser::buildEnemySpec(enemyId, guildId, killInfoBtoA->first, (int)killInfoBtoA->second, enemySpec);
					m_enemies.erase(enemySpec);

					GuildStringParser::buildEnemySpec(enemyId, guildId, 0, 0, enemySpec);
					m_enemies.insert(enemySpec);
				}

				// if there's at least 1 kill in the guild war, update ended guild war tracking data
				if ((killInfoBtoA->first > 0) || (killInfo.first > 0))
					GuildInterface::updateInactiveGuildWarTrackingInfo(*this, guildId, killInfo.first, enemyId, killInfoBtoA->first);
			}

			// remove any pending kill count updates for the 2 guilds
			m_guildWarKillTrackingAdjustment.erase(std::make_pair(guildId, enemyId));
			m_guildWarKillTrackingAdjustment.erase(std::make_pair(enemyId, guildId));
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::modifyGuildWarKillTracking(int killerGuildId, int victimGuildId, int adjustment, int updateTime)
{
	if (   !GuildInterface::hasDeclaredWarAgainst(killerGuildId, victimGuildId)
		|| !GuildInterface::hasDeclaredWarAgainst(victimGuildId, killerGuildId))
	{
		return;
	}

	if (updateTime <= 0)
		updateTime = static_cast<int>(::time(nullptr));

	// queue up adjustments and periodically update the data
	std::pair<std::map<std::pair<int, int>, std::pair<int, int> >::iterator, bool> result = m_guildWarKillTrackingAdjustment.insert(std::make_pair(std::make_pair(killerGuildId, victimGuildId), std::make_pair(adjustment, updateTime)));
	if (!result.second)
	{
		result.first->second.first += adjustment;
		result.first->second.second = std::max(result.first->second.second, updateTime);
	}

	// non-auth game servers send their updates to auth game server in/every 10 seconds;
	// auth game server updates the universe data in/every 60 seconds
	if (m_nextGuildWarKillTrackingAdjustmentUpdate == 0)
		m_nextGuildWarKillTrackingAdjustmentUpdate = ServerClock::getInstance().getGameTimeSeconds() + (isAuthoritative() ? 60 : 10);
}

// ----------------------------------------------------------------------

void GuildObject::updateGuildWarKillTrackingData()
{
	if (m_nextGuildWarKillTrackingAdjustmentUpdate == 0)
		return;

	if (ServerClock::getInstance().getGameTimeSeconds() <= m_nextGuildWarKillTrackingAdjustmentUpdate)
		return;

	if (!isAuthoritative())
	{
		// send updates to auth game server
		Controller *controller = getController();
		if (controller)
		{
			for (std::map<std::pair<int, int>, std::pair<int, int> >::const_iterator iter = m_guildWarKillTrackingAdjustment.begin(); iter != m_guildWarKillTrackingAdjustment.end(); ++iter)
			{
				controller->appendMessage(
					CM_guildUpdateGuildWarKillTrackingData,
					0.0f,
					new MessageQueueGenericValueType<std::pair<std::pair<int, int>, std::pair<int, int> > >(std::make_pair(std::make_pair(iter->first.first, iter->first.second), std::make_pair(iter->second.first, iter->second.second))),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
	}
	else
	{
		std::string enemySpec;
		for (std::map<std::pair<int, int>, std::pair<int, int> >::const_iterator iter = m_guildWarKillTrackingAdjustment.begin(); iter != m_guildWarKillTrackingAdjustment.end(); ++iter)
		{
			if (   GuildInterface::hasDeclaredWarAgainst(iter->first.first, iter->first.second)
				&& GuildInterface::hasDeclaredWarAgainst(iter->first.second, iter->first.first))
			{
				std::pair<int, time_t> const & killInfo = GuildInterface::getGuildEnemyKillInfo(iter->first.first, iter->first.second);
				GuildStringParser::buildEnemySpec(iter->first.first, iter->first.second, killInfo.first, (int)killInfo.second, enemySpec);
				m_enemies.erase(enemySpec);

				GuildStringParser::buildEnemySpec(iter->first.first, iter->first.second, killInfo.first + iter->second.first, (int)std::max(killInfo.second, static_cast<time_t>(iter->second.second)), enemySpec);
				m_enemies.insert(enemySpec);
			}
		}
	}

	m_guildWarKillTrackingAdjustment.clear();
	m_nextGuildWarKillTrackingAdjustmentUpdate = 0;
}

// ----------------------------------------------------------------------

void GuildObject::setGuildEnemy(int guildId, int enemyId)
{
	if (!GuildInterface::hasDeclaredWarAgainst(guildId, enemyId))
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetEnemy,
					0.0f,
					new MessageQueueGenericValueType<std::pair<int, int> >(std::make_pair(guildId, enemyId)),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else
		{
			std::string enemySpec;
			GuildStringParser::buildEnemySpec(guildId, enemyId, 0, 0, enemySpec);
			m_enemies.insert(enemySpec);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGuildElectionEndTime(int guildId, int electionPreviousEndTime, int electionNextEndTime)
{
	GuildInfo const * const gi = getGuildInfo(guildId);
	if (gi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetGuildElectionEndTime,
					0.0f,
					new MessageQueueGenericValueType<std::pair<std::pair<int, int>, int> >(std::make_pair(std::make_pair(electionPreviousEndTime, electionNextEndTime), guildId)),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else if ((gi->m_guildElectionPreviousEndTime != electionPreviousEndTime) || (gi->m_guildElectionNextEndTime != electionNextEndTime))
		{
			std::string oldNameSpec, newNameSpec;
			GuildStringParser::buildNameSpec(guildId, gi->m_name, gi->m_guildElectionPreviousEndTime, gi->m_guildElectionNextEndTime, gi->m_guildFaction, gi->m_timeLeftGuildFaction, gi->m_guildGcwDefenderRegion, gi->m_timeJoinedGuildGcwDefenderRegion, gi->m_timeLeftGuildGcwDefenderRegion, oldNameSpec);
			GuildStringParser::buildNameSpec(guildId, gi->m_name, electionPreviousEndTime, electionNextEndTime, gi->m_guildFaction, gi->m_timeLeftGuildFaction, gi->m_guildGcwDefenderRegion, gi->m_timeJoinedGuildGcwDefenderRegion, gi->m_timeLeftGuildGcwDefenderRegion, newNameSpec);
			m_names.erase(oldNameSpec);
			m_names.insert(newNameSpec);

			GuildInfo updatedGi(gi);
			updatedGi.m_guildElectionPreviousEndTime = electionPreviousEndTime;
			updatedGi.m_guildElectionNextEndTime = electionNextEndTime;
			m_guildsInfo.set(guildId, updatedGi);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGuildFaction(int guildId, uint32 guildFaction)
{
	GuildInfo const * const gi = getGuildInfo(guildId);
	if (gi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetGuildFaction,
					0.0f,
					new MessageQueueGenericValueType<std::pair<int, int> >(std::make_pair(guildId, static_cast<int>(guildFaction))),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else
		{
			bool factionChange = false;
			bool const isCurrentlyNeutral = ((gi->m_timeLeftGuildFaction > 0) || PvpData::isNeutralFactionId(gi->m_guildFaction));

			if (isCurrentlyNeutral)
			{
				if (!PvpData::isNeutralFactionId(guildFaction))
					factionChange = true;
			}
			else if (gi->m_guildFaction != guildFaction)
			{
				factionChange = true;
			}

			if (factionChange)
			{
				int const timeLeftGuildFaction = static_cast<int>(::time(nullptr));

				std::string oldNameSpec, newNameSpec;
				GuildStringParser::buildNameSpec(guildId, gi->m_name, gi->m_guildElectionPreviousEndTime, gi->m_guildElectionNextEndTime, gi->m_guildFaction, gi->m_timeLeftGuildFaction, gi->m_guildGcwDefenderRegion, gi->m_timeJoinedGuildGcwDefenderRegion, gi->m_timeLeftGuildGcwDefenderRegion, oldNameSpec);
				GuildStringParser::buildNameSpec(guildId, gi->m_name, gi->m_guildElectionPreviousEndTime, gi->m_guildElectionNextEndTime, (PvpData::isNeutralFactionId(guildFaction) ? gi->m_guildFaction : guildFaction), ((PvpData::isNeutralFactionId(guildFaction) && !PvpData::isNeutralFactionId(gi->m_guildFaction)) ? timeLeftGuildFaction : 0), gi->m_guildGcwDefenderRegion, gi->m_timeJoinedGuildGcwDefenderRegion, gi->m_timeLeftGuildGcwDefenderRegion, newNameSpec);
				m_names.erase(oldNameSpec);
				m_names.insert(newNameSpec);

				GuildInfo updatedGi(gi);
				updatedGi.m_guildFaction = (PvpData::isNeutralFactionId(guildFaction) ? gi->m_guildFaction : guildFaction);
				updatedGi.m_timeLeftGuildFaction = ((PvpData::isNeutralFactionId(guildFaction) && !PvpData::isNeutralFactionId(gi->m_guildFaction)) ? timeLeftGuildFaction : 0);

				// guild faction has changed, send mail to guild members
				{
					static const Unicode::String subject = Unicode::narrowToWide("@" + StringId("guild", "mail_subject_guild_faction_changed").getCanonicalRepresentation());

					ProsePackage pp;
					pp.stringId = StringId("guild", "mail_body_guild_faction_changed");
					pp.actor.str = Unicode::narrowToWide(updatedGi.m_name);
					pp.target.str = Unicode::narrowToWide(getFactionString(isCurrentlyNeutral ? PvpData::getNeutralFactionId() : gi->m_guildFaction));
					pp.other.str = Unicode::narrowToWide(getFactionString(guildFaction));

					Unicode::String oob;
					OutOfBandPackager::pack(pp, -1, oob);

					GuildInterface::mailToGuild(guildId, subject, Unicode::emptyString, oob);
				}

				if (!updatedGi.m_guildGcwDefenderRegion.empty() && (updatedGi.m_timeLeftGuildGcwDefenderRegion <= 0))
				{
					removeGcwRegionDefenderGuild(updatedGi.m_guildGcwDefenderRegion, guildId, (isCurrentlyNeutral ? PvpData::getNeutralFactionId() : gi->m_guildFaction));
					addGcwRegionDefenderGuild(updatedGi.m_guildGcwDefenderRegion, guildId, guildFaction);
				}

				m_guildsInfo.set(guildId, updatedGi);
			}
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGuildGcwDefenderRegion(int guildId, std::string const &guildGcwDefenderRegion)
{
	if (!guildGcwDefenderRegion.empty())
	{
		Pvp::GcwScoreCategory const * const gcwScoreCategory = Pvp::getGcwScoreCategory(guildGcwDefenderRegion);
		if (!gcwScoreCategory || !gcwScoreCategory->gcwRegionDefender)
			return;
	}

	GuildInfo const * const gi = getGuildInfo(guildId);
	if (gi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetGuildGcwDefenderRegion,
					0.0f,
					new MessageQueueGenericValueType<std::pair<int, std::string> >(std::make_pair(guildId, guildGcwDefenderRegion)),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else
		{
			// cannot be a GCW region defender if doesn't meet minimum guild member count requirement
			if (!guildGcwDefenderRegion.empty() && (gi->getCountMembersOnly() < ConfigServerGame::getGcwGuildMinMembersForGcwRegionDefender()))
				return;

			bool regionChange = false;
			bool const isCurrentlyNoRegion = ((gi->m_timeLeftGuildGcwDefenderRegion > 0) || gi->m_guildGcwDefenderRegion.empty());

			int timeJoinedGuildGcwDefenderRegion = 0;
			int timeLeftGuildGcwDefenderRegion = 0;
			if (isCurrentlyNoRegion)
			{
				if (!guildGcwDefenderRegion.empty())
				{
					regionChange = true;

					if ((gi->m_guildGcwDefenderRegion == guildGcwDefenderRegion) && (gi->m_timeJoinedGuildGcwDefenderRegion > 0))
						timeJoinedGuildGcwDefenderRegion = gi->m_timeJoinedGuildGcwDefenderRegion;
					else
						timeJoinedGuildGcwDefenderRegion = static_cast<int>(::time(nullptr));
				}
			}
			else if (gi->m_guildGcwDefenderRegion != guildGcwDefenderRegion)
			{
				regionChange = true;

				if (!guildGcwDefenderRegion.empty())
				{
					timeJoinedGuildGcwDefenderRegion = static_cast<int>(::time(nullptr));
				}
				else
				{
					// stop defending
					timeJoinedGuildGcwDefenderRegion = gi->m_timeJoinedGuildGcwDefenderRegion;
					timeLeftGuildGcwDefenderRegion = static_cast<int>(::time(nullptr));
				}
			}

			if (regionChange)
			{
				std::string oldNameSpec, newNameSpec;
				GuildStringParser::buildNameSpec(guildId, gi->m_name, gi->m_guildElectionPreviousEndTime, gi->m_guildElectionNextEndTime, gi->m_guildFaction, gi->m_timeLeftGuildFaction, gi->m_guildGcwDefenderRegion, gi->m_timeJoinedGuildGcwDefenderRegion, gi->m_timeLeftGuildGcwDefenderRegion, oldNameSpec);
				GuildStringParser::buildNameSpec(guildId, gi->m_name, gi->m_guildElectionPreviousEndTime, gi->m_guildElectionNextEndTime, gi->m_guildFaction, gi->m_timeLeftGuildFaction, (guildGcwDefenderRegion.empty() ? gi->m_guildGcwDefenderRegion : guildGcwDefenderRegion), timeJoinedGuildGcwDefenderRegion, timeLeftGuildGcwDefenderRegion, newNameSpec);
				m_names.erase(oldNameSpec);
				m_names.insert(newNameSpec);

				GuildInfo updatedGi(gi);
				updatedGi.m_guildGcwDefenderRegion = (guildGcwDefenderRegion.empty() ? gi->m_guildGcwDefenderRegion : guildGcwDefenderRegion);
				updatedGi.m_timeJoinedGuildGcwDefenderRegion = timeJoinedGuildGcwDefenderRegion;
				updatedGi.m_timeLeftGuildGcwDefenderRegion = timeLeftGuildGcwDefenderRegion;

				// guild gcw defender region has changed, send mail to guild members
				{
					static const Unicode::String subject = Unicode::narrowToWide("@" + StringId("guild", "mail_subject_guild_gcw_defender_region_changed").getCanonicalRepresentation());

					ProsePackage pp;
					pp.stringId = StringId("guild", "mail_body_guild_gcw_defender_region_changed");
					pp.actor.str = Unicode::narrowToWide(updatedGi.m_name);
					pp.target.str = Unicode::narrowToWide(isCurrentlyNoRegion ? "(None)" : ("@" + StringId("gcw_regions", gi->m_guildGcwDefenderRegion).getCanonicalRepresentation()));
					pp.other.str = Unicode::narrowToWide(guildGcwDefenderRegion.empty() ? "(None)" : ("@" + StringId("gcw_regions", guildGcwDefenderRegion).getCanonicalRepresentation()));

					Unicode::String oob;
					OutOfBandPackager::pack(pp, -1, oob);

					GuildInterface::mailToGuild(guildId, subject, Unicode::emptyString, oob);
				}

				removeGcwRegionDefenderGuild((isCurrentlyNoRegion ? std::string() : gi->m_guildGcwDefenderRegion), guildId, (((updatedGi.m_timeLeftGuildFaction > 0) || PvpData::isNeutralFactionId(updatedGi.m_guildFaction)) ? PvpData::getNeutralFactionId() : updatedGi.m_guildFaction));
				addGcwRegionDefenderGuild(guildGcwDefenderRegion, guildId, (((updatedGi.m_timeLeftGuildFaction > 0) || PvpData::isNeutralFactionId(updatedGi.m_guildFaction)) ? PvpData::getNeutralFactionId() : updatedGi.m_guildFaction));

				m_guildsInfo.set(guildId, updatedGi);
			}
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGuildLeader(int guildId, NetworkId const &leaderId)
{
	if (!leaderId.isValid())
		return;

	GuildInfo const * const gi = getGuildInfo(guildId);
	if (!gi || (gi->m_leaderId != leaderId))
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetLeader,
					0.0f,
					new MessageQueueGenericValueType<std::pair<int, NetworkId> >(std::make_pair(guildId, leaderId)),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else if (gi && (gi->m_leaderId != leaderId))
		{
			std::string oldLeaderSpec, newLeaderSpec;
			GuildStringParser::buildLeaderSpec(guildId, gi->m_leaderId, oldLeaderSpec);
			GuildStringParser::buildLeaderSpec(guildId, leaderId, newLeaderSpec);
			m_leaders.erase(oldLeaderSpec);
			m_leaders.insert(newLeaderSpec);

			std::map<NetworkId, int>::const_iterator iterGuildLeader = m_guildLeaders.find(gi->m_leaderId);
			if ((iterGuildLeader != m_guildLeaders.end()) && (iterGuildLeader->second == guildId))
				m_guildLeaders.erase(gi->m_leaderId);

			m_guildLeaders.set(leaderId, guildId);

			GuildInfo updatedGi(gi);
			updatedGi.m_leaderId = leaderId;
			m_guildsInfo.set(guildId, updatedGi);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGuildName(int guildId, std::string const &newName)
{
	if (newName.empty())
		return;

	GuildInfo const * const gi = getGuildInfo(guildId);
	if (gi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetName,
					0.0f,
					new MessageQueueGenericValueType<std::pair<int, std::string> >(std::make_pair(guildId, newName)),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else if (gi->m_name != newName)
		{
			std::string oldNameSpec, newNameSpec;
			GuildStringParser::buildNameSpec(guildId, gi->m_name, gi->m_guildElectionPreviousEndTime, gi->m_guildElectionNextEndTime, gi->m_guildFaction, gi->m_timeLeftGuildFaction, gi->m_guildGcwDefenderRegion, gi->m_timeJoinedGuildGcwDefenderRegion, gi->m_timeLeftGuildGcwDefenderRegion, oldNameSpec);
			GuildStringParser::buildNameSpec(guildId, newName, gi->m_guildElectionPreviousEndTime, gi->m_guildElectionNextEndTime, gi->m_guildFaction, gi->m_timeLeftGuildFaction, gi->m_guildGcwDefenderRegion, gi->m_timeJoinedGuildGcwDefenderRegion, gi->m_timeLeftGuildGcwDefenderRegion, newNameSpec);
			m_names.erase(oldNameSpec);
			m_names.insert(newNameSpec);

			GuildInfo updatedGi(gi);
			updatedGi.m_name = newName;
			m_guildsInfo.set(guildId, updatedGi);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGuildAbbrev(int guildId, std::string const &newAbbrev)
{
	if (newAbbrev.empty())
		return;

	GuildInfo const * const gi = getGuildInfo(guildId);
	if (gi)
	{
		if (!isAuthoritative())
		{
			Controller *controller = getController();
			if (controller)
			{
				controller->appendMessage(
					CM_guildSetAbbrev,
					0.0f,
					new MessageQueueGenericValueType<std::pair<int, std::string> >(std::make_pair(guildId, newAbbrev)),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
		else if (gi->m_abbrev != newAbbrev)
		{
			std::string oldAbbrevSpec, newAbbrevSpec;
			GuildStringParser::buildAbbrevSpec(guildId, gi->m_abbrev, oldAbbrevSpec);
			GuildStringParser::buildAbbrevSpec(guildId, newAbbrev, newAbbrevSpec);
			m_abbrevs.erase(oldAbbrevSpec);
			m_abbrevs.insert(newAbbrevSpec);

			GuildInfo updatedGi(gi);
			updatedGi.m_abbrev = newAbbrev;
			m_guildsInfo.set(guildId, updatedGi);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::calculateGcwImperialScorePercentileForSwg()
{
	if (!isAuthoritative())
		return;

	if (m_gcwImperialScoreOtherGalaxies.empty() && m_gcwRebelScoreOtherGalaxies.empty())
		return;

	PlanetObject const * tatooine = ServerUniverse::getInstance().getTatooinePlanet();
	if (!tatooine || !tatooine->isAuthoritative())
		return;

	std::map<std::string, std::pair<uint64, uint64> > gcwImperialScoreForSwg;
	{
		std::pair<std::map<std::string, std::pair<uint64, uint64> >::iterator, bool> insertResult;

		std::map<std::string, std::pair<int64, int64> > const & gcwImperialScoreThisGalaxy = tatooine->getGcwImperialScore();
		for (std::map<std::string, std::pair<int64, int64> >::const_iterator iterThisGalaxy = gcwImperialScoreThisGalaxy.begin(), iterThisGalaxyEnd = gcwImperialScoreThisGalaxy.end(); iterThisGalaxy != iterThisGalaxyEnd; ++iterThisGalaxy)
		{
			insertResult = gcwImperialScoreForSwg.insert(std::make_pair(iterThisGalaxy->first, std::make_pair(static_cast<uint64>(iterThisGalaxy->second.first), static_cast<uint64>(iterThisGalaxy->second.second))));
			if (!insertResult.second)
			{
				insertResult.first->second.first += static_cast<uint64>(iterThisGalaxy->second.first);
				insertResult.first->second.second += static_cast<uint64>(iterThisGalaxy->second.second);
			}
		}

		for (std::map<std::pair<std::string, std::string>, std::pair<int64, int64> >::const_iterator iterOtherGalaxies = m_gcwImperialScoreOtherGalaxies.begin(), iterOtherGalaxiesEnd = m_gcwImperialScoreOtherGalaxies.end(); iterOtherGalaxies != iterOtherGalaxiesEnd; ++iterOtherGalaxies)
		{
			insertResult = gcwImperialScoreForSwg.insert(std::make_pair(iterOtherGalaxies->first.second, std::make_pair(static_cast<uint64>(iterOtherGalaxies->second.first), static_cast<uint64>(iterOtherGalaxies->second.second))));
			if (!insertResult.second)
			{
				insertResult.first->second.first += static_cast<uint64>(iterOtherGalaxies->second.first);
				insertResult.first->second.second += static_cast<uint64>(iterOtherGalaxies->second.second);
			}
		}
	}

	std::map<std::string, std::pair<uint64, uint64> > gcwRebelScoreForSwg;
	{
		std::pair<std::map<std::string, std::pair<uint64, uint64> >::iterator, bool> insertResult;

		std::map<std::string, std::pair<int64, int64> > const & gcwRebelScoreThisGalaxy = tatooine->getGcwRebelScore();
		for (std::map<std::string, std::pair<int64, int64> >::const_iterator iterThisGalaxy = gcwRebelScoreThisGalaxy.begin(), iterThisGalaxyEnd = gcwRebelScoreThisGalaxy.end(); iterThisGalaxy != iterThisGalaxyEnd; ++iterThisGalaxy)
		{
			insertResult = gcwRebelScoreForSwg.insert(std::make_pair(iterThisGalaxy->first, std::make_pair(static_cast<uint64>(iterThisGalaxy->second.first), static_cast<uint64>(iterThisGalaxy->second.second))));
			if (!insertResult.second)
			{
				insertResult.first->second.first += static_cast<uint64>(iterThisGalaxy->second.first);
				insertResult.first->second.second += static_cast<uint64>(iterThisGalaxy->second.second);
			}
		}

		for (std::map<std::pair<std::string, std::string>, std::pair<int64, int64> >::const_iterator iterOtherGalaxies = m_gcwRebelScoreOtherGalaxies.begin(), iterOtherGalaxiesEnd = m_gcwRebelScoreOtherGalaxies.end(); iterOtherGalaxies != iterOtherGalaxiesEnd; ++iterOtherGalaxies)
		{
			insertResult = gcwRebelScoreForSwg.insert(std::make_pair(iterOtherGalaxies->first.second, std::make_pair(static_cast<uint64>(iterOtherGalaxies->second.first), static_cast<uint64>(iterOtherGalaxies->second.second))));
			if (!insertResult.second)
			{
				insertResult.first->second.first += static_cast<uint64>(iterOtherGalaxies->second.first);
				insertResult.first->second.second += static_cast<uint64>(iterOtherGalaxies->second.second);
			}
		}
	}

	// GCW category
	std::map<std::string, int> gcwImperialScorePercentileForSwg;
	{
		int imperialScorePercentile;

		std::map<std::string, Pvp::GcwScoreCategory const *> const & allGcwScoreCategory = Pvp::getAllGcwScoreCategory(); 
		for (std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
		{
			imperialScorePercentile = Pvp::calculateGcwImperialScorePercentile(iter->first, gcwImperialScoreForSwg, gcwRebelScoreForSwg);
			m_gcwImperialScorePercentileOtherGalaxies.set(std::make_pair("SWG", iter->first), imperialScorePercentile);
			gcwImperialScorePercentileForSwg.insert(std::make_pair(iter->first, imperialScorePercentile));
		}
	}

	// GCW group
	{
		std::map<std::string, int> gcwGroupImperialScoreRawTotal;
		std::map<std::string, int>::iterator iterInsertionPoint;

		std::map<std::string, int>::const_iterator iterGcwImperialScorePercentileForSwg;

		std::map<std::string, std::map<std::string, int> > const & gcwScoreCategoryGroups = Pvp::getGcwScoreCategoryGroups();
		for (std::map<std::string, std::map<std::string, int> >::const_iterator iterGroup = gcwScoreCategoryGroups.begin(); iterGroup != gcwScoreCategoryGroups.end(); ++iterGroup)
		{
			iterInsertionPoint = gcwGroupImperialScoreRawTotal.insert(std::make_pair(iterGroup->first, 0)).first;

			for (std::map<std::string, int>::const_iterator iterCategory = iterGroup->second.begin(); iterCategory != iterGroup->second.end(); ++iterCategory)
			{
				iterGcwImperialScorePercentileForSwg = gcwImperialScorePercentileForSwg.find(iterCategory->first);
				if (iterGcwImperialScorePercentileForSwg == gcwImperialScorePercentileForSwg.end())
					continue;

				iterInsertionPoint->second += static_cast<int>(static_cast<int64>(iterGcwImperialScorePercentileForSwg->second) * static_cast<int64>(iterCategory->second) / static_cast<int64>(100));
			}
		}

		int64 scaledScore;
		int percentile;
		for (std::map<std::string, int>::const_iterator iterGcwGroupImperialScoreRawTotal = gcwGroupImperialScoreRawTotal.begin(); iterGcwGroupImperialScoreRawTotal != gcwGroupImperialScoreRawTotal.end(); ++iterGcwGroupImperialScoreRawTotal)
		{
			scaledScore = static_cast<int64>(iterGcwGroupImperialScoreRawTotal->second) * static_cast<int64>(100);
			percentile = static_cast<int>(scaledScore / static_cast<int64>(1000000000));

			// round up to 50% so that when there is no score, it will be a tie
			if ((percentile == 49) && ((scaledScore % static_cast<int64>(1000000000)) >= static_cast<int64>(500000000)))
				percentile = 50;

			// if the galaxies score is changing, CS log it
			if (iterGcwGroupImperialScoreRawTotal->first == "galaxy")
			{
				Archive::AutoDeltaMap<std::pair<std::string, std::string>, int>::const_iterator iterFind = m_gcwGroupImperialScorePercentileOtherGalaxies.find(std::make_pair("SWG", iterGcwGroupImperialScoreRawTotal->first));
				if ((iterFind != m_gcwGroupImperialScorePercentileOtherGalaxies.end()) && (iterFind->second != percentile))
					LOG("CustomerService", ("GcwScore: galaxy SWG imperial percentile change (%d -> %d)", iterFind->second, percentile));
			}

			m_gcwGroupImperialScorePercentileOtherGalaxies.set(std::make_pair("SWG", iterGcwGroupImperialScoreRawTotal->first), percentile);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::depersistGcwImperialScorePercentile()
{
	if (!isAuthoritative())
		return;

	PlanetObject * const tatooine = ServerUniverse::getInstance().getTatooinePlanet();
	if (!tatooine || !tatooine->isAuthoritative())
		return;

	m_gcwImperialScorePercentileThisGalaxy.clear();
	m_gcwRegionDefenderBonus.clear();
	m_gcwGroupImperialScorePercentileThisGalaxy.clear();
	m_gcwImperialScorePercentileHistoryThisGalaxy.clear();
	m_gcwGroupImperialScorePercentileHistoryThisGalaxy.clear();
	m_gcwImperialScorePercentileHistoryCountThisGalaxy.clear();
	m_gcwGroupImperialScorePercentileHistoryCountThisGalaxy.clear();
	m_gcwGroupCategoryImperialScoreRawThisGalaxy.clear();
	m_gcwGroupImperialScoreRawThisGalaxy.clear();

	// GCW category
	{
		int const timeNow = static_cast<int>(::time(nullptr));
		int gcwImperialScorePercentile;
		std::map<std::string, Pvp::GcwScoreCategory const *> const & allGcwScoreCategory = Pvp::getAllGcwScoreCategory(); 
		for (std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
		{
			if (m_gcwImperialScorePercentileThisGalaxy.find(iter->first) == m_gcwImperialScorePercentileThisGalaxy.end())
			{
				gcwImperialScorePercentile = tatooine->getGcwImperialScorePercentile(iter->first);
				m_gcwImperialScorePercentileThisGalaxy.set(iter->first, gcwImperialScorePercentile);
				updateGcwPercentileHistory(m_gcwImperialScorePercentileHistoryThisGalaxy, m_gcwImperialScorePercentileHistoryCountThisGalaxy, iter->first, gcwImperialScorePercentile);
				updateGcwRegionDefenderBonus(iter->first);

				// if the percentile is at least 70% for either side, start
				// tracking how long the side has held the region at at least 70%
				if (gcwImperialScorePercentile >= 70)
				{
					if (!tatooine->getObjVars().hasItem(std::string("gcwScore.timeControlled.") + iter->first))
						IGNORE_RETURN(tatooine->setObjVarItem(std::string("gcwScore.timeControlled.") + iter->first, timeNow)); // imperial controlled
				}
				else if (gcwImperialScorePercentile <= 30)
				{
					if (!tatooine->getObjVars().hasItem(std::string("gcwScore.timeControlled.") + iter->first))
						IGNORE_RETURN(tatooine->setObjVarItem(std::string("gcwScore.timeControlled.") + iter->first, -timeNow)); // rebel controlled
				}
				else
				{
					tatooine->removeObjVarItem(std::string("gcwScore.timeControlled.") + iter->first);
				}
			}
		}
	}

	// GCW group
	{
		std::map<std::string, int> gcwGroupImperialScoreRawTotal;
		std::map<std::string, int>::iterator iterInsertionPoint;

		std::map<std::pair<std::string, std::string>, int>::const_iterator iterScoreRaw;
		int scoreRaw;

		std::map<std::string, std::map<std::string, int> > const & gcwScoreCategoryGroups = Pvp::getGcwScoreCategoryGroups();
		for (std::map<std::string, std::map<std::string, int> >::const_iterator iterGroup = gcwScoreCategoryGroups.begin(); iterGroup != gcwScoreCategoryGroups.end(); ++iterGroup)
		{
			iterInsertionPoint = gcwGroupImperialScoreRawTotal.insert(std::make_pair(iterGroup->first, 0)).first;

			for (std::map<std::string, int>::const_iterator iterCategory = iterGroup->second.begin(); iterCategory != iterGroup->second.end(); ++iterCategory)
			{
				iterScoreRaw = m_gcwGroupCategoryImperialScoreRawThisGalaxy.find(std::make_pair(iterGroup->first, iterCategory->first));
				if (iterScoreRaw == m_gcwGroupCategoryImperialScoreRawThisGalaxy.end())
				{
					scoreRaw = static_cast<int>(static_cast<int64>(getGcwImperialScorePercentile(iterCategory->first)) * static_cast<int64>(iterCategory->second) / static_cast<int64>(100));
					m_gcwGroupCategoryImperialScoreRawThisGalaxy.set(std::make_pair(iterGroup->first, iterCategory->first), scoreRaw);

					iterInsertionPoint->second += scoreRaw;
				}
				else
				{
					iterInsertionPoint->second += iterScoreRaw->second;
				}
			}
		}

		int64 scaledScore;
		int percentile;
		for (std::map<std::string, int>::const_iterator iterGcwGroupImperialScoreRawTotal = gcwGroupImperialScoreRawTotal.begin(); iterGcwGroupImperialScoreRawTotal != gcwGroupImperialScoreRawTotal.end(); ++iterGcwGroupImperialScoreRawTotal)
		{
			if (m_gcwGroupImperialScoreRawThisGalaxy.find(iterGcwGroupImperialScoreRawTotal->first) == m_gcwGroupImperialScoreRawThisGalaxy.end())
			{
				m_gcwGroupImperialScoreRawThisGalaxy.set(iterGcwGroupImperialScoreRawTotal->first, iterGcwGroupImperialScoreRawTotal->second);
			}

			if (m_gcwGroupImperialScorePercentileThisGalaxy.find(iterGcwGroupImperialScoreRawTotal->first) == m_gcwGroupImperialScorePercentileThisGalaxy.end())
			{
				scaledScore = static_cast<int64>(iterGcwGroupImperialScoreRawTotal->second) * static_cast<int64>(100);
				percentile = static_cast<int>(scaledScore / static_cast<int64>(1000000000));

				// round up to 50% so that when there is no score, it will be a tie
				if ((percentile == 49) && ((scaledScore % static_cast<int64>(1000000000)) >= static_cast<int64>(500000000)))
					percentile = 50;

				m_gcwGroupImperialScorePercentileThisGalaxy.set(iterGcwGroupImperialScoreRawTotal->first, percentile);
				updateGcwPercentileHistory(m_gcwGroupImperialScorePercentileHistoryThisGalaxy, m_gcwGroupImperialScorePercentileHistoryCountThisGalaxy, iterGcwGroupImperialScoreRawTotal->first, percentile);
			}
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::updateGcwImperialScorePercentile(std::set<std::string> const & gcwScoreCategories)
{
	if (!isAuthoritative())
		return;

	PlanetObject * const tatooine = ServerUniverse::getInstance().getTatooinePlanet();
	if (!tatooine || !tatooine->isAuthoritative())
		return;

	// if score hasn't been depersisted yet, do it now
	if (m_gcwImperialScorePercentileThisGalaxy.empty())
		depersistGcwImperialScorePercentile();

	int const timeNow = static_cast<int>(::time(nullptr));
	GameScriptObject * const gameScriptObject = tatooine->getScriptObject();
	int currentScorePercentile, newScorePercentile, newGroupCategoryScoreRaw, scoreCategoryGroupTotalPoints, deltaGroupCategoryScoreRaw;
	std::map<std::pair<std::string, std::string>, int>::const_iterator iterGroupCategoryScoreRaw;
	Pvp::GcwScoreCategory const * scoreCategory;
	std::map<std::string, int> groupImperialScoreRawDelta;
	std::map<std::string, int>::iterator iterGroupImperialScoreRawDelta;
	std::map<std::string, std::pair<int, int> > notifyOnPercentileChange;
	for (std::set<std::string>::const_iterator iter = gcwScoreCategories.begin(); iter != gcwScoreCategories.end(); ++iter)
	{
		scoreCategory = Pvp::getGcwScoreCategory(*iter);
		if (scoreCategory)
		{
			currentScorePercentile = getGcwImperialScorePercentile(*iter);
			newScorePercentile = tatooine->getGcwImperialScorePercentile(*iter);

			if (currentScorePercentile != newScorePercentile)
			{
				m_gcwImperialScorePercentileThisGalaxy.set(*iter, newScorePercentile);
				updateGcwPercentileHistory(m_gcwImperialScorePercentileHistoryThisGalaxy, m_gcwImperialScorePercentileHistoryCountThisGalaxy, *iter, newScorePercentile);
				updateGcwRegionDefenderBonus(*iter);

				// if the percentile is at least 70% for either side, start
				// tracking how long the side has held the region at at least 70%
				if (newScorePercentile >= 70)
				{
					if (currentScorePercentile < 70)
						IGNORE_RETURN(tatooine->setObjVarItem(std::string("gcwScore.timeControlled.") + *iter, timeNow)); // imperial controlled
				}
				else if (newScorePercentile <= 30)
				{
					if (currentScorePercentile > 30)
						IGNORE_RETURN(tatooine->setObjVarItem(std::string("gcwScore.timeControlled.") + *iter, -timeNow)); // rebel controlled
				}
				else
				{
					tatooine->removeObjVarItem(std::string("gcwScore.timeControlled.") + *iter);
				}

				// handles changes to the GCW group score for the
				// GCW groups that this GCW category belong to
				if (!scoreCategory->gcwScoreCategoryGroups.empty())
				{
					for (std::set<std::string>::const_iterator iterGroup = scoreCategory->gcwScoreCategoryGroups.begin(); iterGroup != scoreCategory->gcwScoreCategoryGroups.end(); ++iterGroup)
					{
						iterGroupCategoryScoreRaw = m_gcwGroupCategoryImperialScoreRawThisGalaxy.find(std::make_pair(*iterGroup, *iter));
						if (iterGroupCategoryScoreRaw == m_gcwGroupCategoryImperialScoreRawThisGalaxy.end())
							continue;

						if (!Pvp::getGcwScoreCategoryGroupTotalPoints(*iterGroup, *iter, scoreCategoryGroupTotalPoints))
							continue;

						newGroupCategoryScoreRaw = static_cast<int>(static_cast<int64>(newScorePercentile) * static_cast<int64>(scoreCategoryGroupTotalPoints) / static_cast<int64>(100));
						deltaGroupCategoryScoreRaw = newGroupCategoryScoreRaw - iterGroupCategoryScoreRaw->second;
						if (deltaGroupCategoryScoreRaw == 0)
							continue;

						iterGroupImperialScoreRawDelta = groupImperialScoreRawDelta.find(*iterGroup);
						if (iterGroupImperialScoreRawDelta != groupImperialScoreRawDelta.end())
						{
							iterGroupImperialScoreRawDelta->second += deltaGroupCategoryScoreRaw;
						}
						else
						{
							IGNORE_RETURN(groupImperialScoreRawDelta.insert(std::make_pair(*iterGroup, deltaGroupCategoryScoreRaw)));
						}

						m_gcwGroupCategoryImperialScoreRawThisGalaxy.set(std::make_pair(*iterGroup, *iter), newGroupCategoryScoreRaw);
					}
				}

				// if requested, notify script if the score changes
				if (scoreCategory->notifyOnPercentileChange && gameScriptObject)
				{
					notifyOnPercentileChange[*iter] = std::make_pair(currentScorePercentile, newScorePercentile);
				}
			}
		}
	}

	// update changes to GCW group score
	if (!groupImperialScoreRawDelta.empty())
	{
		std::map<std::string, int>::const_iterator iterFindGcwGroupImperialScoreRawThisGalaxy;
		int64 scaledScore;
		int newScore, percentile, oldPercentile;
		for (std::map<std::string, int>::const_iterator iterGroup = groupImperialScoreRawDelta.begin(); iterGroup != groupImperialScoreRawDelta.end(); ++iterGroup)
		{
			if (iterGroup->second == 0)
				continue;

			iterFindGcwGroupImperialScoreRawThisGalaxy = m_gcwGroupImperialScoreRawThisGalaxy.find(iterGroup->first);
			if (iterFindGcwGroupImperialScoreRawThisGalaxy == m_gcwGroupImperialScoreRawThisGalaxy.end())
				continue;

			newScore = iterFindGcwGroupImperialScoreRawThisGalaxy->second + iterGroup->second;
			m_gcwGroupImperialScoreRawThisGalaxy.set(iterGroup->first, newScore);

			scaledScore = static_cast<int64>(newScore) * static_cast<int64>(100);
			percentile = static_cast<int>(scaledScore / static_cast<int64>(1000000000));

			// round up to 50% so that when there is no score, it will be a tie
			if ((percentile == 49) && ((scaledScore % static_cast<int64>(1000000000)) >= static_cast<int64>(500000000)))
				percentile = 50;

			oldPercentile = getGcwGroupImperialScorePercentile(iterGroup->first);
			if (oldPercentile != percentile)
			{
				m_gcwGroupImperialScorePercentileThisGalaxy.set(iterGroup->first, percentile);
				updateGcwPercentileHistory(m_gcwGroupImperialScorePercentileHistoryThisGalaxy, m_gcwGroupImperialScorePercentileHistoryCountThisGalaxy, iterGroup->first, percentile);

				// if the galaxy score is changing, CS log it
				if (iterGroup->first == "galaxy")
					LOG("CustomerService", ("GcwScore: galaxy %s imperial percentile change (%d -> %d)", GameServer::getInstance().getClusterName().c_str(), oldPercentile, percentile));
			}
		}
	}

	// if requested, notify script if the score changes
	if (gameScriptObject && !notifyOnPercentileChange.empty())
	{
		for (std::map<std::string, std::pair<int, int> >::const_iterator iterNotify = notifyOnPercentileChange.begin(); iterNotify != notifyOnPercentileChange.end(); ++iterNotify)
		{
			ScriptParams sp;
			sp.addParam(iterNotify->first.c_str());
			sp.addParam(iterNotify->second.first);
			sp.addParam(iterNotify->second.second);

			IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_ON_GCW_SCORE_CATEGORY_PERCENTILE_CHANGE, sp));
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::updateGcwRegionDefenderBonus(std::string const & gcwScoreCategory)
{
	if (!isAuthoritative())
		return;

	// cannot use CityInterface::getGcwRegionDefenderCitiesCount() here
	// because it is not guaranteed to be available at the time this is called
	CityObject const * const co = ServerUniverse::getInstance().getMasterCityObject();
	int totalImperial = 0;
	int totalRebel = 0;

	if (co)
	{
		std::map<std::string, std::pair<int, int> > const & gcwRegionDefenderCitiesCount = co->getGcwRegionDefenderCitiesCount();
		std::map<std::string, std::pair<int, int> >::const_iterator iterFind = gcwRegionDefenderCitiesCount.find(gcwScoreCategory);
		if (iterFind != gcwRegionDefenderCitiesCount.end())
		{
			totalImperial += iterFind->second.first;
			totalRebel += iterFind->second.second;
		}
	}

	{
		std::map<std::string, std::pair<int, int> >::const_iterator iterFind = m_gcwRegionDefenderGuildsCount.find(gcwScoreCategory);
		if (iterFind != m_gcwRegionDefenderGuildsCount.end())
		{
			totalImperial += iterFind->second.first;
			totalRebel += iterFind->second.second;
		}
	}

	float const imperialBonus = static_cast<float>((static_cast<double>(ServerUniverse::getInstance().getGcwImperialScorePercentile(gcwScoreCategory)) * static_cast<double>(ConfigServerGame::getGcwRegionDefenderTotalBonusPct())) / (static_cast<double>(100) * static_cast<double>(std::max(1, totalImperial))));
	float const rebelBonus = static_cast<float>((static_cast<double>(100 - ServerUniverse::getInstance().getGcwImperialScorePercentile(gcwScoreCategory)) * static_cast<double>(ConfigServerGame::getGcwRegionDefenderTotalBonusPct())) / (static_cast<double>(100) * static_cast<double>(std::max(1, totalRebel))));

	m_gcwRegionDefenderBonus.set(gcwScoreCategory, std::make_pair(imperialBonus, rebelBonus));
}

// ----------------------------------------------------------------------

int GuildObject::getGcwImperialScorePercentile(std::string const & gcwCategory) const
{
	Archive::AutoDeltaMap<std::string, int>::const_iterator const iterFind = m_gcwImperialScorePercentileThisGalaxy.find(gcwCategory);
	if (iterFind != m_gcwImperialScorePercentileThisGalaxy.end())
	{
		return iterFind->second;
	}

	return 50; // tie
}

// ----------------------------------------------------------------------

int GuildObject::getGcwGroupImperialScorePercentile(std::string const & gcwGroup) const
{
	Archive::AutoDeltaMap<std::string, int>::const_iterator const iterFind = m_gcwGroupImperialScorePercentileThisGalaxy.find(gcwGroup);
	if (iterFind != m_gcwGroupImperialScorePercentileThisGalaxy.end())
	{
		return iterFind->second;
	}

	return 50; // tie
}

// ----------------------------------------------------------------------

float GuildObject::getGcwDefenderRegionImperialBonus(std::string const & gcwScoreCategory) const
{
	Archive::AutoDeltaMap<std::string, std::pair<float, float> >::const_iterator const iterFind = m_gcwRegionDefenderBonus.find(gcwScoreCategory);
	if (iterFind != m_gcwRegionDefenderBonus.end())
		return iterFind->second.first;

	return 0.0f;
}

// ----------------------------------------------------------------------

float GuildObject::getGcwDefenderRegionRebelBonus(std::string const & gcwScoreCategory) const
{
	Archive::AutoDeltaMap<std::string, std::pair<float, float> >::const_iterator const iterFind = m_gcwRegionDefenderBonus.find(gcwScoreCategory);
	if (iterFind != m_gcwRegionDefenderBonus.end())
		return iterFind->second.second;

	return 0.0f;
}

// ----------------------------------------------------------------------

void GuildObject::setGcwImperialScorePercentileFromOtherGalaxy(std::string const & clusterName, std::map<std::string, int> const & gcwImperialScorePercentile, std::map<std::string, int> const & gcwGroupImperialScorePercentile)
{
	if (!isAuthoritative())
		return;

	if (0 == _stricmp(GameServer::getInstance().getClusterName().c_str(), clusterName.c_str()))
		return;

	if (!ConfigServerGame::getReceiveGcwScoreFromOtherGalaxies())
		return;

	for (std::map<std::string, int>::const_iterator iterCategory = gcwImperialScorePercentile.begin(); iterCategory != gcwImperialScorePercentile.end(); ++iterCategory)
	{
		m_gcwImperialScorePercentileOtherGalaxies.set(std::make_pair(clusterName, iterCategory->first), iterCategory->second);
	}

	for (std::map<std::string, int>::const_iterator iterGroup = gcwGroupImperialScorePercentile.begin(); iterGroup != gcwGroupImperialScorePercentile.end(); ++iterGroup)
	{
		m_gcwGroupImperialScorePercentileOtherGalaxies.set(std::make_pair(clusterName, iterGroup->first), iterGroup->second);
	}
}

// ----------------------------------------------------------------------

void GuildObject::setGcwRawScoreFromOtherGalaxy(std::string const & clusterName, std::map<std::string, std::pair<int64, int64> > const & gcwImperialScore, std::map<std::string, std::pair<int64, int64> > const & gcwRebelScore)
{
	if (!isAuthoritative())
		return;

	if (0 == _stricmp(GameServer::getInstance().getClusterName().c_str(), clusterName.c_str()))
		return;

	if (!ConfigServerGame::getReceiveGcwScoreFromOtherGalaxies())
		return;

	for (std::map<std::string, std::pair<int64, int64> >::const_iterator iterImperial = gcwImperialScore.begin(); iterImperial != gcwImperialScore.end(); ++iterImperial)
	{
		m_gcwImperialScoreOtherGalaxies.set(std::make_pair(clusterName, iterImperial->first), iterImperial->second);
	}

	for (std::map<std::string, std::pair<int64, int64> >::const_iterator iterRebel = gcwRebelScore.begin(); iterRebel != gcwRebelScore.end(); ++iterRebel)
	{
		m_gcwRebelScoreOtherGalaxies.set(std::make_pair(clusterName, iterRebel->first), iterRebel->second);
	}
}

// ----------------------------------------------------------------------

void GuildObject::unload()
{
	FATAL(true, ("Tried to unload guild object.\n"));
}

// ----------------------------------------------------------------------

void GuildObject::onGuildEnemiesChanged()
{
	GuildInterface::touchEnemies();
}

// ----------------------------------------------------------------------

void GuildObject::onMembersInfoErase(std::pair<int, NetworkId> const & key, GuildMemberInfo const & value)
{
	UNREF(value);

	// member has been removed from a guild, so need to update CreatureObject
	ServerObject * const member = ServerWorld::findObjectByNetworkId(key.second);
	if (member)
	{
		CreatureObject * const memberCreatureObject = member->asCreatureObject();
		if (memberCreatureObject && memberCreatureObject->isAuthoritative() && memberCreatureObject->isInitialized() && (memberCreatureObject->getGuildId() == key.first))
		{
			memberCreatureObject->setGuildId(0);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::onMembersInfoInsert(std::pair<int, NetworkId> const & key, GuildMemberInfo const & value)
{
	// member has been added to a guild, so need to update CreatureObject
	ServerObject * const member = ServerWorld::findObjectByNetworkId(key.second);
	if (member)
	{
		CreatureObject * const memberCreatureObject = member->asCreatureObject();
		if (memberCreatureObject && memberCreatureObject->isAuthoritative() && memberCreatureObject->isInitialized() && (memberCreatureObject->getGuildId() == 0) && (value.m_permissions & GuildInterface::Member))
		{
			memberCreatureObject->setGuildId(key.first);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::onMembersInfoSet(std::pair<int, NetworkId> const & key, GuildMemberInfo const & oldValue, GuildMemberInfo const & newValue)
{
	UNREF(oldValue);

	// member has been added to a guild, so need to update CreatureObject
	ServerObject * const member = ServerWorld::findObjectByNetworkId(key.second);
	if (member)
	{
		CreatureObject * const memberCreatureObject = member->asCreatureObject();
		if (memberCreatureObject && memberCreatureObject->isAuthoritative() && memberCreatureObject->isInitialized() && (memberCreatureObject->getGuildId() == 0) && (newValue.m_permissions & GuildInterface::Member))
		{
			memberCreatureObject->setGuildId(key.first);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObjectNamespace::replaceSetIfNeeded(char const *label, Archive::AutoDeltaSet<std::string> &oldSet, std::set<std::string> const &newSet, bool force)
{
	if (force || (oldSet.get() != newSet))
	{
		WARNING(true, ("Guild vector %s was inconsistant on load.", label));
		oldSet.clear();
		for (std::set<std::string>::const_iterator i = newSet.begin(); i != newSet.end(); ++i)
			oldSet.insert(*i);
	}
}

// ----------------------------------------------------------------------

void GuildObjectNamespace::replaceSetIfNeeded(char const *label, Archive::AutoDeltaSet<std::string, GuildObject> &oldSet, std::set<std::string> const &newSet, bool force)
{
	if (force || (oldSet.get() != newSet))
	{
		WARNING(true, ("Guild vector %s was inconsistant on load.", label));
		oldSet.clear();
		for (std::set<std::string>::const_iterator i = newSet.begin(); i != newSet.end(); ++i)
			oldSet.insert(*i);
	}
}

// ----------------------------------------------------------------------
// nullptr oldPermissions means wasn't an existing guild member
// nullptr newPermissions means will not be a guild member
void GuildObjectNamespace::updateGuildInfoMembersCount(GuildInfo & gi, int const * const oldPermissions, int const * const newPermissions)
{
	bool existingMember = false;
	bool existingSponsoredMember = false;
	bool existingGuildWarInclusionMember = false;
	bool existingGuildWarExclusionMember = false;

	if (oldPermissions) // was a guild member
	{
		existingMember = true;
		if (*oldPermissions == GuildInterface::None)
		{
			existingSponsoredMember = true;
		}
		else
		{
			existingGuildWarInclusionMember = ((*oldPermissions & GuildInterface::WarInclusion) == GuildInterface::WarInclusion);
			existingGuildWarExclusionMember = ((*oldPermissions & GuildInterface::WarExclusion) == GuildInterface::WarExclusion);
		}
	}

	if (!newPermissions) // no longer a guild member
	{
		if (existingMember)
			--gi.m_sponsoredAndMemberCount;

		if (existingSponsoredMember)
			--gi.m_sponsoredCount;

		if (existingGuildWarInclusionMember)
			--gi.m_guildWarInclusionCount;

		if (existingGuildWarExclusionMember)
			--gi.m_guildWarExclusionCount;
	}
	else // still (or will be) a guild member
	{
		if (!existingMember)
			++gi.m_sponsoredAndMemberCount;

		if (*newPermissions == GuildInterface::None)
		{
			if (!existingSponsoredMember)
				++gi.m_sponsoredCount;
		}
		else if (existingSponsoredMember)
		{
			--gi.m_sponsoredCount;
		}

		if (*newPermissions & GuildInterface::WarInclusion)
		{
			if (!existingGuildWarInclusionMember)
				++gi.m_guildWarInclusionCount;
		}
		else if (existingGuildWarInclusionMember)
		{
			--gi.m_guildWarInclusionCount;
		}

		if (*newPermissions & GuildInterface::WarExclusion)
		{
			if (!existingGuildWarExclusionMember)
				++gi.m_guildWarExclusionCount;
		}
		else if (existingGuildWarExclusionMember)
		{
			--gi.m_guildWarExclusionCount;
		}
	}
}

// ----------------------------------------------------------------------

void GuildObjectNamespace::updateGcwPercentileHistory(Archive::AutoDeltaMap<std::pair<std::string, int>, int> & history, Archive::AutoDeltaMap<std::string, int> & historyCount, std::string const & scoreName, int score)
{
	if (ConfigServerGame::getTrackGcwPercentileHistoryCount() <= 0)
		return;

	history.set(std::make_pair(scoreName, static_cast<int>(::time(nullptr))), score);

	int newCount = 1;
	Archive::AutoDeltaMap<std::string, int>::const_iterator const iterHistoryCount = historyCount.find(scoreName);
	if (iterHistoryCount != historyCount.end())
	{
		newCount += iterHistoryCount->second;
	}

	if (newCount <= ConfigServerGame::getTrackGcwPercentileHistoryCount())
	{
		// another history has been added
		historyCount.set(scoreName, newCount);
	}
	else
	{
		// too many histories, purge the oldest entry
		Archive::AutoDeltaMap<std::pair<std::string, int>, int>::const_iterator const iterOldestHistory = history.lower_bound(std::make_pair(scoreName, 0));
		if (iterOldestHistory != history.end())
		{
			history.erase(iterOldestHistory->first);
		}
	}
}

// ----------------------------------------------------------------------

void GuildObject::addGcwRegionDefenderGuild(std::string const & gcwRegion, int guildId, uint32 faction)
{
	if (!gcwRegion.empty())
	{
		m_gcwRegionDefenderGuilds.set(std::make_pair(gcwRegion, guildId), faction);

		Archive::AutoDeltaMap<std::string, std::pair<int, int> >::const_iterator iter = m_gcwRegionDefenderGuildsCount.find(gcwRegion);
		if (iter != m_gcwRegionDefenderGuildsCount.end())
		{
			if (PvpData::isImperialFactionId(faction))
				m_gcwRegionDefenderGuildsCount.set(gcwRegion, std::make_pair(iter->second.first + 1, iter->second.second));
			else if (PvpData::isRebelFactionId(faction))
				m_gcwRegionDefenderGuildsCount.set(gcwRegion, std::make_pair(iter->second.first, iter->second.second + 1));
		}
		else
		{
			if (PvpData::isImperialFactionId(faction))
				m_gcwRegionDefenderGuildsCount.set(gcwRegion, std::make_pair(1, 0));
			else if (PvpData::isRebelFactionId(faction))
				m_gcwRegionDefenderGuildsCount.set(gcwRegion, std::make_pair(0, 1));
		}

		m_gcwRegionDefenderGuildsVersion.set(m_gcwRegionDefenderGuildsVersion.get() + 1);

		updateGcwRegionDefenderBonus(gcwRegion);
	}
}

// ----------------------------------------------------------------------

void GuildObject::removeGcwRegionDefenderGuild(std::string const & gcwRegion, int guildId, uint32 faction)
{
	if (!gcwRegion.empty())
	{
		m_gcwRegionDefenderGuilds.erase(std::make_pair(gcwRegion, guildId));

		Archive::AutoDeltaMap<std::string, std::pair<int, int> >::const_iterator iter = m_gcwRegionDefenderGuildsCount.find(gcwRegion);
		if (iter != m_gcwRegionDefenderGuildsCount.end())
		{
			std::pair<int, int> value = iter->second;

			if (PvpData::isImperialFactionId(faction))
				value.first = std::max(0, value.first - 1);
			else if (PvpData::isRebelFactionId(faction))
				value.second = std::max(0, value.second - 1);

			if ((value.first > 0) || (value.second > 0))
				m_gcwRegionDefenderGuildsCount.set(gcwRegion, value);
			else
				m_gcwRegionDefenderGuildsCount.erase(gcwRegion);
		}

		m_gcwRegionDefenderGuildsVersion.set(m_gcwRegionDefenderGuildsVersion.get() + 1);

		updateGcwRegionDefenderBonus(gcwRegion);
	}
}

// ======================================================================
