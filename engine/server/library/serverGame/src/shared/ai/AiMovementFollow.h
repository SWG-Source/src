// ======================================================================
//
// AiMovementFollow.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementFollow_H
#define	INCLUDED_AiMovementFollow_H

#include "serverGame/AiMovementTarget.h"

// ======================================================================

class AiMovementFollow : public AiMovementTarget
{
public:

	explicit AiMovementFollow( AICreatureController * controller );

	AiMovementFollow( AICreatureController * controller, 
	                  ServerObject const * target, 
	                  float minDistance, 
	                  float maxDistance );
	
	AiMovementFollow( AICreatureController * controller, 
	                  ServerObject const * target, 
	                  Vector const & offset, 
	                  bool relativeOffset, 
	                  float minDistance, 
	                  float maxDistance );

	AiMovementFollow( AICreatureController * controller, 
	                  Archive::ReadIterator & source );

	virtual ~AiMovementFollow();
	
	virtual AiMovementType getType() const;
	virtual AiMovementFollow * asAiMovementFollow();

	// ----------
	
	virtual void               pack                 ( Archive::ByteStream & target ) const;
	virtual void               alter                ( float time );
	virtual void               refresh              ( void );

	virtual void               getDebugInfo         ( std::string & outString ) const;

	// states & triggers

	virtual AiStateResult      stateWaiting         ( float time );
	virtual AiStateResult      stateMoving          ( float time );
	
	virtual AiStateResult      triggerWaiting       ( void );
	virtual AiStateResult      triggerMoving        ( void );
	virtual AiStateResult      triggerTargetLost    ( void );

	// waypoints

	virtual AiLocation const & getFinalWaypoint() const;
	virtual bool               updateWaypoint       ( void );

	// ----------

	AiLocation const & getOffsetTarget() const;
	float getMinDistance() const;
	float getMaxDistance() const;

#ifdef _DEBUG
	virtual void addDebug(AiDebugString & aiDebugString);
#endif // _DEBUG

protected:

	bool targetTooFar ( void ) const;

	AiLocation  m_offsetTarget;
	float       m_minDistance;
	float       m_maxDistance;
};

// ======================================================================

#endif

