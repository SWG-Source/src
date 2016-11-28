// ======================================================================
//
// InstallTimer.h
// Copyright 2004 Sony Online Entertainment Inc
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_InstallTimer_H
#define INCLUDED_InstallTimer_H

// ======================================================================

#include "sharedDebug/PerformanceTimer.h"

// ======================================================================

class InstallTimer
{
public:
	
	static void enable();
	static void checkConfigFile();

public:

	InstallTimer(char const * description);
	~InstallTimer();

	void manualExit();

private:

	InstallTimer(InstallTimer const &);
	InstallTimer & operator=(InstallTimer const &);

private:

	char const * m_description;
	PerformanceTimer m_performanceTimer;
};

// ======================================================================

#endif
