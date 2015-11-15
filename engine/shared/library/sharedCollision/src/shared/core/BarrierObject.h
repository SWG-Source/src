// ======================================================================
//
// BarrierObject.h
// Copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_BarrierObject_H
#define INCLUDED_BarrierObject_H

#include "sharedCollision/NeighborObject.h"

class DoorObject;

// ----------------------------------------------------------------------

class BarrierObject : public NeighborObject
{
public:

	BarrierObject(DoorObject const * door, bool needsAppearance);

	virtual bool    isActive            ( void ) const;
	virtual bool    shouldRender        ( void ) const;

	DoorObject const * getDoor          ( void ) const;

protected:

	void createCollisionExtent ( void );
	void createAppearance ( void );

	DoorObject const * m_door;

private:

	BarrierObject( BarrierObject const & copy );
	BarrierObject & operator = ( BarrierObject const & copy );
};

// ======================================================================

#endif
