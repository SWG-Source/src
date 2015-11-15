// ======================================================================
//
// Cylinder.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Cylinder.h"

#include "sharedMath/Circle.h"
#include "sharedMath/Range.h"
#include "sharedMath/Ring.h"

Ring Cylinder::getTopRing ( void ) const
{
	return Ring( Vector(m_base.x, m_base.y + m_height, m_base.z), m_radius );
}

Ring Cylinder::getBaseRing ( void ) const
{
	return Ring( m_base, m_radius );
}

Range Cylinder::getRangeY ( void ) const
{
	return Range( m_base.y, m_base.y + m_height );
}

Circle Cylinder::getTopCircle ( void ) const
{
	return Circle( Vector(m_base.x, m_base.y + m_height, m_base.z), m_radius );
}

Circle Cylinder::getBaseCircle ( void ) const
{
	return Circle( m_base, m_radius );
}

