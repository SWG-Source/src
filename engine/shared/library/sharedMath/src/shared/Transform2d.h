//===================================================================
//
// Transform2d.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_Transform2d_H
#define INCLUDED_Transform2d_H

//===================================================================

#include "sharedMath/Vector2d.h"

//===================================================================

class Rectangle2d;

//===================================================================

class Transform2d
{
public:

	static const Transform2d identity;

public:

	Transform2d ();
	
	void           move_l (const Vector2d& v);
	void           move_p (const Vector2d& v);

	void           yaw_l (float radians);
	void           resetRotate_l2p ();
	void           resetRotateTranslate_l2p ();

	const Vector2d getPosition_p () const;
	void           setPosition_p (const Vector2d& v);
	void           setPosition_p (float x, float y);

	const Vector2d rotate_l2p (const Vector2d& v) const;
	const Vector2d rotateTranslate_l2p (const Vector2d& v) const;
	const Vector2d rotate_p2l (const Vector2d& v) const;
	const Vector2d rotateTranslate_p2l (const Vector2d& v) const;

	//----------------------------------------------------------------------------
	// Returns the result equivalent to expanding the source rectangle into its
	// four corner points, transform each four points using rotateTranslate_p2l,
	// and then finding the resulting bounding rectangle of those four points.
	// 'r' and 'o_result' may reference the same object.
	void rotateTranslate_p2l(Rectangle2d &o_result, const Rectangle2d &r) const;
	//----------------------------------------------------------------------------

	bool           operator== (const Transform2d& rhs) const;
	bool           operator!= (const Transform2d& rhs) const;

private:

	float          m_matrix [2][3];
};

//-------------------------------------------------------------------

inline void Transform2d::resetRotateTranslate_l2p ()
{
	// make the diagonal 1's to form the identity m_matrix
	m_matrix [0][0] = 1.f;
	m_matrix [0][1] = 0.f;
	m_matrix [0][2] = 0.f;

	m_matrix [1][0] = 0.f;
	m_matrix [1][1] = 1.f;
	m_matrix [1][2] = 0.f;
}

//-------------------------------------------------------------------

inline void Transform2d::resetRotate_l2p ()
{
	m_matrix [0][0] = 1.f;
	m_matrix [0][1] = 0.f;

	m_matrix [1][0] = 0.f;
	m_matrix [1][1] = 1.f;
}

//-------------------------------------------------------------------

inline Transform2d::Transform2d ()
{
	resetRotateTranslate_l2p ();
}

//-------------------------------------------------------------------

inline void Transform2d::move_p (const Vector2d& v)
{
	m_matrix [0][2] += v.x;
	m_matrix [1][2] += v.y;
}

//-------------------------------------------------------------------

inline void Transform2d::move_l (const Vector2d& v)
{
	move_p (rotate_l2p (v));
}

//-------------------------------------------------------------------

inline const Vector2d Transform2d::getPosition_p () const
{
	return Vector2d (m_matrix [0][2], m_matrix [1][2]);
}

//-------------------------------------------------------------------

inline void Transform2d::setPosition_p (const Vector2d& v)
{
	m_matrix [0][2] = v.x;
	m_matrix [1][2] = v.y;
}

//-------------------------------------------------------------------

inline void Transform2d::setPosition_p (float x, float y)
{
	m_matrix [0][2] = x;
	m_matrix [1][2] = y;
}

//-------------------------------------------------------------------

inline const Vector2d Transform2d::rotate_l2p (const Vector2d& v) const
{
	return Vector2d (m_matrix [0][0] * v.x + m_matrix [0][1] * v.y, m_matrix [1][0] * v.x + m_matrix [1][1] * v.y);
}

//-------------------------------------------------------------------

inline const Vector2d Transform2d::rotateTranslate_l2p (const Vector2d& v) const
{
	return Vector2d (m_matrix [0][0] * v.x + m_matrix [0][1] * v.y + m_matrix [0][2], m_matrix [1][0] * v.x + m_matrix [1][1] * v.y + m_matrix [1][2]);
}

//-------------------------------------------------------------------

inline const Vector2d Transform2d::rotate_p2l (const Vector2d& v) const
{
	return Vector2d (m_matrix [0][0] * v.x + m_matrix [1][0] * v.y, m_matrix [0][1] * v.x + m_matrix [1][1] * v.y);
}

//-------------------------------------------------------------------

inline const Vector2d Transform2d::rotateTranslate_p2l (const Vector2d& v) const
{
	const float x = v.x - m_matrix[0][2];
	const float y = v.y - m_matrix[1][2];

	return Vector2d(
		m_matrix[0][0]*x + m_matrix[1][0]*y, 
		m_matrix[0][1]*x + m_matrix[1][1]*y
	);
}

//===================================================================

#endif
