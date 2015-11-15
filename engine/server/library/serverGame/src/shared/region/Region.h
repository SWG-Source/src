//========================================================================
//
// Region.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_Region_H
#define INCLUDED_Region_H

#include "Unicode.h"
#include "UnicodeUtils.h"
#include "sharedObject/CachedNetworkId.h"

// ======================================================================

class MxCifQuadTreeBounds;
class RegionPvp;

// ======================================================================

namespace RegionNamespace
{

enum RegionPvp
{
	RP_normal,
	RP_truce,
	RP_pvpBattlefield,
	RP_pveBattlefield
};

enum RegionGeography
{
	RG_desert,
	RG_forest,
	RG_jungle,
	RG_plateau,
	RG_oasis,
	RG_canyon,
	RG_lake,
	RG_lakeshore,
	RG_beach,
	RG_ocean,
	RG_underwater,
	RG_river,
	RG_volcano,
	RG_swamp,
	RG_plains,
	RG_prairie,
	RG_mountains,
	RG_hills,
	RG_grassland,
	RG_wasteland,
	RG_city,
	RG_fictional,
	RG_pathfind,
	RG_default,
	RG_theater,
	RG_overload
};

}

/** 
 * Represents a game Region.  This is a structure that has a shape (rectangle or circle) 
 * that covers an area of a game planet.  This structure also has certain properties 
 * (such as PvP status, geographical type, etc.).
 */
typedef int32 EnvironmentInfo; // Type def'd incase we want to change this later.

class Region
{
public:

	virtual ~Region();

	static Region const *findRegionByNameCrc(uint32 nameCrc);

	virtual RegionPvp *asRegionPvp();
	virtual RegionPvp const *asRegionPvp() const;
	virtual Vector getCenter() const = 0;
	void setName(const Unicode::String & name);
	void setPlanet(const std::string & planet);
	void setPvp(int pvp);
	void setGeography(int geography);
	void setMinDifficulty(int difficulty);
	void setMaxDifficulty(int difficulty);
	void setSpawn(int spawn);
	void setMission(int mission);
	void setBuildable(int buildable);
	void setMunicipal(int municipal);
	void setVisible(bool visible);
	void setNotify(bool notify);
	void setEnvironmentFlags(EnvironmentInfo const & flags);

	const Unicode::String & getName() const;
	uint32                  getNameCrc() const;
	const std::string &     getPlanet() const;
	int                     getPvp() const;
	int                     getGeography() const;
	int                     getMinDifficulty() const;
	int                     getMaxDifficulty() const;
	int                     getSpawn() const;
	int                     getMission() const;
	int                     getBuildable() const;
	int                     getMunicipal() const;
	bool                    isVisible() const;
	bool                    getNotify() const;

	bool                    isDynamic() const;
	bool                    isStatic() const;
	const CachedNetworkId & getDynamicRegionId() const;

	virtual float           getArea(void) const = 0;

	const MxCifQuadTreeBounds & getBounds(void) const;

	const EnvironmentInfo & getEnvironmentFlags() const;

protected:
	         Region();
	explicit Region(const CachedNetworkId & dynamicRegionId);
	
	void setBounds(const MxCifQuadTreeBounds * bounds);

private:
	//disabled
	Region(const Region& rhs);
	Region& operator=(const Region& rhs);

private:
	const MxCifQuadTreeBounds * m_bounds;
	const CachedNetworkId       m_dynamicRegionId;

	Unicode::String             m_name;
	uint32                      m_nameCrc;
	std::string                 m_planet;
	int                         m_pvp;
	int                         m_geography;
	int                         m_minDifficulty;
	int                         m_maxDifficulty;
	int                         m_spawn;
	int                         m_mission;
	int                         m_buildable;
	int                         m_municipal;
	bool                        m_visible;
	bool                        m_notify;
	EnvironmentInfo             m_environmentFlags;
};


inline void Region::setPlanet(const std::string & planet)
{
	m_planet = planet;
}

inline void Region::setPvp(int pvp)
{
	m_pvp = pvp;
}

inline void Region::setGeography(int geography)
{
	m_geography = geography;
}

inline void Region::setMinDifficulty(int difficulty)
{
	m_minDifficulty = difficulty;
}

inline void Region::setMaxDifficulty(int difficulty)
{
	m_maxDifficulty = difficulty;
}

inline void Region::setSpawn(int spawn)
{
	m_spawn = spawn;
}

inline void Region::setMission(int mission)
{
	m_mission = mission;
}

inline void Region::setBuildable(int buildable)
{
	m_buildable = buildable;
}

inline void Region::setMunicipal(int municipal)
{
	m_municipal = municipal;
}

inline void Region::setVisible(bool visible)
{
	m_visible = visible;
}

inline const Unicode::String & Region::getName() const
{
	return m_name;
}

inline uint32 Region::getNameCrc() const
{
	return m_nameCrc;
}

inline const std::string & Region::getPlanet() const
{
	return m_planet;
}

inline int Region::getPvp() const
{
	return m_pvp;
}

inline int Region::getGeography() const
{
	return m_geography;
}

inline int Region::getMinDifficulty() const
{
	return m_minDifficulty;
}

inline int Region::getMaxDifficulty() const
{
	return m_maxDifficulty;
}

inline int Region::getSpawn() const
{
	return m_spawn;
}

inline int Region::getMission() const
{
	return m_mission;
}

inline int Region::getBuildable() const
{
	return m_buildable;
}

inline int Region::getMunicipal() const
{
	return m_municipal;
}

inline bool Region::isVisible() const
{
	return m_visible;
}

inline bool Region::isDynamic() const
{
	return m_dynamicRegionId != CachedNetworkId::cms_cachedInvalid;
}

inline bool Region::isStatic() const
{
	return !isDynamic();
}

inline const CachedNetworkId & Region::getDynamicRegionId() const
{
	WARNING_STRICT_FATAL(isStatic(), ("Calling Region::getDynamicRegionId for "
		"static region %s", Unicode::wideToNarrow(m_name).c_str()));
	return m_dynamicRegionId;
}

inline const MxCifQuadTreeBounds & Region::getBounds(void) const
{
	NOT_NULL(m_bounds);
	return *m_bounds;
}

inline void Region::setBounds(const MxCifQuadTreeBounds * bounds)
{
	m_bounds = bounds;
}

inline EnvironmentInfo const & Region::getEnvironmentFlags() const
{
	return m_environmentFlags;
}

inline void Region::setEnvironmentFlags(const EnvironmentInfo &flags)
{
	m_environmentFlags = flags;
}

// ======================================================================

/**
 * A rectangular region.
 */
class RegionRectangle : public Region
{
public:
	RegionRectangle(float minX, float minY, float maxX, float maxY);
	RegionRectangle(float minX, float minY, float maxX, float maxY, const CachedNetworkId & dynamicRegionId);

	virtual float getArea(void) const;

	void getExtent(float & minX, float & minY, float & maxX, float & maxY) const;
	virtual Vector getCenter() const;
	
private:
	// disabled
	RegionRectangle();
	RegionRectangle(const RegionRectangle &);
	RegionRectangle & operator =(const RegionRectangle &);
};


// ======================================================================

/**
 * A circualr region.
 */
class RegionCircle : public Region
{
public:
	RegionCircle(float centerX, float centerY, float radius);
	RegionCircle(float centerX, float centerY, float radius, const CachedNetworkId & dynamicRegionId);

	virtual float getArea(void) const;

	void getExtent(float & centerX, float & centerY, float & radius) const;
	virtual Vector getCenter() const;
	
private:
	// disabled
	RegionCircle();
	RegionCircle(const RegionCircle &);
	RegionCircle & operator =(const RegionCircle &);
};


#endif	// INCLUDED_Region_H
