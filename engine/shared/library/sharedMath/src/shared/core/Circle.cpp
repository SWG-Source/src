// ======================================================================
//
// Circle.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Circle.h"

#include "sharedMath/Plane3d.h"
#include "sharedMath/Range.h"

Range Circle::getRangeX ( void ) const
{
	return Range( m_center.x - m_radius, m_center.x + m_radius );
}

Range Circle::getRangeZ ( void ) const
{
	return Range( m_center.z - m_radius, m_center.z + m_radius );
}

Range Circle::getLocalRangeX ( void ) const
{
	return Range( -m_radius, m_radius );
}

Plane3d Circle::getPlane ( void ) const
{
	return Plane3d(m_center,Vector::unitY);
}


