// ======================================================================
//
// DbProfiler.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DbProfiler_H
#define INCLUDED_DbProfiler_H

// ======================================================================

namespace DB
{
	class Profiler
	{
	  public:
		Profiler();
		~Profiler();
		void addQueryProfileData(const std::string &queryName, int execTime, int fetchTime, int execCount, int fetchCount);
		void debugOutput();

	  private:
		class QueryData
		{
		  public:
			QueryData();
			
		  public:
			int m_execTime;
			int m_fetchTime;
			int m_execCount;
			int m_fetchCount;
		};

		typedef std::map<std::string, QueryData> QueryDataMapType;
		QueryDataMapType *m_queryDataMap;
	};
	
} //namespace
// ======================================================================

#endif
