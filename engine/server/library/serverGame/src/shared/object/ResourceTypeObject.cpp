//========================================================================
//
// ResourceTypeObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ResourceTypeObject.h"

#include "UnicodeUtils.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ResourcePoolObject.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverNetworkMessages/AddResourceTypeMessage.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedLog/Log.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedRandom/Random.h"

#include <map>
#include <cstdio>
#include <vector>

// ======================================================================

/**
 * Constructor to spawn a new resource type from the specified class
 */
ResourceTypeObject::ResourceTypeObject(ServerResourceClassObject & parent) :
		m_networkId(ObjectIdManager::getNewObjectId()),
		m_resourceName((parent.isRecycled() || parent.isPermanent()) ? "@" + parent.getFriendlyName().getCanonicalRepresentation() : ServerUniverse::getInstance().generateRandomResourceName(parent.getNameTable())),
		m_resourceClass(parent),
		m_attributes(),
		m_fractalData(parent.getFractalData()),
		m_fractalSeeds(),
		m_depletedTimestamp((parent.isRecycled() || parent.isPermanent()) ? ServerClock::cms_endOfTime : ServerClock::getInstance().getGameTimeSeconds() + static_cast<uint32>(Random::random(parent.getMinPoolSize(),parent.getMaxPoolSize())) * ConfigServerGame::getResourceTimeScale()),
		m_pools()
{
	
	LOG("CustomerService",("Resource Type %s spawned from class %s",getResourceName().c_str(),m_resourceClass.getResourceClassName().c_str()));

	rollAttributes();

	if (!parent.isRecycled() && !parent.isPermanent())
	{
		// Pick planets and fractal seeds
		std::vector<std::string> possiblePlanets = parent.getPossiblePlanets();
		for (int numPools = Random::random(parent.getMinPools(), parent.getMaxPools()); numPools > 0; --numPools)
		{
			if (possiblePlanets.size()==0)
			{
				WARNING_STRICT_FATAL(true,("Needed to spawn pool for resource %s (%s), but there were no possible planets remaining.",m_resourceName.c_str(), parent.getResourceClassName().c_str()));
				break;
			}

			size_t const choiceIndex = static_cast<size_t>(Random::random(0,static_cast<int>(possiblePlanets.size())-1));
			PlanetObject const * const planet = ServerUniverse::getInstance().getPlanetByName(possiblePlanets[choiceIndex]);
			if (!planet)
				WARNING_STRICT_FATAL(true,("Resource type %s specified planet %s, which could not be found\n",
										   getNetworkId().getValueString().c_str(),possiblePlanets[choiceIndex].c_str()));
			else
			{
				IGNORE_RETURN(m_fractalSeeds.insert(std::make_pair(planet->getNetworkId(), Random::random())));  //lint !e1025 !e1703 // lint can't parse this for some reason
				LOG("CustomerService",("Resource Pool of %s (class %s) spawned on %s",
									   getResourceName().c_str(),m_resourceClass.getResourceClassName().c_str(),possiblePlanets[choiceIndex].c_str()));
				
				possiblePlanets[choiceIndex] = possiblePlanets.back();
				possiblePlanets.pop_back();
			}
		}
	}
}

//-----------------------------------------------------------------------

ResourceTypeObject::~ResourceTypeObject()
{

}

// ----------------------------------------------------------------------

void ResourceTypeObject::debugOutput(std::string &output, int numSpaces) const
{
	char buffer[30];
	sprintf(buffer,"%s",getNetworkId().getValueString().c_str());

	{
		for (int i=0; i<numSpaces; ++i)
		{
			output+="  ";
		}
	}

	output+=buffer;
	output+=":  ";
	output+=getResourceName();
	output+=" ";
	if (!isDepleted())
	{
		output+=ServerClock::getInstance().getDebugPrintableTimeframe(m_depletedTimestamp - ServerClock::getInstance().getGameTimeSeconds());
		output+=" remaining";
	}
	else
		output+="depleted";
	output+=" (";
	{
		for (FractalSeedsType::const_iterator i=m_fractalSeeds.begin(); i!=m_fractalSeeds.end(); )
		{
			PlanetObject const * const planet = safe_cast<PlanetObject const *>(NetworkIdManager::getObjectById(i->first));
			if (planet)
			{
				output += planet->getName();
			}
			if (++i != m_fractalSeeds.end())
				output += ',';
		}
	}
	output+=")\n";
}

// ----------------------------------------------------------------------

/**
 * Change a resource name (console/god command)
 */
void ResourceTypeObject::setName(const std::string &newName)
{
	if (ServerUniverse::getInstance().isAuthoritative())
	{
		ServerUniverse::getInstance().renameResourceType(m_resourceName,newName);
		m_resourceName=newName;
		ServerUniverse::getInstance().sendUpdatedTypeToAllServers(*this);
	}
}

// ----------------------------------------------------------------------

ResourcePoolObject *ResourceTypeObject::getPoolForPlanet(PlanetObject const &planetObject) const
{
	if (isRecycled())
		return nullptr;

	std::map<NetworkId, ResourcePoolObject*>::const_iterator i=m_pools.find(planetObject.getNetworkId());
	if (i==m_pools.end())
	{
		FractalSeedsType::const_iterator seed = m_fractalSeeds.find(planetObject.getNetworkId());
		if (seed != m_fractalSeeds.end())
		{
			// create the object for the pool the first time someone needs it
			ResourcePoolObject *obj = new ResourcePoolObject(*this, planetObject, seed->second);
			m_pools[planetObject.getNetworkId()]=obj;
			return obj;
		}
		else
		{
			if (ConfigServerGame::getSpawnAllResources())
			{
				ResourcePoolObject *obj = new ResourcePoolObject(*this, planetObject, 0);
				m_pools[planetObject.getNetworkId()]=obj;
				return obj;
			}
			return nullptr; // no pool for this planet
		}
	}
	else
		return i->second;
}

// ----------------------------------------------------------------------

bool ResourceTypeObject::hasPoolForPlanet (const NetworkId &planet) const
{
	return (!isDepleted()) && (m_fractalSeeds.find(planet) != m_fractalSeeds.end());

}

// ----------------------------------------------------------------------

ServerResourceClassObject const & ResourceTypeObject::getParentClass() const
{
	return m_resourceClass; //   lint !e1536 // exposing "low access member" -- Lint thinks we're returning a reference to m_resouceClass itself, but we're actually returning a reference to the thing m_resourceClass refers to
}

// ----------------------------------------------------------------------

bool ResourceTypeObject::isDepleted() const
{
	return (m_depletedTimestamp <= ServerClock::getInstance().getGameTimeSeconds());
}

// ----------------------------------------------------------------------

/**
 * Returns the number of non-depleted pools of this resource.
 */
int ResourceTypeObject::getNumPools() const
{
	return isDepleted() ? 0 : static_cast<int>(m_fractalSeeds.size());
}

// ----------------------------------------------------------------------

/** 
 * Returns true if this ResourceType is derived (directly or indirectly) from the class.
 */
bool ResourceTypeObject::isDerivedFrom(const ResourceClassObject &masterClass) const
{
	return m_resourceClass.isDerivedFrom(masterClass);
}

// ----------------------------------------------------------------------

/**
 * Returns the template name for the appropriate kind of crate to use for this resource.
 */
void ResourceTypeObject::getCrateTemplate(std::string &buffer) const
{
	m_resourceClass.getCrateTemplate(buffer);
}

// ----------------------------------------------------------------------

/**
 * Get only those attributes specific to the resource
 */
void ResourceTypeObject::getResourceAttributes (std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	for (AttributesType::const_iterator i=m_attributes.begin(); i!=m_attributes.end(); ++i)
	{
		char buffer[300];
		sprintf(buffer,"%i",i->second);
		data.push_back(std::pair<std::string, Unicode::String>(i->first,Unicode::narrowToWide(buffer)));
	}
}

// ----------------------------------------------------------------------

std::string const & ResourceTypeObject::getResourceName() const
{
	static std::string const unnamed("unnamed");
	
	if (isNamed())
		return m_resourceName;
	else
		return unnamed;
}

// ----------------------------------------------------------------------

/**
 * Force this resource type to be depleted.
 * (Console/God command)
 */
void ResourceTypeObject::deplete()
{
	DEBUG_FATAL(!ServerUniverse::getInstance().isAuthoritative(),("Programmer bug:  ResourceTypeObject::deplete() should only be called on the authoritative server.  Use ServerUniverse::manualDepleteResource() instead."));

	if (ServerUniverse::getInstance().isAuthoritative())
	{
		m_depletedTimestamp = ServerClock::getInstance().getGameTimeSeconds();
		ServerUniverse::getInstance().sendUpdatedTypeToAllServers(*this);
	}
}

// ----------------------------------------------------------------------

ResourcePoolObject * ResourceTypeObject::getPoolForCurrentPlanet() const
{
	return getPoolForPlanet(*(NON_NULL(ServerUniverse::getInstance().getCurrentPlanet())));
}

// ----------------------------------------------------------------------

ResourceFractalData const & ResourceTypeObject::getFractalData() const
{
	return m_fractalData;
}

// ----------------------------------------------------------------------

NetworkId const & ResourceTypeObject::getNetworkId() const
{
	return m_networkId;
}

// ----------------------------------------------------------------------

uint32 ResourceTypeObject::getDepletedTimestamp() const
{
	return m_depletedTimestamp;
}

// ----------------------------------------------------------------------

int ResourceTypeObject::getAttribute(std::string const & attributeName) const
{
	AttributesType::const_iterator i=m_attributes.find(attributeName);
	if (i==m_attributes.end())
		return -1;
	else
		return i->second;	
}

// ----------------------------------------------------------------------

/**
 * Pack this resource type into a AddResourceTypeMessageNamespace::ResourceTypeData,
 * which can be used to send it on the network.
 */
void ResourceTypeObject::getDataForMessage(AddResourceTypeMessageNamespace::ResourceTypeData & buffer) const
{
	buffer.m_networkId = m_networkId;
	buffer.m_name = m_resourceName;
	buffer.m_depletedTimestamp = m_depletedTimestamp;	
	buffer.m_parentClass=m_resourceClass.getResourceClassName();
		
	buffer.m_attributes.clear();
	buffer.m_attributes.reserve(m_attributes.size());
	{
		for(AttributesType::const_iterator i=m_attributes.begin(); i!=m_attributes.end(); ++i)
			buffer.m_attributes.push_back(*i); //lint !e1561 // (Reference initialization causes loss of const/volatile integrity (arg. no. 1))
	}
	buffer.m_fractalSeeds.clear();
	buffer.m_fractalSeeds.reserve(m_fractalSeeds.size());
	{
		for(FractalSeedsType::const_iterator i=m_fractalSeeds.begin(); i!=m_fractalSeeds.end(); ++i)
			buffer.m_fractalSeeds.push_back(*i); //lint !e1025 !e1703 // lint can't parse this line
	}
}

// ----------------------------------------------------------------------

ResourceTypeObject::ResourceTypeObject(AddResourceTypeMessageNamespace::ResourceTypeData const & data, ServerResourceClassObject & parentClass) :
		m_networkId(data.m_networkId),
		m_resourceName(data.m_name),
		m_resourceClass(parentClass),
		m_attributes(),
		m_fractalData(parentClass.getFractalData()),
		m_fractalSeeds(),
		m_depletedTimestamp(data.m_depletedTimestamp),
		m_pools()
{
	if (getParentClass().isRecycled())
	{
		// Recalculate recycled resource attributes on every startup, because they may have changed in the data table.  Otherwise recycled resource attributes
		// could never be changed.  (Regular resources that turn over keep their current attributes, even if the data table changes, until they turn over.)
		// Since each server does this independantly, recycled resources must have fixed attributes (enforced by rollAttributes()).
		rollAttributes();
	}
	else
	{
		for(std::vector<std::pair<std::string, int> >::const_iterator i=data.m_attributes.begin(); i!=data.m_attributes.end(); ++i)
			IGNORE_RETURN(m_attributes.insert(*i));
	}
	for(std::vector<std::pair<NetworkId, int> >::const_iterator j=data.m_fractalSeeds.begin(); j!=data.m_fractalSeeds.end(); ++j)
		IGNORE_RETURN(m_fractalSeeds.insert(*j)); //lint !e1025 !e1703 // lint can't parse this line
}

// ----------------------------------------------------------------------

ResourceTypeObject const * ResourceTypeObject::getRecycledVersion() const
{
	ServerResourceClassObject const * const recycledClass = safe_cast<ServerResourceClassObject const *>(m_resourceClass.getRecycledVersion());
	if (recycledClass)
	{
		ResourceTypeObject const * const result = recycledClass->getAResourceType();
		DEBUG_FATAL(!result,("Programmer or Data bug:  Recycled class %s had no resource types derived from it.  Recycled resources should require 1 resource type be derived from them",recycledClass->getResourceClassName().c_str()));
		DEBUG_FATAL(result->isDepleted(),("Programmer bug:  recycledClass->getAResourceType() returned a depleted resource.  Recycled resources should never be depleted."));
		return result;
	}
	else
		return nullptr;
}

// ----------------------------------------------------------------------

std::string ResourceTypeObject::getResourceTypeDataForExport() const
{
	std::string resourceData;
	if (m_networkId.getValue() <= NetworkId::cms_maxNetworkIdWithoutClusterId)
	{
		resourceData = FormattedString<512>().sprintf("%s~%u~%s~%s~%s", GameServer::getInstance().getClusterName().c_str(), GameServer::getInstance().getClusterId(), getResourceName().c_str(), m_networkId.getValueString().c_str(), m_resourceClass.getResourceClassName().c_str());

		for (std::map<std::string, int>::const_iterator iter = m_attributes.begin(); iter != m_attributes.end(); ++iter)
		{
			resourceData += "~";
			resourceData += FormattedString<512>().sprintf("%s %d", iter->first.c_str(), iter->second);
		}

		return resourceData;
	}

	std::string const & resourceNameWithCluster = getResourceName();
	std::string::size_type const openParenPos = resourceNameWithCluster.find('(');
	if (openParenPos != std::string::npos)
	{
		std::string::size_type const closeParenPos = resourceNameWithCluster.find(')');
		if ((closeParenPos != std::string::npos) && ((openParenPos + 1) < closeParenPos) && ((closeParenPos + 1) < resourceNameWithCluster.size()))
		{
			std::string const clusterName = resourceNameWithCluster.substr((openParenPos + 1), (closeParenPos - openParenPos - 1));
			std::string const resourceName = resourceNameWithCluster.substr(closeParenPos + 1);

			if (!clusterName.empty() && !resourceName.empty())
			{
				NetworkId const resourceId = NetworkId(m_networkId.getValueWithoutClusterId());
				uint8 const clusterId = m_networkId.getValueClusterId();
				if (resourceId.isValid() && (resourceId.getValue() <= NetworkId::cms_maxNetworkIdWithoutClusterId) && (clusterId > 0))
				{
					resourceData = FormattedString<512>().sprintf("%s~%u~%s~%s~%s", clusterName.c_str(), clusterId, resourceName.c_str(), resourceId.getValueString().c_str(), m_resourceClass.getResourceClassName().c_str());

					for (std::map<std::string, int>::const_iterator iter = m_attributes.begin(); iter != m_attributes.end(); ++iter)
					{
						resourceData += "~";
						resourceData += FormattedString<512>().sprintf("%s %d", iter->first.c_str(), iter->second);
					}
				}
			}
		}
	}

	return resourceData;
}

// ----------------------------------------------------------------------

NetworkId ResourceTypeObject::addImportedResourceType(std::string const & resourceData)
{
	static Unicode::String const delimiters(Unicode::narrowToWide("~"));
	static Unicode::String const attributeDelimiters(Unicode::narrowToWide(" "));
	Unicode::UnicodeStringVector tokens;
	static size_t const resourceAttributeIndex = 5;
	if ((Unicode::tokenize(Unicode::narrowToWide(resourceData), tokens, &delimiters, nullptr)) && (tokens.size() > resourceAttributeIndex))
	{
		AddResourceTypeMessageNamespace::ResourceTypeData rtd;
		rtd.m_depletedTimestamp = 1;

		std::string const sourceGalaxy = Unicode::wideToNarrow(tokens[0]);
		int const sourceGalaxyId = ::atoi(Unicode::wideToNarrow(tokens[1]).c_str());
		rtd.m_name = Unicode::wideToNarrow(tokens[2]);
		NetworkId const resourceType = NetworkId(Unicode::wideToNarrow(tokens[3]));
		rtd.m_parentClass = Unicode::wideToNarrow(tokens[4]);

		// save off resource attributes
		Unicode::UnicodeStringVector attributeTokens;
		for (size_t i = resourceAttributeIndex; i < tokens.size(); ++i)
		{
			attributeTokens.clear();
			if ((Unicode::tokenize(tokens[i], attributeTokens, &attributeDelimiters, nullptr)) && (attributeTokens.size() == 2))
			{
				rtd.m_attributes.push_back(std::make_pair(Unicode::wideToNarrow(attributeTokens[0]), ::atoi(Unicode::wideToNarrow(attributeTokens[1]).c_str())));
			}
		}

		if (sourceGalaxy.empty() || (sourceGalaxyId < 1) || (sourceGalaxyId > 255) || rtd.m_name.empty() || !resourceType.isValid() || rtd.m_parentClass.empty() || rtd.m_attributes.empty())
			return NetworkId::cms_invalid;

		rtd.m_name = std::string("(") + sourceGalaxy + std::string(")") + rtd.m_name;

		// resource is locally spawned
		if (sourceGalaxy == GameServer::getInstance().getClusterName())
			return resourceType;

		// get the resource parent class
		ServerResourceClassObject * const parentClass = safe_cast<ServerResourceClassObject *>(ServerUniverse::getInstance().getResourceClassByName(rtd.m_parentClass));
		if (!parentClass)
			return NetworkId::cms_invalid;

		// if the resource container is a recycled or permanent resource,
		// just "convert" it to use the resource on the local galaxy, since
		// recycled or permanent resource are the same on all galaxies
		if (parentClass->isRecycled() || parentClass->isPermanent())
		{
			ResourceTypeObject const * recycledType = parentClass->getAResourceType();
			if (recycledType)
				return recycledType->getNetworkId();
			else
				return NetworkId::cms_invalid;
		}

		rtd.m_networkId = NetworkId(resourceType.getValueWithClusterId(static_cast<uint8>(sourceGalaxyId), static_cast<NetworkId::NetworkIdType>(1), NetworkId::cms_maxNetworkIdWithoutClusterId));
		if (rtd.m_networkId.getValue() <= NetworkId::cms_maxNetworkIdWithoutClusterId)
			return NetworkId::cms_invalid;

		if (!ServerUniverse::getInstance().getImportedResourceTypeById(rtd.m_networkId))
		{
			ResourceTypeObject * const newType = new ResourceTypeObject(rtd, *parentClass);
			ServerUniverse::getInstance().registerResourceTypeObject(*newType, false);
		}

		return rtd.m_networkId;
	}

	return NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

bool ResourceTypeObject::isRecycled() const
{
	return m_resourceClass.isRecycled();
}

// ----------------------------------------------------------------------

/**
 * We received another AddResourceType message for this type.  Check whether
 * any of the data needs to be updated.
 * Updating the name and depleted timestamp is supported.  Other properties
 * can't be updated in this manner.
 */
void ResourceTypeObject::handlePossibleUpdates(AddResourceTypeMessageNamespace::ResourceTypeData const & newData)
{
	m_depletedTimestamp = newData.m_depletedTimestamp;
	if (newData.m_name != m_resourceName)
	{
		ServerUniverse::getInstance().renameResourceType(m_resourceName,newData.m_name);
		m_resourceName=newData.m_name;
	}
}

// ----------------------------------------------------------------------

/**
 * Roll up random attributes
 */
void ResourceTypeObject::rollAttributes()
{
	ServerResourceClassObject const & parent = getParentClass();
	ServerResourceClassObject::ResourceAttributeRangesType const & ranges = parent.getResourceAttributeRanges();
	bool const recycled = parent.isRecycled();
	m_attributes.clear();
	for (ServerResourceClassObject::ResourceAttributeRangesType::const_iterator i=ranges.begin(); i!=ranges.end(); ++i)
	{
		WARNING_DEBUG_FATAL(recycled && (i->second.first != i->second.second),("Data bug:  resource type %s is recycled but specifies a range for attribute %s.  Recycled resources must always have fixed attributes",getResourceName().c_str(), i->first.c_str()));
		IGNORE_RETURN(m_attributes.insert(std::make_pair(i->first, Random::random(i->second.first, i->second.second)))); //lint !e1025 !e1703 // lint can't parse this for some reason
	}
}

// ======================================================================
