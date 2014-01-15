// ======================================================================
//
// GroupUpdateObserver.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _GroupUpdateObserver_H_
#define _GroupUpdateObserver_H_

// ======================================================================

#include "Archive/AutoDeltaObserverOps.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedDebug/Profiler.h"

// ======================================================================

class GroupObject;

// ======================================================================

class GroupUpdateObserver
{
public:
	explicit GroupUpdateObserver(GroupObject *group, Archive::AutoDeltaObserverOp operation);
	~GroupUpdateObserver();

private:
	GroupUpdateObserver(GroupUpdateObserver const &);
	GroupUpdateObserver &operator=(GroupUpdateObserver const &);

private:
	ProfilerAutoBlock m_profilerBlock;
	GroupObject *m_group;
	std::vector<NetworkId> m_members;
	NetworkId m_leaderId;
	NetworkId m_lootMasterId;
};

// ======================================================================

#endif // _GroupUpdateObserver_H_

