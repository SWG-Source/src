//CentralServerMetricsData.h
//Copyright 2002 Sony Online Entertainment


#ifndef	_CentralServerMetricsData_H
#define	_CentralServerMetricsData_H

//-----------------------------------------------------------------------

#include "serverMetrics/MetricsData.h"

//-----------------------------------------------------------------------

class CentralServerMetricsData : public MetricsData
{
public:
	CentralServerMetricsData();
	~CentralServerMetricsData();

	virtual void updateData();

private:
	unsigned long m_numChatServers;
	unsigned long m_numConnectionServers;
	unsigned long m_numDatabaseServers;
	unsigned long m_numGameServers;
	unsigned long m_numPlanetServers;
	unsigned long m_population;
	unsigned long m_freeTrialPopulation;
	unsigned long m_emptyScenePopulation;
	unsigned long m_tutorialScenePopulation;
	unsigned long m_falconScenePopulation;
	unsigned long m_universeProcess;
	unsigned long m_isLocked;
	unsigned long m_isSecret;
	unsigned long m_isLoading;
	unsigned long m_clusterId;
	unsigned long m_clusterStartupTime;
	unsigned long m_systemTimeMismatch;
	unsigned long m_taskManagerDisconnected;
	unsigned long m_clusterWideDataQueuedRequests;
	unsigned long m_characterMatchRequests;
	unsigned long m_characterMatchResultsPerRequest;
	unsigned long m_characterMatchTimePerRequest;

	time_t m_timePopulationStatisticsRefresh;
	std::map<std::string, int> m_mapPopulationStatisticsIndex;

	time_t m_timeGcwScoreStatisticsRefresh;
	std::map<std::string, int> m_mapGcwScoreStatisticsIndex;

	time_t m_timeLastLoginTimeStatisticsRefresh;
	std::map<std::string, int> m_mapLastLoginTimeStatisticsIndex;
	std::map<std::string, int> m_mapCreateTimeStatisticsIndex;

private:

	// Disabled.
	CentralServerMetricsData(const CentralServerMetricsData&);
	CentralServerMetricsData &operator =(const CentralServerMetricsData&);
};


//-----------------------------------------------------------------------
#endif
