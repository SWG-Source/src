// ======================================================================
//
// NeighborObject.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/NeighborObject.h"


// ----------------------------------------------------------------------

NeighborObject::NeighborObject()
: m_neighbor(nullptr)
{
}

void NeighborObject::setNeighbor ( NeighborObject * neighbor )
{
	m_neighbor = neighbor;
}

bool NeighborObject::shouldRender ( void ) const
{
	return true;
}

