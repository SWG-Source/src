// ======================================================================
//
// ProfilerTimer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/ProfilerTimer.h"

// ======================================================================

void ProfilerTimer::install()
{
}

// ----------------------------------------------------------------------

void ProfilerTimer::getTime(Type &time)
{
	timeval tv;
	gettimeofday(&tv, 0);
	time = static_cast<Type>(tv.tv_sec)*static_cast<Type>(1000000)+static_cast<Type>(tv.tv_usec);
}

// ----------------------------------------------------------------------

void ProfilerTimer::getCalibratedTime(Type &time, Type &frequency)
{
	getTime(time);
	frequency = 1000000;
}

void ProfilerTimer::getFrequency(Type &frequency)
{
	frequency = 1000000;
}

// ======================================================================
