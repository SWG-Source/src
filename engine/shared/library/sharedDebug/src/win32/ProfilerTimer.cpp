// ======================================================================
//
// ProfilerTimer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "shareddebug/FirstSharedDebug.h"
#include "shareddebug/ProfilerTimer.h"

#include "shareddebug/DebugFlags.h"
#include "sharedFoundation/WindowsWrapper.h"

// ======================================================================

namespace ProfilerTimerNamespace
{
	ProfilerTimer::Type ms_qpcFrequency;
	float               ms_floatQpcFrequency;
	__int64             ms_rdtsc;
	__int64             ms_qpc;
	bool                ms_useRdtsc;

}
using namespace ProfilerTimerNamespace;

// ======================================================================

static __int64 __declspec(naked) __stdcall readTimeStampCounter()
{
    __asm
		{
        rdtsc;
        ret;
    }
}

// ======================================================================

void ProfilerTimer::install()
{
	IGNORE_RETURN(QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&ms_qpc)));
	ms_rdtsc = readTimeStampCounter();

	IGNORE_RETURN(QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&ms_qpcFrequency)));
	ms_floatQpcFrequency = static_cast<float>(ms_qpcFrequency);

	DebugFlags::registerFlag(ms_useRdtsc, "SharedDebug/Profiler", "useRdtsc");
}

// ----------------------------------------------------------------------

void ProfilerTimer::getTime(Type &time)
{
	if (ms_useRdtsc)
		time = readTimeStampCounter();
	else
		IGNORE_RETURN(QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&time)));	
}

// ----------------------------------------------------------------------

void ProfilerTimer::getCalibratedTime(Type &time, Type &frequency)
{
	if (ms_useRdtsc)
	{
		__int64 qpc;
		IGNORE_RETURN(QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&qpc)));
		__int64 rdtsc = readTimeStampCounter();

		float const t = static_cast<float>(qpc - ms_qpc) / ms_floatQpcFrequency;
		frequency = static_cast<__int64>(static_cast<float>(rdtsc - ms_rdtsc) / t);

		time = rdtsc;
		ms_qpc = qpc;
		ms_rdtsc = time;
	}
	else
	{
		IGNORE_RETURN(QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&time)));
		frequency = ms_qpcFrequency;
	}
}

void ProfilerTimer::getFrequency(Type &frequency)
{
	frequency = ms_qpcFrequency;
}

// ======================================================================
