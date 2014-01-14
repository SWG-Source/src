//===================================================================
//
// Line2d.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_Line2d_H
#define INCLUDED_Line2d_H

//===================================================================

#include "Vector2d.h"

//===================================================================

class Line2d
{
private:

	Vector2d   m_normal;
	float      m_c;

public:

	Line2d ();
	Line2d (float x0, float y0, float x1, float y1);
	Line2d (const Vector2d& normal, float c);
	Line2d (const Vector2d& point0, const Vector2d& point1);

	void set (const Vector2d& normal, float c);
	void set (const Vector2d& point0, const Vector2d& point1);

	const Vector2d& getNormal () const;
	const float     getC () const;

	float           computeDistanceTo (const Vector2d& point) const;
	const Vector2d  project (const Vector2d& point) const;
	bool findIntersection(Vector2d const & point0, Vector2d const & point1, Vector2d & intersection) const;
};

//===================================================================

inline Line2d::Line2d () :
	m_normal (1.f, 0.f),
	m_c (0.f)
{
}

//-------------------------------------------------------------------

inline Line2d::Line2d (float const x0, float const y0, float const x1, float const y1) :
	m_normal(),
	m_c(0.f)
{
	set(Vector2d(x0, y0), Vector2d(x1, y1));
}

//-------------------------------------------------------------------

inline Line2d::Line2d (const Vector2d& normal, float c) :
	m_normal (normal),
	m_c (c)
{
}

//-------------------------------------------------------------------

inline Line2d::Line2d (const Vector2d& point0, const Vector2d& point1) :
	m_normal (),
	m_c (0.f)
{
	set (point0, point1);
}

//-------------------------------------------------------------------

inline void Line2d::set (const Vector2d& normal, float c)
{
	m_normal = normal;
	m_c      = c;
}

//-------------------------------------------------------------------

inline void Line2d::set (const Vector2d& point0, const Vector2d& point1)
{
	m_normal.set (-point1.y + point0.y, point1.x - point0.x);
	if (!m_normal.normalize ())
	{
		m_normal.set (1.f, 0.f);
		DEBUG_FATAL (true, ("Line::set could not normalize vector"));
	}

	m_c = -m_normal.dot (point0);
}

//-------------------------------------------------------------------

inline const Vector2d& Line2d::getNormal () const
{
	return m_normal;
}

//-------------------------------------------------------------------

inline const float Line2d::getC () const
{
	return m_c;
}

//-------------------------------------------------------------------

inline float Line2d::computeDistanceTo (const Vector2d& point) const
{
	return m_normal.dot (point) + m_c;
}

//-------------------------------------------------------------------

inline const Vector2d Line2d::project (const Vector2d& point) const
{
	return point - (m_normal * computeDistanceTo (point));
}

//-------------------------------------------------------------------

inline bool Line2d::findIntersection(Vector2d const & start, Vector2d const & end, Vector2d & intersection) const
{
	float const t0(computeDistanceTo(start));
	float const t1(computeDistanceTo(end));

	// check to make sure the endpoints span the plane
	if ((t0 * t1) > 0.f)
		return false;

	// both zero
	if (t0 == t1) 
	{
		intersection = start;
		return true;
	}

	// safe since sign of t0 is always opposite t1
	float const t = t0 / (t0 - t1); 
	intersection.x = start.x + (end.x - start.x) * t;
	intersection.y = start.y + (end.y - start.y) * t;

	return true;
}

//===================================================================

#endif
