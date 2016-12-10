// ======================================================================
//
// AiMovementPatrol.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementPatrol_H
#define	INCLUDED_AiMovementPatrol_H

#include "serverGame/AiMovementMove.h"

class AiLocation;
class Location;


// ======================================================================
// This behavior causes an ai to move along a series of waypoints

class AiMovementPatrol : public AiMovementMove
{
public:

//	explicit AiMovementPatrol( AICreatureController * controller );
	AiMovementPatrol(AICreatureController * controller, std::vector<Location> const & locations, bool random, bool flip, bool repeat, int startPoint);
	AiMovementPatrol(AICreatureController * controller, std::vector<Unicode::String> const & locations, bool random, bool flip, bool repeat, int startPoint);
	AiMovementPatrol(AICreatureController * controller, Archive::ReadIterator & source);

	virtual ~AiMovementPatrol();
	
	virtual AiMovementType getType() const;
	virtual AiMovementPatrol * asAiMovementPatrol();

	// ----------
	
	virtual void          pack                (Archive::ByteStream & target) const;
	virtual void          alter               (float time);
	virtual void          getDebugInfo        (std::string & outString) const;
	virtual void          endBehavior();
	virtual void          clear               (void);
//	virtual void          refresh             (void);

	virtual bool          getHibernateOk() const;

	// states & triggers
	virtual AiStateResult stateWaiting        ( float time );

	virtual AiStateResult triggerWaiting      ( void );
	virtual AiStateResult triggerPathComplete ( void );
	
	// misc
	virtual bool          setupPathSearch     ( void );

#ifdef _DEBUG
	virtual void addDebug(AiDebugString & aiDebugString);
#endif // _DEBUG

protected:

	std::vector<AiLocation>  m_patrolPath;
	bool                        m_random;
	bool                        m_flip;
	bool                        m_repeat;
	int                         m_patrolPointIndex;
};

// ======================================================================

#endif

