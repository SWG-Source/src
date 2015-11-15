// ======================================================================
//
// MxCifQuadTreeBounds.cpp
//
// Copyright 2002, Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/MxCifQuadTreeBounds.h"

/**
 * Returns if a given point is in our area.
 *
 * @param x		x coordinate of the point
 * @param y		y coordinate of the point
 *
 * @return true we contain the point, false if not
 */
bool MxCifQuadTreeBounds::isPointIn(float x, float y) const
{
	return (x >= m_minX && x <= m_maxX && y >= m_minY && y <= m_maxY);
}	// MxCifQuadTreeBounds::isPointIn


//==============================================================================

/*
 Returns if a given point is in our area.
 *
 * @param x		x coordinate of the point
 * @param y		y coordinate of the point
 *
 * @return true we contain the point, false if not
 */
bool MxCifQuadTreeCircleBounds::isPointIn(float x, float y) const
{
	float dx = x - m_centerX;
	float dy = y - m_centerY;
	return (dx * dx + dy * dy) <= m_radiusSquared;
}	// MxCifQuadTreeCircleBounds::isPointIn
