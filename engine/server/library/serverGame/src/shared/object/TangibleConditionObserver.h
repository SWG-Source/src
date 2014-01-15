// ======================================================================
//
// TangibleConditionObserver.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TangibleConditionObserver_H
#define INCLUDED_TangibleConditionObserver_H

// ======================================================================

#include "serverGame/PvpUpdateObserver.h"

// ======================================================================

class TangibleConditionObserver
{
public:

	TangibleConditionObserver(TangibleObject const *who, Archive::AutoDeltaObserverOp operation);
	~TangibleConditionObserver();

private:

	// Disabled.
	TangibleConditionObserver();
	TangibleConditionObserver(TangibleConditionObserver const&);
	TangibleConditionObserver &operator =(TangibleConditionObserver const&);
	
private:

	TangibleObject const *m_tangibleObject;
	int                   m_oldCondition;
	PvpUpdateObserver     m_pvpUpdateObserver;
	
};

// ======================================================================

#endif
