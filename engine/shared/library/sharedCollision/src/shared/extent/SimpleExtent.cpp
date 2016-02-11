// ======================================================================
//
// SimpleExtent.cpp
// Austin Appleby
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/SimpleExtent.h"

#include "sharedCollision/Intersect3d.h"
#include "sharedCollision/Containment3d.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/ShapeUtils.h"
#include "sharedMath/Sphere.h"

const Tag TAG_XSMP = TAG(X,S,M,P);
const Tag TAG_MSHP = TAG(M,S,H,P);

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(SimpleExtent, true, 0, 0, 0);

// ======================================================================

SimpleExtent::SimpleExtent(void)
:	BaseExtent(ET_Simple),
	m_shape()
{
}

SimpleExtent::SimpleExtent( MultiShape const & newShape )
:	BaseExtent(ET_Simple),
	m_shape(newShape)
{
}

SimpleExtent::~SimpleExtent(void)
{
}

// ----------------------------------------------------------------------
// Virtual Extent interface

void SimpleExtent::load ( Iff & iff )
{
	iff.enterForm(TAG_XSMP);

		switch(iff.getCurrentName())
		{
		case TAG_0000:
			load_0000(iff);
			break;

		default:
			FATAL(true,("SimpleExtent::load - unsupported format\n"));
			break;
		}

	iff.exitForm(TAG_XSMP);
}

void SimpleExtent::write ( Iff & iff ) const
{
	iff.insertForm(TAG_XSMP);

		iff.insertForm(TAG_0000);

			writeShape(iff);

		iff.exitForm(TAG_0000);

	iff.exitForm(TAG_XSMP);
}

// ----------------------------------------------------------------------

bool SimpleExtent::contains ( Vector const & vector ) const
{
	return Containment::isContainment( Containment3d::Test(vector,getShape()) );
}

bool SimpleExtent::contains ( Vector const & begin, Vector const & end ) const
{
	return contains(begin) && contains(end);
}

// ----------------------------------------------------------------------

bool SimpleExtent::realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const
{
	Vector direction(end - begin);
	if (direction.normalize())
	{
		Ray3d const ray(begin, direction);
		bool const getSurfaceNormal = (surfaceNormal != 0);
		Intersect3d::ResultData data(getSurfaceNormal);
		if (Intersect3d::intersectRayShapeWithData(ray, getShape(), &data))
		{
			float const magnitude = (end - begin).magnitude();
			if (data.m_length <= magnitude)
			{
				if (surfaceNormal)
					*surfaceNormal = data.m_surfaceNormal;

				if (time)
					*time = data.m_length / magnitude;

				return true;
			}
		}
	}
	return(false);
}

// ----------------------------------------------------------------------

Range SimpleExtent::rangedIntersect ( Line3d const & line ) const
{
	return Intersect3d::Intersect( line, getShape() );
}

Range SimpleExtent::rangedIntersect ( Ray3d const & ray ) const
{
	return Intersect3d::Intersect( ray, getShape() );
}

Range SimpleExtent::rangedIntersect ( Segment3d const & seg ) const
{
	return Intersect3d::Intersect( seg, getShape() );
}

// ----------------------------------------------------------------------

void SimpleExtent::drawDebugShapes ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG

	if(renderer == nullptr) return;

	switch( getShape().getShapeType() )
	{
	case MultiShape::MST_Sphere :
		renderer->setColor( VectorArgb::solidYellow );
		renderer->drawSphere( getShape().getSphere() );
		break;

	case MultiShape::MST_Cylinder :
		renderer->setColor( VectorArgb::solidGreen );
		renderer->drawCylinder( getShape().getCylinder() );
		break;

	case MultiShape::MST_AxialBox :
		renderer->setColor( VectorArgb::solidBlue );
		renderer->drawBox( getShape().getAxialBox() );
		break;

	case MultiShape::MST_OrientedCylinder:
	case MultiShape::MST_YawedBox:
	case MultiShape::MST_OrientedBox:
	case MultiShape::MST_Count:
	case MultiShape::MST_Invalid:
	default:
		break;
	}

#endif
}

BaseExtent * SimpleExtent::clone ( void ) const
{
	return new SimpleExtent(m_shape);
}

void SimpleExtent::copy ( BaseExtent const * source )
{
	SimpleExtent const * simpleSource = safe_cast< SimpleExtent const * >(source);

	setShape( simpleSource->getShape() );
}

void SimpleExtent::transform ( BaseExtent const * source, Transform const & tform, float scale )
{
	SimpleExtent const * simpleSource = safe_cast< SimpleExtent const * >(source);

	setShape( ShapeUtils::transform( simpleSource->getShape(), tform, scale ) );
}

AxialBox SimpleExtent::getBoundingBox ( void ) const
{
	return getShape().getBoundingBox();
}

Sphere SimpleExtent::getBoundingSphere ( void ) const
{
	return getShape().getBoundingSphere();
}

Vector SimpleExtent::getCenter ( void ) const
{
	return getShape().getCenter();
}

float SimpleExtent::getRadius ( void ) const
{
	return getShape().calcAvoidanceRadius();
}

// ----------------------------------------------------------------------

void SimpleExtent::load_0000 ( Iff & iff )
{
	iff.enterForm(TAG_0000);

		loadShape(iff);

	iff.exitForm(TAG_0000);
}

// ----------

void SimpleExtent::loadShape ( Iff & iff )
{
	iff.enterForm(TAG_MSHP);

		MultiShape::BaseType baseType = static_cast<MultiShape::BaseType>(iff.read_int32());
		MultiShape::ShapeType shapeType = static_cast<MultiShape::ShapeType>(iff.read_int32());

		Vector center = iff.read_floatVector();

		Vector axisX = iff.read_floatVector();
		Vector axisY = iff.read_floatVector();
		Vector axisZ = iff.read_floatVector();

		float extentX = iff.read_float();
		float extentY = iff.read_float();
		float extentZ = iff.read_float();

	iff.exitForm(TAG_MSHP);

	m_shape = MultiShape( baseType, shapeType, center, axisX, axisY, axisZ, extentX, extentY, extentZ );
}

// ----------

void SimpleExtent::writeShape ( Iff & iff ) const
{
	iff.insertChunk(TAG_MSHP);

		iff.insertChunkData( getShape().getBaseType() );
		iff.insertChunkData( getShape().getShapeType() );

		iff.insertChunkData( getShape().getCenter() );

		iff.insertChunkData( getShape().getAxisX() );
		iff.insertChunkData( getShape().getAxisY() );
		iff.insertChunkData( getShape().getAxisZ() );

		iff.insertChunkData( getShape().getExtentX() );
		iff.insertChunkData( getShape().getExtentY() );
		iff.insertChunkData( getShape().getExtentZ() );

	iff.exitChunk(TAG_MSHP);
}

// ----------------------------------------------------------------------

