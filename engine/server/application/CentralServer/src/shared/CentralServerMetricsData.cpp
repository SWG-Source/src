// ======================================================================
//
// CentralServerMetricsData.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstCentralServer.h"
#include "CentralServerMetricsData.h"

#include "CentralServer.h"
#include "UniverseManager.h"
#include "ConfigCentralServer.h"
#ifndef WIN32
#include "MonAPI2/MonitorData.h"
#endif
#include "serverNetworkMessages/MetricsDataMessage.h"
#include "serverUtility/ClusterWideDataManagerList.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <algorithm>

// ======================================================================

CentralServerMetricsData::CentralServerMetricsData() :
	MetricsData(),
	m_numChatServers(0),
	m_numConnectionServers(0),
	m_numDatabaseServers(0),
	m_numGameServers(0),
	m_numPlanetServers(0),
	m_population(0),
	m_freeTrialPopulation(0),
	m_emptyScenePopulation(0),
	m_tutorialScenePopulation(0),
	m_falconScenePopulation(0),
	m_universeProcess(0),
	m_isLocked(0),
	m_isSecret(0),
	m_isLoading(0),
	m_clusterId(0),
	m_clusterStartupTime(0),
	m_systemTimeMismatch(0),
	m_taskManagerDisconnected(0),
	m_clusterWideDataQueuedRequests(0),
	m_characterMatchRequests(0),
	m_characterMatchResultsPerRequest(0),
	m_characterMatchTimePerRequest(0),
	m_timePopulationStatisticsRefresh(0),
	m_mapPopulationStatisticsIndex(),
	m_timeGcwScoreStatisticsRefresh(0),
	m_mapGcwScoreStatisticsIndex(),
	m_timeLastLoginTimeStatisticsRefresh(0),
	m_mapLastLoginTimeStatisticsIndex(),
	m_mapCreateTimeStatisticsIndex()
{
	MetricsPair p;

	ADD_METRICS_DATA(numChatServers, 0, true);
	ADD_METRICS_DATA(numConnectionServers, 0, true);
	ADD_METRICS_DATA(numDatabaseServers, 0, false);
	ADD_METRICS_DATA(numGameServers, 0, true);
	ADD_METRICS_DATA(numPlanetServers, 0, true);
	ADD_METRICS_DATA(population, 0, true); // ***MUST*** be before isLocked, isSecret, and isLoading
	ADD_METRICS_DATA(freeTrialPopulation, 0, true);
	ADD_METRICS_DATA(emptyScenePopulation, 0, true);
	ADD_METRICS_DATA(tutorialScenePopulation, 0, true);
	ADD_METRICS_DATA(falconScenePopulation, 0, true);
	ADD_METRICS_DATA(universeProcess, 0, false);
	ADD_METRICS_DATA(isLocked, 0, false);
	ADD_METRICS_DATA(isSecret, 0, false);
	ADD_METRICS_DATA(isLoading, 0, true);
	ADD_METRICS_DATA(clusterId, 0, false);
	ADD_METRICS_DATA(clusterStartupTime, 0, true);
	ADD_METRICS_DATA(systemTimeMismatch, 0, false);
	ADD_METRICS_DATA(taskManagerDisconnected, 0, false);
	ADD_METRICS_DATA(clusterWideDataQueuedRequests, 0, false)
	ADD_METRICS_DATA(characterMatchRequests, 0, false);
	ADD_METRICS_DATA(characterMatchResultsPerRequest, 0, false);
	ADD_METRICS_DATA(characterMatchTimePerRequest, 0, false);

	m_data[m_clusterId].m_description = ConfigCentralServer::getClusterName();
	m_data[m_clusterStartupTime].m_description = "minutes";
	m_data[m_characterMatchTimePerRequest].m_description = "ms";
}

//-----------------------------------------------------------------------

CentralServerMetricsData::~CentralServerMetricsData()
{
}

//-----------------------------------------------------------------------

void CentralServerMetricsData::updateData() 
{
	MetricsData::updateData();
	m_data[m_numChatServers].m_value = CentralServer::getInstance().getNumChatServers();
	m_data[m_numConnectionServers].m_value = CentralServer::getInstance().getNumConnectionServers();
	m_data[m_numDatabaseServers].m_value = CentralServer::getInstance().getNumDatabaseServers();
	m_data[m_numGameServers].m_value = CentralServer::getInstance().getNumGameServers();
	m_data[m_numPlanetServers].m_value = CentralServer::getInstance().getNumPlanetServers();

#ifndef WIN32
	// if the cluster is still in initial startup, set population value to
	// "loading" to cause the top level node to appear yellow
	if (CentralServer::getInstance().isInClusterInitialStartup())
		m_data[m_population].m_value = STATUS_LOADING;
	else
#endif
		m_data[m_population].m_value = CentralServer::getInstance().getPlayerCount();

	m_data[m_freeTrialPopulation].m_value = CentralServer::getInstance().getFreeTrialCount();
	m_data[m_emptyScenePopulation].m_value = CentralServer::getInstance().getEmptySceneCount();
	m_data[m_tutorialScenePopulation].m_value = CentralServer::getInstance().getTutorialSceneCount();
	m_data[m_falconScenePopulation].m_value = CentralServer::getInstance().getFalconSceneCount();
	m_data[m_universeProcess].m_value = static_cast<int>(UniverseManager::getInstance().getUniverseProcess());
	m_data[m_isLocked].m_value = ( CentralServer::getInstance().getIsClusterLocked() ) ? 1 : 0;
	m_data[m_isSecret].m_value = ( CentralServer::getInstance().getIsClusterSecret() ) ? 1 : 0;
	if (CentralServer::getInstance().isPreloadFinished())
	{
		m_data[m_isLoading].m_value = 0;
		m_data[m_isLoading].m_description="Load finished.";
	}
	else
	{
		m_data[m_isLoading].m_value = std::max(1, CentralServer::getInstance().getSecondsClusterHasBeenInLoadingState());
		if (CentralServer::getInstance().getNumPlanetServers() ==0)
			m_data[m_isLoading].m_description = "(seconds) No planet servers running.";
		else if (CentralServer::getInstance().getPlanetsWaitingForPreloadCount() > 0)
			m_data[m_isLoading].m_description = "(seconds) Starting or recovering game servers.";
		else if (CentralServer::getInstance().isDatabaseBacklogged())
			m_data[m_isLoading].m_description = "(seconds) Waiting for database backlog to clear.";
		else
			m_data[m_isLoading].m_description = "(seconds) Reason for loading state is not available.";
	}

	m_data[m_clusterId].m_value = static_cast<int>(CentralServer::getInstance().getClusterId());
	m_data[m_clusterStartupTime].m_value = CentralServer::getInstance().getClusterStartupTime();

	time_t const lastTimeSystemTimeMismatchNotification = CentralServer::getInstance().getLastTimeSystemTimeMismatchNotification();
	if (lastTimeSystemTimeMismatchNotification)
	{
		m_data[m_systemTimeMismatch].m_description = CentralServer::getInstance().getLastTimeSystemTimeMismatchNotificationDescription();

		// display the node as red (causing the top level node to
		// appear yellow to draw attention) for some amount of time
		// after detecting a system time mismatch issue
#ifndef WIN32
		if ((lastTimeSystemTimeMismatchNotification + ConfigCentralServer::getSystemTimeMismatchAlertIntervalSeconds()) > ::time(nullptr))
			m_data[m_systemTimeMismatch].m_value = STATUS_LOADING;
		else
			m_data[m_systemTimeMismatch].m_value = 1;
#else
		m_data[m_systemTimeMismatch].m_value = 1;
#endif
	}
	else
	{
		m_data[m_systemTimeMismatch].m_value = 0;
		m_data[m_systemTimeMismatch].m_description = "None detected so far.";
	}

	std::string const & disconnectedTaskManagerList = CentralServer::getInstance().getDisconnectedTaskManagerList();
	if (disconnectedTaskManagerList.empty())
	{
		m_data[m_taskManagerDisconnected].m_value = 0;
		m_data[m_taskManagerDisconnected].m_description = "None detected so far.";
	}
	else
	{
#ifndef WIN32
		m_data[m_taskManagerDisconnected].m_value = STATUS_LOADING;
#else
		m_data[m_taskManagerDisconnected].m_value = 1;
#endif

		m_data[m_taskManagerDisconnected].m_description = disconnectedTaskManagerList;
	}

	m_data[m_clusterWideDataQueuedRequests].m_value = ClusterWideDataManagerList::getNumberOfQueuedRequests();

	// handle character match statistics
	int numberOfCharacterMatchRequests, numberOfCharacterMatchResultsPerRequest, timeSpentPerCharacterMatchRequestMs;
	CentralServer::getInstance().getCharacterMatchStatistics(numberOfCharacterMatchRequests, numberOfCharacterMatchResultsPerRequest, timeSpentPerCharacterMatchRequestMs);
	m_data[m_characterMatchRequests].m_value = numberOfCharacterMatchRequests;
	m_data[m_characterMatchResultsPerRequest].m_value = numberOfCharacterMatchResultsPerRequest;
	m_data[m_characterMatchTimePerRequest].m_value = timeSpentPerCharacterMatchRequestMs;

	// handle population statistics
	time_t timePopulationStatisticsRefresh;
	const std::map<std::string, int> & populationStatistics = CentralServer::getInstance().getPopulationStatistics(timePopulationStatisticsRefresh);
	if (!populationStatistics.empty() && (timePopulationStatisticsRefresh != m_timePopulationStatisticsRefresh))
	{
		if (m_mapPopulationStatisticsIndex.empty())
		{
			for (std::map<std::string, int>::const_iterator iter = populationStatistics.begin(); iter != populationStatistics.end(); ++iter)
			{
				if (!iter->first.empty() && (iter->second >= 0) && (m_mapPopulationStatisticsIndex.count(iter->first) < 1))
				{
					std::string label("population.");
					label += iter->first;

					m_mapPopulationStatisticsIndex[iter->first] = addMetric(label.c_str(), 0, nullptr, false, false);
				}
			}
		}

		for (std::map<std::string, int>::const_iterator iter = m_mapPopulationStatisticsIndex.begin(); iter != m_mapPopulationStatisticsIndex.end(); ++iter)
		{
			std::map<std::string, int>::const_iterator iterFind = populationStatistics.find(iter->first);
			if (iterFind != populationStatistics.end())
				updateMetric(iter->second, iterFind->second);
			else
				updateMetric(iter->second, 0);
		}

		m_timePopulationStatisticsRefresh = timePopulationStatisticsRefresh;
	}

	// handle GCW score statistics
	time_t timeGcwScoreStatisticsRefresh;
	const std::map<std::string, std::pair<int, std::pair<std::string, std::string> > > & gcwScoreStatistics = CentralServer::getInstance().getGcwScoreStatistics(timeGcwScoreStatisticsRefresh);
	std::map<std::string, int>::const_iterator iterFind;
	int gcwScoreStatisticsIndex;
	if (!gcwScoreStatistics.empty() && (timeGcwScoreStatisticsRefresh != m_timeGcwScoreStatisticsRefresh))
	{
		for (std::map<std::string, std::pair<int, std::pair<std::string, std::string> > >::const_iterator iter = gcwScoreStatistics.begin(); iter != gcwScoreStatistics.end(); ++iter)
		{
			iterFind = m_mapGcwScoreStatisticsIndex.find(iter->first);
			if (iterFind != m_mapGcwScoreStatisticsIndex.end())
			{
				gcwScoreStatisticsIndex = iterFind->second;
			}
			else
			{
				gcwScoreStatisticsIndex = addMetric(iter->first.c_str(), 0, nullptr, false, false);
				IGNORE_RETURN(m_mapGcwScoreStatisticsIndex.insert(std::make_pair(iter->first, gcwScoreStatisticsIndex)));
			}

			m_data[gcwScoreStatisticsIndex].m_value = iter->second.first;
			m_data[gcwScoreStatisticsIndex].m_description = iter->second.second.second;
		}

		m_timeGcwScoreStatisticsRefresh = timeGcwScoreStatisticsRefresh;
	}

	// handle total characters on cluster and character last login time statistics
	time_t timeLastLoginTimeStatisticsRefresh;
	std::pair<std::map<int, std::pair<std::string, int> > const *, std::map<int, std::pair<std::string, int> > const *> statistics = CentralServer::getInstance().getLastLoginTimeStatistics(timeLastLoginTimeStatisticsRefresh);
	const std::map<int, std::pair<std::string, int> > & lastLoginTimeStatistics = *(statistics.first);
	const std::map<int, std::pair<std::string, int> > & createTimeStatistics = *(statistics.second);
	if (!lastLoginTimeStatistics.empty() && !createTimeStatistics.empty() && (timeLastLoginTimeStatisticsRefresh != m_timeLastLoginTimeStatisticsRefresh))
	{
		if (m_mapLastLoginTimeStatisticsIndex.empty())
		{
			for (std::map<int, std::pair<std::string, int> >::const_iterator iter = lastLoginTimeStatistics.begin(); iter != lastLoginTimeStatistics.end(); ++iter)
			{
				std::pair<std::string, int> secondIter = iter->second;
				if (!secondIter.first.empty() && (m_mapLastLoginTimeStatisticsIndex.count(secondIter.first) < 1))
				{
					std::string label("population.");
					label += secondIter.first;

					m_mapLastLoginTimeStatisticsIndex[secondIter.first] = addMetric(label.c_str(), 0, nullptr, false, false);
				}
			}
		}

		std::map<int, std::pair<std::string, int> >::const_iterator iter;
		for (iter = lastLoginTimeStatistics.begin(); iter != lastLoginTimeStatistics.end(); ++iter)
		{
			if (!iter->second.first.empty())
			{
				std::map<std::string, int>::const_iterator iterFind = m_mapLastLoginTimeStatisticsIndex.find(iter->second.first);
				if (iterFind != m_mapLastLoginTimeStatisticsIndex.end())
					updateMetric(iterFind->second, iter->second.second);
			}
		}

		if (m_mapCreateTimeStatisticsIndex.empty())
		{
			for (std::map<int, std::pair<std::string, int> >::const_iterator iter = createTimeStatistics.begin(); iter != createTimeStatistics.end(); ++iter)
			{
				if (!iter->second.first.empty() && (m_mapCreateTimeStatisticsIndex.count(iter->second.first) < 1) && (m_mapLastLoginTimeStatisticsIndex.count(iter->second.first) < 1))
				{
					std::string label("population.");
					label += iter->second.first;

					m_mapCreateTimeStatisticsIndex[iter->second.first] = addMetric(label.c_str(), 0, nullptr, false, false);
				}
			}
		}

		for (iter = createTimeStatistics.begin(); iter != createTimeStatistics.end(); ++iter)
		{
			if (!iter->second.first.empty())
			{
				std::map<std::string, int>::const_iterator iterFind = m_mapCreateTimeStatisticsIndex.find(iter->second.first);
				if (iterFind != m_mapCreateTimeStatisticsIndex.end())
					updateMetric(iterFind->second, iter->second.second);
			}
		}

		m_timeLastLoginTimeStatisticsRefresh = timeLastLoginTimeStatisticsRefresh;
	}
}

// ======================================================================
