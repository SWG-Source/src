//DatabaseMetricsData.h
//Copyright 2002 Sony Online Entertainment


#ifndef	_DatabaseMetricsData_H
#define	_DatabaseMetricsData_H

//-----------------------------------------------------------------------

#include "serverMetrics/MetricsData.h"

//-----------------------------------------------------------------------

class DatabaseMetricsData : public MetricsData
{
public:
	DatabaseMetricsData();
	~DatabaseMetricsData();

	virtual void updateData();

private:
	unsigned long m_loadQueueTasks;
	unsigned long m_preloadsInProgress;
	unsigned long m_oldestUnackedLoad;
	unsigned long m_oldestUnackedLoadAlert;
	unsigned long m_timeSinceLastSaveStarted;
	unsigned long m_lastSaveTime;
	unsigned long m_lastSaveTotalObjectCount;
	unsigned long m_lastSaveNewObjectCount;
	unsigned long m_lazyDeleteQueueSize;
	unsigned long m_lazyDeleteTotalCount;
	unsigned long m_lazyDeletesPerMinute;
	unsigned long m_queryRate;
	unsigned long m_fetchRate;

	unsigned long m_snapshotCreationCount;
	unsigned long m_snapshotDeletionCount;
	unsigned long m_snapshotPendingCount;

	unsigned long m_snapshotRowCreationCount;
	unsigned long m_snapshotRowDeletionCount;
	unsigned long m_snapshotRowPendingCount;

	unsigned long m_taskQueueTotalNumRequests;
	unsigned long m_taskQueueTotalNumResults;

	unsigned long m_cmLoadTime;

private:

	// Disabled.
	DatabaseMetricsData(const DatabaseMetricsData&);
	DatabaseMetricsData &operator =(const DatabaseMetricsData&);
};


//-----------------------------------------------------------------------
#endif
