// NetworkIdManager.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/NetworkIdManager.h"

#include "sharedFoundation/NetworkId.h"
#include "sharedObject/Object.h"

#include <unordered_map>
#include <string>

//-----------------------------------------------------------------------

NetworkIdManager NetworkIdManager::ms_instance;
bool NetworkIdManager::ms_reportObjectLeaks = true;


//----------------------------------------------------------------------

NetworkIdManager::NetworkIdManager() :
m_objectHashMap(new NetworkIdObjectHashMap)
{
	NOT_NULL(m_objectHashMap);
}

//-----------------------------------------------------------------------

NetworkIdManager::~NetworkIdManager()
{
	NOT_NULL(m_objectHashMap);

	if (ms_reportObjectLeaks)
	{
		NetworkIdObjectHashMap::iterator i = m_objectHashMap->begin();
		for (; i != m_objectHashMap->end(); ++i)
		{
			DEBUG_WARNING(true, ("Object %s wasn't removed cleanly from the game\n", i->second->getDebugInformation(true).c_str()));
//			delete i->second;
		}
	}

	m_objectHashMap->clear();
	delete m_objectHashMap;
}

//-----------------------------------------------------------------------

NetworkIdManager & NetworkIdManager::getInstance ()
{
	return ms_instance;
}

//-----------------------------------------------------------------------

void NetworkIdManager::addObject(Object & sourceObject)
{
	static NetworkIdManager & instance = getInstance();
	static NetworkIdObjectHashMap & map = *instance.m_objectHashMap;
	if(sourceObject.getNetworkId() != NetworkId::cms_invalid)
	{
		WARNING_STRICT_FATAL(NetworkIdManager::getObjectById(sourceObject.getNetworkId()), ("Cannot add object with id %s because one already exists", sourceObject.getNetworkId().getValueString().c_str()));
		IGNORE_RETURN(map.insert(std::pair<NetworkId, Object *>(sourceObject.getNetworkId(), &sourceObject)));
	}
} //lint !e1764 // sourceObject could be declared const ref // No, I need a non-const pointer to it.

//-----------------------------------------------------------------------

Object * NetworkIdManager::getObjectById(const NetworkId & source)
{
	static NetworkIdManager & instance = getInstance();
	static NetworkIdObjectHashMap & map = *instance.m_objectHashMap;
	NetworkIdObjectHashMap::iterator i = map.find(source);
	if(i != map.end())
		return (*i).second;
	return 0;
}

//-----------------------------------------------------------------------

void NetworkIdManager::removeObject(const Object & sourceObject)
{
	static NetworkIdManager & instance = getInstance();
	static NetworkIdObjectHashMap & map = *instance.m_objectHashMap;

	if(sourceObject.getNetworkId() != NetworkId::cms_invalid)
		map.erase(map.find(sourceObject.getNetworkId()));
}

//-----------------------------------------------------------------------

NetworkIdManager::NetworkIdObjectHashMap const & NetworkIdManager::getAllObjects()
{
	static NetworkIdManager & instance = getInstance();
	static NetworkIdObjectHashMap & map = *instance.m_objectHashMap;

	return map;
}

//-----------------------------------------------------------------------
