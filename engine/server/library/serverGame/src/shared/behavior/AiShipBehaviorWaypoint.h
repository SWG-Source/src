// ======================================================================
//
// AiShipBehaviorWaypoint.h
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiShipBehaviorWaypoint_H
#define	INCLUDED_AiShipBehaviorWaypoint_H

#include "serverGame/AiShipBehaviorBase.h"

class AiDebugString;
class AiShipController;
class Vector;

// ======================================================================

class AiShipBehaviorWaypoint : public AiShipBehaviorBase
{
public:

	AiShipBehaviorWaypoint(AiShipController & aiShipController, bool const cyclic);
	
	virtual void alter(float deltaSeconds);
	virtual AiShipBehaviorType getBehaviorType() const;

#ifdef _DEBUG
	virtual void addDebug(AiDebugString & aiDebugString);
#endif // _DEBUG

private:

	bool getNextPosition_w(Vector & position_w);

	Vector getGoalPosition_w() const;
	//Vector getPreviousGoalPosition() const;
	//Vector getNextGoalPosition() const;

	bool m_cyclic;
	bool m_moveToCompleteTriggerSent;

	// Disabled
	AiShipBehaviorWaypoint();
	AiShipBehaviorWaypoint(AiShipBehaviorWaypoint const &);
	AiShipBehaviorWaypoint &operator =(AiShipBehaviorWaypoint const &);
};

// ======================================================================

#endif // INCLUDED_AiShipBehaviorWaypoint_H
