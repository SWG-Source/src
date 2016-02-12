// ======================================================================
//
// BoxExtent.cpp
// jeff grills
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/BoxExtent.h"

#include "sharedCollision/ExtentList.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/Range.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/ShapeUtils.h"
#include "sharedFoundation/MemoryBlockManager.h"

#include <algorithm>

//-----------------------------------------------------------------

static const Tag TAG_BOX  = TAG3 (B,O,X);
static const Tag TAG_EXBX = TAG (E,X,B,X);

namespace
{
	MemoryBlockManager* ms_memoryBlockManager;
}

//-------------------------------------------------------------------

void BoxExtent::install (void)
{
	DEBUG_FATAL(ms_memoryBlockManager, ("BoxExtent already installed"));

	ms_memoryBlockManager = new MemoryBlockManager("BoxExtent::memoryBlockManager", true, sizeof(BoxExtent), 0, 0, 0);

	ExtentList::assignBinding(TAG_EXBX,BoxExtent::create);

	ExitChain::add(BoxExtent::remove,"BoxExtent::remove");
}

//-------------------------------------------------------------------

void BoxExtent::remove ()
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("BoxExtent is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------
// The terrain system creates BoxExtents for its chunks, which means
// that many thousands of BoxExtents are created and destroyed during
// a run of the game. Because of this, making BoxExtent use a block
// manager is probably a good idea.

void *BoxExtent::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(BoxExtent), ("bad size"));
	DEBUG_FATAL(size != static_cast<size_t>(ms_memoryBlockManager->getElementSize()), ("BoxExtent installed with bad size"));

	return ms_memoryBlockManager->allocate();
}

// ----------

void BoxExtent::operator delete(void* pointer)
{
	NOT_NULL(ms_memoryBlockManager);

	ms_memoryBlockManager->free(pointer);
}

// ======================================================================
// Create a BoxExtent from the file

Extent *BoxExtent::create( Iff & iff )
{
	BoxExtent * extent = new BoxExtent();

	extent->load(iff);

	return extent;
}

// ----------------------------------------------------------------------
/**
 * Create a BoxExtent.
 */

BoxExtent::BoxExtent(void)
: Extent(ET_Box),
  m_box()
{
}

// ----------

BoxExtent::BoxExtent( AxialBox const & newBox )
: Extent(ET_Box),
  m_box(newBox)
{
}

// ----------

BoxExtent::BoxExtent(const Vector &vmin, const Vector &vmax, const Sphere & s)
: Extent(ET_Box,s),
  m_box(vmin,vmax)
{
}

// ----------------------------------------------------------------------
/**
 * Destroy a BoxExtent.
 */

BoxExtent::~BoxExtent(void)
{
}

// ----------------------------------------------------------------------

void BoxExtent::loadBox ( Iff & iff )
{
	iff.enterChunk(TAG_BOX);

		Vector max = iff.read_floatVector();
		Vector min = iff.read_floatVector();

		if (max.x < min.x || max.y < min.y || max.z < min.z)
		{
			DEBUG_WARNING(true, ("box extent is inside out"));
			min = -Vector::xyz111;
			max = Vector::xyz111;
			m_sphere.setCenter(Vector::zero);
			m_sphere.setRadius(Vector::xyz111.magnitude());
		}

		m_box = AxialBox(min,max);

	iff.exitChunk(TAG_BOX);
}

// ----------------------------------------------------------------------

void BoxExtent::load_0000(Iff & iff)
{
	iff.enterForm(TAG_0000);

		loadBox(iff);
		Extent::loadSphere_old(iff);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void BoxExtent::load_0001(Iff & iff)
{
	iff.enterForm(TAG_0001);

		Extent::load(iff);
		loadBox(iff);

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void BoxExtent::load(Iff & iff)
{
	iff.enterForm(TAG_EXBX);

		switch (iff.getCurrentName())
		{
		case TAG_0000:
			load_0000(iff);
			break;

		case TAG_0001:
			load_0001(iff);
			break;

 		default:
			FATAL (true, ("BoxExtent::load - unsupported format"));
			break;
		}

	iff.exitForm(TAG_EXBX);
}

//-----------------------------------------------------------------

void BoxExtent::writeBox( Iff & iff ) const
{
	// insert bounding box extent
	iff.insertChunk(TAG_BOX);

		iff.insertChunkFloatVector( m_box.getMax() );
		iff.insertChunkFloatVector( m_box.getMin() );

	iff.exitChunk(TAG_BOX);
}

// ----------------------------------------------------------------------

void BoxExtent::write(Iff & iff) const
{
	iff.insertForm(TAG_EXBX);

		iff.insertForm(TAG_0001);

			Extent::write(iff);

			writeBox(iff);

		iff.exitForm(TAG_0001);

	iff.exitForm(TAG_EXBX);
}

// ----------------------------------------------------------------------

void BoxExtent::setNegative ()
{
	setMin (Vector::maxXYZ);
	setMax (Vector::negativeMaxXYZ);
	setSphere (Sphere (Vector::zero, 0.f));
}

// ----------------------------------------------------------------------
/**
 * Update members if vector is beyond the current box extent.
 */

void BoxExtent::updateMinAndMax (const Vector &vector)
{
	m_box.add(vector);
}

// ----------------------------------------------------------------------
/**
 * Recompute the sphere extent around this box extent.
 *
 * Based on the current values of the extents, calculate the enclosing
 * sphere's center and radius.
 */

void BoxExtent::calculateCenterAndRadius (void)
{
	Vector center = m_box.getCenter();
	const real radius = m_box.getRadius();

	setSphere(Sphere(center, radius));
}

// ----------------------------------------------------------------------
/**
 * Check if a point is contained within the box extent.
 *
 * This routine will take care of trivial rejection itself, so do not
 * call the testSphereOnly() routine before calling this routine.
 *
 * @param vector  Point to test to see if it is within the extent
 * @return True if the point is inside the extent, otherwise false.
 */

bool BoxExtent::contains(const Vector &vector) const
{
	return testSphereOnly(vector) && m_box.contains(vector);
}

// Boxes are convex volumes; a line segment is contained in a box if both of its endpoints are
// contained by the box.

bool BoxExtent::contains(const Vector &begin, const Vector &end) const
{
	return contains(begin) && contains(end);
}

// ----------------------------------------------------------------------
/**
 * Determine if a line segment intersects the BoxExtent.
 *
 * This routine will take care of trivial rejection itself, so do not
 * call the testSphereOnly() routine before calling this routine.
 *
 * @param begin  Beginning of the line segment
 * @param end  End of the line segment
 * @param time  [OUT] Parametric time of the intersection
 * @return True if the line segment intersects the Extent volume, otherwise false
 */

bool BoxExtent::realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, real * time) const
{
	Vector direction(end - begin);
	if (direction.normalize())
	{
		Ray3d const ray(begin, direction);
		bool const getSurfaceNormal = (surfaceNormal != 0);
		Intersect3d::ResultData data(getSurfaceNormal);
		if (Intersect3d::intersectRayABoxWithData(ray, m_box, &data))
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
/**
 * Determine if a line segment intersects the BoxExtent.
 *
 * This routine will take care of trivial rejection itself, so do not
 * call the testSphereOnly() routine before calling this routine.
 *
 * @param begin  Beginning of the line segment
 * @param end  End of the line segment
 * @return True if the line segment intersects the Extent volume, otherwise false
 */

Range BoxExtent::rangedIntersect ( Line3d const & line ) const
{
	return Intersect3d::Intersect(line,m_box);
}

Range BoxExtent::rangedIntersect ( Ray3d const & ray ) const
{
	return Intersect3d::Intersect(ray,m_box);
}

Range BoxExtent::rangedIntersect ( Segment3d const & seg ) const
{
	return Intersect3d::Intersect(seg,m_box);
}

/** ----------------------------------------------------------------------
*
* Causes a BoxExtent to grow large enough to contain the other.
* @param other The BoxExtent which we must grow to contain
*/

void BoxExtent::grow( const BoxExtent & other)
{
	m_box.add(other.m_box);

	calculateCenterAndRadius ();
}

//----------------------------------------------------------------------

void BoxExtent::grow (const Vector & v, bool recompute)
{
	m_box.add (v);
	if (recompute)
		calculateCenterAndRadius ();
}

// ----------------------------------------------------------------------

void BoxExtent::setBox( AxialBox const & box )
{
	m_box = box;

	calculateCenterAndRadius();
}

// ----------------------------------------------------------------------

void BoxExtent::drawDebugShapes ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG

	if(renderer == nullptr) return;

	renderer->setColor( VectorArgb::solidBlue );
	renderer->drawBox(m_box);

#endif
}

// ----------------------------------------------------------------------

real BoxExtent::getRight  ( void ) const { return m_box.getMax().x; }
real BoxExtent::getTop    ( void ) const { return m_box.getMax().y; }
real BoxExtent::getFront  ( void ) const { return m_box.getMax().z; }
real BoxExtent::getLeft   ( void ) const { return m_box.getMin().x; }
real BoxExtent::getBottom ( void ) const { return m_box.getMin().y; }
real BoxExtent::getBack   ( void ) const { return m_box.getMin().z; }

// ----------------------------------------------------------------------

BaseExtent * BoxExtent::clone ( void ) const
{
    return new BoxExtent( getBox() );
}

// ----------

void BoxExtent::copy ( BaseExtent const * source )
{
    if(!source) return;

    BoxExtent const * boxSource = safe_cast< BoxExtent const * >(source);

    setBox( boxSource->getBox() );
}

// ----------

void BoxExtent::transform ( BaseExtent const * source, Transform const & tform, float scale )
{
    if(!source) return;

    BoxExtent const * boxSource = safe_cast< BoxExtent const * >(source);

    setBox( ShapeUtils::transform( boxSource->getBox(), tform, scale ) );
}

// ----------

AxialBox BoxExtent::getBoundingBox ( void ) const
{
	return m_box;
}

// ----------

Sphere BoxExtent::getBoundingSphere ( void ) const
{
    return getSphere();
}

// ======================================================================

