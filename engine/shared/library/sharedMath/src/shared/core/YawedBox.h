// ======================================================================
//
// YawedBox.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_YawedBox_H
#define INCLUDED_YawedBox_H

#include "sharedMath/Vector.h"

class AxialBox;
class Range;
class Transform;

// ======================================================================

class YawedBox
{
public:

	YawedBox();
	YawedBox( AxialBox const & box, float yaw );
	YawedBox( Vector const & base, Vector const & axisX, Vector const & axisZ, float extentX, float extentZ, float height );

	// ----------

	Range           getRangeY         ( void ) const;

	Vector const &  getBase           ( void ) const;

	float           getHeight         ( void ) const;

	Vector          getCorner         ( int whichCorner ) const;

	// ----------

	Vector          getCenter         ( void ) const;

	Vector const &  getAxisX          ( void ) const;
	Vector const &  getAxisY          ( void ) const;
	Vector const &  getAxisZ          ( void ) const;

	float           getExtentX        ( void ) const;
	float           getExtentY        ( void ) const;
	float           getExtentZ        ( void ) const;

	// ----------

	AxialBox        getLocalShape     ( void ) const;

	Transform       getTransform_l2p  ( void ) const;
	Transform       getTransform_p2l  ( void ) const;

	Vector          transformToLocal  ( Vector const & V ) const;
	Vector          transformToWorld  ( Vector const & V ) const;

	Vector          rotateToLocal     ( Vector const & V ) const;
	Vector          rotateToWorld     ( Vector const & V ) const;

	// ----------

protected:
		
	Vector	m_base;

	Vector  m_axisX;
	Vector  m_axisZ;

	float   m_extentX;
	float   m_extentZ;

	float   m_height;
};

// ----------------------------------------------------------------------

inline YawedBox::YawedBox ( Vector const & base, 
						    Vector const & axisX, 
						    Vector const & axisZ, 
						    float extentX, 
						    float extentZ, 
						    float height )
: m_base(base),
  m_axisX(axisX),
  m_axisZ(axisZ),
  m_extentX(extentX),
  m_extentZ(extentZ),
  m_height(height)
{
}

// ----------------------------------------------------------------------

inline Vector const & YawedBox::getBase ( void ) const
{
	return m_base;
}

inline float YawedBox::getHeight ( void ) const
{
	return m_height;
}

inline Vector YawedBox::getCorner ( int whichCorner ) const
{
	// These corners are ordered so that the first 4 are on the bottom of the box

	Vector X = m_axisX * m_extentX;
	Vector Y = Vector::unitY * m_height;
	Vector Z = m_axisZ * m_extentZ;

	switch(whichCorner)
	{
	case 0: return m_base - X - Z;
	case 1: return m_base + X - Z;
	case 2: return m_base - X + Z;
	case 3: return m_base + X + Z;
	case 4: return m_base - X - Z + Y;
	case 5: return m_base + X - Z + Y;
	case 6: return m_base - X + Z + Y;
	case 7: return m_base + X + Z + Y;

	default: return Vector(0,0,0);
	}
}

// ----------------------------------------------------------------------

inline Vector YawedBox::getCenter ( void ) const
{
	return getBase() + Vector(0,getExtentY(),0);
}

// ----------

inline Vector const & YawedBox::getAxisX ( void ) const
{
	return m_axisX;
}

inline Vector const & YawedBox::getAxisY ( void ) const
{
	return Vector::unitY;
}

inline Vector const & YawedBox::getAxisZ ( void ) const
{
	return m_axisZ;
}

// ----------

inline float YawedBox::getExtentX ( void ) const
{
	return m_extentX;
}

inline float YawedBox::getExtentY ( void ) const
{
	return getHeight() / 2.0f;
}

inline float YawedBox::getExtentZ ( void ) const
{
	return m_extentZ;
}

// ======================================================================

#endif

