// ======================================================================
//
// ContactPoint.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/ContactPoint.h"

#include "sharedFile/Iff.h"

#include "sharedCollision/CollisionSurface.h"
#include "sharedCollision/CollisionUtils.h"

#include "sharedMath/Transform.h"

// ----------------------------------------------------------------------

ContactPoint::ContactPoint()
: m_pSurface( nullptr ),
  m_position( Vector::zero ),
  m_offset( 0.0f ),
  m_hitId( -1 )
{
}

ContactPoint::ContactPoint( CollisionSurface const * surface, Vector const & point, int id, float offset )
: m_pSurface(surface),
  m_position(point),
  m_offset(offset),
  m_hitId(id)
{
}

ContactPoint::ContactPoint ( ContactPoint const & locCopy )
: m_pSurface(locCopy.m_pSurface),	//lint !e1554
  m_position(locCopy.m_position),
  m_offset(locCopy.m_offset),
  m_hitId(locCopy.m_hitId)
{
}

// ----------------------------------------------------------------------

bool ContactPoint::isAttached ( void ) const
{
	return (m_hitId != -1) && (m_pSurface != nullptr);
}

// ----------------------------------------------------------------------

Vector const & ContactPoint::getPosition_l ( void ) const
{
	return m_position;
}

// ----------

void ContactPoint::setPosition_l ( Vector const & newPos )
{
	NAN_CHECK(newPos);

	m_position = newPos;
}

// ----------------------------------------------------------------------

real ContactPoint::getOffset ( void ) const
{
	return m_offset;
}

void ContactPoint::setOffset ( float newOffset )
{
	NAN_CHECK(newOffset);

	m_offset = newOffset;
}

// ----------

Vector ContactPoint::getOffsetPosition_l ( void ) const
{
	return m_position + Vector(0.0f,m_offset,0.0f);
}

// ----------

int ContactPoint::getId ( void ) const
{
	return m_hitId;
}

void ContactPoint::setId ( int newId )
{
	m_hitId = newId;
}

// ----------

CollisionSurface const * ContactPoint::getSurface ( void ) const
{
	return m_pSurface;
}

void ContactPoint::setSurface ( CollisionSurface const * pSurf )
{
	m_pSurface = pSurf;
}

// ----------------------------------------------------------------------

void ContactPoint::read_0000 ( Iff & iff )
{
	m_position = iff.read_floatVector();
	m_offset = iff.read_float();
	m_pSurface = nullptr;
	m_hitId = iff.read_int32();

	NAN_CHECK(m_position);
	NAN_CHECK(m_offset);
}

// ----------

void ContactPoint::write ( Iff & iff ) const
{
	iff.insertChunkData(m_position);
	iff.insertChunkData(m_offset);
	iff.insertChunkData(m_hitId);
}

