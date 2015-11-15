// ======================================================================
//
// AiMovementTarget.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementTarget_H
#define	INCLUDED_AiMovementTarget_H

#include "serverGame/AiMovementPathFollow.h"



// ======================================================================

class AiMovementTarget : public AiMovementPathFollow
{
public:

	explicit AiMovementTarget( AICreatureController * controller );

	AiMovementTarget( AICreatureController * controller, 
	                  ServerObject const * target );

	AiMovementTarget( AICreatureController * controller, CellProperty const * cell, Vector const & target, float radius );

	AiMovementTarget( AICreatureController * controller, 
	                 Archive::ReadIterator & source );

	virtual ~AiMovementTarget();
	
	// ----------
	
	virtual void           pack                 ( Archive::ByteStream & target ) const;
	virtual bool           getHibernateOk       ( void ) const;
	
	// ----------

	AiLocation const & getTarget() const;

protected:

	AiLocation     m_target;
};

// ======================================================================

#endif

