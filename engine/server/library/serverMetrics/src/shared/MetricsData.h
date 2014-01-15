
// MetricsData.h
// Copyright 2002 Sony Online Entertainment


#ifndef	_MetricsData_H
#define	_MetricsData_H

class GameNetworkMessage;

#include "serverNetworkMessages/MetricsDataMessage.h"

class MetricsData
{
	friend class MetricsManager;
public:
	MetricsData();
	virtual ~MetricsData() = 0;

	const GameNetworkMessage & getDataToSend();
	virtual void updateData();

	virtual int addMetric( const char* sLabel, int iValue=0, const char* sDescription=0, bool bPersist=true, bool bSummary=false);
	virtual void updateMetric( int iKey, int iValue );

	static MetricsData* getInstance() { return m_spInstance; }

protected:
	std::vector<MetricsPair> m_data;

private:

	MetricsDataMessage  m_message;
	unsigned long       m_memoryUtilization;
	unsigned long       m_memoryUtilizationNoLeakTest;
	unsigned long       m_memoryAllocated;
#ifndef _WIN32
	unsigned long       m_memoryVmSize;
#endif
	unsigned long       m_loopTimeMs;

private:

	static MetricsData* m_spInstance;

	// Disabled.
	MetricsData(const MetricsData&);
	MetricsData &operator =(const MetricsData&);

	std::vector<float> m_frameTimeHistory;
	int m_frameTimeHistoryIndex;
	int m_frameTimeHistorySize;
	float m_frameTimeHistoryTotalTime;
};


#endif
