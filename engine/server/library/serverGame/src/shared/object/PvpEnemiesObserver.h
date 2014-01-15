// ======================================================================
//
// PvpEnemiesObserver.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _PvpEnemiesObserver_H_
#define _PvpEnemiesObserver_H_

// ======================================================================

#include "Archive/AutoDeltaObserverOps.h"
#include "serverGame/PvpUpdateObserver.h"
#include "sharedDebug/Profiler.h"

// ======================================================================

class CreatureObject;
class TangibleObject;

// ======================================================================

class PvpEnemiesObserver
{
public:
	PvpEnemiesObserver(TangibleObject *who, Archive::AutoDeltaObserverOp operation);
	~PvpEnemiesObserver();

private:
	PvpEnemiesObserver(PvpEnemiesObserver const &);
	PvpEnemiesObserver& operator=(PvpEnemiesObserver const &);

private:
	ProfilerAutoBlock m_profilerBlock;
	CreatureObject *m_obj;
	PvpUpdateObserver m_pvpObserver;
	bool m_hadAnyAlignedTimedEnemyFlag;
	bool m_hadAnyBountyDuelEnemyFlag;
};

// ======================================================================

#endif // _PvpEnemiesObserver_H_

