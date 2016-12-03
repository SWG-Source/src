// ======================================================================
//
// InstallTimer.cpp
// Copyright 2004 Sony Online Entertainment Inc
// All Rights Reserved
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/InstallTimer.h"

#include "sharedFoundation/ConfigFile.h"

// ======================================================================

namespace InstallTimerNamespace
{
	bool ms_enabled;
	int ms_indent = 1;
}
using namespace InstallTimerNamespace;

// ======================================================================

void InstallTimer::enable()
{
	ms_enabled = true;
}

// ----------------------------------------------------------------------

void InstallTimer::checkConfigFile()
{
	ms_enabled = ConfigFile::getKeyBool("SharedDebug/InstallTimer", "enabled", false);
}

// ======================================================================

InstallTimer::InstallTimer(char const * description)
:
	m_description(description),
	m_performanceTimer()
{
	NOT_NULL(m_description);
	m_performanceTimer.start();
	++ms_indent;
}

// ----------------------------------------------------------------------

InstallTimer::~InstallTimer()
{
	manualExit();
}

// ----------------------------------------------------------------------

void InstallTimer::manualExit()
{
	if (m_description)
	{
		m_performanceTimer.stop();
		--ms_indent;
		m_description = nullptr;
	}
}

// ======================================================================
