// ======================================================================
//
// ObjectTemplateList.cpp
// copyright 1999 Bootprint Entertainment
// copyright 2000 sonny online entertainment
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/ObjectTemplateList.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/TreeFile.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/Object.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcStringTable.h"
#include "sharedFoundation/DataResourceList.h"

#include <vector>

// ======================================================================

typedef DataResourceList<ObjectTemplate>          ObjectTemplateListDataResourceList;

template <> ObjectTemplateListDataResourceList::CreateDataResourceMap *ObjectTemplateListDataResourceList::ms_bindings = nullptr;
template <> ObjectTemplateListDataResourceList::LoadedDataResourceMap *ObjectTemplateListDataResourceList::ms_loaded = nullptr;

namespace ObjectTemplateListNamespace
{
	bool           ms_logLoadedObjectTemplates;
	bool ms_logFetch;
	CrcStringTable ms_crcStringTable;
}
using namespace ObjectTemplateListNamespace;

// ======================================================================

void ObjectTemplateList::install(bool const loadObjectTemplateCrcStringTable)
{
	InstallTimer const installTimer("ObjectTemplateList::install");

	ObjectTemplateListDataResourceList::install();

	if (loadObjectTemplateCrcStringTable)
		ms_crcStringTable.load("misc/object_template_crc_string_table.iff");

	DebugFlags::registerFlag(ms_logLoadedObjectTemplates, "SharedObject/ObjectTemplateList", "logLoadedObjectTemplates", logLoadedObjectTemplates);
	DebugFlags::registerFlag(ms_logFetch, "SharedObject/ObjectTemplateList", "logFetch");
}	

// ----------------------------------------------------------------------

void ObjectTemplateList::remove()
{
	DebugFlags::unregisterFlag(ms_logLoadedObjectTemplates);
	DebugFlags::unregisterFlag(ms_logFetch);

	ObjectTemplateListDataResourceList::remove();
}	

// ----------------------------------------------------------------------

void ObjectTemplateList::registerTemplate(Tag id, CreateDataResourceFunc createFunc)
{
	ObjectTemplateListDataResourceList::registerTemplate(id, createFunc);
}	

// ----------------------------------------------------------------------

ObjectTemplateList::CreateDataResourceFunc ObjectTemplateList::assignBinding(Tag id, CreateDataResourceFunc createFunc)
{
	return ObjectTemplateListDataResourceList::assignBinding(id, createFunc);
}	

// ----------------------------------------------------------------------

ObjectTemplateList::CreateDataResourceFunc ObjectTemplateList::removeBinding(Tag id)
{
	return ObjectTemplateListDataResourceList::removeBinding(id);
}	

// ----------------------------------------------------------------------

void ObjectTemplateList::logLoadedObjectTemplates()
{
	ObjectTemplateListDataResourceList::logLoadedResources("ObjectTemplates");
	ms_logLoadedObjectTemplates = false;
}

// ----------------------------------------------------------------------

const ObjectTemplate *ObjectTemplateList::fetch(const std::string &filename)
{
	DEBUG_REPORT_LOG(ms_logFetch, ("[fetch] ObjectTemplateList::fetch: %s\n", filename.c_str()));
	return ObjectTemplateListDataResourceList::fetch(filename);
}	

// ----------------------------------------------------------------------

const ObjectTemplate *ObjectTemplateList::fetch(const char *filename)
{
	DEBUG_REPORT_LOG(ms_logFetch, ("[fetch] ObjectTemplateList::fetch: %s\n", filename));
	return ObjectTemplateListDataResourceList::fetch(filename);
}	

// ----------------------------------------------------------------------

const ObjectTemplate *ObjectTemplateList::fetch(Iff &source)
{
	return ObjectTemplateListDataResourceList::fetch(source);
}	

// ----------------------------------------------------------------------

const ObjectTemplate *ObjectTemplateList::fetch(const CrcString &filename)
{
	DEBUG_REPORT_LOG(ms_logFetch, ("[fetch] ObjectTemplateList::fetch: %s\n", filename.getString()));
	return ObjectTemplateListDataResourceList::fetch(filename);
}	

// ----------------------------------------------------------------------

const ObjectTemplate *ObjectTemplateList::fetch(uint32 crc)
{
	return ObjectTemplateListDataResourceList::fetch(lookUp(crc));
}

// ----------------------------------------------------------------------

Object *ObjectTemplateList::createObject (const CrcString &fileName)
{
	const ObjectTemplate *objectTemplate = fetch(fileName.getString());
	if (!objectTemplate)
		return 0;

	Object* const object = objectTemplate->createObject();
	objectTemplate->releaseReference();

	return object;
}

// ----------------------------------------------------------------------

void ObjectTemplateList::release(const ObjectTemplate &dataResource)
{
	ObjectTemplateListDataResourceList::release(dataResource);
}	

// ----------------------------------------------------------------------

ObjectTemplate *ObjectTemplateList::reload(Iff &source)
{
	return ObjectTemplateListDataResourceList::reload(source);
}	

//-----------------------------------------------------------------------

bool ObjectTemplateList::isLoaded(const std::string &fileName)
{
	return ObjectTemplateListDataResourceList::isLoaded(fileName);
}

//----------------------------------------------------------------------

void ObjectTemplateList::loadCrcStringTable(const char *fileName)
{
	ms_crcStringTable.load(fileName);
}

//----------------------------------------------------------------------

ConstCharCrcString const ObjectTemplateList::lookUp(const char * string)
{
	ConstCharCrcString result = ms_crcStringTable.lookUp(string);
	WARNING(result.isEmpty(), ("ObjectTemplateList::lookUp objectTemplate %s not found in table", string));
	return result;
}

//----------------------------------------------------------------------

ConstCharCrcString const ObjectTemplateList::lookUp(uint32 crc)
{
	return ms_crcStringTable.lookUp(crc);
}

//----------------------------------------------------------------------

void ObjectTemplateList::garbageCollect ()
{
	ObjectTemplateListDataResourceList::garbageCollect ();
}

//----------------------------------------------------------------------

void ObjectTemplateList::getAllTemplateNamesFromCrcStringTable(std::vector<const char *> & objectTemplateNames)
{
	ms_crcStringTable.getAllStrings(objectTemplateNames);
}

//----------------------------------------------------------------------
