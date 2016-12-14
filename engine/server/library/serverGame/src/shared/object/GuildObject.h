//========================================================================
//
// GuildObject.h
//
// Copyright 2002 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_GuildObject_H
#define INCLUDED_GuildObject_H

// ======================================================================

#include "Archive/AutoDeltaMap.h"
#include "Archive/AutoDeltaSet.h"
#include "serverGame/GuildInfo.h"
#include "serverGame/GuildMemberInfo.h"
#include "serverGame/UniverseObject.h"

// ======================================================================

class ServerGuildObjectTemplate;

// ======================================================================

class GuildObject: public UniverseObject
{
public:
	GuildObject(ServerGuildObjectTemplate const *newTemplate);
	virtual ~GuildObject();

	virtual Controller* createDefaultController();
	virtual void setupUniverse();
	virtual bool isVisibleOnClient(const Client &client) const;
	virtual void getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const;
	virtual void unload();

	GuildInfo const * getGuildInfo(int guildId) const;
	GuildMemberInfo const * getGuildMemberInfo(int guildId, NetworkId const &memberId) const;

	std::set<std::string> const &getGuildEnemies() const;

	void getAllGuildIds(std::vector<int> &results) const;
	int findGuild(std::string const &name) const;
	bool guildExists(int guildId) const;
	int getGuildId(NetworkId const &memberId) const;
	int getGuildIdForSponsored(NetworkId const &memberId) const;
	int getGuildIdForGuildLeader(NetworkId const &leaderId) const;
	void getGuildMemberIds(int guildId, std::vector<NetworkId> &results) const;
	void getMemberIdsWithPermissions(int guildId, int permissions, std::vector<NetworkId> &results) const;

	int createGuild(std::string const &guildName, std::string const &guildAbbrev, int guildId = 0);
	void disbandGuild(int guildId);
	void removeGuildMember(int guildId, NetworkId const &memberId);
	void addGuildCreatorMember(int guildId, NetworkId const &memberId, std::string const &memberName, std::string const &memberProfessionSkillTemplate, int memberLevel);
	void addGuildSponsorMember(int guildId, NetworkId const &memberId, std::string const &memberName, std::string const &memberProfessionSkillTemplate, int memberLevel);
	void setGuildMemberPermission(int guildId, NetworkId const &memberId, int permissions);
	void setGuildMemberTitle(int guildId, NetworkId const &memberId, std::string const &title);
	void setGuildMemberAllegiance(int guildId, NetworkId const &memberId, NetworkId const &allegiance);
	void setGuildMemberPermissionAndAllegiance(int guildId, NetworkId const &memberId, int permissions, NetworkId const &allegiance);
	void setGuildMemberNameAndPermision(int guildId, NetworkId const &memberId, std::string const &memberName, int permissions);
	void setGuildMemberProfessionInfo(int guildId, NetworkId const &memberId, std::string const &memberProfessionSkillTemplate, int memberLevel);
	void addGuildMemberRank(int guildId, NetworkId const &memberId, std::string const &rankName);
	void removeGuildMemberRank(int guildId, NetworkId const &memberId, std::string const &rankName);
	bool hasGuildMemberRank(int guildId, NetworkId const &memberId, std::string const &rankName);
	void getGuildMemberRank(int guildId, NetworkId const &memberId, std::vector<std::string> &ranks);
	void setGuildLeader(int guildId, NetworkId const &leaderId);
	void setGuildElectionEndTime(int guildId, int electionPreviousEndTime, int electionNextEndTime);
	void setGuildFaction(int guildId, uint32 guildFaction);
	void setGuildGcwDefenderRegion(int guildId, std::string const &guildGcwDefenderRegion);
	void setGuildEnemy(int guildId, int enemyId);
	void removeGuildEnemy(int guildId, int enemyId);
	void modifyGuildWarKillTracking(int killerGuildId, int victimGuildId, int adjustment, int updateTime);
	void updateGuildWarKillTrackingData();
	void setGuildName(int guildId, std::string const &newName);
	void setGuildAbbrev(int guildId, std::string const &newAbbrev);

	void calculateGcwImperialScorePercentileForSwg();
	void depersistGcwImperialScorePercentile();
	void updateGcwImperialScorePercentile(std::set<std::string> const & gcwScoreCategories);
	void updateGcwRegionDefenderBonus(std::string const & gcwScoreCategory);
	std::map<std::string, int> const & getGcwImperialScorePercentile() const;
	std::map<std::string, int> const & getGcwGroupImperialScorePercentile() const;
	int getGcwImperialScorePercentile(std::string const & gcwCategory) const; // the rebel score is 100 - the imperial score
	int getGcwGroupImperialScorePercentile(std::string const & gcwGroup) const; // the rebel score is 100 - the imperial score
	float getGcwDefenderRegionImperialBonus(std::string const & gcwScoreCategory) const;
	float getGcwDefenderRegionRebelBonus(std::string const & gcwScoreCategory) const;
	std::map<std::pair<std::string, std::string>, int> const & getGcwGroupCategoryImperialScoreRaw() const;
	std::map<std::string, int> const & getGcwGroupImperialScoreRaw() const;
	void setGcwImperialScorePercentileFromOtherGalaxy(std::string const & clusterName, std::map<std::string, int> const & gcwImperialScorePercentile, std::map<std::string, int> const & gcwGroupImperialScorePercentile);
	void setGcwRawScoreFromOtherGalaxy(std::string const & clusterName, std::map<std::string, std::pair<int64, int64> > const & gcwImperialScore, std::map<std::string, std::pair<int64, int64> > const & gcwRebelScore);

	std::map<std::pair<std::string, int>, uint32> const & getGcwRegionDefenderGuilds() const;
	std::map<std::string, std::pair<int, int> > const & getGcwRegionDefenderGuildsCount() const;
	int getGcwRegionDefenderGuildsVersion() const;

private:
	GuildObject();
	GuildObject(GuildObject const &rhs);
	GuildObject& operator=(GuildObject const &rhs);

	void addMembersToPackages();
	int nextFreeGuildId();
	void onGuildEnemiesChanged();

	void onMembersInfoErase(std::pair<int, NetworkId> const & key, GuildMemberInfo const & value);
	void onMembersInfoInsert(std::pair<int, NetworkId> const & key, GuildMemberInfo const & value);
	void onMembersInfoSet(std::pair<int, NetworkId> const & key, GuildMemberInfo const & oldValue, GuildMemberInfo const & newValue);

	void addGcwRegionDefenderGuild(std::string const & gcwRegion, int guildId, uint32 faction);
	void removeGcwRegionDefenderGuild(std::string const & gcwRegion, int guildId, uint32 faction);

private:
// BPM GuildObject : UniverseObject // Begin persisted members
	Archive::AutoDeltaSet<std::string> m_names;
	Archive::AutoDeltaSet<std::string> m_abbrevs;
	Archive::AutoDeltaSet<std::string> m_leaders;
	Archive::AutoDeltaSet<std::string> m_members;
	Archive::AutoDeltaSet<std::string, GuildObject> m_enemies;
// EPM

	// parallel maps to persisted members m_names, m_abbrevs, m_leaders, and m_members that
	// will provide access to the data because the data in the persisted members are strings
	Archive::AutoDeltaMap<int, GuildInfo> m_guildsInfo; // guild id -> GuildInfo
	Archive::AutoDeltaMap<std::pair<int, NetworkId>, GuildMemberInfo, GuildObject> m_membersInfo; // (guild id, member id) -> GuildMemberInfo
	Archive::AutoDeltaMap<NetworkId, int> m_fullMembers; // member id -> guild id
	Archive::AutoDeltaMap<NetworkId, int> m_sponsoredMembers; // member id -> guild id
	Archive::AutoDeltaMap<NetworkId, int> m_guildLeaders; // leader id -> guild id

	// GCW score tracking
	Archive::AutoDeltaMap<std::string, int> m_gcwImperialScorePercentileThisGalaxy;
	Archive::AutoDeltaMap<std::string, std::pair<float, float> > m_gcwRegionDefenderBonus;
	Archive::AutoDeltaMap<std::string, int> m_gcwGroupImperialScorePercentileThisGalaxy;

	Archive::AutoDeltaMap<std::pair<std::string, int>, int> m_gcwImperialScorePercentileHistoryThisGalaxy;
	Archive::AutoDeltaMap<std::pair<std::string, int>, int> m_gcwGroupImperialScorePercentileHistoryThisGalaxy;

	Archive::AutoDeltaMap<std::string, int> m_gcwImperialScorePercentileHistoryCountThisGalaxy;
	Archive::AutoDeltaMap<std::string, int> m_gcwGroupImperialScorePercentileHistoryCountThisGalaxy;

	Archive::AutoDeltaMap<std::pair<std::string, std::string>, int> m_gcwImperialScorePercentileOtherGalaxies;
	Archive::AutoDeltaMap<std::pair<std::string, std::string>, int> m_gcwGroupImperialScorePercentileOtherGalaxies;

	Archive::AutoDeltaMap<std::pair<std::string, std::string>, int> m_gcwGroupCategoryImperialScoreRawThisGalaxy;
	Archive::AutoDeltaMap<std::string, int> m_gcwGroupImperialScoreRawThisGalaxy;

	Archive::AutoDeltaMap<std::pair<std::string, std::string>, std::pair<int64, int64> > m_gcwImperialScoreOtherGalaxies;
	Archive::AutoDeltaMap<std::pair<std::string, std::string>, std::pair<int64, int64> > m_gcwRebelScoreOtherGalaxies;

	int m_lowFreeGuildId;
	unsigned long m_nextGuildWarKillTrackingAdjustmentUpdate;
	std::map<std::pair<int, int>, std::pair<int, int> > m_guildWarKillTrackingAdjustment;

	Archive::AutoDeltaMap<std::pair<std::string, int>, uint32> m_gcwRegionDefenderGuilds;
	Archive::AutoDeltaMap<std::string, std::pair<int, int> > m_gcwRegionDefenderGuildsCount;
	Archive::AutoDeltaVariable<int> m_gcwRegionDefenderGuildsVersion;
};

// ----------------------------------------------------------------------

inline std::map<std::string, int> const & GuildObject::getGcwImperialScorePercentile() const
{
	return m_gcwImperialScorePercentileThisGalaxy.getMap();
}

// ----------------------------------------------------------------------

inline std::map<std::string, int> const & GuildObject::getGcwGroupImperialScorePercentile() const
{
	return m_gcwGroupImperialScorePercentileThisGalaxy.getMap();
}

// ----------------------------------------------------------------------

inline std::map<std::pair<std::string, std::string>, int> const & GuildObject::getGcwGroupCategoryImperialScoreRaw() const
{
	return m_gcwGroupCategoryImperialScoreRawThisGalaxy.getMap();
}

// ----------------------------------------------------------------------

inline std::map<std::string, int> const & GuildObject::getGcwGroupImperialScoreRaw() const
{
	return m_gcwGroupImperialScoreRawThisGalaxy.getMap();
}

// ----------------------------------------------------------------------

inline std::map<std::pair<std::string, int>, uint32> const & GuildObject::getGcwRegionDefenderGuilds() const
{
	return m_gcwRegionDefenderGuilds.getMap();
}

// ----------------------------------------------------------------------

inline std::map<std::string, std::pair<int, int> > const & GuildObject::getGcwRegionDefenderGuildsCount() const
{
	return m_gcwRegionDefenderGuildsCount.getMap();
}

// ----------------------------------------------------------------------

inline int GuildObject::getGcwRegionDefenderGuildsVersion() const
{
	return m_gcwRegionDefenderGuildsVersion.get();
}

// ======================================================================

#endif
