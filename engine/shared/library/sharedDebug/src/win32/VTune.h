// ======================================================================
//
// VTune.h
// Copyright 2000-01, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_VTune_H
#define INCLUDED_VTune_H

// ======================================================================

#include "sharedFoundation/Production.h"

// ======================================================================

#if PRODUCTION == 0

class VTune
{
public:

	static void        install();
	
	static void        resume();
	static void        pause();

	static void        resumeNextFrame();
	static void        pauseNextFrame();
	static void        beginFrame();

private:

	static void        remove();
	static void        debugReport();

private:

	enum State
	{
		S_default,
		S_sampling,
		S_paused
	};

	typedef void (__cdecl *PauseFunction)(void);
	typedef void (__cdecl *ResumeFunction)(void);

private:

	static HMODULE        ms_module;
	static PauseFunction  ms_pauseFunction;
	static ResumeFunction ms_resumeFunction;
	static State          ms_state;
	static bool           ms_resumeNextFrame;
	static bool           ms_pauseNextFrame;
	static bool           ms_debugReport;
};

#endif // PRODUCTION == 0

// ======================================================================

#endif
