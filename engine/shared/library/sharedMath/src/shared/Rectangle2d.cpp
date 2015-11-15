//===================================================================
//
// Rectangle2d.cpp
// asommers 7-26-99
//
// copyright 1999, bootprint entertainment
// copyright 2001, sony online entertainment
//
//===================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Rectangle2d.h"

#include "sharedMath/Line2d.h"
#include "sharedMath/Vector2d.h"

#include <algorithm>

//===================================================================

inline bool WithinRange(float rangeMin, float const value, float rangeMax)
{
	if (rangeMin > rangeMax)
		std::swap(rangeMin, rangeMax);

	return (value >= rangeMin) && (value <= rangeMax);
}

//===================================================================

const Vector2d Rectangle2d::getCenter () const
{
	Vector2d tmp;
	tmp.x = x0 + getWidth () * 0.5f;
	tmp.y = y0 + getHeight () * 0.5f;
	return tmp;
}

//-------------------------------------------------------------------

bool Rectangle2d::isWithin (const Vector2d& point) const
{
	return isWithin (point.x, point.y);
}

//-------------------------------------------------------------------

void Rectangle2d::expand (const Vector2d& point)
{
	expand (point.x, point.y);
}

//-------------------------------------------------------------------

void Rectangle2d::translate (const Vector2d& point)
{
	translate (point.x, point.y);
}

//-------------------------------------------------------------------

void Rectangle2d::scale (const float scalar)
{
	const Vector2d center = getCenter ();

	x0 = (x0 - center.x) * scalar + center.x;
	y0 = (y0 - center.y) * scalar + center.y;
	x1 = (x1 - center.x) * scalar + center.x;
	y1 = (y1 - center.y) * scalar + center.y;
}

//-------------------------------------------------------------------

bool Rectangle2d::intersects (Line2d const & line) const
{
	Vector2d intersection;

	Vector2d const v0(x0, y0);
	Vector2d const v1(x1, y0);
	if (line.findIntersection(v0, v1, intersection) && WithinRange(v0.x, intersection.x, v1.x) && WithinRange(v0.y, intersection.y, v1.y))
		return true;

	Vector2d const v2(x1, y1);
	if (line.findIntersection(v1, v2, intersection) && WithinRange(v1.x, intersection.x, v2.x) && WithinRange(v1.y, intersection.y, v2.y))
		return true;

	Vector2d const v3(x0, y1);
	if (line.findIntersection(v2, v3, intersection) && WithinRange(v2.x, intersection.x, v3.x) && WithinRange(v2.y, intersection.y, v3.y))
		return true;

	if (line.findIntersection(v3, v0, intersection) && WithinRange(v3.x, intersection.x, v0.x) && WithinRange(v3.y, intersection.y, v0.y))
		return true;

	return false;
}

//===================================================================

