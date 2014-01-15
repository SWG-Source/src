#ifndef _ObjectTracker_H_
#define _ObjectTracker_H_

// ======================================================================

class ObjectTracker
{
public:

	static int getNumAI();
	static int getNumBuildings();
	static int getNumCreatures();
	static int getNumInstallations();
	static int getNumObjects();
	static int getNumPlayers();
	static void getNumPlayersByFaction(int & imperial, int & rebel, int & neutral);
	static int getNumRunTimeRules();
	static int getNumTangibles();
	static int getNumUniverseObjects();

	static int getNumDynamicAI();
	static int getNumStaticAI();
	static int getNumCombatAI();
	static int getNumHibernatingAI();
	static int getNumDelayedHibernatingAI();

	static int getNumIntangibles();
	static int getNumMissionDatas();
	static int getNumMissionObjects();
	static int getNumGroupObjects();
	static int getNumMissionListEntries();
	static int getNumWaypoints();

	static int getNumPlayerQuestObjects();

	static void addBuilding();
	static void addCreature();
	static void addInstallation();
	static void addObject();
	static void addPlayer();
	static void addRunTimeRule();
	static void addTangible();
	static void addUniverseObject();
	static void addCombatAI();
	static void addHibernatingAI();
	static void addDelayedHibernatingAI();

	static void addIntangible();
	static void addGroupObject();
	static void addMissionData();
	static void addMissionObject();
	static void addMissionListEntry();
	static void addWaypoint();

	static void addPlayerQuestObject();
	
	static void removeBuilding();
	static void removeCreature();
	static void removeInstallation();
	static void removeObject();
	static void removePlayer();
	static void removeRunTimeRule();
	static void removeTangible();
	static void removeUniverseObject();
	static void removeCombatAI();
	static void removeHibernatingAI();
	static void removeDelayedHibernatingAI();

	static void removeIntangible();
	static void removeGroupObject();
	static void removeMissionData();
	static void removeMissionObject();
	static void removeMissionListEntry();
	static void removeWaypoint();

	static void removePlayerQuestObject();

	
private:
	static int m_numBuildings;
	static int m_numCreatures;
	static int m_numInstallations;
	static int m_numObjects;
	static int m_numPlayers;

	// we don't want to recalculate the factional
	// distribution of the players too often
	static time_t m_lastTimeCalculateFactionalPlayersCount;
	static int m_numPlayersImperial;
	static int m_numPlayersRebel;
	static int m_numPlayersNeutral;

	static int m_numRunTimeRules;
	static int m_numTangibles;
	static int m_numUniverseObjects;
	static int m_numCombatAI;
	static int m_numHibernatingAI;
	static int m_numDelayedHibernatingAI;

	static int m_numIntangibles;
	static int m_numGroupObjects;
	static int m_numMissionDatas;
	static int m_numMissionObjects;
	static int m_numMissionListEntries;
	static int m_numWaypoints;

	static int m_numPlayerQuestObjects;

	
private:
	ObjectTracker();
	~ObjectTracker();
	ObjectTracker(const ObjectTracker&);
	ObjectTracker& operator= (const ObjectTracker&);
};

#endif
