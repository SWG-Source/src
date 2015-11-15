// ======================================================================
//
// Ray3d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Ray3d.h"

#include "sharedMath/Line3d.h"

Line3d	Ray3d::getLine	( void ) const
{
	return Line3d(m_point,m_normal);
}


