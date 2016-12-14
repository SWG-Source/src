// ======================================================================
//
// AiShipBehaviorDock.h
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiShipBehaviorDock_H
#define	INCLUDED_AiShipBehaviorDock_H

#include "sharedFoundation/Timer.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedObject/CachedNetworkId.h"

class AiDebugString;
class NetworkId;
class ShipController;
class ShipObject;

// ======================================================================

class AiShipBehaviorDock
{
public:

	AiShipBehaviorDock(ShipController & shipController, ShipObject & dockTarget, float const secondsAtDock);
	~AiShipBehaviorDock();

	void alter(float deltaSeconds);
	bool isDocked() const;
	void unDock();
	bool isDockFinished() const;
	CachedNetworkId const & getDockTarget() const;

#ifdef _DEBUG
	void addDebug(AiDebugString & aiDebugString);
#endif // _DEBUG

private:

	enum DockingState
	{
		DS_approach,
		DS_approachTractorBeam,
		DS_landing,
		DS_docked,
		DS_exitTractorBeam
	};

#ifdef _DEBUG
	static char const * getDockingStateString(DockingState const dockingState);
#endif // _DEBUG

	typedef std::list<Transform> HardPointList;

	ShipController & m_shipController;
	CachedNetworkId m_dockTarget;
	Timer m_timeAtDockTimer;
	HardPointList * const m_approachHardPointList;
	int m_initialApproachHardPointCount;
	HardPointList * const m_exitHardPointList;
	Transform m_dockHardPoint;
	Transform m_landingHardPoint;
	bool m_hasLandingHardPoint;
	DockingState m_dockingState;
	float m_dockingStateTime;
	bool m_infiniteDockTime;
	bool m_dockFinished;
	Vector m_goalPosition_w;
	bool m_wingsOpenedBeforeDock;
	
	void approach(float const deltaSeconds);
	void approachTractorBeam(float const deltaSeconds);
	void landing(float const deltaSeconds);
	void docked(float const deltaSeconds);
	void unDock(float const deltaSeconds);
	bool alignWithDock(float const deltaSeconds);
	void setApproachPosition();
	void setDockPosition();
	void setExitPosition();
	void triggerDocked();
	void triggerStartUnDock();
	void triggerUnDockWithSuccess();
	void triggerUnDockWithFailure();
	float getMaxTractorBeamSpeed() const;
	void tractorBeamToGoalPosition(float const deltaSeconds, bool const allowOrientation);

	// Disabled
	AiShipBehaviorDock();
	AiShipBehaviorDock(AiShipBehaviorDock const &);
	AiShipBehaviorDock &operator =(AiShipBehaviorDock const &);
};

// ======================================================================

#endif // INCLUDED_AiShipBehaviorDock_H
