// ======================================================================
//
// AiMovementLoiter.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementLoiter_H
#define	INCLUDED_AiMovementLoiter_H

#include "serverGame/AiMovementWaypoint.h"

class AiDebugString;

class AiMovementLoiter : public AiMovementWaypoint
{
public:

	explicit AiMovementLoiter( AICreatureController * controller );

	AiMovementLoiter( AICreatureController * controller,
	                  CellProperty const * anchorCell,
	                  Vector const & anchor,
	                  float minDistance,
	                  float maxDistance,
	                  float minDelay,
	                  float maxDelay );

	AiMovementLoiter( AICreatureController * controller,
	                  ServerObject const * anchor,
	                  float minDistance,
	                  float maxDistance,
	                  float minDelay,
	                  float maxDelay );

	AiMovementLoiter( AICreatureController * controller,
	                  Archive::ReadIterator & source );

	virtual ~AiMovementLoiter();

	virtual AiMovementType getType() const;
	virtual AiMovementLoiter * asAiMovementLoiter();

	// ----------
	// from AiMovementBase

	virtual void               pack              ( Archive::ByteStream & target ) const;
	virtual void               alter             ( float time );
	virtual bool               getHibernateOk    ( void ) const;

	virtual void               getDebugInfo      ( std::string & outString ) const;

	// states & triggers

	virtual AiStateResult      stateWaiting      ( float time );
	virtual AiStateResult      stateThinking     ( float time );
	virtual AiStateResult      stateMoving       ( float time );

	virtual AiStateResult      triggerWaiting    ( void );
	virtual AiStateResult      triggerThinking   ( void );
	virtual AiStateResult      triggerMoving     ( void );

	virtual AiStateResult      triggerWaypoint   ( void );
	virtual AiStateResult      triggerTargetLost ( void );

	// waypoints

	virtual bool               hasWaypoint       ( void ) const;
	virtual AiLocation const & getWaypoint       ( void ) const;
	virtual bool               updateWaypoint    ( void );
	virtual void               clearWaypoint     ( void );
	virtual bool			   generateWaypoint  ( void );

#ifdef _DEBUG
	virtual void addDebug(AiDebugString & aiDebugString);
#endif // _DEBUG

protected:

	enum BubbleCheckResult
	{
		  BCR_invalid
		, BCR_success
		, BCR_failure
	};

	typedef std::vector<AiLocation> CachedAiLocations;

	AiLocation m_anchor;
	AiLocation m_target;
	float m_sleepTime;
	float m_minDistance;
	float m_maxDistance;
	float m_minDelay;
	float m_maxDelay;
	bool m_active;
	CachedAiLocations * m_cachedAiLocations;
	int m_goalLocationIndex;
	BubbleCheckResult m_bubbleCheckResult;

private:

	bool isAnchorValid() const;
};

// ======================================================================


#endif

