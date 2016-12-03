//MetricsData.cpp
//Copyright 2002 Sony Online Entertainment


#include "serverMetrics/FirstServerMetrics.h"
#include "serverMetrics/MetricsData.h"

#include "serverNetworkMessages/MetricsDataMessage.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Os.h"

#include <algorithm>
#include <vector>

//---------------------------------------------

MetricsData* MetricsData::m_spInstance = 0;


MetricsData::MetricsData() :
m_data(),
m_message(),
m_memoryUtilization(0),
m_memoryUtilizationNoLeakTest(0),
m_memoryAllocated(0),
#ifndef _WIN32
	m_memoryVmSize(0),
#endif
m_loopTimeMs(0),
m_frameTimeHistory(),
m_frameTimeHistoryIndex(0),
m_frameTimeHistorySize(0),
m_frameTimeHistoryTotalTime(0.0f)
{
	MetricsPair p;
	ADD_METRICS_DATA(memoryUtilization, 0, true);
	ADD_METRICS_DATA(memoryUtilizationNoLeakTest, 0, true);
	ADD_METRICS_DATA(memoryAllocated, 0, true);
#ifndef _WIN32
	ADD_METRICS_DATA(memoryVmSize, 0, true);
#endif
	ADD_METRICS_DATA(loopTimeMs, 0, true);

	m_frameTimeHistorySize = ConfigFile::getKeyInt("ServerMetrics", "frameTimeAveragingSize", 11);
	if (m_frameTimeHistorySize <= 0)
	{
		m_frameTimeHistorySize = 1;
	}
	m_frameTimeHistory.resize(m_frameTimeHistorySize);
	for (int i = 0; i < m_frameTimeHistorySize; ++i)
	{
		m_frameTimeHistory[i] = 0.0f;
	}

	m_spInstance = this;
}


//---------------------------------------------


MetricsData::~MetricsData()
{
}

//---------------------------------------------

const GameNetworkMessage & MetricsData::getDataToSend()
{
	updateData();
	m_message.setData(m_data);
	return m_message;
}

//---------------------------------------------

void MetricsData::updateData()
{
	//deal with frame time
	float frameTime = Clock::frameTime() * 1000;
	float oldFrameTime = m_frameTimeHistory[m_frameTimeHistoryIndex];

	m_frameTimeHistoryTotalTime += (frameTime - oldFrameTime);
	m_frameTimeHistory[m_frameTimeHistoryIndex] = frameTime;
	++m_frameTimeHistoryIndex;
	if (m_frameTimeHistoryIndex >= m_frameTimeHistorySize)
		m_frameTimeHistoryIndex = 0;
	
	m_data[m_loopTimeMs].m_value = std::max(0, static_cast<int>(m_frameTimeHistoryTotalTime / m_frameTimeHistorySize));
}

//---------------------------------------------
int MetricsData::addMetric( const char* sLabel, int iValue, const char* sDescription, bool bPersist, bool bSummary)
{
	MetricsPair p;
	p.m_label = sLabel;
	p.m_value = iValue;
	p.m_description = sDescription ? sDescription : "";
	p.m_persistData = bPersist;
	p.m_summary = bSummary;
	m_data.push_back(p);
	return( m_data.size()-1 );
}

//---------------------------------------------
void MetricsData::updateMetric( int iKey, int iValue )
{
	if ( iKey >= int(m_data.size()) )
		return;
	if ( iKey < 0 )
		return;
	m_data[ iKey ].m_value = iValue;
}








