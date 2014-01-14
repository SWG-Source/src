// ======================================================================
//
// MultiShape.h
// copyright (c) 2001 Sony Online Entertainment
//
// MultiShape is a simple class that can be used to represent a sphere,
// cylinder, or box, oriented or axial.
// 
// ----------------------------------------------------------------------

#ifndef INCLUDED_MultiShape_H
#define INCLUDED_MultiShape_H

#include "sharedMath/Vector.h"

class Sphere;
class Cylinder;
class OrientedCylinder;
class AxialBox;
class YawedBox;
class OrientedBox;
class Transform;

// ======================================================================

class MultiShape
{
public:

	enum BaseType
	{
		MSBT_Sphere,
		MSBT_Cylinder,
		MSBT_Box,

		MSBT_Count,
		MSBT_Invalid,
	};

	enum OrientType
	{
		MSOT_AxisAligned,
		MSOT_Yawed,
		MSOT_Oriented,

		MSOT_Count,
		MSOT_Invalid,
	};

	enum ShapeType
	{
		MST_Sphere,
		MST_Cylinder,
		MST_OrientedCylinder,
		MST_AxialBox,
		MST_YawedBox,
		MST_OrientedBox,

		MST_Count,
		MST_Invalid,
	};

	// ----------

	MultiShape();

    MultiShape ( BaseType baseType, 
                 Vector const & center,
                 Vector const & axisX,
                 Vector const & axisY,
                 Vector const & axisZ,
                 float extentX,
                 float extentY,
                 float extentZ );

    MultiShape ( BaseType baseType,
                 ShapeType shapeType,
                 Vector const & center,
                 Vector const & axisX,
                 Vector const & axisY,
                 Vector const & axisZ,
                 float extentX,
                 float extentY,
                 float extentZ );

	explicit MultiShape ( Sphere const & shape );
	explicit MultiShape ( Cylinder const & shape );
	explicit MultiShape ( OrientedCylinder const & shape );
	explicit MultiShape ( AxialBox const & shape );
	explicit MultiShape ( YawedBox const & shape );
	explicit MultiShape ( OrientedBox const & shape );

	// ----------

	BaseType            getBaseType         ( void ) const;
	ShapeType           getShapeType        ( void ) const;

	Vector const &      getCenter           ( void ) const;

	Vector const &      getAxisX            ( void ) const;
	Vector const &      getAxisY            ( void ) const;
	Vector const &      getAxisZ            ( void ) const;

	float               getExtentX          ( void ) const;
	float               getExtentY          ( void ) const;
	float               getExtentZ          ( void ) const;

	void                setExtentX          ( float newExtent );
	void                setExtentY          ( float newExtent );
	void                setExtentZ          ( float newExtent );

	// ----------

	Vector              getBase             ( void ) const;

	float               getWidth            ( void ) const;
	float               getHeight           ( void ) const;
	float               getDepth            ( void ) const;

	float               calcAvoidanceRadius ( void ) const;
	void                updateShapeType     ( void );

	Sphere              getBoundingSphere   ( void ) const;
	AxialBox            getBoundingBox      ( void ) const;

	// ----------

	Sphere              getSphere           ( void ) const;
	Cylinder            getCylinder         ( void ) const;
	OrientedCylinder    getOrientedCylinder ( void ) const;
	AxialBox            getAxialBox         ( void ) const;
	YawedBox            getYawedBox         ( void ) const;
	OrientedBox         getOrientedBox      ( void ) const;

	// ----------

	Sphere              getLocalSphere      ( void ) const;
	Cylinder            getLocalCylinder    ( void ) const;
	AxialBox            getLocalAxialBox    ( void ) const;

	// ----------

	Transform           getTransform_l2p    ( void ) const;
	Transform           getTransform_p2l    ( void ) const;

protected:

	BaseType    m_baseType;
	ShapeType   m_shapeType;

	Vector      m_center;

	Vector      m_axisX;
	Vector      m_axisY;
	Vector      m_axisZ;

	float       m_extentX;
	float       m_extentY;
	float       m_extentZ;
};

// ----------------------------------------------------------------------

inline MultiShape::BaseType MultiShape::getBaseType ( void ) const
{
	return m_baseType;
}

inline MultiShape::ShapeType MultiShape::getShapeType ( void ) const
{
	return m_shapeType;
}

// ----------

inline Vector const & MultiShape::getCenter ( void ) const
{
	return m_center;
}

// ----------

inline Vector const & MultiShape::getAxisX ( void ) const
{
	return m_axisX;
}

inline Vector const & MultiShape::getAxisY ( void ) const
{
	return m_axisY;
}

inline Vector const & MultiShape::getAxisZ ( void ) const
{
	return m_axisZ;
}

// ----------

inline float MultiShape::getExtentX ( void ) const
{
	return m_extentX;
}

inline float MultiShape::getExtentY ( void ) const
{
	return m_extentY;
}

inline float MultiShape::getExtentZ ( void ) const
{
	return m_extentZ;
}

// ----------

inline void MultiShape::setExtentX ( float newExtent )
{
	m_extentX = newExtent;
}

inline void MultiShape::setExtentY ( float newExtent )
{
	m_extentY = newExtent;
}

inline void MultiShape::setExtentZ ( float newExtent )
{
	m_extentZ = newExtent;
}

// ----------------------------------------------------------------------

inline Vector MultiShape::getBase ( void ) const
{
	return getCenter() - getAxisY() * getExtentY();
}

// ----------

inline float MultiShape::getWidth ( void ) const
{
	return getExtentX() * 2.0f;
}

inline float MultiShape::getHeight ( void ) const
{
	return getExtentY() * 2.0f;
}

inline float MultiShape::getDepth ( void ) const
{
	return getExtentZ() * 2.0f;
}

// ======================================================================

#endif
