//
// PerformanceTimer.cpp
// Copyright 2000-2004 Sony Online Entertainment
//

//-------------------------------------------------------------------

#include "shareddebug/FirstSharedDebug.h"
#include "shareddebug/PerformanceTimer.h"

//-------------------------------------------------------------------

#include <cstdio>

//-------------------------------------------------------------------

__int64 PerformanceTimer::ms_frequency;

//-------------------------------------------------------------------

void PerformanceTimer::install()
{
	BOOL result = QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&ms_frequency));
	FATAL(!result, ("PerformanceTimer::install QPF failed"));
}

//-------------------------------------------------------------------

PerformanceTimer::PerformanceTimer() :
	m_startTime (0),
	m_stopTime (0)
{
	DEBUG_FATAL (ms_frequency == 0.f, ("PerformanceTimer not installed"));
}

//-------------------------------------------------------------------

PerformanceTimer::~PerformanceTimer()
{
}

//-------------------------------------------------------------------

void PerformanceTimer::start()
{
	//-- get the current time
	BOOL result = QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&m_startTime));
	DEBUG_FATAL(!result, ("PerformanceTimer::start QPC failed"));
	UNREF (result);
}

//-------------------------------------------------------------------

void PerformanceTimer::resume()
{
	__int64 delta = m_stopTime - m_startTime;
	start();
	m_startTime -= delta;
}

//-------------------------------------------------------------------

void PerformanceTimer::stop ()
{
	//-- get the current time
	BOOL result = QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&m_stopTime));
	DEBUG_FATAL(!result, ("PerformanceTimer::stop QPC failed"));
	UNREF (result);
}

//-------------------------------------------------------------------

float PerformanceTimer::getElapsedTime() const
{
	return static_cast<float> (m_stopTime - m_startTime) / static_cast<float> (ms_frequency);
}

// ----------------------------------------------------------------------

float PerformanceTimer::getSplitTime() const
{
	__int64 currentTime;
	BOOL result = QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&currentTime));
	UNREF(result);
	DEBUG_FATAL(!result, ("PerformanceTimer::getSplitTime QPC failed"));

	return static_cast<float> (currentTime - m_startTime) / static_cast<float> (ms_frequency);
}

//-------------------------------------------------------------------

void PerformanceTimer::logElapsedTime(const char* string) const
{
	UNREF (string);

#ifdef _DEBUG
	static char buffer [1000];
	sprintf (buffer, "%s : %1.5f seconds\n", string ? string : "null", getElapsedTime());
	DEBUG_REPORT_LOG_PRINT (true, ("%s", buffer));
	DEBUG_OUTPUT_CHANNEL("Foundation\\PerformanceTimer", ("%s", buffer));
#endif
}

//-------------------------------------------------------------------

