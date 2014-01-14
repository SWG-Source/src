// ======================================================================
//
// Ring.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Ring.h"

#include "sharedMath/Range.h"
#include "sharedMath/Plane3d.h"

Range Ring::getRangeX ( void ) const
{
	return Range( m_center.x - m_radius, m_center.x + m_radius );
}

Range Ring::getRangeZ ( void ) const
{
	return Range( m_center.z - m_radius, m_center.z + m_radius );
}

Range Ring::getLocalRangeX ( void ) const
{
	return Range( -m_radius, m_radius );
}

Plane3d Ring::getPlane ( void ) const
{
	return Plane3d(m_center,Vector(0,1,0));
}



