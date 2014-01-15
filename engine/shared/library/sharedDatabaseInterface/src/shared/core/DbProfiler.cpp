// ======================================================================
//
// DbProfiler.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbProfiler.h"

#include <map>
#include <string>
#include "sharedLog/Log.h"

// ======================================================================

namespace DB
{

	Profiler::Profiler() :
			m_queryDataMap(new QueryDataMapType)
	{
	}

// ----------------------------------------------------------------------

	Profiler::~Profiler()
	{
		delete m_queryDataMap;
		m_queryDataMap = 0;
	}

// ----------------------------------------------------------------------

	void Profiler::addQueryProfileData(const std::string &queryName, int execTime, int fetchTime, int execCount, int fetchCount)
	{
		QueryData &data = (*m_queryDataMap)[queryName];
		data.m_execTime   += execTime;
		data.m_fetchTime  += fetchTime;
		data.m_execCount  += execCount;
		data.m_fetchCount += fetchCount;
	}

// ----------------------------------------------------------------------

	void Profiler::debugOutput()
	{
//		DEBUG_REPORT_LOG(true,("Query profile:\n"));
		for (QueryDataMapType::const_iterator i=m_queryDataMap->begin(); i!=m_queryDataMap->end(); ++i)
		{
			LOG("QueryProfile",("\t%s\t%i ms/exec (%i execs in %i ms, %i fetches in %i ms)\n",
								i->first.c_str(), (i->second.m_execTime / i->second.m_execCount), i->second.m_execCount,
								i->second.m_execTime, i->second.m_fetchCount, i->second.m_fetchTime));
		}
//		DEBUG_REPORT_LOG(true,("End query profile\n"));
	}

// ----------------------------------------------------------------------

	Profiler::QueryData::QueryData() :
			m_execTime(0),
			m_fetchTime(0),
			m_execCount(0),
			m_fetchCount(0)
	{
	}

} // namespace

// ======================================================================
