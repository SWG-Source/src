// ======================================================================
//
// GameScheduler.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/GameScheduler.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Scheduler.h"

// ======================================================================

namespace GameSchedulerNamespace
{
	const int    cs_ticksPerSecond = 60;
	//const float  cs_secondsPerTick = 1.0f / static_cast<float>(cs_ticksPerSecond);

	Scheduler *s_scheduler;
	bool       s_installed;
}

using namespace GameSchedulerNamespace;

// ======================================================================
// class GameScheduler: public static member functions.
// ======================================================================

void GameScheduler::install()
{
	InstallTimer const installTimer("GameScheduler::install");

	DEBUG_FATAL(s_installed, ("GameScheduler already installed."));

	IS_NULL(s_scheduler);
	s_scheduler = new Scheduler();

	s_installed = true;
	ExitChain::add(remove, "GameScheduler");
}

// ----------------------------------------------------------------------

void GameScheduler::addCallback(Callback callback, const void *context, float elapsedTime)
{
	DEBUG_FATAL(!s_installed, ("GameScheduler not installed."));
	NOT_NULL(s_scheduler);
	
	const unsigned long elapsedTickCount = static_cast<unsigned long>(elapsedTime * static_cast<float>(cs_ticksPerSecond));
	s_scheduler->setCallback(callback, context, elapsedTickCount); 
}

// ----------------------------------------------------------------------

void GameScheduler::alter(float elapsedTime)
{
	DEBUG_FATAL(!s_installed, ("GameScheduler not installed."));
	NOT_NULL(s_scheduler);

	const unsigned long schedulerTickCount = s_scheduler->getCurrentCount() + static_cast<unsigned long>(elapsedTime * static_cast<float>(cs_ticksPerSecond));
	s_scheduler->update(schedulerTickCount);
}

// ======================================================================
// class GameScheduler: private static member functions.
// ======================================================================

void GameScheduler::remove()
{
	DEBUG_FATAL(!s_installed, ("GameScheduler not installed."));

	delete s_scheduler;
	s_scheduler = nullptr;

	s_installed = false;
}

// ======================================================================

