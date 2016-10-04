// ======================================================================
//
// ScheduleData.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/ScheduleData.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ScheduleData, true, 0, 0, 0);

// ======================================================================

ScheduleData::ScheduleData(AlterScheduler::ScheduleTime initialMostRecentAlterTime):
	m_mostRecentAlterTime(initialMostRecentAlterTime),
	m_alterNowNext(nullptr),
	m_alterNowPrevious(nullptr),
	m_alterNextFrameNext(nullptr),
	m_alterNextFramePrevious(nullptr),
	m_concludeNext(nullptr),
	m_concludePrevious(nullptr),
	m_scheduleTimeMapIterator(),
	m_schedulePhase(0)
{
}

// ----------------------------------------------------------------------

ScheduleData::~ScheduleData()
{
	DEBUG_WARNING(m_alterNowNext || m_alterNowPrevious, ("ScheduleData for owning object is still in the AlterScheduler AlterNow list, improper object cleanup."));
	DEBUG_WARNING(m_alterNextFrameNext || m_alterNextFramePrevious, ("ScheduleData for owning object is still in the AlterScheduler AlterNextFrame list, improper object cleanup."));
	DEBUG_WARNING(m_concludeNext || m_concludePrevious, ("ScheduleData for owning object is still in the AlterScheduler Conclude list, improper object cleanup."));
	DEBUG_WARNING(AlterScheduler::isIteratorInScheduleTimeMap(&m_scheduleTimeMapIterator), ("ScheduleData for owning object is still in the AlterScheduler ScheduleTimeMap, improper object cleanup."));
	DEBUG_FATAL(m_alterNowNext || m_alterNowPrevious 
		|| m_alterNextFrameNext || m_alterNextFramePrevious 
		|| m_concludeNext || m_concludePrevious
		|| AlterScheduler::isIteratorInScheduleTimeMap(&m_scheduleTimeMapIterator), 
		("ScheduleData not cleaned up properly, referring object not removed from alter scheduler."));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_schedulePhase, AS_MAX_SCHEDULE_PHASE_COUNT);
}

// ======================================================================
