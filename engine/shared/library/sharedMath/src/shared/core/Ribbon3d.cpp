// ======================================================================
//
// Ribbon3d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Ribbon3d.h"

#include "sharedMath/Segment3d.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Plane3d.h"


Ribbon3d::Ribbon3d ( Vector const & pointA, Vector const & pointB, Vector const & dir )
: m_pointA( pointA ),
  m_pointB( pointB ),
  m_dir( dir )
{
}

Ribbon3d::Ribbon3d ( Segment3d const & seg, Vector const & dir ) 
: m_pointA( seg.getBegin() ),
  m_pointB( seg.getEnd() ),
  m_dir( dir )
{
}

Ribbon3d::Ribbon3d ( Line3d const & line, Vector const & delta )
: m_pointA( line.getPoint() ),
  m_pointB( line.getPoint() + delta ),
  m_dir( line.getNormal() )
{
}

// ----------

Plane3d Ribbon3d::getPlane( void ) const
{
	Vector E = m_pointB - m_pointA;
	Vector N = E.cross( m_dir );

	return Plane3d( m_pointA, N );
}

Line3d Ribbon3d::getEdgeA( void ) const
{
	return Line3d( m_pointA, m_dir );
}

Line3d Ribbon3d::getEdgeB( void ) const
{
	return Line3d( m_pointB, m_dir );
}

