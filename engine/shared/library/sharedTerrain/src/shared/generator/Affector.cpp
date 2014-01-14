//===================================================================
//
// Affector.cpp
// asommers 2001-01-17
//
// copyright 2001, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/Affector.h"

#include "sharedMath/Line2d.h"

#include <algorithm>

//===================================================================

namespace
{
	inline const float computeT (const Vector2d& start, const Vector2d& end, const Vector2d& point)
	{
		return ((point.x - start.x) * (end.x - start.x) + (point.y - start.y) * (end.y - start.y)) / (sqr (end.x - start.x) + sqr (end.y - start.y));
	}

	template<class T>
	inline const T min (const T& a, const T& b)
	{
		return a < b ? a : b;
	}
}

//===================================================================

void AffectorBoundaryPoly::FindData::reset ()
{
	height           = 0.f;
	distanceToCenter = 0.f;
	t                = 0.f;
	length           = 0.f;
}

//===================================================================

bool AffectorBoundaryPoly::ms_enabled = true;

//===================================================================

AffectorBoundaryPoly::AffectorBoundaryPoly (const Tag newTag, const TerrainGeneratorAffectorType newType) :
	TerrainGenerator::Affector (newTag, newType),
	m_featherFunction (TGFF_linear),
	m_featherDistance (0.5f),
	m_pointList (),
	m_lengths (),
	m_lengthTotals (),
	m_width (4.f),
	m_extent (),
	m_heightData ()
{
}

//-------------------------------------------------------------------

AffectorBoundaryPoly::~AffectorBoundaryPoly ()
{
}

//-------------------------------------------------------------------

float AffectorBoundaryPoly::isWithin (const float worldX, const float worldZ) const
{
	if (!m_extent.isWithin (worldX, worldZ))
		return 0.f;

	const float widthSquared = sqr (m_width);

	float distanceSquared = widthSquared;

	//-- first, scan how far we are from the points
	{
		const int n = m_pointList.getNumberOfElements ();
		int i;
		for (i = 0; i < n; ++i)
		{
			const float x = m_pointList [i].x;
			const float y = m_pointList [i].y;
			const float thisDistanceSquared = sqr (worldX - x) + sqr (worldZ - y);
			if (thisDistanceSquared < distanceSquared)
				distanceSquared = thisDistanceSquared;
		}
	}

	//-- next, scan each line
	{
		const int n = m_pointList.getNumberOfElements () - 1;
		int i;
		for (i = 0; i < n; ++i)
		{
			const float x1 = m_pointList [i].x;
			const float y1 = m_pointList [i].y;  //lint !e578  //-- hides y1 (double)
			const float x2 = m_pointList [i + 1].x;
			const float y2 = m_pointList [i + 1].y;

			const float u = ((worldX - x1) * (x2 - x1) + (worldZ - y1) * (y2 - y1)) / (sqr (x2 - x1) + sqr (y2 - y1));
			if (u >= 0 && u <= 1)
			{
				const float x = x1 + u * (x2 - x1);
				const float y = y1 + u * (y2 - y1);
				const float thisDistanceSquared = sqr (worldX - x) + sqr (worldZ - y);
				if (thisDistanceSquared < distanceSquared)
					distanceSquared = thisDistanceSquared;
			}
		}
	}

	if (distanceSquared < widthSquared)
	{
		const float newFeatherDistance        = m_width * (1.f - getFeatherDistance ());
		const float newFeatherDistanceSquared = sqr (newFeatherDistance);

		if (distanceSquared < newFeatherDistanceSquared)
			return 1.f;

		return 1.f - (sqrt (distanceSquared) - newFeatherDistance) / (m_width - newFeatherDistance);
	}

	return 0.f;
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::addPoint (const Vector2d& point)
{
	m_pointList.add (point);

	recalculate ();
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::replacePoint (const int index, const Vector2d& newPoint)
{
	m_pointList [index] = newPoint;

	recalculate ();
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::removePoint (const int index)
{
	m_pointList.removeIndexAndCompactList (index);

	recalculate ();
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::clearPointList ()
{
	m_pointList.clear ();
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::copyPointList (const ArrayList<Vector2d>& newPointList)
{
	m_pointList = newPointList;

	recalculate ();
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::recalculate ()
{
	//
	// recalculate extent
	//
	{
		m_extent.x0 = FLT_MAX;
		m_extent.y0 = FLT_MAX;
		m_extent.x1 = -FLT_MAX;
		m_extent.y1 = -FLT_MAX;

		int i;
		for (i = 0; i < m_pointList.getNumberOfElements (); ++i)
			m_extent.expand (m_pointList [i].x, m_pointList [i].y);

		m_extent.x0 -= m_width;
		m_extent.y0 -= m_width;
		m_extent.x1 += m_width;
		m_extent.y1 += m_width;
	}

	//
	// recalculate lengths
	//
	{
		m_lengths.clear ();
		m_lengthTotals.clear ();

		m_lengths.push_back (0.f);
		m_lengthTotals.push_back (0.f);

		int i;
		for (i = 1; i < m_pointList.getNumberOfElements (); ++i)
		{
			m_lengths.push_back (m_pointList [i].magnitudeBetween (m_pointList [i - 1]));
			m_lengthTotals.push_back (m_lengths [i] + m_lengthTotals [i - 1]);
		}
	}
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::rotate (const float angle)
{
	if (m_pointList.getNumberOfElements () == 0)
		return;

	Vector2d center;

	int i;
	for (i = 0; i < m_pointList.getNumberOfElements (); i++)
		center += m_pointList [i];

	center.x /= static_cast<float> (m_pointList.getNumberOfElements ());
	center.y /= static_cast<float> (m_pointList.getNumberOfElements ());

	rotate (angle, center);
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::rotate (const float angle, const Vector2d& center)
{
	if (m_pointList.getNumberOfElements () == 0)
		return;

	const float cosAngle = cos (angle);
	const float sinAngle = sin (angle);

	int i;
	for (i = 0; i < m_pointList.getNumberOfElements (); i++)
	{
		Vector2d point;
		point.x = m_pointList [i].x - center.x;
		point.y = m_pointList [i].y - center.y;

		Vector2d newPoint;
		newPoint.x = center.x + point.x * cosAngle - point.y * sinAngle;
		newPoint.y = center.y + point.x * sinAngle + point.y * cosAngle;

		m_pointList [i] = newPoint;
	}
	
	recalculate ();
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::translate (const Vector2d& translation)
{
	int i;
	for (i = 0; i < m_pointList.getNumberOfElements (); i++)
		m_pointList [i] += translation;
	
	recalculate ();
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::scale (const float scalar)
{
	if (m_pointList.getNumberOfElements () == 0)
		return;

	Vector2d center;

	int i;
	for (i = 0; i < m_pointList.getNumberOfElements (); i++)
		center += m_pointList [i];

	center.x /= static_cast<float> (m_pointList.getNumberOfElements ());
	center.y /= static_cast<float> (m_pointList.getNumberOfElements ());

	for (i = 0; i < m_pointList.getNumberOfElements (); i++)
	{
		m_pointList [i].x = (m_pointList [i].x - center.x) * scalar + center.x;
		m_pointList [i].y = (m_pointList [i].y - center.y) * scalar + center.y;
	}
	
	recalculate ();
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::expand (Rectangle2d& parentExtent) const
{
	parentExtent.expand (m_extent.x0, m_extent.y0);
	parentExtent.expand (m_extent.x1, m_extent.y1);
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::setWidth (const float newWidth)
{
	m_width = newWidth;

	recalculate ();
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::setFeatherFunction (TerrainGeneratorFeatherFunction newFeatherFunction)
{
	m_featherFunction = newFeatherFunction;
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::setFeatherDistance (const float newFeatherDistance)
{
	m_featherDistance = newFeatherDistance;
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::enable ()
{
	ms_enabled = true;
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::disable ()
{
	ms_enabled = false;
}

//-------------------------------------------------------------------

bool AffectorBoundaryPoly::isEnabled ()
{
	return ms_enabled;
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::clearHeightData ()
{
	m_heightData.clear ();
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::addSegmentHeightData ()
{
	m_heightData.addSegment ();
}

//-------------------------------------------------------------------

void AffectorBoundaryPoly::addPointHeightData (const Vector& point)
{
	m_heightData.addPoint (point);
}

//-------------------------------------------------------------------

bool AffectorBoundaryPoly::find (const Vector2d& point, const float width, FindData& result, bool ignoreHeight) const
{
	if (m_pointList.empty ())
		return false;

	const float widthSquared    = sqr (width);
	float       distanceSquared = widthSquared;

	//-- first, scan how far we are from the points
	{
		int i;
		for (i = 0; i < m_pointList.size (); ++i)
		{
			const float thisDistanceSquared = point.magnitudeBetweenSquared (m_pointList [i]);
			if (thisDistanceSquared < distanceSquared)
			{
				distanceSquared = thisDistanceSquared;
				result.t        = m_lengthTotals [i];
				result.height   = (i != m_pointList.size () - 1) ? m_heightData.getPoint (i, 0).y : m_heightData.getPoint (i - 1, m_heightData.getNumberOfPoints (i - 1) - 1).y;
			}
		}
	}

	int      segmentIndex = 0;
	Vector2d resultPoint;
	bool     searchHeightData = false;

	//-- next, scan each line
	{
		int i;
		for (i = 0; i < m_pointList.size () - 1; ++i)
		{
			const Vector2d start = m_pointList [i];
			const Vector2d end   = m_pointList [i + 1];

			const float t = computeT (start, end, point);
			if (t >= 0 && t <= 1)
			{
				const Vector2d lerpedPoint = Vector2d::linearInterpolate (start, end, t);
				const float thisDistanceSquared = point.magnitudeBetweenSquared (lerpedPoint);
				if (thisDistanceSquared < distanceSquared)
				{
					distanceSquared  = thisDistanceSquared;
					segmentIndex     = i;
					resultPoint      = lerpedPoint;
					if(!ignoreHeight) // some affectors don't have height data (ribbon)
					{
						searchHeightData = true;
					}

					result.t         = m_lengthTotals [i] + (t * m_lengths [i + 1]);
				}
			}
		}
	}

	if (distanceSquared < widthSquared)
	{
		result.distanceToCenter = static_cast<float> (sqrt (distanceSquared));
		result.length           = m_lengthTotals.back ();

		if (searchHeightData && !m_heightData.find (segmentIndex, resultPoint, result.height))
		{
			static bool s_warnedMissingHeightData = false;

			if (!s_warnedMissingHeightData)
			{
				s_warnedMissingHeightData = true;
				DEBUG_WARNING (true, ("AffectorBoundaryPoly::find - couldn't find height data for %s.  Did you not bake the river/road data?", getName ()));
			}

			result.height = 0.f;
		}

		return true;
	}

	return false;
}

//===================================================================


