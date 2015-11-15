// ======================================================================
//
// AiMovementSwarm.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementSwarm_H
#define	INCLUDED_AiMovementSwarm_H

#include "serverGame/AiMovementFollow.h"


class CreatureObject;


// ======================================================================

class AiMovementSwarm : public AiMovementFollow
{
public:
	typedef ConstWatcher<CreatureObject> CreatureWatcher;

public:

	explicit AiMovementSwarm( AICreatureController * controller );

	AiMovementSwarm( AICreatureController * controller, 
	                  ServerObject const * target );

	AiMovementSwarm( AICreatureController * controller, 
	                  ServerObject const * target,
	                  float offset );
	
	AiMovementSwarm( AICreatureController * controller, 
	                 Archive::ReadIterator & source );

	virtual ~AiMovementSwarm();
	
	virtual AiMovementType getType() const;
	virtual AiMovementSwarm * asAiMovementSwarm();

	// ----------
	
	virtual void           pack                 (Archive::ByteStream & target) const;
	virtual void           alter                (float time );
	virtual void           refresh              ();
	virtual void           clear                ();
	
	virtual void           getDebugInfo         (std::string & outString) const;

	// states & triggers
	virtual AiStateResult  triggerWaiting       ( void );

	// ----------

	float getOffset() const;

private:

	void init();
	void cleanup();
	
	static void computeGoals();
	static void computeGoals(const CreatureObject & target, std::vector<CreatureWatcher> & movers);

	// Disabled.
	AiMovementSwarm();
	AiMovementSwarm(AiMovementSwarm const&);
	AiMovementSwarm &operator =(AiMovementSwarm const&);

private:

	float m_offset;         // how far to stay from our target
};


//-----------------------------------------------------------------------

inline float AiMovementSwarm::getOffset() const
{
	return m_offset;
}


// ======================================================================

#endif

