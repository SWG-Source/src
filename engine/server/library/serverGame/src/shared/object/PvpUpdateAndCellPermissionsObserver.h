// ======================================================================
//
// PvpUpdateAndCellPermissionsObserver.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _PvpUpdateAndCellPermissionsObserver_H_
#define _PvpUpdateAndCellPermissionsObserver_H_

// ======================================================================

#include "Archive/AutoDeltaObserverOps.h"
#include "serverGame/PvpUpdateObserver.h"
#include "serverGame/CellPermissions.h"
#include "sharedDebug/Profiler.h"

// ======================================================================

class TangibleObject;

// ======================================================================

class PvpUpdateAndCellPermissionsObserver
{
public:
	PvpUpdateAndCellPermissionsObserver(TangibleObject *who, Archive::AutoDeltaObserverOp operation);
	~PvpUpdateAndCellPermissionsObserver();

private:
	PvpUpdateAndCellPermissionsObserver(PvpUpdateAndCellPermissionsObserver const &);
	PvpUpdateAndCellPermissionsObserver& operator=(PvpUpdateAndCellPermissionsObserver const &);

private:
	ProfilerAutoBlock m_profilerBlock;
	PvpUpdateObserver m_pvpObserver;
	CellPermissions::ViewerChangeObserver m_cellPermsObserver;
};

// ======================================================================

#endif // _PvpUpdateAndCellPermissionsObserver_H_

