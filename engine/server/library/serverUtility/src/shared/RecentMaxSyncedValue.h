// ======================================================================
//
// RecentMaxSyncedValue.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_RecentMaxSyncedValue_H
#define INCLUDED_RecentMaxSyncedValue_H

class RecentMaxSyncedValue
{
public:
	RecentMaxSyncedValue(float value, int checkIntervalMs, int fudgeIntervalMs);
	float getLastSetValue() const;
	float getRecentMaximum(uint32 curSyncStamp);
	void setValue(float value, uint32 curSyncStamp);

private:
	RecentMaxSyncedValue();
	RecentMaxSyncedValue(RecentMaxSyncedValue const &);
	RecentMaxSyncedValue &operator=(RecentMaxSyncedValue const &);

	float m_maximum;
	float m_oldMaximum;
	uint32 m_changeSyncStamp;
	int const m_checkIntervalMs;
	int const m_fudgeIntervalMs;
};

// ======================================================================

#endif // INCLUDED_RecentMaxSyncedValue_H

