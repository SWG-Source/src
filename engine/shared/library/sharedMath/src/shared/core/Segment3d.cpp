// ======================================================================
//
// Segment3d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Segment3d.h"

#include "sharedMath/Line3d.h"
#include "sharedMath/Range.h"

Line3d	Segment3d::getLine	( void ) const
{
	return Line3d(m_begin,m_end-m_begin);
}

Line3d Segment3d::getReverseLine ( void ) const
{
	return Line3d(m_end,m_begin-m_end);
}

Range Segment3d::getRangeX ( void ) const
{
	return Range(m_begin.x,m_end.x);
}

Range Segment3d::getRangeY ( void ) const
{
	return Range(m_begin.y,m_end.y);
}

Range Segment3d::getRangeZ ( void ) const
{
	return Range(m_begin.z,m_end.z);	
}

