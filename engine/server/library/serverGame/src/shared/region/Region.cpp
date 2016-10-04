//========================================================================
//
// Region.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/Region.h"
#include "sharedFoundation/Crc.h"
#include "sharedMath/MxCifQuadTreeBounds.h"


// ======================================================================

static std::map<uint32, Region *> s_nameCrcRegionMap;

// ======================================================================

/**
 * Class constructor for a static region.
 */
Region::Region() :
	m_bounds(nullptr),
	m_dynamicRegionId(),
	m_name(),
	m_nameCrc(0),
	m_planet(),
	m_pvp(0),
	m_geography(0),
	m_minDifficulty(0),
	m_maxDifficulty(0),
	m_spawn(0),
	m_mission(0),
	m_buildable(0),
	m_municipal(0),
	m_visible(false),
	m_notify(false),
	m_environmentFlags(0)
{
}

//-----------------------------------------------------------------------

/**
 * Class constructor for a dynamic region.
 *
 * @param dynamicRegionId		the id of the dynamic region object
 */
Region::Region(const CachedNetworkId & dynamicRegionId) :
	m_bounds(nullptr),
	m_dynamicRegionId(dynamicRegionId),
	m_name(),
	m_nameCrc(0),
	m_planet(),
	m_pvp(0),
	m_geography(0),
	m_minDifficulty(0),
	m_maxDifficulty(0),
	m_spawn(0),
	m_mission(0),
	m_buildable(0),
	m_municipal(0),
	m_visible(false),
	m_notify(false),
	m_environmentFlags(0)
{
}

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
Region::~Region()
{
	if (m_nameCrc)
	{
		std::map<uint32, Region *>::iterator i = s_nameCrcRegionMap.find(m_nameCrc);
		if (i != s_nameCrcRegionMap.end())
			s_nameCrcRegionMap.erase(i);
	}

	delete const_cast<MxCifQuadTreeBounds *>(m_bounds);
	m_bounds = nullptr;
}

// ----------------------------------------------------------------------

void Region::setName(const Unicode::String & name)
{
	// remove from map if changing name
	if (m_nameCrc)
	{
		std::map<uint32, Region *>::iterator i = s_nameCrcRegionMap.find(m_nameCrc);
		if (i != s_nameCrcRegionMap.end())
			s_nameCrcRegionMap.erase(i);
	}

	m_name = name;
	m_nameCrc = Crc::calculate(name.c_str(), name.length()*2);

	// add to map if non-empty name
	if (m_nameCrc)
	{
		std::map<uint32, Region *>::iterator i = s_nameCrcRegionMap.find(m_nameCrc);
		if (i != s_nameCrcRegionMap.end())
		{
			FATAL(true, ("Tried to add a region with a duplicate crc (%s)", Unicode::wideToNarrow(name).c_str()));
		}
		s_nameCrcRegionMap[m_nameCrc] = this;
	}

}

// ----------------------------------------------------------------------

Region const *Region::findRegionByNameCrc(uint32 nameCrc) // static
{
	std::map<uint32, Region *>::const_iterator i = s_nameCrcRegionMap.find(nameCrc);
	if (i != s_nameCrcRegionMap.end())
		return (*i).second;
	return 0;
}

// ----------------------------------------------------------------------

void Region::setNotify(bool notify)
{
	m_notify = notify;
}

// ----------------------------------------------------------------------

bool Region::getNotify() const
{
	return m_notify;
}

// ----------------------------------------------------------------------

RegionPvp *Region::asRegionPvp()
{
	return 0;
}

// ----------------------------------------------------------------------

RegionPvp const *Region::asRegionPvp() const
{
	return 0;
}

// ======================================================================

/**
 * Class constructor for a static region.
 *
 * @param minX		min x value of the rectangle
 * @param minY		min y value of the rectangle
 * @param maxX		max x value of the rectangle
 * @param maxY		max y value of the rectangle
 */
RegionRectangle::RegionRectangle(float minX, float minY, float maxX, float maxY) :
	Region()
{
	if (minX > maxX)
		std::swap(minX, maxX);
	if (minY > maxY)
		std::swap(minY, maxY);
	setBounds(new MxCifQuadTreeBounds(minX, minY, maxX, maxY, this));
}

//-----------------------------------------------------------------------

/**
 * Class constructor for a dynamic region.
 *
 * @param minX		min x value of the rectangle
 * @param minY		min y value of the rectangle
 * @param maxX		max x value of the rectangle
 * @param maxY		max y value of the rectangle
 */
RegionRectangle::RegionRectangle(float minX, float minY, float maxX, float maxY, 
	const CachedNetworkId & dynamicRegionId) :
	Region(dynamicRegionId)
{
	if (minX > maxX)
		std::swap(minX, maxX);
	if (minY > maxY)
		std::swap(minY, maxY);
	setBounds(new MxCifQuadTreeBounds(minX, minY, maxX, maxY, this));
}

//-----------------------------------------------------------------------

/**
 * Returns the area of this region.
 *
 * @return the area
 */
float RegionRectangle::getArea() const
{
	float minX, minY, maxX, maxY;
	getExtent(minX, minY, maxX, maxY);
	return (maxX - minX) * (maxY - minY);
}	// RegionRectangle::getArea

//-----------------------------------------------------------------------

/**
 * Returns the data that defines the size of this region.
 *
 * @param minX		filled with the min x value of the rectangle
 * @param minY		filled with the min y value of the rectangle
 * @param maxX		filled with the max x value of the rectangle
 * @param maxY		filled with the max y value of the rectangle
 */
void RegionRectangle::getExtent(float & minX, float & minY, float & maxX, float & maxY) const
{
	const MxCifQuadTreeBounds & bounds = getBounds();
	
	minX = bounds.getMinX();
	minY = bounds.getMinY();
	maxX = bounds.getMaxX();
	maxY = bounds.getMaxY();
}	// RegionRectangle::getExtent

Vector RegionRectangle::getCenter() const
{
	const MxCifQuadTreeBounds & bounds = getBounds();
	return Vector(bounds.getMinX() + ((bounds.getMaxX() - bounds.getMinX()) * 0.5f), 0, bounds.getMinY() + ((bounds.getMaxY() - bounds.getMinY()) * 0.5f));
}

// ======================================================================

/**
 * Class constructor for a static region.
 *
 * @param centerX		x value of the circle's center
 * @param centerY		y value of the circle's center
 * @param radius		radius of the circle
 */
RegionCircle::RegionCircle(float centerX, float centerY, float radius) :
	Region()
{
	setBounds(new MxCifQuadTreeCircleBounds(centerX, centerY, radius, this));
}

//-----------------------------------------------------------------------

/**
 * Class constructor for a dynamic region.
 *
 * @param centerX		x value of the circle's center
 * @param centerY		y value of the circle's center
 * @param radius		radius of the circle
 */
RegionCircle::RegionCircle(float centerX, float centerY, float radius,
	const CachedNetworkId & dynamicRegionId) :
	Region(dynamicRegionId)
{
	setBounds(new MxCifQuadTreeCircleBounds(centerX, centerY, radius, this));
}

//-----------------------------------------------------------------------

/**
 * Returns the area of this region.
 *
 * @return the area
 */
float RegionCircle::getArea() const
{
	float centerX, centerY, radius;
	getExtent(centerX, centerY, radius);
	return PI * radius * radius;
}	// RegionCircle::getArea

//-----------------------------------------------------------------------

/**
 * Returns the data that defines the size of this region.
 *
 * @param centerX		filled with the x value of the circle's center
 * @param centerY		filled with the y value of the circle's center
 * @param radius		filled with the radius of the circle
 */
void RegionCircle::getExtent(float & centerX, float & centerY, float & radius) const
{
	const MxCifQuadTreeCircleBounds & bounds = static_cast<const MxCifQuadTreeCircleBounds &>(getBounds());
	
	centerX = bounds.getCenterX();
	centerY = bounds.getCenterY();
	radius = bounds.getRadius();
}	// RegionCircle::getExtent

Vector RegionCircle::getCenter() const
{
	const MxCifQuadTreeCircleBounds & bounds = static_cast<const MxCifQuadTreeCircleBounds &>(getBounds());
	return Vector(bounds.getCenterX(), 0.0f, bounds.getCenterY());
}

