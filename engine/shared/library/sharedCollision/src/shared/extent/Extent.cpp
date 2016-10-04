// ======================================================================
//
// Extent.cpp
// jeff grills
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Extent.h"

#include "sharedCollision/ExtentList.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedCollision/Containment3d.h"
#include "sharedFile/Iff.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/Range.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/ShapeUtils.h"
#include "sharedMath/AxialBox.h"

//-----------------------------------------------------------------

static const Tag TAG_EXSP = TAG (E,X,S,P);
static const Tag TAG_CNTR = TAG (C,N,T,R);
static const Tag TAG_RADI = TAG (R,A,D,I);
static const Tag TAG_SPHR = TAG (S,P,H,R);

// ======================================================================

void Extent::install ( void )
{
	ExtentList::assignBinding(TAG_EXSP, Extent::create);
}

void Extent::remove ( void )
{
}

// ----------------------------------------------------------------------

Extent *Extent::create(Iff & iff)
{
	Extent * extent = new Extent();

	extent->load(iff);

	return extent;
}

// ======================================================================
// Create an extent

Extent::Extent(void)
: BaseExtent(ET_Sphere),
  m_sphere(),
  m_referenceCount(0)
{
}

Extent::Extent( ExtentType const & type )
: BaseExtent(type),
  m_sphere(),
  m_referenceCount(0)
{
}

Extent::Extent(const Sphere &newSphere)
: BaseExtent(ET_Sphere),
  m_sphere(newSphere),
  m_referenceCount(0)
{
}

Extent::Extent( ExtentType const & type, Sphere const & sphere )
: BaseExtent(type),
  m_sphere(sphere),
  m_referenceCount(0)
{
}

Extent::Extent(const Vector &newSphereCenter, real newSphereRadius)
: BaseExtent(ET_Sphere),
  m_sphere(newSphereCenter, newSphereRadius),
  m_referenceCount(0)
{
}

// ----------------------------------------------------------------------
/**
 * Destroy an extent.
 */

Extent::~Extent(void)
{
	DEBUG_FATAL(m_referenceCount > 0, ("referenceCount not zero %d", m_referenceCount));
}

// ----------------------------------------------------------------------

int Extent::getReferenceCount ( void ) const
{
	return m_referenceCount;
}

// ----------

int Extent::incrementReference ( void ) const
{
	m_referenceCount++;

	return m_referenceCount;
}

// ----------

int Extent::decrementReference ( void ) const
{
	m_referenceCount--;

	return m_referenceCount;
}

// ----------------------------------------------------------------------

void Extent::loadSphere_old (Iff & iff )
{
	iff.enterChunk(TAG_CNTR);
		const Vector center = iff.read_floatVector();
	iff.exitChunk(TAG_CNTR);

	iff.enterChunk(TAG_RADI);
		const real  radius = iff.read_float();
	iff.exitChunk(TAG_RADI);

	setSphere(Sphere(center, radius));
}

// ----------------------------------------------------------------------

void Extent::loadSphere( Iff & iff )
{
	iff.enterChunk(TAG_SPHR);

		Vector center = iff.read_floatVector();
		real radius = iff.read_float();

		setSphere( Sphere(center,radius) );

	iff.exitChunk(TAG_SPHR);
}

// ----------------------------------------------------------------------

void Extent::load_0000(Iff & iff)
{
	iff.enterForm(TAG_0000);

		loadSphere_old(iff);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void Extent::load_0001(Iff & iff)
{
	iff.enterForm(TAG_0001);

		loadSphere(iff);

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void Extent::load(Iff & iff)
{
	iff.enterForm(TAG_EXSP);

		switch (iff.getCurrentName())
		{
		case TAG_0000:
			load_0000(iff);
			break;

		case TAG_0001:
			load_0001(iff);
			break;

 		default:
			FATAL (true, ("Extent::load - unsupported format"));
			break;
		}

	iff.exitForm(TAG_EXSP);
}

//-----------------------------------------------------------------
/**
 * Write a complete Extent object.
 */

void Extent::write(Iff & iff) const
{
	iff.insertForm(TAG_EXSP);

		iff.insertForm(TAG_0001);

			writeSphere(iff);

		iff.exitForm(TAG_0001);

	iff.exitForm(TAG_EXSP);
}

// ----------------------------------------------------------------------

void Extent::writeSphere( Iff & iff ) const
{
	iff.insertChunk(TAG_SPHR);

		iff.insertChunkFloatVector( m_sphere.getCenter() );
		iff.insertChunkData( m_sphere.getRadius() );

	iff.exitChunk(TAG_SPHR);
}

// ----------------------------------------------------------------------
/**
 * Test a line segment against the Extent's bounding sphere.
 *
 * If the begining point is contained within the sphere, this routine will return true,
 * and the parametric time will be 0.
 *
 * @param begin  Beginning of the line segment
 * @param end  End of the line segment
 * @param time  [OUT] Parametric time of intersection along the line segment
 * @return True if the any part of the line segment is inside the line segment, otherwise false
 */

bool Extent::testSphereOnly(Vector const & begin, Vector const & end, Vector * surfaceNormal, float * time) const
{
	Vector direction(end - begin);
	if (!direction.normalize())
	{
		// handle the line segment being very short to avoid a divide-by-zero
		if (testSphereOnly(begin))
		{
			if (surfaceNormal)
			{
				Vector normal(begin - m_sphere.getCenter());
				if (!normal.normalize())
					normal = -direction;

				*surfaceNormal = normal;
			}

			if (time)
				*time = 0.f;

			return true;
		}
		
		if (testSphereOnly(end))
		{
			if (surfaceNormal)
			{
				Vector normal(begin - m_sphere.getCenter());
				if (!normal.normalize())
					normal = -direction;

				*surfaceNormal = normal;
			}

			if (time)
				*time = 1.f;

			return true;
		}

		return false;
	}

	Ray3d const ray(begin, direction);
	bool const getSurfaceNormal = (surfaceNormal != 0);
	Intersect3d::ResultData data(getSurfaceNormal);
	if (Intersect3d::intersectRaySphereWithData(ray, m_sphere, &data))
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

	return false;
}

// ----------------------------------------------------------------------
/**
 * Determine if a point is inside the Extent.
 *
 * This routine will take care of trivial rejection itself, so do not
 * call the testSphereOnly() routine before calling this routine.
 *
 * @param vector  Point to test for inclusion in the Extent
 */

bool Extent::contains(const Vector &vector) const
{
	return testSphereOnly(vector);
}

// ----------------------------------------------------------------------
/**
 * Determine if a line segment is completely contained within the Extent.
 *
 * This routine will take care of trivial rejection itself, so do not
 * call the testSphereOnly() routine before calling this routine.
 *
 * The default implementation of this routine is valid for all convex shapes.
 *
 * @param begin  Beginning of the line segment
 * @param end  End of the line segment
 */

bool Extent::contains(const Vector &begin, const Vector &end) const
{
	return contains(begin) && contains(end);
}

// ----------------------------------------------------------------------

bool Extent::realIntersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, float * time ) const
{
	return testSphereOnly(begin, end, surfaceNormal, time);
}

// ----------------------------------------------------------------------

Range Extent::rangedIntersect ( Line3d const & line ) const
{
	return Intersect3d::Intersect(line,m_sphere);
}

Range Extent::rangedIntersect ( Ray3d const & ray ) const
{
	return Intersect3d::Intersect(ray,m_sphere);
}

Range Extent::rangedIntersect ( Segment3d const & seg ) const
{
	return Intersect3d::Intersect(seg,m_sphere);
}

// ----------------------------------------------------------------------

void Extent::drawDebugShapes ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG

	if(renderer == nullptr) return;

	renderer->setColor( VectorArgb::solidYellow );
	renderer->drawSphere(m_sphere);

#endif
}

// ----------------------------------------------------------------------

BaseExtent * Extent::clone ( void ) const
{
    return new Extent( getSphere() );
}

// ----------

void Extent::copy ( BaseExtent const * source )
{
    if(!source) return;

    Extent const * sphereSource = safe_cast< Extent const * >(source);

    setSphere( sphereSource->getSphere() );
}

// ----------

void Extent::transform ( BaseExtent const * source, Transform const & tform, float scale )
{
    if(!source) return;

    Extent const * sphereSource = safe_cast< Extent const * >(source);

    setSphere( ShapeUtils::transform( sphereSource->getSphere(), tform, scale ) );
}

// ----------

AxialBox Extent::getBoundingBox ( void ) const
{
	return Containment3d::EncloseABox(getSphere());
}

// ----------

Sphere Extent::getBoundingSphere ( void ) const
{
    return getSphere();
}

// ======================================================================
