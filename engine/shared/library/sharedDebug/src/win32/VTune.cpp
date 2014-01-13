// ======================================================================
//
// VTune.cpp
// Copyright 2000-01, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/VTune.h"

#if PRODUCTION == 0

#include "sharedDebug/DebugFlags.h"

#include <vtune/vtuneapi.h>

// ======================================================================

HMODULE               VTune::ms_module;
VTune::PauseFunction  VTune::ms_pauseFunction;
VTune::ResumeFunction VTune::ms_resumeFunction;
VTune::State          VTune::ms_state;
bool                  VTune::ms_resumeNextFrame;
bool                  VTune::ms_pauseNextFrame;
bool                  VTune::ms_debugReport;

// ======================================================================

void VTune::install()
{
	DEBUG_FATAL(ms_module, ("vtune already installed"));

	ms_module = LoadLibrary("VTuneAPI");
	if (!ms_module)
		return;

	ms_pauseFunction = reinterpret_cast<PauseFunction>(GetProcAddress(ms_module, "VTPause"));
	ms_resumeFunction = reinterpret_cast<PauseFunction>(GetProcAddress(ms_module, "VTResume"));
	ms_state = S_default;

#if PRODUCTION == 0
	DebugFlags::registerFlag(ms_debugReport, "SharedDebug", "vtuneState", debugReport);
#endif
}

// ----------------------------------------------------------------------

void VTune::remove()
{
	if (ms_module)
	{
		FreeLibrary(ms_module);
		ms_pauseFunction = 0;
		ms_resumeFunction = 0;
	}
}

// ----------------------------------------------------------------------

void VTune::debugReport()
{
	switch (ms_state)
	{
		case S_default:
			REPORT_PRINT(true, ("Vtune state unknown\n"));
			break;

		case S_sampling:
			REPORT_PRINT(true, ("Vtune is sampling\n"));
			break;

		case S_paused:
			REPORT_PRINT(true, ("Vtune is NOT sampling\n"));
			break;

		default:
			DEBUG_FATAL(true, ("bad case"));

	}
}

// ----------------------------------------------------------------------

void VTune::resume()
{
	if (ms_module && ms_resumeFunction)
	{
		MessageBeep(MB_OK);
		(*ms_resumeFunction)();
		ms_state = S_sampling;
	}
}

// ----------------------------------------------------------------------

void VTune::pause()
{
	if (ms_module && ms_pauseFunction)
	{
		(*ms_pauseFunction)();
		MessageBeep(MB_ICONEXCLAMATION);
		ms_state = S_paused;
	}
}

// ----------------------------------------------------------------------

void VTune::pauseNextFrame()
{
	ms_pauseNextFrame = true;
	ms_resumeNextFrame = false;
}

// ----------------------------------------------------------------------

void VTune::resumeNextFrame()
{
	ms_pauseNextFrame = false;
	ms_resumeNextFrame = true;
}

// ----------------------------------------------------------------------

void VTune::beginFrame()
{
	if (ms_pauseNextFrame)
	{
		pause();
		ms_pauseNextFrame = false;
	}

	if (ms_resumeNextFrame)
	{
		resume();
		ms_resumeNextFrame = false;
	}
}

// ======================================================================

#endif // PRODUCTION == 0
