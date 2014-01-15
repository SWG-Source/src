//GameServerMetricsData.h
//Copyright 2002 Sony Online Entertainment


#ifndef	_GameServerMetricsData_H
#define	_GameServerMetricsData_H

//-----------------------------------------------------------------------

#include "serverMetrics/MetricsData.h"
#include <map>

//-----------------------------------------------------------------------

class GameServerMetricsData : public MetricsData
{
public:
	GameServerMetricsData();
	~GameServerMetricsData();

	virtual void updateData();

private:
	unsigned long m_numClients;

	unsigned long m_numBuildings;
	unsigned long m_numCreatures;
	unsigned long m_numSpawnLimit;
	unsigned long m_numInstallations;
	unsigned long m_numObjects;
	unsigned long m_numPlayers;
	unsigned long m_numPlayersImperial;
	unsigned long m_numPlayersRebel;
	unsigned long m_numPlayersNeutral;
	unsigned long m_numRunTimeRules;
	unsigned long m_numTangibles;
	unsigned long m_numUniverseObjects;

	unsigned long m_numIntangibles;
	unsigned long m_numGroupObjects;
	unsigned long m_numWaypoints;

	unsigned long m_numAI;
	unsigned long m_numDynAI;
	unsigned long m_numStaticAI;
	unsigned long m_numCombatAI;
	unsigned long m_numHibernatingAI;
	unsigned long m_numDelayedHibernatingAI;

	unsigned long m_allocations;

	unsigned long m_gameProcessId;

	unsigned long m_noBuild;

	unsigned long m_numMessages;
	unsigned long m_numObjectsWithBackloggedMessages;

	unsigned long m_numObjectCreatesReceived;
	unsigned long m_numObjectCreatesSent;

	unsigned long m_freeJavaMemory;

	unsigned long m_numServerUIPages;

	unsigned long m_numPendingLoadRequests;

	unsigned long m_numResourceTypesNative;
	unsigned long m_numResourceTypesImported;

	std::map< std::string, unsigned long > m_packetDataMap;

private:

	// Disabled.
	GameServerMetricsData(const GameServerMetricsData&);
	GameServerMetricsData &operator =(const GameServerMetricsData&);
};


//-----------------------------------------------------------------------
#endif
