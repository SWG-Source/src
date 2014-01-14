// ======================================================================
//
// AxialBox.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/AxialBox.h"

#include "sharedMath/Range.h"

#include <algorithm>    // for min/max
#include <vector>       // for min/max

// ======================================================================

AxialBox::AxialBox() : 
  m_min( Vector::maxXYZ ),
  m_max( Vector::negativeMaxXYZ )
{
}

AxialBox::AxialBox ( Vector const & cornerA, Vector const & cornerB )
{
	if (cornerA.x < cornerB.x)
	{
		m_min.x = cornerA.x;
		m_max.x = cornerB.x;
	}
	else
	{
		m_min.x = cornerB.x;
		m_max.x = cornerA.x;
	}
	if (cornerA.y < cornerB.y)
	{
		m_min.y = cornerA.y;
		m_max.y = cornerB.y;
	}
	else
	{
		m_min.y = cornerB.y;
		m_max.y = cornerA.y;
	}
	if (cornerA.z < cornerB.z)
	{
		m_min.z = cornerA.z;
		m_max.z = cornerB.z;
	}
	else
	{
		m_min.z = cornerB.z;
		m_max.z = cornerA.z;
	}
}

AxialBox::AxialBox( Range const & rX, Range const & rY, Range const & rZ )
: m_min(rX.getMin(),rY.getMin(),rZ.getMin()),
  m_max(rX.getMax(),rY.getMax(),rZ.getMax())
{
}

AxialBox::AxialBox( AxialBox const & boxA, AxialBox const & boxB )
: m_min( boxA.getMin() ),
  m_max( boxA.getMax() )
{
	add(boxB);
}

// ----------------------------------------------------------------------

void AxialBox::clear ( void )
{
	m_min = Vector::maxXYZ;
	m_max = Vector::negativeMaxXYZ;
}

void AxialBox::add ( Vector const & V )
{
	if (m_min.x > m_max.x)
	{
		addMin(V);
		addMax(V);
	}
	else
	{
		if (V.x < m_min.x)
			m_min.x = V.x;
		else if (V.x > m_max.x)
			m_max.x = V.x;

		if (V.y < m_min.y)
			m_min.y = V.y;
		else if (V.y > m_max.y)
			m_max.y = V.y;

		if (V.z < m_min.z)
			m_min.z = V.z;
		else if (V.z > m_max.z)
			m_max.z = V.z;
	}
}

void AxialBox::addMin ( Vector const & V )
{
	if (V.x < m_min.x)
		m_min.x = V.x;

	if (V.y < m_min.y)
		m_min.y = V.y;

	if (V.z < m_min.z)
		m_min.z = V.z;
}

void AxialBox::addMax ( Vector const & V )
{
	if (V.x > m_max.x)
		m_max.x = V.x;

	if (V.y > m_max.y)
		m_max.y = V.y;

	if (V.z > m_max.z)
		m_max.z = V.z;
}

void AxialBox::add ( std::vector<Vector> const & vertices )
{
	VertexList::const_iterator const iEnd = vertices.end();
	for (VertexList::const_iterator i = vertices.begin(); i != iEnd; ++i)
		add(*i);
}

void AxialBox::add ( AxialBox const & A )
{
	addMin(A.getMin());
	addMax(A.getMax());
}

bool AxialBox::contains ( Vector const & V ) const
{
	if( V.z < m_min.z ) return false;
	if( V.x < m_min.x ) return false;
	if( V.y < m_min.y ) return false;

	if( V.z > m_max.z ) return false;
	if( V.x > m_max.x ) return false;
	if( V.y > m_max.y ) return false;

	return true;
}

bool AxialBox::contains ( AxialBox const & A ) const
{
	return contains ( A.m_min ) && contains ( A.m_max );
}

bool AxialBox::isEmpty ( void ) const
{
	// All should be valid if we have initialized it at all.
	return m_min.x > m_max.x;
}

Range   AxialBox::getRangeX ( void ) const  { return Range(m_min.x,m_max.x); }
Range   AxialBox::getRangeY ( void ) const  { return Range(m_min.y,m_max.y); }
Range   AxialBox::getRangeZ ( void ) const  { return Range(m_min.z,m_max.z); }

bool AxialBox::intersects (const AxialBox& other) const
{
	return !(m_max.x < other.m_min.x || m_min.x > other.m_max.x || m_max.y < other.m_min.y || m_min.y > other.m_max.y || m_max.z < other.m_min.z || m_min.z > other.m_max.z);
}

