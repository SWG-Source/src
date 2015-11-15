// ======================================================================
//
// GroupIdObserver.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _GroupIdObserver_H_
#define _GroupIdObserver_H_

#include "Archive/AutoDeltaObserverOps.h"
#include "serverGame/PvpUpdateObserver.h"
#include "sharedDebug/Profiler.h"

// ======================================================================

class CreatureObject;
class GroupObject;

// ======================================================================

class GroupIdObserver
{
public:
	GroupIdObserver(CreatureObject *who, Archive::AutoDeltaObserverOp operation);
	~GroupIdObserver();

private:
	GroupIdObserver(GroupIdObserver const &);
	GroupIdObserver& operator=(GroupIdObserver const &);

private:
	ProfilerAutoBlock m_profilerBlock;
	PvpUpdateObserver m_pvpObserver;
	CreatureObject *m_creature;
	GroupObject *m_group;
};

// ======================================================================

#endif // _GroupIdObserver_H_

