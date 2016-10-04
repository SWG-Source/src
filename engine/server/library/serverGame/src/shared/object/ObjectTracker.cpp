

#include "serverGame/FirstServerGame.h"
#include "ObjectTracker.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerObject.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedGame/PvpData.h"
#include "sharedTerrain/TerrainObject.h"


int ObjectTracker::m_numBuildings=0;
int ObjectTracker::m_numCreatures=0;
int ObjectTracker::m_numInstallations=0;
int ObjectTracker::m_numObjects=0;
int ObjectTracker::m_numPlayers=0;
time_t ObjectTracker::m_lastTimeCalculateFactionalPlayersCount=0;
int ObjectTracker::m_numPlayersImperial=0;
int ObjectTracker::m_numPlayersRebel=0;
int ObjectTracker::m_numPlayersNeutral=0;
int ObjectTracker::m_numRunTimeRules=0;
int ObjectTracker::m_numTangibles=0;
int ObjectTracker::m_numUniverseObjects=0;
int ObjectTracker::m_numCombatAI=0;
int ObjectTracker::m_numHibernatingAI=0;
int ObjectTracker::m_numDelayedHibernatingAI=0;

int ObjectTracker::m_numIntangibles=0;
int ObjectTracker::m_numGroupObjects=0;
int ObjectTracker::m_numMissionDatas=0;
int ObjectTracker::m_numMissionObjects=0;
int ObjectTracker::m_numMissionListEntries=0;
int ObjectTracker::m_numWaypoints=0;

int ObjectTracker::m_numPlayerQuestObjects = 0;

//==========================================================================================

int ObjectTracker::getNumAI()
{
	return m_numCreatures - m_numPlayers;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumBuildings()
{
	return m_numBuildings;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumCreatures()
{
	return m_numCreatures;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumInstallations()
{
	return m_numInstallations;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumObjects()
{
	return m_numObjects;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumPlayers()
{
	return m_numPlayers;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::getNumPlayersByFaction(int & imperial, int & rebel, int & neutral)
{
	time_t const now = ::time(nullptr);
	if ((now - m_lastTimeCalculateFactionalPlayersCount) > 60)
	{
		m_numPlayersImperial = 0;
		m_numPlayersRebel = 0;
		m_numPlayersNeutral = 0;

		std::set<PlayerObject const *> const &players = PlayerObject::getAllPlayerObjects();
		if (!players.empty())
		{
			CreatureObject const * creatureObject;
			for (std::set<PlayerObject const *>::const_iterator i = players.begin(); i != players.end(); ++i)
			{
				creatureObject = (*i)->getCreatureObject();
				if (creatureObject && creatureObject->isAuthoritative() && creatureObject->getClient())
				{
					if (PvpData::isImperialFactionId(creatureObject->getPvpFaction()))
						++m_numPlayersImperial;
					else if (PvpData::isRebelFactionId(creatureObject->getPvpFaction()))
						++m_numPlayersRebel;
					else
						++m_numPlayersNeutral;
				}
			}
		}

		m_lastTimeCalculateFactionalPlayersCount = now;
	}

	imperial = m_numPlayersImperial;
	rebel = m_numPlayersRebel;
	neutral = m_numPlayersNeutral;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumRunTimeRules()
{
	return m_numRunTimeRules;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumTangibles()
{
	return m_numTangibles;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumUniverseObjects()
{
	return m_numUniverseObjects;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumDynamicAI()
{
	int retval = 0;
	TerrainObject* const terrain = TerrainObject::getInstance();
	if (terrain)
		retval = terrain->getNumberOfReferenceObjects() - getNumPlayers();

	return retval < 0 ? 0 : retval;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumStaticAI()
{
	int retval = getNumAI() - getNumDynamicAI();
	return retval < 0 ? 0 : retval;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumCombatAI()
{
	return m_numCombatAI;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumHibernatingAI()
{
	return m_numHibernatingAI;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumDelayedHibernatingAI()
{
	return m_numDelayedHibernatingAI;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumIntangibles()
{
	return m_numIntangibles;
}

//------------------------------------------------------------------------------------------
int ObjectTracker::getNumMissionDatas()
{
	return m_numMissionDatas;
}
//------------------------------------------------------------------------------------------
int ObjectTracker::getNumMissionObjects()
{
	return m_numMissionObjects;
}
//------------------------------------------------------------------------------------------
int ObjectTracker::getNumMissionListEntries()
{
	return m_numMissionListEntries;
}
//------------------------------------------------------------------------------------------
int ObjectTracker::getNumGroupObjects()
{
	return m_numGroupObjects;
}
//------------------------------------------------------------------------------------------

int ObjectTracker::getNumWaypoints()
{
	return m_numWaypoints;
}

//------------------------------------------------------------------------------------------

int ObjectTracker::getNumPlayerQuestObjects()
{
	return m_numPlayerQuestObjects;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addBuilding()
{
	++m_numBuildings;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addCreature()
{
	++m_numCreatures;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addInstallation()
{
	++m_numInstallations;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addObject()
{
	++m_numObjects;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addPlayer()
{
	++m_numPlayers;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addRunTimeRule()
{
	++m_numRunTimeRules;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addTangible()
{
	++m_numTangibles;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addUniverseObject()
{
	++m_numUniverseObjects;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addCombatAI()
{
	++m_numCombatAI;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addHibernatingAI()
{
	++m_numHibernatingAI;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addDelayedHibernatingAI()
{
	++m_numDelayedHibernatingAI;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addIntangible()
{
	++m_numIntangibles;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addMissionData()
{
	++m_numMissionDatas;
}
//------------------------------------------------------------------------------------------
void ObjectTracker::addMissionObject()
{
	++m_numMissionObjects;
}
//------------------------------------------------------------------------------------------
void ObjectTracker::addMissionListEntry()
{
	++m_numMissionListEntries;
}
//------------------------------------------------------------------------------------------
void ObjectTracker::addGroupObject()
{
	++m_numGroupObjects;
}
//------------------------------------------------------------------------------------------

void ObjectTracker::addWaypoint()
{
	++m_numWaypoints;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::addPlayerQuestObject()
{
	++m_numPlayerQuestObjects;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::removeBuilding()
{
	--m_numBuildings;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::removeCreature()
{
	--m_numCreatures;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::removeInstallation()
{
	--m_numInstallations;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::removeObject()
{
	--m_numObjects;
}

//------------------------------------------------------------------------------------------


void ObjectTracker::removePlayer()
{
	--m_numPlayers;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::removeRunTimeRule()
{
	--m_numRunTimeRules;
}

//------------------------------------------------------------------------------------------


void ObjectTracker::removeTangible()
{
	--m_numTangibles;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::removeUniverseObject()
{
	--m_numUniverseObjects;
}


//------------------------------------------------------------------------------------------

void ObjectTracker::removeCombatAI()
{
	if (m_numCombatAI > 0) --m_numCombatAI;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::removeHibernatingAI()
{
	if (m_numHibernatingAI > 0)
		--m_numHibernatingAI;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::removeDelayedHibernatingAI()
{
	if (m_numDelayedHibernatingAI > 0)
		--m_numDelayedHibernatingAI;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::removeIntangible()
{
	--m_numIntangibles;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::removeMissionData()
{
	--m_numMissionDatas;
}
//------------------------------------------------------------------------------------------
void ObjectTracker::removeMissionObject()
{
	--m_numMissionObjects;
}
//------------------------------------------------------------------------------------------
void ObjectTracker::removeMissionListEntry()
{
	--m_numMissionListEntries;
}
//------------------------------------------------------------------------------------------
void ObjectTracker::removeGroupObject()
{
	--m_numGroupObjects;
}
//------------------------------------------------------------------------------------------

void ObjectTracker::removeWaypoint()
{
	--m_numWaypoints;
}

//------------------------------------------------------------------------------------------

void ObjectTracker::removePlayerQuestObject()
{
	--m_numPlayerQuestObjects;
}

//------------------------------------------------------------------------------------------
