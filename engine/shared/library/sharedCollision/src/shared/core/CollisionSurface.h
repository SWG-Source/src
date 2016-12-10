// ======================================================================
//
// CollisionSurface.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CollisionSurface_H
#define INCLUDED_CollisionSurface_H

#include "sharedCollision/ContactPoint.h"

// ----------

class Transform;
class BaseExtent;
class Vector;
class Line3d;
class Segment3d;

// ----------

struct LineIntersectResult
{
	LineIntersectResult ()
	: m_hit(false),
	  m_hitTime(0.0f),
	  m_hitLoc()
	{
	}

	LineIntersectResult ( bool hit, float hitTime, ContactPoint const & contact )
	: m_hit(hit),
	  m_hitTime(hitTime),
	  m_hitLoc(contact)
	{
	}

	// ----------

	static LineIntersectResult null;

	// ----------
	
	bool operator < ( LineIntersectResult const & result ) const
	{
		return m_hitTime < result.m_hitTime;
	}

	bool            m_hit;
	float           m_hitTime;
	ContactPoint    m_hitLoc;
};

typedef std::vector<LineIntersectResult> LineIntersectResultVec;

// ======================================================================

class CollisionSurface
{
public:

	CollisionSurface();
	virtual ~CollisionSurface();

	// ----------

	virtual Transform const &   getTransform_o2p    ( void ) const = 0;
	virtual Transform const &   getTransform_o2w    ( void ) const = 0;
	virtual float               getScale            ( void ) const = 0;

	virtual BaseExtent const *  getExtent_l         ( void ) const = 0;
	virtual BaseExtent const *  getExtent_p         ( void ) const = 0;

	// ----------
	// Intersection methods

protected:

	// Disable copying

	CollisionSurface(const CollisionSurface &source);
	CollisionSurface & operator =(const CollisionSurface &source);
};

// ----------------------------------------------------------------------

#endif
