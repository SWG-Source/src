// ======================================================================
//
// CylinderExtent.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/CylinderExtent.h"

#include "sharedCollision/ExtentList.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Intersect1d.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedCollision/Intersect2d.h"
#include "sharedFile/Iff.h"
#include "sharedMath/Circle.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Plane3d.h"
#include "sharedMath/Plane.h"
#include "sharedMath/Range.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/ShapeUtils.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Segment3d.h"

const Tag TAG_XCYL = TAG(X,C,Y,L);
const Tag TAG_CYLN = TAG(C,Y,L,N);

// ======================================================================

void CylinderExtent::install ( void )
{
	ExtentList::assignBinding(TAG_XCYL,CylinderExtent::build);
}

void CylinderExtent::remove ( void )
{
}

Extent *     CylinderExtent::build       ( Iff & iff )
{
	CylinderExtent * extent = new CylinderExtent();

	extent->load(iff);

	return extent;
}

// ----------------------------------------------------------------------

CylinderExtent::CylinderExtent()
: Extent(ET_Cylinder),
  m_cylinder()
{
}

CylinderExtent::CylinderExtent( Cylinder const & cylinder )
: Extent(ET_Cylinder),
  m_cylinder(cylinder)
{
}

CylinderExtent::~CylinderExtent()
{
}

// ----------------------------------------------------------------------

bool CylinderExtent::contains ( Vector const & point ) const
{
	return Containment::isContainment( Containment3d::Test(point,m_cylinder) );
}

bool CylinderExtent::contains ( Vector const & begin, Vector const & end ) const
{
	return contains(begin) && contains(end);
}

// ----------------------------------------------------------------------

bool CylinderExtent::realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, float * time ) const
{
	Vector direction(end - begin);
	if (direction.normalize())
	{
		Ray3d const ray(begin, direction);
		bool const getSurfaceNormal = (surfaceNormal != 0);
		Intersect3d::ResultData data(getSurfaceNormal);
		if (Intersect3d::intersectRayCylinderWithData(ray, m_cylinder, &data))
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

	return false;
}

// ----------------------------------------------------------------------

Range CylinderExtent::rangedIntersect ( Line3d const & line ) const
{
	return Intersect3d::Intersect(line,m_cylinder);
}

Range CylinderExtent::rangedIntersect ( Ray3d const & ray ) const
{
	return Intersect3d::Intersect(ray,m_cylinder);
}

Range CylinderExtent::rangedIntersect ( Segment3d const & seg ) const
{
	return Intersect3d::Intersect(seg,m_cylinder);
}

// ----------------------------------------------------------------------

Cylinder const &    CylinderExtent::getCylinder ( void ) const
{
	return m_cylinder;
}

// ----------

void                CylinderExtent::setCylinder ( Cylinder const & newCylinder )
{
	m_cylinder = newCylinder;
}

// ----------------------------------------------------------------------

void CylinderExtent::drawDebugShapes ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG

	if(renderer == nullptr) return;

	renderer->setColor( VectorArgb::solidGreen );
	renderer->drawCylinder(m_cylinder);

#endif
}

// ----------------------------------------------------------------------

void CylinderExtent::load ( Iff & iff )
{
	iff.enterForm(TAG_XCYL);

		switch (iff.getCurrentName())
		{
		case TAG_0000:
			load_0000(iff);
			break;

 		default:
			FATAL (true, ("CylinderExtent::load - unsupported format"));
			break;
		}

	iff.exitForm(TAG_XCYL);

}

// ----------

void CylinderExtent::write ( Iff & iff ) const
{
	iff.insertForm(TAG_XCYL);

		iff.insertForm(TAG_0000);

			writeCylinder(iff);

		iff.exitForm(TAG_0000);

	iff.exitForm(TAG_XCYL);
}

// ----------------------------------------------------------------------

void CylinderExtent::loadCylinder ( Iff & iff )
{
	iff.enterChunk(TAG_CYLN);

		Vector base = iff.read_floatVector();
		real radius = iff.read_float();
		real height = iff.read_float();

		m_cylinder = Cylinder(base,radius,height);

	iff.exitChunk(TAG_CYLN);
}

// ----------------------------------------------------------------------

void CylinderExtent::load_0000 ( Iff & iff )
{
	iff.enterForm(TAG_0000);

		loadCylinder(iff);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void CylinderExtent::writeCylinder ( Iff & iff ) const
{
	iff.insertChunk(TAG_CYLN);

		iff.insertChunkFloatVector( m_cylinder.getBase() );
		iff.insertChunkData( m_cylinder.getRadius() );
		iff.insertChunkData( m_cylinder.getHeight() );

	iff.exitChunk(TAG_CYLN);
}

// ----------------------------------------------------------------------

BaseExtent * CylinderExtent::clone ( void ) const
{
    return new CylinderExtent( getCylinder() );
}

// ----------

void CylinderExtent::copy ( BaseExtent const * source )
{
    if(!source) return;

    CylinderExtent const * cylinderSource = safe_cast<CylinderExtent const *>(source);

    setCylinder( cylinderSource->getCylinder() );
}

// ----------

void CylinderExtent::transform ( BaseExtent const * source, Transform const & tform, float scale )
{
    if(!source) return;

    CylinderExtent const * cylinderSource = safe_cast<CylinderExtent const *>(source);

    setCylinder( ShapeUtils::transform( cylinderSource->getCylinder(), tform, scale ) );
}

// ----------

AxialBox CylinderExtent::getBoundingBox ( void ) const
{
	return Containment3d::EncloseABox( m_cylinder );
}

// ----------

Sphere CylinderExtent::getBoundingSphere ( void ) const
{
    return Containment3d::EncloseSphere( m_cylinder );
}

// ----------------------------------------------------------------------
