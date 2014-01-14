// ======================================================================
//
// NeighborObject.h
// Copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_NeighborObject_H
#define INCLUDED_NeighborObject_H

#include "sharedObject/Object.h"

// ----------------------------------------------------------------------

class NeighborObject : public Object
{
public:

	NeighborObject();

	virtual void	setNeighbor			( NeighborObject * neighbor );
	virtual bool    shouldRender        ( void ) const;

protected:

	NeighborObject *	m_neighbor;

private:

	NeighborObject( NeighborObject const & copy );
	NeighborObject & operator = ( NeighborObject const & copy );
};

// ======================================================================

#endif
