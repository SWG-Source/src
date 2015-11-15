//
// Rectangle2d.h
// asommers 7-26-99
//
// copyright 1999, bootprint entertainment
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_Rectangle2d_H
#define INCLUDED_Rectangle2d_H

#include <algorithm>
#include <cmath>

//-------------------------------------------------------------------

class Line2d;
class Vector2d;

//-------------------------------------------------------------------

class Rectangle2d
{
public:

	//-- lower left
	float x0;  
	float y0;

	//-- upper right
	float x1;  
	float y1;

public:

	Rectangle2d ();
	Rectangle2d (float newX0, float newY0, float newX1, float newY1);
	~Rectangle2d ();

	void           set (float newX0, float newY0, float newX1, float newY1);
	float          getWidth () const;
	float          getHeight () const;
	const Vector2d getCenter () const;
	bool           isWithin (float x, float y) const;
	bool           isWithin (const Vector2d& point) const;
	bool           isVector2d () const;
	void           expand (float x, float y);
	void           expand (const Vector2d& point);
	void           expand (const Rectangle2d& rectangle);
	void           translate (float x, float y);
	void           translate (const Vector2d& point);
	void           scale (float scalar);
	bool           intersects (const Rectangle2d& other) const;
	bool           contains (Rectangle2d const & other) const;
	bool           intersects (Line2d const & line) const;

	bool           operator== (const Rectangle2d& rhs) const;
	bool           operator!= (const Rectangle2d& rhs) const;
};

//-------------------------------------------------------------------

inline Rectangle2d::Rectangle2d () :
	x0 (0),
	y0 (0),
	x1 (0),
	y1 (0)
{
}

//-------------------------------------------------------------------

inline Rectangle2d::Rectangle2d (const float newX0, const float newY0, const float newX1, const float newY1) :
	x0 (newX0),
	y0 (newY0),
	x1 (newX1),
	y1 (newY1)
{
}

//-------------------------------------------------------------------

inline Rectangle2d::~Rectangle2d ()
{
}

//-------------------------------------------------------------------

inline void Rectangle2d::set (const float newX0, const float newY0, const float newX1, const float newY1)
{
	x0 = newX0;
	y0 = newY0;
	x1 = newX1;
	y1 = newY1;
}

//-------------------------------------------------------------------

inline float Rectangle2d::getWidth () const
{	
	return std::abs(x1 - x0);
}

//-------------------------------------------------------------------

inline float Rectangle2d::getHeight () const
{
	return std::abs(y1 - y0);
}

//-------------------------------------------------------------------

inline bool Rectangle2d::isWithin (const float x, const float y) const
{
	if (x0 < x1)
		if (y0 < y1)
			return 
				WithinRangeInclusiveInclusive (x0, x, x1) &&
				WithinRangeInclusiveInclusive (y0, y, y1);
		else
			return 
				WithinRangeInclusiveInclusive (x0, x, x1) &&
				WithinRangeInclusiveInclusive (y1, y, y0);
	else
		if (y0 < y1)
			return 
				WithinRangeInclusiveInclusive (x1, x, x0) &&
				WithinRangeInclusiveInclusive (y0, y, y1);
		else
			return 
				WithinRangeInclusiveInclusive (x1, x, x0) &&
				WithinRangeInclusiveInclusive (y1, y, y0);
}

//-------------------------------------------------------------------

inline bool Rectangle2d::isVector2d () const
{
	return x0 == x1 && y0 == y1;
}

//-------------------------------------------------------------------

inline void Rectangle2d::expand (const float x, const float y)
{
	if (x < x0)
		x0 = x;

	if (y < y0)
		y0 = y;

	if (x > x1)
		x1 = x;

	if (y > y1)
		y1 = y;
}

//-------------------------------------------------------------------

inline void Rectangle2d::expand (const Rectangle2d& rectangle)
{
	expand (rectangle.x0, rectangle.y0);
	expand (rectangle.x1, rectangle.y0);
	expand (rectangle.x0, rectangle.y1);
	expand (rectangle.x1, rectangle.y1);
}

//-------------------------------------------------------------------

inline void Rectangle2d::translate (const float x, const float y)
{
	x0 += x;
	y0 += y;
	x1 += x;
	y1 += y;
}

//-------------------------------------------------------------------

inline bool Rectangle2d::intersects (const Rectangle2d& other) const
{
	return !(x1 < other.x0 || x0 > other.x1 || y1 < other.y0 || y0 > other.y1);
}

//-------------------------------------------------------------------

inline bool Rectangle2d::contains (Rectangle2d const & other) const
{
	return other.x0 >= x0 && other.x1 <= x1 && other.y0 >= y0 && other.y1 <= y1;
}

//-------------------------------------------------------------------

inline bool Rectangle2d::operator== (const Rectangle2d& rhs) const
{
	return x0 == rhs.x0 && x1 == rhs.x1 && y0 == rhs.y0 && y1 == rhs.y1;
}

//-------------------------------------------------------------------

inline bool Rectangle2d::operator!= (const Rectangle2d& rhs) const
{
	return !operator== (rhs);
}

//-------------------------------------------------------------------

#endif
