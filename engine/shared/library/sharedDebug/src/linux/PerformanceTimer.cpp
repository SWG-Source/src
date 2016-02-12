//=================================================================== //
//
// PerformanceTimer.cpp
// copyright 2000-2004 Sony Online Entertainment
// All Rights Reserved
//
//===================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedDebug/PerformanceTimer.h"

//===================================================================

#include <cstdio>

//===================================================================

void PerformanceTimer::install()
{
}

//-------------------------------------------------------------------

PerformanceTimer::PerformanceTimer()
{
	startTime.tv_sec = 0;
	startTime.tv_usec = 0;
	stopTime.tv_sec = 0;
	stopTime.tv_usec = 0;
}

//-------------------------------------------------------------------

PerformanceTimer::~PerformanceTimer()
{
}

//-------------------------------------------------------------------

void PerformanceTimer::start()
{
	int const result = gettimeofday(&startTime, nullptr);
	FATAL(result != 0,("PerformanceTimer::start failed"));
}

//-------------------------------------------------------------------

void PerformanceTimer::resume()
{
	long sec = stopTime.tv_sec - startTime.tv_sec;
	long usec = stopTime.tv_usec - startTime.tv_usec;
	if(usec < 0)
	{
	  --sec;
	  usec += 1000000;
	}

	int const result = gettimeofday(&startTime, nullptr);
	FATAL(result != 0,("PerformanceTimer::resume failed"));

	startTime.tv_sec -= sec;
	startTime.tv_usec -= usec;
	if(startTime.tv_usec < 0)
	{
	  --startTime.tv_sec;
	  startTime.tv_usec += 1000000;
	}
}

//-------------------------------------------------------------------

void PerformanceTimer::stop()
{
	int  result = gettimeofday(&stopTime, nullptr);
	FATAL(result != 0,("PerformanceTimer::start failed"));
}

//-------------------------------------------------------------------

float PerformanceTimer::getElapsedTime() const
{
	long sec = stopTime.tv_sec - startTime.tv_sec;
	long usec = stopTime.tv_usec - startTime.tv_usec;
	if(usec < 0)
	{
	  --sec;
	  usec += 1000000;
	}
	return static_cast<float>(sec) +(static_cast<float>(usec) / 1000000.0f);
}

//-------------------------------------------------------------------

float PerformanceTimer::getSplitTime() const
{
	timeval currentTime;

	int const result = gettimeofday(&currentTime, nullptr);
	FATAL(result != 0,("PerformanceTimer::getSplitTime failed"));

	long sec = currentTime.tv_sec - startTime.tv_sec;
	long usec = currentTime.tv_usec - startTime.tv_usec;
	if(usec < 0)
	{
	  --sec;
	  usec += 1000000;
	}

	return static_cast<float>(sec) +(static_cast<float>(usec) / 1000000.0f);
}

//-------------------------------------------------------------------

void PerformanceTimer::logElapsedTime(const char* string) const
{
	UNREF(string);

#ifdef _DEBUG
	static char buffer [1000];
	sprintf(buffer, "%s : %1.5f seconds\n", string ? string : "nullptr", getElapsedTime());
	DEBUG_REPORT_LOG_PRINT(true, ("%s", buffer));
#endif
}

//-------------------------------------------------------------------
