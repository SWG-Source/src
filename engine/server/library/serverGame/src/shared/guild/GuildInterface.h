// ======================================================================
//
// GuildInterface.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _GuildInterface_H_
#define _GuildInterface_H_

// ======================================================================

class CreatureObject;
class GuildInfo;
class GuildMemberInfo;
class GuildObject;
class NetworkId;
class ScriptParams;

// ======================================================================

class GuildInterface // static
{
public:
	// Note: These should never be moved, and should be in sync with what is in
	// guild.scriptlib and ConsoleCommandParserGuild.cpp::getGuildPermissionsText()
	enum GuildPermissions
	{
		None       = 0,      // sponsored

		// administrative permissions
		Member     = (1<<0),
		Sponsor    = (1<<1),
		Disband    = (1<<2),
		Accept     = (1<<3),
		Kick       = (1<<4),
		Mail       = (1<<5),
		Title      = (1<<6),
		Namechange = (1<<7),
		War        = (1<<8),
		Rank       = (1<<9),

		// if you need to add a new administrative guild permission,
		// use one of the following reserved administrative guild
		// permissions, which has been reserved for that specific purpose
		AdministrativeReserved1  = (1<<10),
		AdministrativeReserved2  = (1<<11),
		AdministrativeReserved3  = (1<<12),
		AdministrativeReserved4  = (1<<13),
		AdministrativeReserved5  = (1<<14),
		AdministrativeReserved6  = (1<<15),
		AdministrativeReserved7  = (1<<16),
		AdministrativeReserved8  = (1<<17),
		AdministrativeReserved9  = (1<<18),
		AdministrativeReserved10 = (1<<19),

		AllAdministrativePermissions = (Member | Sponsor | Disband | Accept | Kick | Mail | Title | Namechange | War | Rank | AdministrativeReserved1 | AdministrativeReserved2 | AdministrativeReserved3 | AdministrativeReserved4 | AdministrativeReserved5 | AdministrativeReserved6 | AdministrativeReserved7 | AdministrativeReserved8 | AdministrativeReserved9 | AdministrativeReserved10),

		// non-administrative permissions
		ElectionCandidate = (1<<20), // guild member is running in the current guild election
		WarInclusion      = (1<<21), // guild member is in the guild war inclusion list
		WarExclusion      = (1<<22)  // guild member is in the guild war exclusion list
	};

	static void update();
	static void touchEnemies();

	static int getGuildId(NetworkId const &memberId);
	static int getGuildIdForSponsored(NetworkId const &memberId);
	static int getGuildIdForGuildLeader(NetworkId const &leaderId);
	static void getAllGuildIds(std::vector<int> &results);
	static int findGuild(std::string const &name);
	static bool guildExists(int guildId);
	static std::pair<int, time_t> const *hasDeclaredWarAgainst(int actorGuildId, int targetGuildId);
	static GuildInfo const *getGuildInfo(int guildId);
	static std::string const &getGuildName(int guildId);
	static std::string const &getGuildAbbrev(int guildId);
	static NetworkId const &getGuildLeaderId(int guildId);
	static int getGuildElectionPreviousEndTime(int guildId);
	static int getGuildElectionNextEndTime(int guildId);
	static uint32 getGuildCurrentFaction(int guildId);
	static uint32 getGuildCurrentFaction(GuildInfo const & gi);
	static uint32 getGuildPreviousFaction(int guildId);
	static uint32 getGuildPreviousFaction(GuildInfo const & gi);
	static int getTimeLeftGuildPreviousFaction(int guildId);
	static int getTimeLeftGuildPreviousFaction(GuildInfo const & gi);
	static std::string const &getGuildCurrentGcwDefenderRegion(int guildId);
	static std::string const &getGuildCurrentGcwDefenderRegion(GuildInfo const & gi);
	static int getTimeJoinedGuildCurrentGcwDefenderRegion(int guildId);
	static int getTimeJoinedGuildCurrentGcwDefenderRegion(GuildInfo const & gi);
	static std::string const &getGuildPreviousGcwDefenderRegion(int guildId);
	static std::string const &getGuildPreviousGcwDefenderRegion(GuildInfo const & gi);
	static int getTimeLeftGuildPreviousGcwDefenderRegion(int guildId);
	static int getTimeLeftGuildPreviousGcwDefenderRegion(GuildInfo const & gi);
	static std::map<std::pair<std::string, int>, uint32> const & getGcwRegionDefenderGuilds();
	static std::map<std::string, std::pair<int, int> > const & getGcwRegionDefenderGuildsCount();
	static int getGcwRegionDefenderGuildsVersion();
	static void getGuildMemberIds(int guildId, std::vector<NetworkId> &results);
	static void getGuildMemberIdsWithPermissions(int guildId, int permissions, std::vector<NetworkId> &results);
	static int getGuildCountMembersOnly(int guildId);
	static int getGuildCountSponsoredOnly(int guildId);
	static int getGuildCountMembersAndSponsored(int guildId);
	static int getGuildCountMembersGuildWarPvPEnabled(int guildId);
	static GuildMemberInfo const *getGuildMemberInfo(int guildId, NetworkId const &memberId);
	static bool getGuildMemberGuildWarEnabled(int guildId, NetworkId const &memberId);
	static std::vector<int> const &getGuildEnemies(int guildId);
	static std::map<int, std::pair<int, time_t> > const &getGuildEnemiesKillInfo(int guildId);
	static std::pair<int, time_t> const &getGuildEnemyKillInfo(int guildIdA, int guildIdB);
	static std::vector<int> const &getGuildsAtWarWith(int guildId);
	static ScriptParams const *getMasterGuildWarTableDictionary();
	static void updateInactiveGuildWarTrackingInfo(GuildObject &masterGuildObject, int guildAId, int guildAKillCount, int guildBId, int guildBKillCount);
	static ScriptParams const *getInactiveGuildWarTableDictionary();

	static bool isValidGuildName(std::string const &guildName);
	static bool isValidGuildAbbrev(std::string const &guildAbbrev);
	static int createGuild(std::string const &guildName, std::string const &guildAbbrev);
	static void disbandGuild(int guildId);
	static void removeGuildMember(int guildId, NetworkId const &memberId);
	static void setGuildLeader(int guildId, NetworkId const &leaderId);
	static void setGuildElectionEndTime(int guildId, int electionPreviousEndTime, int electionNextEndTime);
	static void setGuildFaction(int guildId, uint32 guildFaction);
	static void setGuildGcwDefenderRegion(int guildId, std::string const &guildGcwDefenderRegion);
	static void addGuildCreatorMember(int guildId, NetworkId const &memberId);
	static void addGuildSponsorMember(int guildId, NetworkId const &memberId);
	static void setGuildMemberPermission(int guildId, NetworkId const &memberId, int permissions);
	static void setGuildMemberTitle(int guildId, NetworkId const &memberId, std::string const &title);
	static void setGuildMemberAllegiance(int guildId, NetworkId const &memberId, NetworkId const &allegiance);
	static void setGuildMemberPermissionAndAllegiance(int guildId, NetworkId const &memberId, int permissions, NetworkId const &allegiance);
	static void setGuildMemberProfessionInfo(int guildId, NetworkId const &memberId, std::string const &memberProfessionSkillTemplate, int memberLevel);
	static void addGuildMemberRank(int guildId, NetworkId const &memberId, std::string const &rankName);
	static void removeGuildMemberRank(int guildId, NetworkId const &memberId, std::string const &rankName);
	static bool hasGuildMemberRank(int guildId, NetworkId const &memberId, std::string const &rankName);
	static void getGuildMemberRank(int guildId, NetworkId const &memberId, std::vector<std::string> &ranks);
	static void setGuildEnemy(int guildId, int enemyId);
	static void removeGuildEnemy(int guildId, int enemyId);
	static void updateGuildWarKillTracking(CreatureObject const &killer, CreatureObject const &victim);
	static void setGuildName(int guildId, std::string const &newName);
	static void setGuildAbbrev(int guildId, std::string const &newAbbrev);

	static void onGuildIdSet(CreatureObject const &who);
	static void onAboutToClearGuildId(CreatureObject const &who);
	static void verifyGuildMemberName(NetworkId const &memberId, std::string const &memberName);

	static void enterGuildChatRoom(CreatureObject const &who);
	static void reenterGuildChatRoom(CreatureObject const &who);
	static void leaveGuildChatRoom(CreatureObject const &who);
	static void sendGuildChat(CreatureObject const &who, Unicode::String const &text);
	static void onChatRoomCreate(std::string const &path);
	static void createAllGuildChatRooms();
	static std::pair<int, std::pair<std::string, std::string> > mailToGuild(CreatureObject const &who, Unicode::String const &subject, Unicode::String const &message, Unicode::String const &oob);
	static std::pair<int, std::pair<std::string, std::string> > mailToGuild(int guildId, Unicode::String const &subject, Unicode::String const &message, Unicode::String const &oob);
	static std::pair<int, std::pair<std::string, std::string> > mailToGuildRank(CreatureObject const &who, int guildRankSlotId, Unicode::String const &subject, Unicode::String const &message, Unicode::String const &oob);

	static void onVoiceChatRoomCreate(std::string const &path);
	static void enterGuildVoiceChatRoom(CreatureObject const &who);
	static void leaveGuildVoiceChatRoom(NetworkId const & netID, std::string const & firstName, int guildID);

private:
	static void buildEnemies(GuildObject const &guildObject);
};

// ======================================================================

#endif // _GuildInterface_H_

