// ======================================================================
//
// OrientedCylinderExtent.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/OrientedCylinderExtent.h"

#include "sharedCollision/ExtentList.h"
#include "sharedCollision/Containment3d.h"
#include "sharedFile/Iff.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/ShapeUtils.h"

const Tag TAG_XOCL = TAG(X,O,C,L);
const Tag TAG_OCYL = TAG(O,C,Y,L);

// ======================================================================

void OrientedCylinderExtent::install ( void )
{
	ExtentList::assignBinding(TAG_XOCL,OrientedCylinderExtent::build);
}

void OrientedCylinderExtent::remove ( void )
{
}

Extent *     OrientedCylinderExtent::build       ( Iff & iff )
{
	OrientedCylinderExtent * extent = new OrientedCylinderExtent();

	extent->load(iff);

	return extent;
}

// ----------------------------------------------------------------------

OrientedCylinderExtent::OrientedCylinderExtent()
: Extent(ET_Cylinder),
  m_cylinder()
{
	DEBUG_WARNING(true, ("OrientedCylinderExtent::OrientedCylinderExtent: this class does not implement the necessary intersect functions."));
}

OrientedCylinderExtent::OrientedCylinderExtent( OrientedCylinder const & cylinder )
: Extent(ET_Cylinder),
  m_cylinder(cylinder)
{
	DEBUG_WARNING(true, ("OrientedCylinderExtent::OrientedCylinderExtent: this class does not implement the necessary intersect functions."));
}

OrientedCylinderExtent::~OrientedCylinderExtent()
{
}

// ----------------------------------------------------------------------
//@todo - Implement these!

bool OrientedCylinderExtent::contains ( Vector const & point ) const
{
	UNREF(point);

	return false;
}

bool OrientedCylinderExtent::contains ( Vector const & begin, Vector const & end ) const
{
	UNREF(begin);
	UNREF(end);

	return false;
}

bool OrientedCylinderExtent::realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const
{
	UNREF(begin);
	UNREF(end);
	UNREF(surfaceNormal);
	UNREF(time);
	return false;
}

// ----------------------------------------------------------------------

OrientedCylinder const & OrientedCylinderExtent::getCylinder ( void ) const
{
	return m_cylinder;
}

// ----------

void OrientedCylinderExtent::setCylinder ( OrientedCylinder const & newCylinder )
{
	m_cylinder = newCylinder;
}

// ----------------------------------------------------------------------

void OrientedCylinderExtent::drawDebugShapes ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG

	if(renderer == nullptr) return;

	renderer->setColor( VectorArgb::solidGreen );
	renderer->draw(m_cylinder);

#endif
}

// ----------------------------------------------------------------------

void OrientedCylinderExtent::load ( Iff & iff ) 
{
	iff.enterForm(TAG_XOCL);

		switch (iff.getCurrentName())
		{
		case TAG_0000:
			load_0000(iff);
			break;

 		default:
			FATAL (true, ("OrientedCylinderExtent::load - unsupported format"));
			break;
		}

	iff.exitForm(TAG_XOCL);
	
}

// ----------

void OrientedCylinderExtent::write ( Iff & iff ) const
{
	iff.insertForm(TAG_XOCL);

		iff.insertForm(TAG_0000);
			
			writeCylinder(iff);

		iff.exitForm(TAG_0000);

	iff.exitForm(TAG_XOCL);
}

// ----------------------------------------------------------------------

void OrientedCylinderExtent::loadCylinder ( Iff & iff )
{
	iff.enterChunk(TAG_OCYL);

		Vector base = iff.read_floatVector();
		Vector axis = iff.read_floatVector();
		real radius = iff.read_float();
		real height = iff.read_float();

		m_cylinder = OrientedCylinder(base,axis,radius,height);

	iff.exitChunk(TAG_OCYL);
}

// ----------------------------------------------------------------------

void OrientedCylinderExtent::load_0000 ( Iff & iff )
{
	iff.enterForm(TAG_0000);
		
		loadCylinder(iff);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void OrientedCylinderExtent::writeCylinder ( Iff & iff ) const
{
	iff.insertChunk(TAG_OCYL);

		iff.insertChunkFloatVector( m_cylinder.getBase() );
		iff.insertChunkFloatVector( m_cylinder.getAxis() );
		iff.insertChunkData( m_cylinder.getRadius() );
		iff.insertChunkData( m_cylinder.getHeight() );

	iff.exitChunk(TAG_OCYL);
}

// ----------------------------------------------------------------------

BaseExtent * OrientedCylinderExtent::clone ( void ) const
{
    return new OrientedCylinderExtent( getCylinder() );
}

// ----------

void OrientedCylinderExtent::copy ( BaseExtent const * source )
{
    if(!source) return;

    OrientedCylinderExtent const * cylinderSource = safe_cast<OrientedCylinderExtent const *>(source);

    setCylinder( cylinderSource->getCylinder() );
}

// ----------

void OrientedCylinderExtent::transform ( BaseExtent const * source, Transform const & tform, float scale )
{
    if(!source) return;

    OrientedCylinderExtent const * cylinderSource = safe_cast<OrientedCylinderExtent const *>(source);

    setCylinder( ShapeUtils::transform( cylinderSource->getCylinder(), tform, scale ) );
}

// ----------

Sphere OrientedCylinderExtent::getBoundingSphere ( void ) const
{
    return Containment3d::EncloseSphere( m_cylinder );
}

// ----------------------------------------------------------------------
