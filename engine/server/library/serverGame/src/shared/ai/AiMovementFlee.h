// ======================================================================
//
// AiMovementFlee.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementFlee_H
#define	INCLUDED_AiMovementFlee_H

#include "serverGame/AiMovementWaypoint.h"
#include "serverGame/AiLocation.h"

// ======================================================================

class AiMovementFlee : public AiMovementWaypoint
{
public:

	explicit AiMovementFlee( AICreatureController * controller );

	AiMovementFlee( AICreatureController * controller, 
	                ServerObject const * target, 
	                float minDistance, 
	                float maxDistance );

	AiMovementFlee( AICreatureController * controller, 
	                Archive::ReadIterator & source );

	virtual ~AiMovementFlee();

	virtual AiMovementType getType() const;
	virtual AiMovementFlee * asAiMovementFlee();

	// ----------

	virtual void               pack                 ( Archive::ByteStream & target ) const;
	virtual void               alter                ( float time );

	virtual void               getDebugInfo         ( std::string & outString ) const;

	// states & triggers

	virtual AiStateResult      stateWaiting         ( float time );
	virtual AiStateResult      stateThinking        ( float time );
	virtual AiStateResult      stateMoving          ( float time );

	virtual AiStateResult      triggerThinking      ( void );
	virtual AiStateResult      triggerWaypoint      ( void );
	virtual AiStateResult      triggerTargetLost    ( void );
	virtual AiStateResult      triggerPathNotFound  ( void );
	
	// waypoints

	virtual bool               hasWaypoint          ( void ) const;
	virtual AiLocation const & getWaypoint          ( void ) const;
	virtual bool               updateWaypoint       ( void );
	virtual bool               generateWaypoint     ( void );

	// misc

	virtual bool               getDecelerate        ( void ) const;

protected:

	AiLocation m_fleeFrom;     // this is what we're running from
	AiLocation m_fleeTo;       // this is where we're running to

	float m_minDistance;
	float m_maxDistance;
};

// ======================================================================

#endif

