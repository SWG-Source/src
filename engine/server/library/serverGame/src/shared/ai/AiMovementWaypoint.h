// ======================================================================
//
// AiMovementWaypoint.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementWaypoint_H
#define	INCLUDED_AiMovementWaypoint_H

#include "serverGame/AiMovementBase.h"
#include "serverGame/AiLocation.h"

#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Circle.h"

class AiDebugString;
class CellProperty;
class CollisionProperty;
class ServerObject;
class Object;

// ======================================================================
// This is the waypoint-following behavior base class. It's very simple -
// it thinks about how to get to its waypoint, it moves there, and then
// it waits there.

// A number of useful behaviors (flee, wander, loiter, sentinel, move, 
// follow) are based off of this behavior

class AiMovementWaypoint : public AiMovementBase
{
public:

	explicit AiMovementWaypoint( AICreatureController * controller );
	AiMovementWaypoint( AICreatureController * controller,
	                    Archive::ReadIterator & source );
	
	virtual ~AiMovementWaypoint();

	// ----------
	// from AiMovementBase
	
	virtual void               pack                ( Archive::ByteStream & target ) const;
	virtual void               alter               ( float time );
	virtual void               refresh             ( void );
	virtual bool               getHibernateOk      ( void ) const;
	virtual void               getDebugInfo        ( std::string & outString ) const;

	// states & triggers
	
	virtual AiStateResult      stateWaiting        ( float time );
	virtual AiStateResult      stateMoving         ( float time );
	virtual AiStateResult      stateAvoiding       ( float time );

	virtual AiStateResult      triggerWaiting      ( void );
	virtual AiStateResult      triggerMoving       ( void );
	virtual AiStateResult      triggerAvoiding     ( void );
	virtual AiStateResult      triggerDoneAvoiding ( void );

	virtual AiStateResult      triggerWaypoint     ( void );
	virtual AiStateResult      triggerTargetLost   ( void );
	
	// waypoints

	virtual bool               hasWaypoint         ( void ) const;
	virtual AiLocation const & getWaypoint         ( void ) const;
	virtual AiLocation const & getFinalWaypoint() const;
	virtual bool               updateWaypoint      ( void );
	virtual void               clearWaypoint       ( void );

	// misc
	
	virtual float              getDesiredSpeed     ( void ) const;
	virtual bool               getDecelerate       ( void ) const;

	// ----------
	
#ifdef _DEBUG
	void addDebug(AiDebugString & aiDebugString);
#endif // _DEBUG

protected:

	bool findObstacle ( void );
	bool updateAvoidancePoint ( float time );

	AiLocation m_avoidanceLocation;
	float m_avoidanceStuckTime;
	float m_avoidanceHeadedToWaypointTime;
	int m_mostRecentAvoidanceFrameNumber;
	AiLocation m_obstacleLocation;

private:

	bool moveTowards(AiLocation const & nextLocation, AiLocation const & finalLocation, float time);
};

// ======================================================================

#endif

