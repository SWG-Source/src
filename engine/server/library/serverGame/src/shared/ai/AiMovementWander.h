// ======================================================================
//
// AiMovementWander.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementWander_H
#define	INCLUDED_AiMovementWander_H

#include "serverGame/AiMovementWaypoint.h"

// ======================================================================
// The simple wander behavior picks random waypoints and walks between
// them, pausing at each one.

class AiMovementWander : public AiMovementWaypoint
{
public:

	explicit AiMovementWander( AICreatureController * controller );

	AiMovementWander( AICreatureController * controller, 
	                  float minDistance, 
	                  float maxDistance, 
	                  float minAngle, 
	                  float maxAngle, 
	                  float minDelay, 
	                  float maxDelay );
	
	AiMovementWander( AICreatureController * controller,
	                  Archive::ReadIterator & source );

	virtual ~AiMovementWander();
	
	virtual AiMovementType getType() const;
	virtual AiMovementWander * asAiMovementWander();

	// ----------
	
	virtual void               pack                ( Archive::ByteStream & target ) const;
	virtual void               alter               ( float time );

	virtual void               getDebugInfo        ( std::string & outString ) const;

	// states & triggers

	virtual AiStateResult      stateWaiting        ( float time );
	virtual AiStateResult      stateThinking       ( float time );
	virtual AiStateResult      stateMoving         ( float time );
	
	virtual AiStateResult      triggerWaiting      ( void );
	virtual AiStateResult      triggerThinking     ( void );
	virtual AiStateResult      triggerMoving       ( void );
	virtual AiStateResult      triggerWaypoint     ( void );

	// waypoints

	virtual bool               hasWaypoint         ( void ) const;
	virtual AiLocation const & getWaypoint         ( void ) const;
	virtual bool               updateWaypoint      ( void );
	virtual void               clearWaypoint       ( void );
	virtual bool               generateWaypoint    ( void );
	
protected:

	AiLocation  m_target;
	float       m_sleepTime;
	
	float       m_minDistance;
	float       m_maxDistance;
	
	float       m_minAngle;
	float       m_maxAngle;
	
	float       m_minDelay;
	float       m_maxDelay;
};

// ======================================================================

#endif

