// ======================================================================
//
// AiMovementMove.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementMove_H
#define	INCLUDED_AiMovementMove_H

#include "serverGame/AiMovementTarget.h"

class CellProperty;
class PathGraph;
class PortalProperty;
class AiLocation;

typedef std::vector<int> IndexList;

// ======================================================================
// The simple move behavior moves to the waypoint, and then transitions
// back to idle state

class AiMovementMove : public AiMovementTarget
{
public:

	explicit AiMovementMove( AICreatureController * controller );
	AiMovementMove( AICreatureController * controller, CellProperty const * cell, Vector const & target, float radius = 0.0f );
	AiMovementMove( AICreatureController * controller, Unicode::String const & targetName );	
	AiMovementMove( AICreatureController * controller, Archive::ReadIterator & source );

	virtual ~AiMovementMove();
	
	virtual AiMovementType getType() const;
	virtual AiMovementMove * asAiMovementMove();

	// ----------
	
	virtual void          pack                ( Archive::ByteStream & target ) const;
	virtual void          alter               ( float time );
	virtual void          getDebugInfo        ( std::string & outString ) const;
	virtual void          clear               ( void );
	virtual void          refresh             ( void );

	// states & triggers

	virtual AiStateResult stateWaiting        ( float time );
	virtual AiStateResult statePathing        ( float time );

	virtual AiStateResult triggerPathing      ( void );
	virtual AiStateResult triggerMoving       ( void );

	virtual AiStateResult triggerTargetLost   ( void );
	virtual AiStateResult triggerPathComplete ( void );
	virtual AiStateResult triggerPathNotFound ( void );
	
	// misc

	virtual bool          setupPathSearch     ( void );

	Unicode::String const & getTargetName() const;

protected:

	AiLocation          m_start;
	Unicode::String     m_targetName;
	ServerPathBuilder * m_pathBuilder;
};

// ======================================================================

#endif

