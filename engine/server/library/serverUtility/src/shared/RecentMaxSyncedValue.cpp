// ======================================================================
//
// RecentMaxSyncedValue.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/RecentMaxSyncedValue.h"

// ======================================================================

namespace RecentMaxSyncedValueNamespace
{

	// ----------------------------------------------------------------------

	int syncStampLongDeltaTime(uint32 stamp1, uint32 stamp2)
	{
		uint32 const delta = stamp1 - stamp2;
		if (delta > 0x7fffffff)
			return static_cast<int>(0xffffffff-delta);
		return static_cast<int>(delta);
	}

	// ----------------------------------------------------------------------

}

using namespace RecentMaxSyncedValueNamespace;

// ======================================================================

RecentMaxSyncedValue::RecentMaxSyncedValue(float value, int checkIntervalMs, int fudgeIntervalMs) :
	m_maximum(value),
	m_oldMaximum(value),
	m_changeSyncStamp(0),
	m_checkIntervalMs(checkIntervalMs),
	m_fudgeIntervalMs(fudgeIntervalMs)
{
}

// ----------------------------------------------------------------------

float RecentMaxSyncedValue::getLastSetValue() const
{
	return m_maximum;
}

// ----------------------------------------------------------------------

float RecentMaxSyncedValue::getRecentMaximum(uint32 curSyncStamp)
{
	// If we have reduced the maximum recently, see if we're still in the
	// interval we allow for keeping old maximums around, and if so,
	// use the faster max speed.  If we have passed the end of the
	// interval, clear the stamp so we don't have to check it anymore.
	if (m_changeSyncStamp)
	{
		int const fudgeTime = m_checkIntervalMs + m_fudgeIntervalMs;
		if (syncStampLongDeltaTime(m_changeSyncStamp, curSyncStamp) < fudgeTime)
			return m_oldMaximum;
		m_changeSyncStamp = 0;
	}
	return m_maximum;
}

// ----------------------------------------------------------------------

void RecentMaxSyncedValue::setValue(float value, uint32 curSyncStamp)
{
	if (value >= getRecentMaximum(curSyncStamp))
	{
		// If we are setting a maximum higher than our current effective maximum,
		// just clear the sync stamp.
		m_changeSyncStamp = 0;
	}
	else
	{
		// We're reducing our maximum, so we need to preserve our old maximum to
		// deal with usage in the time following the change.  We also need to
		// deal with reducing the maximum multiple times in a short interval.

		// m_maximum is our previous maximum.  We know that the new maximum being
		//   set is less than this value from the previous check, so the maximum
		//   is being reduced.

		// m_oldMaximum was our previous old maximum, being kept around to make
		//   sure we're being liberal when determining what the maximum possible
		//   value over a given time interval should be.

		// 1) If our previous maximum is greater than our previous old maximum,
		//    or we don't have a previous old maximum, we push the previous
		//    maximum into the old maximum for future use.
		// 2) Otherwise, we have an old maximum greater than our previous
		//    maximum, so we want to lower the old maximum iff more time has
		//    elapsed than we allow for keeping old maximums around.  Not doing
		//    this could allow us to lower the maximum too quickly.
		if (   m_maximum >= m_oldMaximum
		    || !m_changeSyncStamp
		    || syncStampLongDeltaTime(m_changeSyncStamp, curSyncStamp) >= m_fudgeIntervalMs)
			m_oldMaximum = m_maximum;
		m_changeSyncStamp = curSyncStamp;
	}
	m_maximum = value;
}

// ======================================================================

