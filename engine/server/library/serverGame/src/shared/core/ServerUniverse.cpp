// ======================================================================
//
// ServerUniverse.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerUniverse.h"

#include "UnicodeUtils.h"
#include "serverGame/AttribModNameManager.h"
#include "serverGame/CityObject.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverGame/GroupObject.h"
#include "serverGame/GuildObject.h"
#include "serverGame/NameManager.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ResourcePoolObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/AddResourceTypeMessage.h"
#include "serverNetworkMessages/CreateGroupMessage.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/PopulationList.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/ObjectTemplateList.h"

#include <list>
#include <map>

// ======================================================================

namespace ServerUniverseNamespace
{
	const Tag TAG_PLTS = TAG (P,L,T,S);
	const Tag TAG_PLNT = TAG (P,L,N,T);

	std::string s_emptyString = "";
}

using namespace ServerUniverseNamespace;

// ======================================================================

void ServerUniverse::install()
{
	if (!ConfigServerGame::getDisableResources())
		ResourceClassObject::loadTreeFromIff();
}

// ----------------------------------------------------------------------

ServerUniverse::ServerUniverse() :
		Universe                 (),
		m_universeObjectList     (new UniverseObjectList),
		m_pendingUniverseObjects (new UniverseObjectList),
		m_loaded                 (false),
		m_authoritative          (false),
		m_universeProcess        (0),
		m_nextCheckTimer         (static_cast<float>(ConfigServerGame::getUniverseCheckFrequencySeconds())),
		m_doImmediateCheck       (false),
		m_thisPlanet             (nullptr),
		m_tatooinePlanet         (nullptr),
		m_planetNameMap          (new PlanetNameMap),
		m_resourceTypeNameMap    (new ResourceTypeNameMap),
		m_resourceTypeIdMap      (new ResourceTypeIdMap),
		m_importedResourceTypeIdMap(new ResourceTypeIdMap),
		m_resourcesToSend        (new ResourcesToSendType),
		m_masterGuildObject      (nullptr),
		m_masterCityObject       (nullptr),
		m_theaterNameIdMap       (new TheaterNameIdMap),
		m_theaterIdNameMap       (new TheaterIdNameMap),
		m_populationList         (new PopulationList),
		m_pendingProxyRequests   (),
		m_pendingUniverseLoadedAckList(),
		m_timeUniverseDataSent   (0)
{
}

// ----------------------------------------------------------------------

ServerUniverse::~ServerUniverse()
{
	for (UniverseObjectList::const_iterator j=m_universeObjectList->begin(); j!=m_universeObjectList->end(); ++j)
		delete *j;
	
	delete m_pendingUniverseObjects;
	delete m_planetNameMap;
	delete m_resourceTypeNameMap;
	delete m_resourceTypeIdMap;
	delete m_importedResourceTypeIdMap;
	delete m_resourcesToSend;
	delete m_universeObjectList;
	delete m_populationList;
	delete m_theaterNameIdMap;
	delete m_theaterIdNameMap;

	m_pendingUniverseObjects =0;
	m_planetNameMap=0;
	m_resourceTreeRoot=0;
	m_resourceTypeNameMap=0;
	m_resourceTypeIdMap=0;
	m_importedResourceTypeIdMap=0;
	m_resourcesToSend=0;
	m_universeObjectList = 0;
	m_populationList = 0;
	m_thisPlanet = 0;
	m_tatooinePlanet = 0;
	m_masterGuildObject = 0;
	m_masterCityObject = 0;
	m_theaterNameIdMap = 0;
	m_theaterIdNameMap = 0;
}

// ----------------------------------------------------------------------

PlanetObject *ServerUniverse::getPlanetByName(const std::string &sceneId) const
{
	PlanetNameMap::iterator i=m_planetNameMap->find(sceneId);
	if (i!=m_planetNameMap->end())
	{
		return (*i).second;
	}
	else
	{
		if (!m_loaded)
			WARNING(true,("getPlanetByName() was called for (%s), but there is no such PlanetObject. m_loaded=(%s), m_authoritative=(%s), m_universeProcess=(%lu), m_thisPlanet=(%p), m_tatooinePlanet=(%p), m_masterGuildObject=(%p), m_masterCityObject=(%p), m_planetNameMap->size()=(%d), m_universeObjectList->size()=(%d), m_pendingUniverseObjects->size()=(%d), m_resourceTypeNameMap->size()=(%d), NameManager::getTotalPlayerCount()=(%d)", sceneId.c_str(), (m_loaded ? "true" : "false"), (m_authoritative ? "true" : "false"), m_universeProcess, m_thisPlanet, m_tatooinePlanet, m_masterGuildObject, m_masterCityObject, m_planetNameMap->size(), m_universeObjectList->size(), m_pendingUniverseObjects->size(), m_resourceTypeNameMap->size(), NameManager::getInstance().getTotalPlayerCount()));

		return 0;
	}
}

// ----------------------------------------------------------------------

ResourceTypeObject *ServerUniverse::getResourceTypeByName(std::string const & name) const
{
	ResourceTypeNameMap::iterator i=m_resourceTypeNameMap->find(name);
	if (i!=m_resourceTypeNameMap->end())
		return (*i).second;
	else
		return 0;
}

// ----------------------------------------------------------------------

ResourceTypeObject * ServerUniverse::getResourceTypeById(NetworkId const & id) const
{
	if (id==NetworkId::cms_invalid)
		return nullptr;
	
	if (id.getValue() <= NetworkId::cms_maxNetworkIdWithoutClusterId)
	{
	ResourceTypeIdMap::const_iterator i=m_resourceTypeIdMap->find(id);
	if (i!=m_resourceTypeIdMap->end())
		return (*i).second;
	else
		return nullptr;
	}
	else
	{
		ResourceTypeIdMap::const_iterator i=m_importedResourceTypeIdMap->find(id);
		if (i!=m_importedResourceTypeIdMap->end())
			return (*i).second;
		else
			return nullptr;
	}
}

// ----------------------------------------------------------------------

ResourceTypeObject * ServerUniverse::getImportedResourceTypeById(NetworkId const & id) const
{
	if (id==NetworkId::cms_invalid)
		return nullptr;

	ResourceTypeIdMap::const_iterator i=m_importedResourceTypeIdMap->find(id);
	if (i!=m_importedResourceTypeIdMap->end())
		return (*i).second;
	else
		return nullptr;
}

// ----------------------------------------------------------------------

/** 
 * Registers a new PlanetObject.
 * Puts the object into a map of name->planet, so that it can be found later.
 */

void ServerUniverse::registerPlanetObject (PlanetObject &newPlanet)
{
	(*m_planetNameMap)[newPlanet.getName()]=&newPlanet;
	if (newPlanet.getName()==ServerWorld::getSceneId())
		m_thisPlanet=&newPlanet;
	if (newPlanet.getName()=="tatooine")
		m_tatooinePlanet=&newPlanet;
}

// ----------------------------------------------------------------------

/** 
 * Registers a new ResourceTypeObject.
 * Puts the object into a map of name->type, so that it can be found later.
 */

void ServerUniverse::registerResourceTypeObject (ResourceTypeObject &newType, bool newlySpawned)
{
	(*m_resourceTypeNameMap)[newType.getResourceName()]=&newType;

	if (newType.getNetworkId().getValue() <= NetworkId::cms_maxNetworkIdWithoutClusterId)
	(*m_resourceTypeIdMap)[newType.getNetworkId()]=&newType;
	else
		(*m_importedResourceTypeIdMap)[newType.getNetworkId()]=&newType;

	if (newlySpawned)
	{
		DEBUG_FATAL(!isAuthoritative(),("Programmer bug:  ServerUniverse::registerResourceTypeObject(..., true) called on a server that was not authoritative for the universe."));
		m_resourcesToSend->push_back(&newType);
	} 
}

// ----------------------------------------------------------------------

/**
 * Send updates to a resource class to all the other servers (name change / force depletion)
 */
void ServerUniverse::sendUpdatedTypeToAllServers(ResourceTypeObject &changedType)
{
	DEBUG_FATAL(!isAuthoritative(),("Programmer bug:  ServerUniverse::sendUpdatedTypeToAllServers() called on a server that was not authoritative for the universe."));
	m_resourcesToSend->push_back(&changedType);
} //lint !e1764 // Lint wants changedType to be const, but the push_back requires a non-const pointer

// ----------------------------------------------------------------------

/** 
 * Registers a new Universe Object.  If we are authoritative for
 * universe objects, also proxies the object on the other game servers
 */
void ServerUniverse::addUniverseObject(UniverseObject &newObject)
{
	if (m_loaded)
	{
		m_universeObjectList->push_back(new CachedNetworkId(newObject));
		newObject.setupUniverse();

		if (m_authoritative)
		{
			GameServer::getInstance().createProxyOnAllServers(&newObject);
		}
	}
	else
	{
		m_pendingUniverseObjects->push_back(new CachedNetworkId(newObject));
	}
}

// ----------------------------------------------------------------------

/**
 * Give a new server proxies of all of the universe objects.
 */
void ServerUniverse::createProxiesOnServer(std::vector<uint32> const & remoteProcesses)
{
	m_pendingUniverseLoadedAckList.clear();

	if (m_loaded)
	{
		// Names are included with the universe objects
		NameManager::getInstance().sendAllNamesToServer(remoteProcesses);
		// So are attrib mod names
		AttribModNameManager::getInstance().sendAllNamesToServer(remoteProcesses);
		
		{
			for (std::vector<uint32>::const_iterator remoteProcessId=remoteProcesses.begin(); remoteProcessId!=remoteProcesses.end(); ++remoteProcessId)
			{
				for (UniverseObjectList::const_iterator i=m_universeObjectList->begin(); i!=m_universeObjectList->end(); ++i)
				{
					if ((*i)->getObject())
						GameServer::getInstance().createRemoteProxy(*remoteProcessId, safe_cast<ServerObject*>((*i)->getObject()));
					else
						DEBUG_REPORT_LOG(true,("Not proxying Universe Object %s because it has been deleted.\n",(*i)->getValueString().c_str()));
				}
			}
		}

		ServerMessageForwarding::begin(remoteProcesses);

		{
			std::vector<AddResourceTypeMessageNamespace::ResourceTypeData> resourceTypeDataVector;
			AddResourceTypeMessageNamespace::ResourceTypeData resourceTypeData;
			resourceTypeDataVector.reserve(m_resourceTypeIdMap->size());
			
			for (ResourceTypeIdMap::const_iterator i=m_resourceTypeIdMap->begin(); i!=m_resourceTypeIdMap->end(); ++i)
			{
				NON_NULL(i->second)->getDataForMessage(resourceTypeData);
				resourceTypeDataVector.push_back(resourceTypeData);
			}

			AddResourceTypeMessage const addResourceTypeMessage(resourceTypeDataVector);
			ServerMessageForwarding::send(addResourceTypeMessage);
		}
		
		GenericValueTypeMessage<uint32> const universeCompleteMessage("UniverseCompleteMessage", GameServer::getInstance().getProcessId());
		ServerMessageForwarding::send(universeCompleteMessage);

		ServerMessageForwarding::end();

		if (!remoteProcesses.empty())
		{
			std::string serverListAsString;
			char oneServer[10];
			for (std::vector<uint32>::const_iterator i=remoteProcesses.begin(); i!=remoteProcesses.end(); ++i)
			{
				if (ConfigServerGame::getTimeoutToAckUniverseDataReceived() > 0)
					IGNORE_RETURN(m_pendingUniverseLoadedAckList.insert(*i));

				if (i!=remoteProcesses.begin())
					serverListAsString+=", ";
				snprintf(oneServer,sizeof(oneServer)-1,"%lu",*i);
				oneServer[sizeof(oneServer)-1] = '\0';
				serverListAsString+=oneServer;
			}
			LOG("UniverseLoading", ("Game Server %lu sent UniverseComplete to Game Servers %s.", GameServer::getInstance().getProcessId(), serverListAsString.c_str()));

			if (ConfigServerGame::getTimeoutToAckUniverseDataReceived() > 0)
				m_timeUniverseDataSent = ::time(nullptr);
		}
	}
	else
	{
		if (!remoteProcesses.empty())
			IGNORE_RETURN(m_pendingProxyRequests.insert(remoteProcesses.begin(),remoteProcesses.end()));
	}
}

// ----------------------------------------------------------------------

void ServerUniverse::requestCreateProxiesOnServer(uint32 remoteProcess)
{
	IGNORE_RETURN(m_pendingProxyRequests.insert(remoteProcess));
}

// ----------------------------------------------------------------------

/**
 * Set which process should manage the universe objects, changing authority
 * if necessary.
 */
void ServerUniverse::setUniverseProcess(uint32 processId)
{
	m_universeProcess = processId;
	
	if (processId==GameServer::getInstance().getProcessId())
	{
		for (UniverseObjectList::iterator i=m_universeObjectList->begin(); i!=m_universeObjectList->end(); ++i)
		{
			ServerObject *obj=safe_cast<ServerObject*>((*i)->getObject());
			if (obj)
				obj->setAuthority();
		}

		m_authoritative=true;
		updateAndValidateData();
		m_nextCheckTimer.reset();

		// let all universe objects know that we have gained authority for universe objects
		for (UniverseObjectList::iterator j=m_universeObjectList->begin(); j!=m_universeObjectList->end(); ++j)
		{
			UniverseObject *obj=safe_cast<UniverseObject*>((*j)->getObject());
			if (obj)
				obj->onServerUniverseGainedAuthority();
		}
	}
	else
	{
		for (UniverseObjectList::iterator i=m_universeObjectList->begin(); i!=m_universeObjectList->end(); ++i)
		{
			ServerObject *obj=safe_cast<ServerObject*>((*i)->getObject());
			if (obj && obj->isAuthoritative())
				obj->releaseAuthority(processId);
		}

		m_authoritative=false;
	}
}

// ----------------------------------------------------------------------

void ServerUniverse::registerMasterGuildObject(GuildObject &masterGuildObject)
{
	m_masterGuildObject = &masterGuildObject;
}

// ----------------------------------------------------------------------

void ServerUniverse::registerMasterCityObject(CityObject &masterCityObject)
{
	m_masterCityObject = &masterCityObject;
}

// ----------------------------------------------------------------------

/** 
 * Create a PlanetObject for a planet that doesn't have one.
 */
PlanetObject *ServerUniverse::createPlanetObject(const std::string &planetName)
{
	WARNING_STRICT_FATAL(!m_authoritative,("Creating planet object on non-universe-authoritative server.\n"));
	
	DEBUG_REPORT_LOG(true,("Creating PlanetObject for %s\n",planetName.c_str()));
	PlanetObject * newObject = dynamic_cast<PlanetObject *>(ServerWorld::createNewObject(ConfigServerGame::getPlanetObjectTemplate(), Transform::identity, 0, true));
	DEBUG_FATAL(!newObject,("Attempted to create a PlanetObject, got something else.\n"));
	
	newObject->setPlanetName(planetName);
	registerPlanetObject (*newObject);
	newObject->addToWorld();

	return newObject;
}

// ----------------------------------------------------------------------

/**
 * Called when we have proxies of all universe objects.
 * Register the objects, set up relationships between them, etc.
 */

void ServerUniverse::universeComplete(uint32 sourceServer)
{
	// automatically set the universe process if it hasn't been set.  Don't change it if it has been set.  If it has already been set,
	// it probably indicates a server crash happened, and we're receiving queued data from a server that Central thinks is already down.
	// So we want to avoid changing m_universeProcess so as not to cause race conditions.
	if (m_universeProcess==0)
		m_universeProcess=sourceServer;
	
	LOG("UniverseLoading",("Game Server %lu got UniverseComplete message.",GameServer::getInstance().getProcessId()));

	m_loaded=true;
	{
		for (UniverseObjectList::iterator i=m_pendingUniverseObjects->begin(); i!=m_pendingUniverseObjects->end(); ++i)
		{
			UniverseObject *obj=safe_cast<UniverseObject*>((*i)->getObject());
			if (obj)
			{
				m_universeObjectList->push_back(*i);
				obj->setupUniverse();
			}
			else
			{
				DEBUG_REPORT_LOG(true,("Pending Universe Object %s was deleted.\n",(*i)->getValueString().c_str()));
				delete *i;
				*i=0;
			}
		}
	}

	m_pendingUniverseObjects->clear();

	// calculate the initial GCW percentile score from the depersisted GCW score data
	if (m_masterGuildObject && m_tatooinePlanet && m_masterGuildObject->isAuthoritative() && m_tatooinePlanet->isAuthoritative())
	{
		m_masterGuildObject->depersistGcwImperialScorePercentile();
	}
}

// ----------------------------------------------------------------------

/**
* Called when another game server has received its UniverseCompleteMessage
*/

void ServerUniverse::universeLoadedAck(uint32 sourceProcess)
{
	LOG("UniverseLoading", ("Game Server %lu got GameServerUniverseLoadedMessage from Game Server %lu.", GameServer::getInstance().getProcessId(), sourceProcess));
	IGNORE_RETURN(m_pendingUniverseLoadedAckList.erase(sourceProcess));
}

// ----------------------------------------------------------------------

GuildObject *ServerUniverse::getMasterGuildObject()
{
	return m_masterGuildObject;
}

// ----------------------------------------------------------------------

CityObject *ServerUniverse::getMasterCityObject()
{
	return m_masterCityObject;
}

// ----------------------------------------------------------------------

/**
 * Rename a resource type, if it is our map.
 * If it is not in the map, do nothing.
 */
void ServerUniverse::renameResourceType (const std::string &oldname, const std::string &newname)
{
	ResourceTypeNameMap::iterator i=m_resourceTypeNameMap->find(oldname);
	if (i!=m_resourceTypeNameMap->end())
	{
		ResourceTypeObject *theObj=(*i).second;
		m_resourceTypeNameMap->erase(i);
		(*m_resourceTypeNameMap)[newname]=theObj;
	}
}

// ----------------------------------------------------------------------

std::string ServerUniverse::generateRandomResourceName(const std::string &nameTable) const
{
	std::string newName(Unicode::wideToNarrow(NameManager::getInstance().generateRandomName("name_resource",nameTable))); // name generator always builds ASCII names, although it returns a Unicode string
	int failCount = 0;
	UNREF(failCount); // because of Windows release build 
	for(; getResourceTypeByName(newName) != nullptr; newName = Unicode::wideToNarrow(NameManager::getInstance().generateRandomName("name_resource",nameTable))) // generate names until we find an unused one
		DEBUG_FATAL(++failCount > 100,("Failed to generate an unused resource name in 100 tries.\n"));

	return newName; 
}

// ----------------------------------------------------------------------

void ServerUniverse::resourceClassTreeLoaded(std::map<int, ResourceClassObject*> &resourceClasses)
{
	ServerResourceClassObject::loadDistributionFromIff(resourceClasses);
}

// ----------------------------------------------------------------------

ResourceClassObject * ServerUniverse::makeResourceClassObject()
{
	return new ServerResourceClassObject;
}

// ----------------------------------------------------------------------

/** 
 * Check the Universe Objects.  Spawn new objects as
 * needed, update the amount remaining on all the resource pools, etc.
 *
 * May be time consuming, so this should not be called every frame.
 */

void ServerUniverse::updateAndValidateData()
{
	WARNING_STRICT_FATAL(!m_authoritative,("UpdateAndValidateData() should only be called on the process that is authoritative for UniverseObjects.\n"));

	//-- create master guild object
	if (!m_masterGuildObject)
	{
		FATAL(!ConfigServerGame::getAllowMasterObjectCreation(), ("Master guild object was not found!"));
		ServerObjectTemplate const * const objTemplate = safe_cast<ServerObjectTemplate const *>(ObjectTemplateList::fetch("object/guild/guild.iff"));
		m_masterGuildObject = safe_cast<GuildObject *>(ServerWorld::createNewObject(*objTemplate, Transform::identity, 0, true));
		m_masterGuildObject->addToWorld();
	}

	//-- create master city object
	if (!m_masterCityObject)
	{
		FATAL(!ConfigServerGame::getAllowMasterObjectCreation(), ("Master city object was not found!"));
		ServerObjectTemplate const * const objTemplate = safe_cast<ServerObjectTemplate const *>(ObjectTemplateList::fetch("object/city/city.iff"));
		m_masterCityObject = safe_cast<CityObject *>(ServerWorld::createNewObject(*objTemplate, Transform::identity, 0, true));
		m_masterCityObject->addToWorld();
	}

	//-- create planet objects if they don't exist
	{
		//-- load planet table
		Iff iff;
		if (iff.open ("universe/planets.iff", true))
		{
			iff.enterForm (TAG_PLTS);

				iff.enterForm (TAG_0000);

					const int numberOfPlanets = iff.getNumberOfBlocksLeft ();
					int i;
					for (i = 0; i < numberOfPlanets; ++i)
					{
						iff.enterChunk (TAG_PLNT);

							std::string planetName;
							iff.read_string (planetName);

							//-- create all planets
							if (!getPlanetByName (planetName))
							{
								FATAL(!ConfigServerGame::getAllowMasterObjectCreation(), ("Planet object %s was not found!", planetName.c_str()));
								IGNORE_RETURN (createPlanetObject (planetName));
							}

						iff.exitChunk (TAG_PLNT);
					}

				iff.exitForm (TAG_0000);

			iff.exitForm (TAG_PLTS);
		}
		else
			DEBUG_WARNING (true, ("universe/planets.iff not found"));
	}
	
	//-- create this planet, in case it's not in the table
	if (!m_thisPlanet)
	{
		FATAL(!ConfigServerGame::getAllowMasterObjectCreation(), ("Planet object %s was not found!", ServerWorld::getSceneId().c_str()));
		DEBUG_WARNING(true,("Spawning planet %s, which was not listed in planets.iff", ServerWorld::getSceneId().c_str()));
		IGNORE_RETURN(createPlanetObject(ServerWorld::getSceneId()));
	}
	
	//Spawn resources
	ServerResourceClassObject *root=safe_cast<ServerResourceClassObject*>(getResourceTreeRoot());
	if (root)
	{
		root->checkRanges();
		root->checkAttributeLimits();
		root->spawnAsNeeded();
	}

	//trigger scripts
	for (UniverseObjectList::iterator j = m_universeObjectList->begin(); j != m_universeObjectList->end(); ++j)
	{ 
		UniverseObject *obj=safe_cast<UniverseObject*>((*j)->getObject());
		if (obj)
		{
			ScriptParams params;
			IGNORE_RETURN(obj->getScriptObject()->trigAllScripts(Scripting::TRIG_UNIVERSE_COMPLETE, params));
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Picks a random resource type that is not depleted on this planet.
 * @parentResourceClassName The type must derive from this class
 */
ResourceTypeObject const * ServerUniverse::pickRandomNonDepletedResource(const std::string &parentResourceClassName) const
{
	ResourceClassObject *parentClass = getResourceClassByName(parentResourceClassName);
	if (!parentClass)
		return 0;
	
	std::vector<ResourceTypeObject const *> possibleTypes;
	if (!getCurrentPlanet())
		return 0;
	getCurrentPlanet()->getAvailableResourceList(possibleTypes,*parentClass);
	if (possibleTypes.size() == 0)
		return 0;
	int choice = Random::random(static_cast<int>(possibleTypes.size())-1);
	return possibleTypes[static_cast<size_t>(choice)];
}

// ----------------------------------------------------------------------

void ServerUniverse::createGroup(GroupMemberParam const & leader, std::vector<GroupMemberParam> const & members)
{
	if (m_authoritative)
		GroupObject::createGroup(leader, members);
	else
	{
		// send to central to be forwarded to universe authoritative server
		CreateGroupMessage const createMessage(leader, members);
		GameServer::getInstance().sendToCentralServer(createMessage);
	}
}

// ----------------------------------------------------------------------

void ServerUniverse::onServerConnectionClosed(uint32 processId)
{
	IGNORE_RETURN(m_pendingUniverseLoadedAckList.erase(processId));
	IGNORE_RETURN(m_pendingProxyRequests.erase(processId));

	if (m_authoritative)
	{
		// remove proxies on the crashed server
		for (UniverseObjectList::iterator i=m_universeObjectList->begin(); i!=m_universeObjectList->end(); ++i)
		{
			ServerObject *obj=safe_cast<ServerObject*>((*i)->getObject());
			if (obj)
				obj->removeServerFromProxyList(processId);
		}		
	}
}

// ----------------------------------------------------------------------

void ServerUniverse::updatePopulationList(const PopulationList &newData)
{
	m_populationList->update(newData);
}

// ----------------------------------------------------------------------

int ServerUniverse::getPopulationAtLocation(const std::string &scene, int x, int z) const
{
	if (ConfigServerGame::getDebugAllAreasOverpopulated())
		return ConfigServerGame::getMaxPopulationForNewbieTravel() * 2;
	else
		return (m_populationList->getNearestPopulation(scene, x, z));
}

// ----------------------------------------------------------------------

bool ServerUniverse::isUniverseLoaded() const
{
	return m_loaded;
}

// ----------------------------------------------------------------------

/**
 * Called once per frame.
 * Increment a counter, and revalidate the data if it has expired.
 */
void ServerUniverse::update(float frameTime)
{
	if (!m_pendingUniverseLoadedAckList.empty())
	{
		time_t const nowTime = ::time(nullptr);
		int const secondsSinceUniverseDataSent = (int)(nowTime - m_timeUniverseDataSent);

		// don't wait forever for ack from another game server
		// that it has received the universe data
		if (secondsSinceUniverseDataSent >= ConfigServerGame::getTimeoutToAckUniverseDataReceived())
		{
			std::string serverListAsString;
			char oneServer[10];
			for (PendingUniverseLoadedAckList::const_iterator i=m_pendingUniverseLoadedAckList.begin(); i!=m_pendingUniverseLoadedAckList.end(); ++i)
			{
				if (i!=m_pendingUniverseLoadedAckList.begin())
					serverListAsString+=", ";
				snprintf(oneServer,sizeof(oneServer)-1,"%lu",*i);
				oneServer[sizeof(oneServer)-1] = '\0';
				serverListAsString+=oneServer;
			}

			LOG("UniverseLoading", ("Game Server %lu hasn't received (and will stop waiting for) GameServerUniverseLoadedMessage in %d seconds from Game Servers %s.", GameServer::getInstance().getProcessId(), secondsSinceUniverseDataSent, serverListAsString.c_str()));
			m_pendingUniverseLoadedAckList.clear();
		}
		else // periodically log which game servers we are waiting on for GameServerUniverseLoadedMessage
		{
			static int const logIntervalSeconds = 60;
			static time_t lastLogTime = 0;

			if ((lastLogTime != nowTime) && (secondsSinceUniverseDataSent >= logIntervalSeconds) && ((nowTime - lastLogTime) >= logIntervalSeconds))
			{
				std::string serverListAsString;
				char oneServer[10];
				for (PendingUniverseLoadedAckList::const_iterator i=m_pendingUniverseLoadedAckList.begin(); i!=m_pendingUniverseLoadedAckList.end(); ++i)
				{
					if (i!=m_pendingUniverseLoadedAckList.begin())
						serverListAsString+=", ";
					snprintf(oneServer,sizeof(oneServer)-1,"%lu",*i);
					oneServer[sizeof(oneServer)-1] = '\0';
					serverListAsString+=oneServer;
				}

				LOG("UniverseLoading", ("Game Server %lu hasn't received (and will keep waiting for) GameServerUniverseLoadedMessage in %d seconds from Game Servers %s.", GameServer::getInstance().getProcessId(), secondsSinceUniverseDataSent, serverListAsString.c_str()));
				lastLogTime = nowTime;
			}
		}
	}

	// check to see if we need to send universe data to other game servers
	if (m_authoritative && m_loaded && !m_pendingProxyRequests.empty() && m_pendingUniverseLoadedAckList.empty())
	{
		std::vector<uint32> servers;
		PendingProxyRequestList::iterator iter;
		int maxGameServerToSendUniverseData = ConfigServerGame::getMaxGameServerToSendUniverseData();
		while ((maxGameServerToSendUniverseData > 0) && (!m_pendingProxyRequests.empty()))
		{
			iter = m_pendingProxyRequests.begin();
			servers.push_back(*iter);
			m_pendingProxyRequests.erase(iter);
			--maxGameServerToSendUniverseData;
		}

		if (!servers.empty())
			createProxiesOnServer(servers);
	}

	if (m_nextCheckTimer.updateZero(frameTime) || m_doImmediateCheck || !m_resourcesToSend->empty())
	{
		m_nextCheckTimer.reset();
		m_doImmediateCheck = false;
		
		if (m_authoritative)
		{
			updateAndValidateData();

			if (!m_resourcesToSend->empty())
			{
				std::vector<AddResourceTypeMessageNamespace::ResourceTypeData> resourceTypeDataVector;
				AddResourceTypeMessageNamespace::ResourceTypeData resourceTypeData;
				resourceTypeDataVector.reserve(m_resourcesToSend->size());
				
				for (ResourcesToSendType::const_iterator i=m_resourcesToSend->begin(); i!=m_resourcesToSend->end(); ++i)
				{
					NON_NULL(*i)->getDataForMessage(resourceTypeData);
					resourceTypeDataVector.push_back(resourceTypeData);
				}

				ServerMessageForwarding::beginBroadcast();

				AddResourceTypeMessage const addResourceTypeMessage(resourceTypeDataVector);
				GameServer::getInstance().sendToDatabaseServer(addResourceTypeMessage);
				ServerMessageForwarding::send(addResourceTypeMessage);

				ServerMessageForwarding::end();
			}

			m_resourcesToSend->clear();
		}
	}

	// request guild object to update its guild war kill tracking data, if necessary
	if (m_masterGuildObject)
		m_masterGuildObject->updateGuildWarKillTrackingData();

	// update GCW tracking data
	if (m_tatooinePlanet)
		m_tatooinePlanet->updateGcwTrackingData();
}

// ----------------------------------------------------------------------

void ServerUniverse::handleAddResourceTypeMessage(AddResourceTypeMessage const & message)
{
	FATAL(isAuthoritative(),("Programmer bug:  Received AddResourceTypeMessage on the authoritative server"));
	typedef std::vector<AddResourceTypeMessageNamespace::ResourceTypeData> DataVectorType;
	DataVectorType const & data = message.getData();

	for (DataVectorType::const_iterator i=data.begin(); i!=data.end(); ++i)
	{
		ServerResourceClassObject * const parentClass = safe_cast<ServerResourceClassObject *>(getResourceClassByName(i->m_parentClass));
		if (parentClass)
		{
			ResourceTypeObject * const existingType = getResourceTypeById(i->m_networkId);
			if (!existingType)
			{
				ResourceTypeObject * const newType = new ResourceTypeObject(*i, *parentClass);
				registerResourceTypeObject(*newType, false);
				parentClass->addType(*newType);
			} //lint !e429 // didn't delete newType (registerResourceTypeObject keeps a pointer to it)
			else
			{
				// update to existing type, or possibly a duplicate message (if a type spawns the same frame another servers connects, the message may be sent twice)
				existingType->handlePossibleUpdates(*i);
			}
		}
		else
			WARNING(true,("Got message to add resource %s, derived from class %s.  The class could not be found.", i->m_name.c_str(), i->m_parentClass.c_str()));
	}
}

// ----------------------------------------------------------------------

const NetworkId & ServerUniverse::findTheaterId(const std::string & name)
{
	if (m_theaterNameIdMap == nullptr)
		return NetworkId::cms_invalid;

	TheaterNameIdMap::const_iterator result = m_theaterNameIdMap->find(name);
	if (result == m_theaterNameIdMap->end())
		return NetworkId::cms_invalid;

	return (*result).second;
}

// ----------------------------------------------------------------------

const std::string & ServerUniverse::findTheaterName(const NetworkId & id)
{
static const std::string emptyString;

	if (m_theaterIdNameMap == nullptr)
		return emptyString;

	TheaterIdNameMap::const_iterator result = m_theaterIdNameMap->find(id);
	if (result == m_theaterIdNameMap->end())
		return emptyString;

	return (*result).second;
}

// ----------------------------------------------------------------------

bool ServerUniverse::setTheater(const std::string & name, const NetworkId & id)
{
	bool result = remoteSetTheater(name, id);
	if (result)
	{
		// tell the other servers to set the theater

		ServerMessageForwarding::beginBroadcast();

		GenericValueTypeMessage<std::pair<std::string, NetworkId> > const setTheaterMessage("SetTheaterMessage", std::make_pair(name, id));
		ServerMessageForwarding::send(setTheaterMessage);

		ServerMessageForwarding::end();
	}
	return result;
}

// ----------------------------------------------------------------------

bool ServerUniverse::remoteSetTheater(const std::string & name, const NetworkId & id)
{
	const NetworkId & testTheater = findTheaterId(name);
	if (testTheater != NetworkId::cms_invalid)
	{
		WARNING(true, ("ServerUniverse::remoteSetTheater tried to set theater name %s, id %s when the name is being used by %s",
			name.c_str(), id.getValueString().c_str(), 
			testTheater.getValueString().c_str()));
		return false;
	}

	if (m_theaterNameIdMap != nullptr)
		m_theaterNameIdMap->insert(std::make_pair(name, id));
	if (m_theaterIdNameMap != nullptr)
		m_theaterIdNameMap->insert(std::make_pair(id, name));
	return true;
}

// ----------------------------------------------------------------------

void ServerUniverse::clearTheater(const std::string & name)
{
	remoteClearTheater(name);

	// tell the other servers to clear the theater

	ServerMessageForwarding::beginBroadcast();

	GenericValueTypeMessage<std::string> const clearTheaterMessage("ClearTheaterMessage", name);
	ServerMessageForwarding::send(clearTheaterMessage);

	ServerMessageForwarding::end();
}

// ----------------------------------------------------------------------

void ServerUniverse::remoteClearTheater(const std::string & name)
{
	const NetworkId & id = findTheaterId(name);

	if (m_theaterNameIdMap != nullptr)
		m_theaterNameIdMap->erase(name);
	if (m_theaterIdNameMap != nullptr)
		m_theaterIdNameMap->erase(id);
}

// ----------------------------------------------------------------------

void ServerUniverse::adjustGcwImperialScore(std::string const & source, CreatureObject * sourceObject, std::string const & gcwCategory, int adjustment)
{
	if (m_tatooinePlanet)
		m_tatooinePlanet->adjustGcwImperialScore(source, sourceObject, gcwCategory, static_cast<int64>(adjustment));
}

// ----------------------------------------------------------------------

void ServerUniverse::adjustGcwRebelScore(std::string const & source, CreatureObject * sourceObject, std::string const & gcwCategory, int adjustment)
{
	if (m_tatooinePlanet)
		m_tatooinePlanet->adjustGcwRebelScore(source, sourceObject, gcwCategory, static_cast<int64>(adjustment));
}

// ----------------------------------------------------------------------
// the rebel score is 100 - the imperial score
int ServerUniverse::getGcwImperialScorePercentile(std::string const & gcwCategory) const
{
	if (m_masterGuildObject)
		return m_masterGuildObject->getGcwImperialScorePercentile(gcwCategory);

	return 50; // tie
}

// ----------------------------------------------------------------------
// the rebel score is 100 - the imperial score
int ServerUniverse::getGcwGroupImperialScorePercentile(std::string const & gcwGroup) const
{
	if (m_masterGuildObject)
		return m_masterGuildObject->getGcwGroupImperialScorePercentile(gcwGroup);

	return 50; // tie
}

// ----------------------------------------------------------------------

/**
 * Force a resource to be depleted (used by console or god commands).
 * Forwards a message to the authoritative server if necessary.
 * @return false if the resource type does not exist.
 */
bool ServerUniverse::manualDepleteResource(NetworkId const & resourceToDeplete)
{
	ResourceTypeObject * const resType=getResourceTypeById(resourceToDeplete);
	if (resType)
	{
		if (isAuthoritative())
		{
			resType->deplete();
			ServerResourceClassObject *root=safe_cast<ServerResourceClassObject*>(getResourceTreeRoot());
			if (root)
				root->spawnAsNeeded();
			m_doImmediateCheck=true;
		}
		else
		{
			ServerMessageForwarding::begin(getUniverseProcess());

			GenericValueTypeMessage<NetworkId> const manualDepleteResourceMessage("ManualDepleteResourceMessage", resourceToDeplete);
			ServerMessageForwarding::send(manualDepleteResourceMessage);

			ServerMessageForwarding::end();
		}
		return true;
	}
	else
		return false;
}

// ----------------------------------------------------------------------

uint32 ServerUniverse::getUniverseProcess() const
{
	return m_universeProcess;
}

// ----------------------------------------------------------------------

void ServerUniverse::setConnectedCharacterLfgData(const NetworkId & characterId, const LfgCharacterData & lfgCharacterData)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->setConnectedCharacterLfgData(characterId, lfgCharacterData);
}

// ----------------------------------------------------------------------

std::map<NetworkId, LfgCharacterData> const & ServerUniverse::getConnectedCharacterLfgData()
{
	static std::map<NetworkId, LfgCharacterData> empty;

	PlanetObject const * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		return planetObject->getConnectedCharacterLfgData();

	return empty;
}

// ----------------------------------------------------------------------

void ServerUniverse::setConnectedCharacterGroupData(const NetworkId & characterId, const NetworkId & groupId)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->setConnectedCharacterGroupData(characterId, groupId);
}

// ----------------------------------------------------------------------

void ServerUniverse::setConnectedCharacterPlanetData(const NetworkId & characterId, const std::string & planetName)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->setConnectedCharacterPlanetData(characterId, planetName);
}

// ----------------------------------------------------------------------

void ServerUniverse::setConnectedCharacterRegionData(const NetworkId & characterId, const std::string & regionName)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->setConnectedCharacterRegionData(characterId, regionName);
}

// ----------------------------------------------------------------------

void ServerUniverse::setConnectedCharacterPlayerCityData(const NetworkId & characterId, const std::string & playerCityName)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->setConnectedCharacterPlayerCityData(characterId, playerCityName);
}

// ----------------------------------------------------------------------

void ServerUniverse::setConnectedCharacterLevelData(const NetworkId & characterId, int16 level)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->setConnectedCharacterLevelData(characterId, level);
}

// ----------------------------------------------------------------------

void ServerUniverse::setConnectedCharacterProfessionData(const NetworkId & characterId, LfgCharacterData::Profession profession)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->setConnectedCharacterProfessionData(characterId, profession);
}

// ----------------------------------------------------------------------

void ServerUniverse::setConnectedCharacterSearchableByCtsSourceGalaxyData(const NetworkId & characterId, bool searchableByCtsSourceGalaxy)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->setConnectedCharacterSearchableByCtsSourceGalaxyData(characterId, searchableByCtsSourceGalaxy);
}

// ----------------------------------------------------------------------

void ServerUniverse::setConnectedCharacterDisplayLocationInSearchResultsData(const NetworkId & characterId, bool displayLocationInSearchResults)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->setConnectedCharacterDisplayLocationInSearchResultsData(characterId, displayLocationInSearchResults);
}

// ----------------------------------------------------------------------

void ServerUniverse::setConnectedCharacterAnonymousData(const NetworkId & characterId, bool anonymous)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->setConnectedCharacterAnonymousData(characterId, anonymous);
}

// ----------------------------------------------------------------------

void ServerUniverse::setConnectedCharacterInterestsData(const NetworkId & characterId, BitArray const & interests)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->setConnectedCharacterInterestsData(characterId, interests);
}

// ----------------------------------------------------------------------

void ServerUniverse::setConnectedCharacterBiographyData(const NetworkId & characterId, const Unicode::String & biography)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->setConnectedCharacterBiographyData(characterId, biography);
}

// ----------------------------------------------------------------------

void ServerUniverse::removeConnectedCharacterBiographyData(const NetworkId & characterId)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->removeConnectedCharacterBiographyData(characterId);
}

// ----------------------------------------------------------------------

std::map<NetworkId, Unicode::String> const & ServerUniverse::getConnectedCharacterBiographyData()
{
	static std::map<NetworkId, Unicode::String> empty;

	PlanetObject const * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		return planetObject->getConnectedCharacterBiographyData();

	return empty;
}

// ----------------------------------------------------------------------

void ServerUniverse::removeConnectedCharacterData(const NetworkId & characterId)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	if (planetObject)
		planetObject->removeConnectedCharacterData(characterId);
}

// ----------------------------------------------------------------------

void ServerUniverse::addUniverseEvent(std::string const & eventName)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	
	if (planetObject)
		planetObject->addUniverseEvent(eventName);
}

// ----------------------------------------------------------------------

void ServerUniverse::removeUniverseEvent(std::string const & eventName)
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	
	if (planetObject)
		planetObject->removeUniverseEvent(eventName);
}

// ----------------------------------------------------------------------

std::string const & ServerUniverse::getCurrentEvents()
{
	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	
	if (planetObject)
		return planetObject->getCurrentEvents();
	
	return s_emptyString;
}

// ======================================================================
