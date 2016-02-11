// ======================================================================
//
// ObjvarNameManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "ObjvarNameManager.h"

#include "SwgDatabaseServer/TaskSaveObjvarNames.h"
#include "sharedFoundation/ExitChain.h"
#include <algorithm>
#include <unordered_map>
#include <string>
#include <vector>

// ======================================================================

ObjvarNameManager * ObjvarNameManager::ms_instance = nullptr;
ObjvarNameManager * ObjvarNameManager::ms_goldInstance = nullptr;

// ======================================================================

void ObjvarNameManager::install()
{
	DEBUG_FATAL(ms_instance,("Installed ObjvarNameManager twice.\n"));
	ExitChain::add(remove,"ObjvarNameManager::remove");

	ms_instance = new ObjvarNameManager;
	ms_goldInstance = new ObjvarNameManager;
}

// ----------------------------------------------------------------------

void ObjvarNameManager::remove()
{
	NOT_NULL(ms_instance);
	delete ms_instance;
	ms_instance = nullptr;

	NOT_NULL(ms_goldInstance);
	delete ms_goldInstance;
	ms_goldInstance = nullptr;
}

// ----------------------------------------------------------------------

ObjvarNameManager::ObjvarNameManager() :
		m_nameToIdMap(new NameToIdMapType),
		m_idToNameMap(new IdToNameMapType),
		m_newNames(new NewNamesType),
		m_nextAvailableId(0),
		m_namesLoaded(false)
{
}

// ----------------------------------------------------------------------

ObjvarNameManager::~ObjvarNameManager()
{
	DEBUG_FATAL(!m_newNames->empty(),("Database server was shut down without allowing all object variable names to be saved."));
	
	delete m_nameToIdMap;
	delete m_idToNameMap;
	delete m_newNames;
	m_nameToIdMap=nullptr;
	m_idToNameMap=nullptr;
	m_newNames=nullptr;
}

// ----------------------------------------------------------------------

int ObjvarNameManager::getNameId(const std::string &name) const
{
	DEBUG_FATAL(!m_namesLoaded,("Programmer bug:  Attempted to look up object variable names before all were loaded."));

	NameToIdMapType::const_iterator i=m_nameToIdMap->find(name);
	if (i!=m_nameToIdMap->end())
		return i->second;
	else
		return 0;
}

// ----------------------------------------------------------------------

/**
 * Lookup a name in the map.  Add the name if it isn't already in use.
 */
int ObjvarNameManager::getOrAddNameId (const std::string &name)
{
	DEBUG_FATAL(!m_namesLoaded,("Programmer bug:  Attempted to look up object variable names before all were loaded."));

	int id=getNameId(name);
	if (id!=0)
		return id;
	else
	{
		id = m_nextAvailableId++;
		(*m_nameToIdMap)[name]=id;
		(*m_idToNameMap)[id]=name;
		m_newNames->push_back(std::make_pair(id, name));		
		return id;
	}
}

// ----------------------------------------------------------------------

/**
 * Lookup a name in the map.
 */
bool ObjvarNameManager::getName(int id, std::string &name) const
{
	DEBUG_FATAL(!m_namesLoaded,("Programmer bug:  Attempted to look up object variable names before all were loaded."));

	IdToNameMapType::const_iterator i=m_idToNameMap->find(id);
	if (i!=m_idToNameMap->end())
	{
		name = i->second;
		return true;
	}
	else
		return false;
}

// ----------------------------------------------------------------------

/**
 * Add a name to the map, e.g. when loaded from the database
 */
void ObjvarNameManager::addNameMapping (int id, const std::string &name)
{
	NameToIdMapType::iterator i=m_nameToIdMap->find(name);
	if (i!=m_nameToIdMap->end())
	{
		int lower=std::min(id, i->second);
		DEBUG_REPORT_LOG(true,("Objvar name %s is duplicated in the database.  Ids are %i and %i.  Using %i for this name\n", name.c_str(), id, i->second, lower));
		i->second = lower;
	}
	else
		(*m_nameToIdMap)[name]=id;
	
	(*m_idToNameMap)[id]=name;
	
	if (id >= m_nextAvailableId)
		m_nextAvailableId = id+1;
}

// ----------------------------------------------------------------------

void ObjvarNameManager::allNamesLoaded()
{
	m_namesLoaded=true;
	if (m_nextAvailableId==0)
		m_nextAvailableId=1; // start with 1 if there are no names in the database.  0 is reserved for "invalid name"
}

// ----------------------------------------------------------------------

/**
 * Make a CustomPersistStep to save all the new names to the database.
 */
DB::TaskRequest *ObjvarNameManager::saveNewNames()
{
	if (!m_newNames->empty())
	{
		DB::TaskRequest *task = new TaskSaveObjvarNames(*m_newNames);
		m_newNames->clear();
		return task;
	}
	else
		return nullptr;
}

// ======================================================================
