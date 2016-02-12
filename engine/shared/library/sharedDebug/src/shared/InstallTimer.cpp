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
	m_performanceTimer(),
	m_startingNumberOfBytesAllocated(MemoryManager::getCurrentNumberOfBytesAllocated())
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
		unsigned long const endingNumberOfBytesAllocated = MemoryManager::getCurrentNumberOfBytesAllocated();
		--ms_indent;
		REPORT_LOG_PRINT(ms_enabled, ("InstallTimer:%*c%6.4f %d %s\n", ms_indent * 2, ' ', m_performanceTimer.getElapsedTime(), static_cast<int>(endingNumberOfBytesAllocated - m_startingNumberOfBytesAllocated), m_description));
		m_description = nullptr;
	}
}

// ======================================================================
