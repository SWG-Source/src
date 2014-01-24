//DatabaseMetricsData.cpp
//Copyright 2002 Sony Online Entertainment

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/DatabaseMetricsData.h"

#ifndef WIN32
#include "MonAPI2/MonitorData.h"
#endif
#include "serverDatabase/CMLoader.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/LazyDeleter.h"
#include "serverDatabase/Loader.h"
#include "serverDatabase/Persister.h"
#include "sharedFoundation/Clock.h"
#include "sharedDatabaseInterface/DbRow.h"
#include "sharedDatabaseInterface/DbTaskQueue.h"
#include "Snapshot.h"

#include <algorithm>

//-----------------------------------------------------------------------

DatabaseMetricsData::DatabaseMetricsData() :
MetricsData(),
m_loadQueueTasks(0),
m_preloadsInProgress(0),
m_oldestUnackedLoad(0),
m_oldestUnackedLoadAlert(0),
m_timeSinceLastSaveStarted(0),
m_lastSaveTime(0),
m_lastSaveTotalObjectCount(0),
m_lastSaveNewObjectCount(0),
m_lazyDeleteQueueSize(0),
m_lazyDeleteTotalCount(0),
m_lazyDeletesPerMinute(0),
m_queryRate(0),
m_fetchRate(0),
m_snapshotCreationCount(0),
m_snapshotDeletionCount(0),
m_snapshotPendingCount(0),
m_snapshotRowCreationCount(0),
m_snapshotRowDeletionCount(0),
m_snapshotRowPendingCount(0),
m_taskQueueTotalNumRequests(0),
m_taskQueueTotalNumResults(0),
m_cmLoadTime(0)
{
	MetricsPair p;

	ADD_METRICS_DATA(loadQueueTasks, 0, false);
	ADD_METRICS_DATA(preloadsInProgress, 0, false);
	ADD_METRICS_DATA(oldestUnackedLoad, 0, false);
	ADD_METRICS_DATA(oldestUnackedLoadAlert, 0, false);
	ADD_METRICS_DATA(timeSinceLastSaveStarted, 0, false);
	ADD_METRICS_DATA(lastSaveTime, 0, true);
	ADD_METRICS_DATA(lastSaveTotalObjectCount, 0, true);
	ADD_METRICS_DATA(lastSaveNewObjectCount, 0, true);
	ADD_METRICS_DATA(lazyDeleteQueueSize, 0, true);
	ADD_METRICS_DATA(lazyDeleteTotalCount, 0, true);
	ADD_METRICS_DATA(lazyDeletesPerMinute, 0, true);
	ADD_METRICS_DATA(queryRate, 0, true);
	ADD_METRICS_DATA(fetchRate, 0, true);

	// snapshot data
	ADD_METRICS_DATA(snapshotCreationCount, 0, true);
	ADD_METRICS_DATA(snapshotDeletionCount, 0, true);
	ADD_METRICS_DATA(snapshotPendingCount, 0, true);

	ADD_METRICS_DATA(snapshotRowCreationCount, 0, true);
	ADD_METRICS_DATA(snapshotRowDeletionCount, 0, true);
	ADD_METRICS_DATA(snapshotRowPendingCount, 0, true);

	// Task Q
	ADD_METRICS_DATA(taskQueueTotalNumRequests, 0, true);
	ADD_METRICS_DATA(taskQueueTotalNumResults, 0, true);

	// commodities data
	ADD_METRICS_DATA(cmLoadTime, 0, true);
	m_data[m_cmLoadTime].m_description = "minutes";
}

//-----------------------------------------------------------------------

DatabaseMetricsData::~DatabaseMetricsData()
{
}

//-----------------------------------------------------------------------

void DatabaseMetricsData::updateData()
{
	MetricsData::updateData();
	m_data[m_loadQueueTasks].m_value = std::max(0, Loader::getInstance().getLoadQueueTasksEstimate());
	m_data[m_preloadsInProgress].m_value = Loader::getInstance().getNumPreloads();

#ifndef WIN32
	int const ageOldestUnackedLoad = Loader::getInstance().getAgeOldestUnackedLoad();
	if (ageOldestUnackedLoad > ConfigServerDatabase::getOldestUnackedLoadAlertThresholdSeconds())
	{
		char buffer[64];
		IGNORE_RETURN(snprintf(buffer, sizeof(buffer)-1, "Threshold %d seconds exceeded by (server %lu)",
			ConfigServerDatabase::getOldestUnackedLoadAlertThresholdSeconds(), Loader::getInstance().getServerOldestUnackedLoad()));
		buffer[sizeof(buffer)-1] = '\0';

		m_data[m_oldestUnackedLoadAlert].m_value = STATUS_LOADING;
		m_data[m_oldestUnackedLoadAlert].m_description = buffer;
	}
	else
	{
		m_data[m_oldestUnackedLoadAlert].m_value = 0;
		m_data[m_oldestUnackedLoadAlert].m_description = "No alert";
	}
	m_data[m_oldestUnackedLoad].m_value = ageOldestUnackedLoad;
	m_data[m_oldestUnackedLoad].m_description = "seconds";
#else
	m_data[m_oldestUnackedLoad].m_value = Loader::getInstance().getAgeOldestUnackedLoad();
	m_data[m_oldestUnackedLoad].m_description = "seconds";
#endif

	m_data[m_timeSinceLastSaveStarted].m_value = Persister::getInstance().getTimeSinceLastSave();
	m_data[m_lastSaveTime].m_value = Persister::getInstance().getLastSaveTime() / 1000;
	m_data[m_lastSaveTotalObjectCount].m_value = Persister::getInstance().getLastSaveTotalObjectCount();
	m_data[m_lastSaveNewObjectCount].m_value = Persister::getInstance().getLastSaveNewObjectCount();
	m_data[m_lazyDeleteQueueSize].m_value = static_cast<int>(LazyDeleter::getInstance().getQueueSize());
	m_data[m_lazyDeleteTotalCount].m_value = LazyDeleter::getInstance().getTotalObjectCount();
	m_data[m_lazyDeletesPerMinute].m_value = static_cast<int>(Clock::getSecondsSinceStart() > 60 ? LazyDeleter::getInstance().getTotalObjectCount() / (Clock::getSecondsSinceStart() / 60) : 0); //lint !e573 !e737 // signed/unsigned int precision
	m_data[m_queryRate].m_value = static_cast<int>(DatabaseProcess::getInstance().getQueryExecRate());
	m_data[m_fetchRate].m_value = static_cast<int>(DatabaseProcess::getInstance().getQueryFetchRate());

	m_data[m_lastSaveTime].m_description = std::string("Completed at ")+Persister::getInstance().getLastSaveCompletionTime();

	float reportRate = static_cast<float>(ConfigServerDatabase::getQueryReportingRate());
	std::string rateDescription;
	if (reportRate == 1)
		rateDescription = "per second";
	else if (reportRate == 60)
		rateDescription = "per minute";
	else
	{
		char buffer[100];
		IGNORE_RETURN(snprintf(buffer,10,"per %f seconds",reportRate));
		rateDescription = buffer;
	}
	m_data[m_queryRate].m_description = rateDescription;
	m_data[m_fetchRate].m_description = rateDescription;
	m_data[m_loadQueueTasks].m_description = ConfigServerDatabase::getDSN();

	m_data[m_snapshotCreationCount].m_value = Snapshot::getCreationCount();
	m_data[m_snapshotDeletionCount].m_value = Snapshot::getDeletionCount();
	m_data[m_snapshotPendingCount].m_value = Snapshot::getPendingCount();

	m_data[m_snapshotRowCreationCount].m_value = DB::Row::getRowsCreated();
	m_data[m_snapshotRowDeletionCount].m_value = DB::Row::getRowsDeleted();

	int i_rows_pending = DB::Row::getRowsCreated() - DB::Row::getRowsDeleted();
	m_data[m_snapshotRowPendingCount].m_value = i_rows_pending > 0 ? i_rows_pending : 0;

	m_data[m_taskQueueTotalNumRequests].m_value = DB::TaskQueue::getTotalNumRequests();
	m_data[m_taskQueueTotalNumResults].m_value = DB::TaskQueue::getTotalNumResults();

	m_data[m_cmLoadTime].m_value = CMLoader::getInstance().getLoadTime();
}

//-----------------------------------------------------------------------
