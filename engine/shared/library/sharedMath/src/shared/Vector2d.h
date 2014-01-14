//===================================================================
//
// Vector2d.h
// asommers 7-26-99
//
// copyright 1999, bootprint entertainment
//
//===================================================================

#ifndef INCLUDED_Vector2d_H
#define INCLUDED_Vector2d_H

//===================================================================

class Vector2d
{
public:

	float x;
	float y;

public:

	Vector2d ();
	Vector2d (float newX, float newY);

	void set (float newX, float newY);
	void makeZero ();
	bool isZero () const;
	bool normalize ();
	float dot (const Vector2d& vector) const;
	float theta () const;
	void rotate (float radians);
	void rotate (float radians, const Vector2d& center);

	const Vector2d operator- () const;
	const Vector2d operator+ (const Vector2d& rhs) const;
	const Vector2d operator- (const Vector2d& rhs) const;
	const Vector2d operator* (float scalar) const;
	const Vector2d operator/ (float scalar) const;

	bool operator== (const Vector2d& rhs) const;
	bool operator!= (const Vector2d& rhs) const;

	Vector2d& operator+= (const Vector2d& rhs);
	Vector2d& operator-= (const Vector2d& rhs);
	Vector2d& operator*= (float scalar);
	Vector2d& operator/= (float scalar);

	float magnitude () const;
	float magnitudeSquared () const;
	float magnitudeBetween (const Vector2d& vector) const;
	float magnitudeBetweenSquared (const Vector2d& vector) const;

	static const Vector2d linearInterpolate (const Vector2d& start, const Vector2d& end, float t);
	static const Vector2d normalized (const Vector2d& vector, bool* result=0);
	static const Vector2d normal (const Vector2d& vector, bool normalize, bool* result=0);
};

//===================================================================

inline Vector2d::Vector2d () :
	x (0),
	y (0)
{
}

//-------------------------------------------------------------------
	
inline Vector2d::Vector2d (float newX, float newY) :
	x (newX),
	y (newY)
{
}

//-------------------------------------------------------------------

inline void Vector2d::set (float newX, float newY)
{
	x = newX;
	y = newY;
}

//-------------------------------------------------------------------
	
inline void Vector2d::makeZero ()
{
	x = 0;
	y = 0;
}

//-------------------------------------------------------------------

inline bool Vector2d::isZero () const
{
	return x == 0 && y == 0;
}

//-------------------------------------------------------------------

inline bool Vector2d::normalize ()
{
	const float mag = magnitude ();

	if (mag < 0.00001f)
		return false;

	*this /= mag;

	return true;
}

//-------------------------------------------------------------------

inline float Vector2d::dot (const Vector2d& vector) const
{
	return (x * vector.x) + (y * vector.y);
}

//-------------------------------------------------------------------

inline float Vector2d::theta () const
{
	return atan2(x, y);
}

//-------------------------------------------------------------------

inline void Vector2d::rotate (float radians)
{
	const float cosAngle = cos (radians);
	const float sinAngle = sin (radians);

	const float oldX = x;
	const float oldY = y;

	x = oldX * cosAngle - oldY * sinAngle;
	y = oldX * sinAngle + oldY * cosAngle;
}

//-------------------------------------------------------------------

inline void Vector2d::rotate (float radians, const Vector2d& origin)
{
	const Vector2d point (x - origin.x, y - origin.y);
	const float    cosAngle = cos (radians);
	const float    sinAngle = sin (radians);

	x = origin.x + point.x * cosAngle - point.y * sinAngle;
	y = origin.y + point.x * sinAngle + point.y * cosAngle;
}

//-------------------------------------------------------------------

inline const Vector2d Vector2d::operator- () const
{
	return Vector2d (-x, -y);
}

//-------------------------------------------------------------------

inline const Vector2d Vector2d::operator+ (const Vector2d& rhs) const
{
	return Vector2d (x + rhs.x, y + rhs.y);
}

//-------------------------------------------------------------------

inline const Vector2d Vector2d::operator- (const Vector2d& rhs) const
{
	return Vector2d (x - rhs.x, y - rhs.y);
}

//-------------------------------------------------------------------

inline const Vector2d Vector2d::operator* (float scalar) const
{
	return Vector2d (x * scalar, y * scalar);
}

//-------------------------------------------------------------------

inline const Vector2d Vector2d::operator/ (float scalar) const
{
	return operator* (RECIP (scalar));
}

//-------------------------------------------------------------------

inline bool Vector2d::operator== (const Vector2d& rhs) const
{
	return x == rhs.x && y == rhs.y;
}

//-------------------------------------------------------------------

inline bool Vector2d::operator!= (const Vector2d& rhs) const
{
	return !operator== (rhs);
}

//-------------------------------------------------------------------

inline float Vector2d::magnitude () const
{
	return static_cast<float> (sqrt (magnitudeSquared ()));
}

//-------------------------------------------------------------------

inline float Vector2d::magnitudeSquared () const
{
	return sqr (x) + sqr (y);
}

//-------------------------------------------------------------------

inline float Vector2d::magnitudeBetween (const Vector2d& vector) const
{
	return static_cast<float> (sqrt (magnitudeBetweenSquared (vector)));
}

//-------------------------------------------------------------------

inline float Vector2d::magnitudeBetweenSquared (const Vector2d& vector) const
{
	return sqr (x - vector.x) + sqr (y - vector.y);
}

//-------------------------------------------------------------------

inline Vector2d& Vector2d::operator+= (const Vector2d& rhs)
{
	x += rhs.x;
	y += rhs.y;

	return *this;
}

//-------------------------------------------------------------------

inline Vector2d& Vector2d::operator-= (const Vector2d& rhs)
{
	x -= rhs.x;
	y -= rhs.y;

	return *this;
}

//-------------------------------------------------------------------

inline Vector2d& Vector2d::operator*= (float scalar)
{
	x *= scalar;
	y *= scalar;

	return *this;
}

//-------------------------------------------------------------------

inline Vector2d& Vector2d::operator/= (float scalar)
{
	*this *= RECIP (scalar);

	return *this;
}

//-------------------------------------------------------------------

inline const Vector2d Vector2d::linearInterpolate (const Vector2d& start, const Vector2d& end, float t)
{
	Vector2d v;
	v.x = ::linearInterpolate (start.x, end.x, t);
	v.y = ::linearInterpolate (start.y, end.y, t);

	return v;
}

//-------------------------------------------------------------------

inline const Vector2d Vector2d::normalized (const Vector2d& vector, bool* result)
{
	Vector2d v = vector;
	bool     r = v.normalize ();
	if (result)
		*result = r;

	return v;
}

//-------------------------------------------------------------------

inline const Vector2d Vector2d::normal (const Vector2d& vector, bool normalize, bool* result)
{
	Vector2d v (-vector.y, vector.x);
	
	if (normalize)
	{
		bool r = v.normalize ();
		if (result)
			*result = r;
	}

	return v;
}

//===================================================================

#endif
