//===================================================================
//
// Universe.cpp
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/Universe.h"

#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/ResourceClassObject.h"

#include <map>

// ======================================================================

Universe * Universe::ms_theInstance = nullptr;
bool       Universe::ms_installed   = false;

//===================================================================

void Universe::installDerived(Universe *derivedInstance)
{
	DEBUG_FATAL(ms_installed || ms_theInstance!=nullptr,("Installed Universe twice.\n"));
	ms_theInstance = derivedInstance;
	ms_installed = true;

	ExitChain::add(&remove, "Universe::remove");
}

// ----------------------------------------------------------------------

void Universe::remove()
{
	DEBUG_FATAL(!ms_installed,("Universe was not installed.\n"));
	delete ms_theInstance;
	ms_theInstance = 0;
}

Universe::Universe() :
	m_resourceClassNameMap (new ResourceClassNameMap),
	m_resourceClassNameCrcMap (new ResourceClassNameCrcMap),
	m_resourceTreeRoot     (nullptr)
{
	ResourceClassObject::install(); // sets up some static strings used by the import process
}

// ----------------------------------------------------------------------

Universe::~Universe()
{
	for (ResourceClassNameMap::iterator i=m_resourceClassNameMap->begin(); i!=m_resourceClassNameMap->end(); ++i)
	{
		delete i->second;
	}
	delete m_resourceClassNameMap;
	delete m_resourceClassNameCrcMap;

	m_resourceClassNameMap = 0;
	m_resourceClassNameCrcMap = 0;
	m_resourceTreeRoot     = 0;
}

// ----------------------------------------------------------------------

ResourceClassObject *Universe::getResourceClassByName(const std::string &name) const
{
	ResourceClassNameMap::iterator i=m_resourceClassNameMap->find(name);
	if (i!=m_resourceClassNameMap->end())
		return (*i).second;
	else
		return 0;
}

// ----------------------------------------------------------------------

ResourceClassObject *Universe::getResourceClassByNameCrc(uint32 nameCrc) const
{
	ResourceClassNameCrcMap::iterator i=m_resourceClassNameCrcMap->find(nameCrc);
	if (i!=m_resourceClassNameCrcMap->end())
		return (*i).second;
	else
		return 0;
}

// ----------------------------------------------------------------------

/** 
 * Registers a new ResourceClassObject
 * Puts the object into a map of name->object, so that it can be found later.
 */

void Universe::registerResourceClassObject (ResourceClassObject &newResourceClass)
{
	if (newResourceClass.isRoot())
		m_resourceTreeRoot=&newResourceClass;
	ResourceClassNameMap::iterator i=m_resourceClassNameMap->find(newResourceClass.getResourceClassName());
	if (i!=m_resourceClassNameMap->end())
		DEBUG_WARNING(true,("RESOURCE TREE ERROR:  Resource class %s appears multiple times in the hierarchy.  Probably there is a problem with the resource data table.  This may cause new pools to get spawned when they shouldn't.\n",newResourceClass.getResourceClassName().c_str()));
	else
	{
		(*m_resourceClassNameMap)[newResourceClass.getResourceClassName()]=&newResourceClass;
		(*m_resourceClassNameCrcMap)[Crc::calculate(newResourceClass.getResourceClassName().c_str())]=&newResourceClass;
	}
}

//----------------------------------------------------------------------

bool Universe::derivesFromResource         (const std::string & base, const std::string & leaf) const
{
	const ResourceClassObject * const baseClass = getResourceClassByName (base);
	const ResourceClassObject *       leafClass = getResourceClassByName (leaf);

	WARNING (!baseClass, ("Universe::derivesFromResource base class [%s] unknown", base.c_str ()));
	WARNING (!leafClass, ("Universe::derivesFromResource leaf class [%s] unknown", leaf.c_str ()));

	if (!baseClass || !leafClass)
		return false;

	while (leafClass != 0)
	{
		if (leafClass == baseClass)
			return true;

		leafClass = leafClass->getParent ();
	}

	return false;
}

//===================================================================
