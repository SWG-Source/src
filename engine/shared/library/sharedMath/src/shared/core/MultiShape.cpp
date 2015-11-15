// ======================================================================
//
// MultiShape.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/MultiShape.h"

#include "sharedMath/Transform.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/OrientedCylinder.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"
#include "sharedMath/OrientedBox.h"

#include <algorithm> // for max

MultiShape::ShapeType	shapeTable[MultiShape::MSBT_Count][MultiShape::MSOT_Count] = 
{
    { MultiShape::MST_Sphere,		MultiShape::MST_Sphere,		MultiShape::MST_Sphere           },
	{ MultiShape::MST_Cylinder,		MultiShape::MST_Cylinder,	MultiShape::MST_OrientedCylinder },
	{ MultiShape::MST_AxialBox,		MultiShape::MST_YawedBox,	MultiShape::MST_OrientedBox      }
};

// ----------------------------------------------------------------------

MultiShape::MultiShape() 
: m_baseType ( MSBT_Invalid ),
  m_shapeType( MST_Invalid ),
  m_center( Vector::zero ),
  m_axisX( Vector::unitX ),
  m_axisY( Vector::unitY ),
  m_axisZ( Vector::unitZ ),
  m_extentX( 1.0f ),
  m_extentY( 1.0f ),
  m_extentZ( 1.0f )
{
}

// ----------

MultiShape::MultiShape ( BaseType baseType, 
                         ShapeType shapeType,
                         Vector const & center,
                         Vector const & axisX,
                         Vector const & axisY,
                         Vector const & axisZ,
                         float extentX,
                         float extentY,
                         float extentZ )
: m_baseType(baseType),
  m_shapeType(shapeType),
  m_center(center),
  m_axisX(axisX),
  m_axisY(axisY),
  m_axisZ(axisZ),
  m_extentX(extentX),
  m_extentY(extentY),
  m_extentZ(extentZ)
{
}

// ----------

MultiShape::MultiShape ( BaseType baseType, 
                         Vector const & center,
                         Vector const & axisX,
                         Vector const & axisY,
                         Vector const & axisZ,
                         float extentX,
                         float extentY,
                         float extentZ )
: m_baseType(baseType),
  m_shapeType(MST_Invalid),
  m_center(center),
  m_axisX(axisX),
  m_axisY(axisY),
  m_axisZ(axisZ),
  m_extentX(extentX),
  m_extentY(extentY),
  m_extentZ(extentZ)
{
	updateShapeType();
}


// ----------

MultiShape::MultiShape ( Sphere const & shape )
: m_baseType    ( MSBT_Sphere ),
  m_shapeType   ( MST_Sphere ),
  m_center      ( shape.getCenter() ),
  m_axisX       ( shape.getAxisX() ),
  m_axisY       ( shape.getAxisY() ),
  m_axisZ       ( shape.getAxisZ() ),
  m_extentX     ( shape.getExtentX() ),
  m_extentY     ( shape.getExtentY() ),
  m_extentZ     ( shape.getExtentZ() )
{
}

// ----------

MultiShape::MultiShape ( Cylinder const & shape )
: m_baseType    ( MSBT_Cylinder ),
  m_shapeType   ( MST_Cylinder ),
  m_center      ( shape.getCenter() ),
  m_axisX       ( shape.getAxisX() ),
  m_axisY       ( shape.getAxisY() ),
  m_axisZ       ( shape.getAxisZ() ),
  m_extentX     ( shape.getExtentX() ),
  m_extentY     ( shape.getExtentY() ),
  m_extentZ     ( shape.getExtentZ() )
{
}

// ----------

MultiShape::MultiShape ( OrientedCylinder const & shape )
: m_baseType    ( MSBT_Cylinder ),
  m_shapeType   ( MST_OrientedCylinder ),
  m_center      ( shape.getCenter() ),
  m_axisX       ( shape.getAxisX() ),
  m_axisY       ( shape.getAxisY() ),
  m_axisZ       ( shape.getAxisZ() ),
  m_extentX     ( shape.getExtentX() ),
  m_extentY     ( shape.getExtentY() ),
  m_extentZ     ( shape.getExtentZ() )
{
}

// ----------

MultiShape::MultiShape ( AxialBox const & shape )
: m_baseType    ( MSBT_Box ),
  m_shapeType   ( MST_AxialBox ),
  m_center      ( shape.getCenter() ),
  m_axisX       ( shape.getAxisX() ),
  m_axisY       ( shape.getAxisY() ),
  m_axisZ       ( shape.getAxisZ() ),
  m_extentX     ( shape.getExtentX() ),
  m_extentY     ( shape.getExtentY() ),
  m_extentZ     ( shape.getExtentZ() )
{
}

// ----------

MultiShape::MultiShape ( YawedBox const & shape )
: m_baseType    ( MSBT_Box ),
  m_shapeType   ( MST_YawedBox ),
  m_center      ( shape.getCenter() ),
  m_axisX       ( shape.getAxisX() ),
  m_axisY       ( shape.getAxisY() ),
  m_axisZ       ( shape.getAxisZ() ),
  m_extentX     ( shape.getExtentX() ),
  m_extentY     ( shape.getExtentY() ),
  m_extentZ     ( shape.getExtentZ() )
{
}

// ----------

MultiShape::MultiShape ( OrientedBox const & shape )
: m_baseType    ( MSBT_Box ),
  m_shapeType   ( MST_OrientedBox ),
  m_center      ( shape.getCenter() ),
  m_axisX       ( shape.getAxisX() ),
  m_axisY       ( shape.getAxisY() ),
  m_axisZ       ( shape.getAxisZ() ),
  m_extentX     ( shape.getExtentX() ),
  m_extentY     ( shape.getExtentY() ),
  m_extentZ     ( shape.getExtentZ() )
{
}

// ----------------------------------------------------------------------

void MultiShape::updateShapeType ( void )
{
	const real axisEpsilon = 0.0000001f;

    bool xOK = m_axisX.magnitudeBetweenSquared( Vector::unitX ) < axisEpsilon;
    bool yOK = m_axisY.magnitudeBetweenSquared( Vector::unitY ) < axisEpsilon;
    bool zOK = m_axisZ.magnitudeBetweenSquared( Vector::unitZ ) < axisEpsilon;

    MultiShape::OrientType orientType = MultiShape::MSOT_Oriented;
	
    if(xOK && yOK && zOK) orientType = MultiShape::MSOT_AxisAligned;
    else if(yOK) orientType = MultiShape::MSOT_Yawed;

    MultiShape::ShapeType shapeType = shapeTable[m_baseType][orientType];

	m_shapeType = shapeType;
}

// ----------------------------------------------------------------------
// @todo - This really needs to go somewhere else.

// Calculate the radius of the tight-fitting axial cylinder.

float MultiShape::calcAvoidanceRadius ( void ) const
{
	switch(m_shapeType)
	{
	case MST_Sphere:
		return m_extentX;
	case MST_Cylinder:
		return m_extentX;
	case MST_OrientedCylinder:
		{
			// Slightly tricky - there are two contact modes between an
			// oriented cylinder and its tight-fitting axial cylinder -
			// two-contact (cylinder is tilted slightly) and four-contact
			// cylinder is on its side). The maximum of those two radii is
			// the radius of the tight-fitting cylinder.

			float sinTheta = m_axisY.y;
			float cosTheta = sqrt( m_axisY.x * m_axisY.x + m_axisY.z * m_axisY.z );

			float twoContactRadius = std::abs(m_extentY * sinTheta + m_extentX * cosTheta);

			float blah = m_extentY * sinTheta;

			float fourContactRadius = sqrt(m_extentX * m_extentX + blah * blah);

			return std::max(twoContactRadius,fourContactRadius);
		}
	case MST_AxialBox:
		return sqrt(m_extentX * m_extentX + m_extentZ * m_extentZ);
	case MST_YawedBox:
		return sqrt(m_extentX * m_extentX + m_extentZ * m_extentZ);
	case MST_OrientedBox:
		{
			// There's gotta be a cheaper way to calculate this radius...

			// Take four corners at one end of the box, figure out which
			// is the farthest from the Y axis. Four is sufficient because
			// of symmetry.

			Vector Y = m_axisY * m_extentY;

			Vector A = Y + m_axisX * m_extentX + m_axisZ * m_extentZ;
			Vector B = Y + m_axisX * m_extentX - m_axisZ * m_extentZ;
			Vector C = Y - m_axisX * m_extentX + m_axisZ * m_extentZ;
			Vector D = Y - m_axisX * m_extentX - m_axisZ * m_extentZ;

			float magA = sqrt( A.x * A.x + A.z * A.z );
			float magB = sqrt( B.x * B.x + B.z * B.z );
			float magC = sqrt( C.x * C.x + C.z * C.z );
			float magD = sqrt( D.x * D.x + D.z * D.z );

			return std::max( std::max(magA,magB), std::max(magC,magD) );
		}
	default:
		return 0.0f;
	}
}

// ----------------------------------------------------------------------

AxialBox MultiShape::getBoundingBox ( void ) const
{
	switch(m_shapeType)
	{
	case MST_Sphere:
	case MST_Cylinder:
	case MST_AxialBox:

		return getAxialBox();

	case MST_YawedBox:

		{
			YawedBox box = getYawedBox();

			AxialBox temp;

			for(int i = 0; i < 8; i++) temp.add(box.getCorner(i));

			return temp;
		}

	case MST_OrientedCylinder:
	case MST_OrientedBox:

		{
			OrientedBox box = getOrientedBox();

			AxialBox temp;

			for(int i = 0; i < 8; i++) temp.add(box.getCorner(i));

			return temp;
		}

	default:
		DEBUG_WARNING(true,("MultiShape::getBoundingBox - Trying to get a bounding box for an unsupported type"));
		return AxialBox();
	}
}

// ----------

Sphere  MultiShape::getBoundingSphere ( void ) const
{
	float radius;
	if (m_baseType == MSBT_Sphere)
	{
		radius = m_extentX;
	}
	else
	{
		radius = sqrt( m_extentX*m_extentX + m_extentY*m_extentY + m_extentZ*m_extentZ );
	}

	return Sphere( m_center, radius );
}

// ----------------------------------------------------------------------

Sphere	MultiShape::getSphere ( void ) const
{
	return Sphere( m_center, m_extentX );
}

// ----------

Cylinder MultiShape::getCylinder ( void ) const
{
	return Cylinder( getBase(), m_extentX, getHeight() );
}

// ----------

OrientedCylinder MultiShape::getOrientedCylinder ( void ) const
{
	return OrientedCylinder( getBase(), m_axisY, m_extentX, getHeight() );
}

// ----------

AxialBox MultiShape::getAxialBox ( void ) const
{
	Vector min = m_center - Vector( m_extentX, m_extentY, m_extentZ );
	Vector max = m_center + Vector( m_extentX, m_extentY, m_extentZ );

	return AxialBox( min, max );
}

// ----------

YawedBox MultiShape::getYawedBox ( void ) const
{
	return YawedBox( getBase(), m_axisX, m_axisZ, m_extentX, m_extentZ, getHeight() );
}

// ----------

OrientedBox MultiShape::getOrientedBox ( void ) const
{
	return OrientedBox( m_center, m_axisX, m_axisY, m_axisZ, m_extentX, m_extentY, m_extentZ );
}

// ----------------------------------------------------------------------

Sphere MultiShape::getLocalSphere ( void ) const
{
	return Sphere( Vector::zero, m_extentX );
}

// ----------

Cylinder MultiShape::getLocalCylinder ( void ) const
{
	return Cylinder( Vector(0.0f,-m_extentY,0.0f), m_extentX, getHeight() );
}

// ----------

AxialBox MultiShape::getLocalAxialBox ( void ) const
{
	Vector min = -Vector( m_extentX, m_extentY, m_extentZ );
	Vector max =  Vector( m_extentX, m_extentY, m_extentZ );

	return AxialBox( min, max );
}

// ----------------------------------------------------------------------

Transform MultiShape::getTransform_l2p ( void ) const
{
	Transform temp(Transform::IF_none);

	temp.setLocalFrameIJK_p( m_axisX, m_axisY, m_axisZ );
	temp.setPosition_p( m_center );

	return temp;
}

Transform MultiShape::getTransform_p2l ( void ) const
{
	Transform temp(Transform::IF_none);

	temp.invert( getTransform_l2p() );

	return temp;
}

// ----------------------------------------------------------------------

