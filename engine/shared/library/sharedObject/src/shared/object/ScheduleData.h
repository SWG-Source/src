// ======================================================================
//
// ScheduleData.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ScheduleData_H
#define INCLUDED_ScheduleData_H

// ======================================================================

#include "sharedObject/AlterScheduler.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include <map>

// ======================================================================

class ScheduleData
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	explicit ScheduleData(AlterScheduler::ScheduleTime initialMostRecentAlterTime);
	~ScheduleData();

	void                          setMostRecentAlterTime(AlterScheduler::ScheduleTime scheduleTime);
	AlterScheduler::ScheduleTime  getMostRecentAlterTime();

	Object                       *getAlterNowNext();
	Object                       *getAlterNowPrevious();

	void                          setAlterNowNext(Object *object);
	void                          setAlterNowPrevious(Object *object);

	Object                       *getAlterNextFrameNext();
	Object                       *getAlterNextFramePrevious();

	void                          setAlterNextFrameNext(Object *object);
	void                          setAlterNextFramePrevious(Object *object);

	Object                       *getConcludeNext();
	Object                       *getConcludePrevious();

	void                          setConcludeNext(Object *object);
	void                          setConcludePrevious(Object *object);

	AlterScheduler::ScheduleTimeMap::iterator &getScheduleTimeMapIterator();

	int                           getSchedulePhase() const;
	void                          setSchedulePhase(int schedulePhase);

private:

	// Disabled.
	ScheduleData();
	ScheduleData(ScheduleData const&);
	ScheduleData &operator =(ScheduleData const&);

private:

	AlterScheduler::ScheduleTime               m_mostRecentAlterTime;

	Object                                    *m_alterNowNext;
	Object                                    *m_alterNowPrevious;

	Object                                    *m_alterNextFrameNext;
	Object                                    *m_alterNextFramePrevious;

	Object                                    *m_concludeNext;
	Object                                    *m_concludePrevious;

	AlterScheduler::ScheduleTimeMap::iterator  m_scheduleTimeMapIterator;

	int                                        m_schedulePhase;

};

// ======================================================================

inline void ScheduleData::setMostRecentAlterTime(AlterScheduler::ScheduleTime scheduleTime)
{
	m_mostRecentAlterTime = scheduleTime;
}

// ----------------------------------------------------------------------

inline AlterScheduler::ScheduleTime ScheduleData::getMostRecentAlterTime()
{
	return m_mostRecentAlterTime;
}

// ----------------------------------------------------------------------

inline Object *ScheduleData::getAlterNowNext()
{
	return m_alterNowNext;
}

// ----------------------------------------------------------------------

inline Object *ScheduleData::getAlterNowPrevious()
{
	return m_alterNowPrevious;
}

// ----------------------------------------------------------------------

inline void ScheduleData::setAlterNowNext(Object *object)
{
	m_alterNowNext = object;
}

// ----------------------------------------------------------------------

inline void ScheduleData::setAlterNowPrevious(Object *object)
{
	m_alterNowPrevious = object;
}

// ----------------------------------------------------------------------

inline Object *ScheduleData::getAlterNextFrameNext()
{
	return m_alterNextFrameNext;
}

// ----------------------------------------------------------------------

inline Object *ScheduleData::getAlterNextFramePrevious()
{
	return m_alterNextFramePrevious;
}

// ----------------------------------------------------------------------

inline void ScheduleData::setAlterNextFrameNext(Object *object)
{
	m_alterNextFrameNext = object;
}

// ----------------------------------------------------------------------

inline void ScheduleData::setAlterNextFramePrevious(Object *object)
{
	m_alterNextFramePrevious = object;
}

// ----------------------------------------------------------------------

inline Object *ScheduleData::getConcludeNext()
{
	return m_concludeNext;
}

// ----------------------------------------------------------------------

inline Object *ScheduleData::getConcludePrevious()
{
	return m_concludePrevious;
}

// ----------------------------------------------------------------------

inline void ScheduleData::setConcludeNext(Object *object)
{
	m_concludeNext = object;
}

// ----------------------------------------------------------------------

inline void ScheduleData::setConcludePrevious(Object *object)
{
	m_concludePrevious = object;
}

// ----------------------------------------------------------------------

inline AlterScheduler::ScheduleTimeMap::iterator &ScheduleData::getScheduleTimeMapIterator()
{
	return m_scheduleTimeMapIterator;
}

// ----------------------------------------------------------------------

inline int ScheduleData::getSchedulePhase() const
{
	return m_schedulePhase;
}

// ----------------------------------------------------------------------

inline void ScheduleData::setSchedulePhase(int schedulePhase)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, schedulePhase, AS_MAX_SCHEDULE_PHASE_COUNT);
	m_schedulePhase = schedulePhase;
}

// ======================================================================

#endif
