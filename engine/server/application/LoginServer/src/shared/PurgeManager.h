// ======================================================================
//
// PurgeManager.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PurgeManager_H
#define INCLUDED_PurgeManager_H

// ======================================================================

class PurgeManager
{
  public:
	static void update(float time);
	static void onCheckStatusForPurge(StationId account, bool isActive);
	static void onGetAccountForPurge(StationId account, int purgePhase);
	static void onClusterNoLongerReady(uint32 clusterId);
	static void handlePurgeCompleteOnCluster(StationId account, uint32 clusterId);
	static void onAllCharactersDeleted(StationId account, bool success);
};

// ======================================================================

#endif
