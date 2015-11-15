// ======================================================================
//
// PvpUpdateAndCellPermissionsObserver.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PvpUpdateAndCellPermissionsObserver.h"
#include "serverGame/TangibleObject.h"

// ======================================================================

PvpUpdateAndCellPermissionsObserver::PvpUpdateAndCellPermissionsObserver(TangibleObject *who, Archive::AutoDeltaObserverOp operation) :
	m_profilerBlock("PvpUpdateAndCellPermissionsObserver"),
	m_pvpObserver(who, operation),
	m_cellPermsObserver(who ? who->asCreatureObject() : 0)
{
}

// ----------------------------------------------------------------------

PvpUpdateAndCellPermissionsObserver::~PvpUpdateAndCellPermissionsObserver()
{
}

// ======================================================================

