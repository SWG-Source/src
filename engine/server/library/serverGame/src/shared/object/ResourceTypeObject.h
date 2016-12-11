//========================================================================
//
// ResourceTypeObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_ResourceTypeObject_H
#define INCLUDED_ResourceTypeObject_H

#include "serverGame/UniverseObject.h"
#include "serverUtility/ResourceFractalData.h"
#include "sharedObject/CachedNetworkId.h"

class PlanetObject;
class ResourceClassObject;
class ResourcePoolObject;
class ServerResourceClassObject;

// ======================================================================

namespace AddResourceTypeMessageNamespace
{
	class ResourceTypeData;
}

// ======================================================================

/** 
 * A pool of a resource found on a planet.
 *
 * Represents the planet's supply of a particular resource.
 */

class ResourceTypeObject
{
  public:
	ResourceTypeObject(ServerResourceClassObject &parent);
	ResourceTypeObject(AddResourceTypeMessageNamespace::ResourceTypeData const & data, ServerResourceClassObject & parentClass);
	virtual ~ResourceTypeObject();

  public:
	void setName(const std::string &newName);
	void debugOutput(std::string &output, int numSpaces=0) const;

	NetworkId const &           getNetworkId     () const;
	std::string const &         getResourceName  () const;
	bool                        isNamed          () const;
	bool                        isDepleted       () const;
	bool                        isRecycled       () const;
	int                         getNumPools      () const;
	ServerResourceClassObject const & getParentClass() const;
	ResourcePoolObject *        getPoolForPlanet(PlanetObject const &planetObject) const;
	ResourcePoolObject *        getPoolForCurrentPlanet() const;
	bool                        hasPoolForPlanet (const NetworkId &planet) const;
	bool                        isDerivedFrom    (const ResourceClassObject &masterClass) const;
	void                        getCrateTemplate (std::string &buffer) const;
	void                        getResourceAttributes (std::vector<std::pair<std::string, Unicode::String> > &data) const;
	const std::map<std::string, int> & getResourceAttributes () const;
	ResourceFractalData const & getFractalData() const;
	uint32                      getDepletedTimestamp() const;
	void                        deplete          ();
	void                        handlePossibleUpdates(AddResourceTypeMessageNamespace::ResourceTypeData const & newData);
	int                         getAttribute     (std::string const & attributeName) const;
	void                        getDataForMessage(AddResourceTypeMessageNamespace::ResourceTypeData & buffer) const;
	ResourceTypeObject const *  getRecycledVersion () const;
	std::string                 getResourceTypeDataForExport() const;
	static NetworkId            addImportedResourceType(std::string const & resourceData);
	
  private:
	PlanetObject *pickPlanetForSpawn ();
	void rollAttributes();
		
  private:
	ResourceTypeObject(const ResourceTypeObject& rhs);
	ResourceTypeObject&	operator=(const ResourceTypeObject& rhs);

  private:
	NetworkId       m_networkId;
	std::string m_resourceName;
	ServerResourceClassObject & m_resourceClass;
	typedef std::map<std::string, int> AttributesType;
	AttributesType m_attributes;
	ResourceFractalData m_fractalData;
	typedef std::map<NetworkId, uint32> FractalSeedsType;
	FractalSeedsType m_fractalSeeds;
	uint32 m_depletedTimestamp;
	
	mutable std::map<NetworkId, ResourcePoolObject*> m_pools;
};

// ======================================================================

inline bool ResourceTypeObject::isNamed() const
{
	return (m_resourceName.size() > 0);
}

inline const std::map<std::string, int> & ResourceTypeObject::getResourceAttributes () const
{
	return m_attributes;
}

// ======================================================================

#endif
