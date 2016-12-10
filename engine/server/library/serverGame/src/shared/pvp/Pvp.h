// ======================================================================
//
// Pvp.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _Pvp_H
#define _Pvp_H

// ======================================================================

#include <set>

// ======================================================================

class Client;
class CreatureObject;
class MessageQueuePvpCommand;
class NetworkId;
class PlayerObject;
class Region;
class RegionPvp;
class Scheduler;
class TangibleObject;
class Vector;

// ======================================================================

const int PvpType_Neutral  = 0;
const int PvpType_Covert   = 1;
const int PvpType_Declared = 2;

// ======================================================================

class Pvp
{
public:
	typedef uint32 FactionId;
	typedef int PvpType;

	static void install                   ();
	static void remove                    ();

	// pvp actions
	static bool canAttack                 (TangibleObject const &actor, TangibleObject const &target);
	static bool canHelp                   (TangibleObject const &actor, TangibleObject const &target);
	static void attackPerformed           (TangibleObject &actor, TangibleObject &target);
	static void helpPerformed             (TangibleObject &actor, TangibleObject &target);
	static bool wouldAttackCauseAlignedEnemyFlag(TangibleObject const &actor, TangibleObject const &target);
	static bool wouldHelpCauseAlignedEnemyFlag(TangibleObject const &actor, TangibleObject const &target);

	// pvp information
	static bool isEnemy                   (TangibleObject const &actor, TangibleObject const &target);
	static bool hasDuelEnemyFlag          (TangibleObject const &who, NetworkId const &enemyId);
	static bool hasPersonalEnemyFlag      (TangibleObject const &who, NetworkId const &enemyId);
	static bool hasFactionEnemyFlag       (TangibleObject const &who, FactionId factionId);
	static bool areFactionsOpposed        (FactionId align1, FactionId align2);
	static FactionId battlefieldGetFaction(TangibleObject const &who, RegionPvp const &region);
	static bool battlefieldIsParticipant  (TangibleObject const &who, RegionPvp const &region);
	static bool hasAnyTimedEnemyFlag      (TangibleObject const &who);
	static bool hasAnyAlignedTimedEnemyFlag(TangibleObject const &who);
	static bool hasAnyBountyDuelEnemyFlag (TangibleObject const &who);
	static bool hasAnyGuildWarCoolDownPeriodEnemyFlag(TangibleObject const &who);
	static bool isDuelingAllowed          (TangibleObject const &actor, TangibleObject const &target);
	static void getPersonalEnemyIds       (TangibleObject const &who, std::vector<NetworkId> &enemyIds);

	// pvp setup
	static void setAlignedFaction         (TangibleObject &who, FactionId factionId);
	static void setNeutralMercenaryFaction(CreatureObject &who, FactionId factionId, PvpType pvpType);
	static void makeOnLeave               (TangibleObject &who);
	static void makeCovert                (TangibleObject &who);
	static void makeDeclared              (TangibleObject &who);
	static void makeNeutral               (TangibleObject &who);
	static void prepareToBeCovert         (TangibleObject &who);
	static void prepareToBeDeclared       (TangibleObject &who);
	static void prepareToBeNeutral        (TangibleObject &who);
	static void setDuelEnemyFlag          (TangibleObject &who, TangibleObject const &enemy);
	static void setPersonalEnemyFlag      (TangibleObject &who, TangibleObject const &enemy);
	static void setPermanentPersonalEnemyFlag(TangibleObject &who, NetworkId const &enemyId);
	static void setFactionEnemyFlag       (TangibleObject &who, FactionId enemyAlign, int expireTimeMs = -1);
	static void removeAllTempEnemyFlags   (TangibleObject &dest);
	static void removeDuelEnemyFlags      (TangibleObject &dest, NetworkId const &enemyId);
	static void removePersonalEnemyFlags  (TangibleObject &dest, NetworkId const &enemyId);
	static void removeTempEnemyFlags      (TangibleObject &dest, NetworkId const &enemyId);
	static void battlefieldSetParticipant (TangibleObject const &who, RegionPvp const &region, Pvp::FactionId factionId);
	static void battlefieldClearParticipants(RegionPvp const &region);
	static void handleCreatureDied        (TangibleObject &creature);

	// client updating
	static Region const *getPvpRegion     (TangibleObject const &who);
	static FactionId getApparantFaction   (FactionId viewerFaction, TangibleObject const &target);
	static void getClientVisibleStatus    (Client const &viewerClient, TangibleObject const &target, uint32 &flags, FactionId &factionId);
	static void checkForRegionChange      (TangibleObject &who);
	static void forceStatusUpdate         (TangibleObject &who);

	// multiserver handling
	static void handlePvpMessage          (TangibleObject &dest, MessageQueuePvpCommand const &pvpMessage);
	static void handleAuthorityAcquire    (TangibleObject &dest);

	// pvp scheduler
	static Scheduler &getScheduler        ();
	static void updateTimedFlags          (const void *context);

	// pvp ranking
	struct PvpRankInfo
	{
		int dataTableRow;
		int rank;
		int minRating;
		int maxRating;
		std::string imperialTitle;
		std::string rebelTitle;
		int ratingEarningCap;
		int ratingDecayBalance;
		int maxRatingDecay;
		int ratingDecayFloor;
		int pointToOffsetDecay;
	};

	static PvpRankInfo const & getRankInfo(int rating);
	static int getMinRatingForRank();
	static int getMaxRatingForRank();
	static std::string const & debugGetGcwRankTable();
	static std::string const & debugGetGcwFormulasTable();
	static std::string const & debugGetGcwRankDecayExclusion();
	static int calculateRatingAdjustment(int gcwPoints, int currentRating, int &totalEarnedRating, int &totalEarnedRatingAfterDecay, int &cappedRatingAdjustment);
	static bool isInGcwRankDecayExclusionInterval(time_t t);

	// gcw score

	// scale score by 1,000 so that we can do diminishing value
	// (like divide score by 2 each week) and not lose any accuracy
	// (i.e. partial points) for 10 weeks
	static int const c_gcwScoreScaleFactor = 1000;

	struct GcwScoreCategory 
	{
		std::string categoryName;
		bool gcwRegionDefender;
		bool notifyOnPercentileChange;
		std::set<std::string> gcwScoreCategoryGroups;
	};

	static GcwScoreCategory const * getGcwScoreCategory(std::string const & scoreCategory);
	static GcwScoreCategory const * getGcwScoreDefaultCategoryForPlanet(std::string const & planetName);
	static std::map<std::string, Pvp::GcwScoreCategory const *> const & getAllGcwScoreCategory();
	static std::map<std::string, std::map<std::string, std::pair<std::pair<float, float>, float> > > const & getGcwScoreCategoryRegions();
	static std::string const & getGcwScoreCategoryRegion(std::string const & planetName, Vector const & position_w);
	static std::map<std::string, std::map<std::string, int> > const & getGcwScoreCategoryGroups();
	static bool getGcwScoreCategoryGroupTotalPoints(std::string const & group, std::string const & category, int & totalPoints);
	static int calculateGcwImperialScorePercentile(std::string const & gcwCategory, std::map<std::string, std::pair<int64, int64> > const & gcwImperialScore, std::map<std::string, std::pair<int64, int64> > const & gcwRebelScore);
	static int calculateGcwImperialScorePercentile(std::string const & gcwCategory, std::map<std::string, std::pair<uint64, uint64> > const & gcwImperialScore, std::map<std::string, std::pair<uint64, uint64> > const & gcwRebelScore);
	static float getGcwDefenderRegionImperialBonus(std::string const & scoreCategory);
	static float getGcwDefenderRegionRebelBonus(std::string const & scoreCategory);
	static bool getGcwDefenderRegionBonus(CreatureObject const & creature, PlayerObject const & player, float & bonus);

private:
	Pvp();
	Pvp(Pvp const &);
};

// ======================================================================

#endif // _Pvp_H
