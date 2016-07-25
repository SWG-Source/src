// ======================================================================
//
// Volume.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Volume.h"

#include "sharedMath/Plane.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Vector.h"

// ======================================================================
// Create an open-ended volume
//
// This constructor creates an open ended volume by projecting a ray from originPoint
// through all the vertices in vertexList, and capping the volume off with the plane
// formed by vertexList.  This routine assumes the vertices in vertexList form a
// planar convex polygon and wind in a clockwise order.
//
// @param originPoint      Point from which the volume is cast.
// @param numberOfVertices Number of vertices in vertexList.
// @param vertexList       Co-planar set of points describing the siloutte edges of the volume.

Volume::Volume(const Vector &originPoint, const int numberOfVertices, const Vector * const vertexList)
	:
	m_numberOfPlanes(numberOfVertices + 1),
	m_plane(new Plane[static_cast<uint>(m_numberOfPlanes)])
{
	NOT_NULL(vertexList);
	DEBUG_FATAL(numberOfVertices < 3, ("At least 3 vertices are necessary"));

	// build the cap
	m_plane[0].set(vertexList[0], vertexList[1], vertexList[2]);

	// build the sides
	for (int i = 1; i < numberOfVertices; ++i)
		m_plane[i].set(originPoint, vertexList[i - 1], vertexList[i]);
	m_plane[numberOfVertices].set(originPoint, vertexList[numberOfVertices - 1], vertexList[0]);
}

// ----------------------------------------------------------------------
/**
 * Create an undefined volume
 *
 * This constructor creates a volume consisting of the specified number of planes,
 * but does not set up the plane data.
 *
 * @param numberOfPlanes Number of planes in the volume.
 */

Volume::Volume(const int numberOfPlanes)
	:
	m_numberOfPlanes(numberOfPlanes),
	m_plane(new Plane[static_cast<uint>(m_numberOfPlanes)])
{
}

// ----------------------------------------------------------------------
/**
 * Construct a new volume from another, transformed by the specified transformation.
 *
 * @param other	The other volume
 * @param trans	The transform to be applied.
 */

Volume::Volume(const Volume &rhs, const Transform &newTransform)
	:
	m_numberOfPlanes(rhs.m_numberOfPlanes),
	m_plane(new Plane[static_cast<uint>(m_numberOfPlanes)])
{
	for (int i = 0; i < m_numberOfPlanes; ++i)
		m_plane[i].set(rhs.m_plane[i], newTransform);
}

// ----------------------------------------------------------------------

Volume::~Volume()
{
	delete[] m_plane;
}

// ----------------------------------------------------------------------
/**
 * Set a specific plane in the volume
 *
 * This routine can be used to build up a volume after having used the
 * constructor that takes a number of planes.
 *
 * @param index The plane to modify.
 * @param plane The new value for the plane data.
 */

void Volume::setPlane(const int index, const Plane &plane)
{
	DEBUG_FATAL(index < 0 || index >= m_numberOfPlanes, ("index out of range %d/%d", index, m_numberOfPlanes));
	m_plane[index] = plane;
}

// ----------------------------------------------------------------------
/**
 * Set a specific plane in the volume
 *
 * This routine can be used to build up a volume after having used the
 * constructor that takes a number of planes.
 *
 * @param index The plane to modify.
 * @param plane The new value for the plane data.
 */

const Plane &Volume::getPlane(const int index) const
{
	DEBUG_FATAL(index < 0 || index >= m_numberOfPlanes, ("index out of range %d/%d", index, m_numberOfPlanes));
	return m_plane[index];
}

// ----------------------------------------------------------------------
/**
 * Test a point against a volume
 *
 * @param  point Point to test.
 * @return       True if the point is within the volume.
 */

bool Volume::contains(const Vector &point) const
{
	for (int i = 0; i < m_numberOfPlanes; ++i)
		if (m_plane[i].computeDistanceTo(point) > 0)
			return false;

	return true;
}

// ----------------------------------------------------------------------
/**
 * See if the volume completely contains the sphere
 *
 * @param  sphere The sphere to check.
 * @return        True if the entire sphere is within the volume, otherwise false.
 */

bool Volume::contains(const Sphere &sphere) const
{
	const Vector &center = sphere.getCenter();
	const real    radius = sphere.getRadius();

	for (int i = 0; i < m_numberOfPlanes; ++i)
		if (m_plane[i].computeDistanceTo(center) > -radius)
			return false;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Test a point cloud against a volume
 *
 * @param  pointCloud     Point cloud to test.
 * @param  numberOfPoints Number of points in the point cloud.
 * @return                True if the point is within the volume.
 */

bool Volume::contains(const Vector *pointCloud, int numberOfPoints) const
{
	NOT_NULL(pointCloud);
	DEBUG_FATAL(numberOfPoints <= 0, ("numberOfPoints is less than 1"));

	for (int i = 0; i < m_numberOfPlanes; ++i)
		for (int j = 0; j < numberOfPoints; ++j)
			if (m_plane[i].computeDistanceTo(pointCloud[j]) > 0)
				return false;

	return true;
}

// ----------------------------------------------------------------------
/**
 * See if the volume intersects a sphere.
 *
 * The sphere is in the volume if any portion of it is within the volume.
 *
 * @param  sphere The sphere to check.
 * @return        True if the any portion of the sphere is within the volume, otherwise false.
 */

bool Volume::intersects(const Sphere &sphere) const
{
	const Vector &center = sphere.getCenter();
	const real    radius = sphere.getRadius();

	for (int i = 0; i < m_numberOfPlanes; ++i)
		if (m_plane[i].computeDistanceTo(center) > radius)
			return false;

	return true;
}

// ----------------------------------------------------------------------
/**
 * See if the volume intersects the segment.
 *
 * The segment is considered to be in the volume if both points test on the negative side of any plane
 *
 * @param   start Start point of segment.
 * @param   end   End point of segment.
 * @return        False if any segment is on the negative side of any plane
 */

bool Volume::intersects(Vector const & start, Vector const & end) const
{
	for (int i = 0; i < m_numberOfPlanes; ++i)
	{
		Plane const & plane = m_plane[i];
		if (plane.computeDistanceTo(start) < 0.f && plane.computeDistanceTo(end) < 0.f)
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * Fast Conservative check of a point cloud against the volume
 *
 * This routine will return true if the point could is completely outside any one
 * plane of the volume. If the routine returns true, the point cloud is definitely
 * outside the volume.  However, a false result does not guarentee that the point
 * cloud intersects the volume.
 *
 * @param  pointCloud     Point cloud to test.
 * @param  numberOfPoints Number of points in the point cloud.
 * @return                See remarks for more information.
 */

bool Volume::fastConservativeExcludes(const Vector *pointCloud, int numberOfPoints) const
{
	NOT_NULL(pointCloud);
	DEBUG_FATAL(numberOfPoints <= 0, ("numberOfPoints is less than 1"));

	// check plane by plane
	for (int i = 0; i < m_numberOfPlanes; ++i)
	{
		bool outside = true;
		for (int j = 0; outside && j < numberOfPoints; ++j)
			outside = m_plane[i].computeDistanceTo(pointCloud[j]) > 0;

		if (outside)
			return true;
	}

	return false;
}
// ----------------------------------------------------------------------
/**
 * Transform the volume by the specified transformation.
 *
 * @param transform	The transform to be applied.
 */

void Volume::transform(const Transform &newTransform)
{
	for (int i = 0; i < m_numberOfPlanes; ++i)
		m_plane[i].transform(newTransform);
}

// ----------------------------------------------------------------------
/**
 * Transform the specified volume by the specified transformation.
 *
 * @param source    The source volume to transform.
 * @param transform	The transform to be applied.
 */

void Volume::transform(const Volume &source, const Transform &newTransform)
{
	if (m_numberOfPlanes != source.m_numberOfPlanes)
	{
		delete[] m_plane;
		m_numberOfPlanes = source.m_numberOfPlanes;
		m_plane = new Plane[static_cast<uint>(m_numberOfPlanes)];
	}

	for (int i = 0; i < m_numberOfPlanes; ++i)
		m_plane[i].set(source.m_plane[i], newTransform);
}

// ======================================================================