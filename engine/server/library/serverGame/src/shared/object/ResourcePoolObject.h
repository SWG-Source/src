//========================================================================
//
// ResourcePoolObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_ResourcePoolObject_H
#define INCLUDED_ResourcePoolObject_H

#include "serverGame/UniverseObject.h"
#include "sharedObject/CachedNetworkId.h"

class DynamicVariableList;
class MultiFractal;
class PlanetObject;
class ResourceTypeObject;
struct ResourceFractalData;

// ======================================================================

/** 
 * A pool of a resource found on a planet.
 *
 * Represents the planet's supply of a particular resource.
 */

class ResourcePoolObject
{
  public:
	// getter functions
	ResourceTypeObject const & getResourceType       () const;
	bool                    isDepleted               () const;
	float                   getEfficiencyAtLocation  (float x, float z) const;
	CachedNetworkId const & getPlanet                () const;
	void                    debugOutput              (Unicode::String &output) const;

	// simulation functions
	float                   harvest                  (float installedEfficiency, uint32 lastHarvestTick) const;
	int                     oneTimeHarvest           (int amount, const Vector &location) const;

	ResourcePoolObject(ResourceTypeObject const & resourceType, PlanetObject const & planetObject, uint32 fractalSeed);
	virtual ~ResourcePoolObject();

  private:
	ResourcePoolObject(const ResourcePoolObject& rhs);
	ResourcePoolObject&	operator=(const ResourcePoolObject& rhs);

  private:
	ResourceTypeObject const & m_resourceType;
	CachedNetworkId const m_planet;
	ResourceFractalData const & m_fractalData;
	uint32 const m_fractalSeed;
	uint32 m_depletedTimestamp;
	MultiFractal mutable * m_efficiencyMap;
};

// ======================================================================

inline ResourceTypeObject const & ResourcePoolObject::getResourceType() const
{
	return m_resourceType;
}

// ----------------------------------------------------------------------

inline CachedNetworkId const & ResourcePoolObject::getPlanet() const
{
	return m_planet;
}

// ======================================================================

#endif
