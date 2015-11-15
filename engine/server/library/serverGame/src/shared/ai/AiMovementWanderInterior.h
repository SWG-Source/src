// ======================================================================
//
// AiMovementWanderInterior.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiMovementWanderInterior_H
#define	INCLUDED_AiMovementWanderInterior_H

#include "serverGame/AiMovementWander.h"

#include "sharedPathfinding/PathGraphIterator.h"

// ======================================================================
// This is a first pass at an interior wandering behavior - randomly walk
// between the path nodes for the cell

class AiMovementWanderInterior : public AiMovementWander
{
public:

	explicit AiMovementWanderInterior( AICreatureController * controller );
	AiMovementWanderInterior( AICreatureController * controller,
	                          Archive::ReadIterator & source );

	virtual ~AiMovementWanderInterior();

	virtual void pack   ( Archive::ByteStream & target ) const;

	bool updateWaypoint ( void );

protected:

	PathGraphIterator m_iterator;

	int m_lastNodeIndex;
};

// ======================================================================

#endif

