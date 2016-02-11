// ======================================================================
//
// ObjectTemplate.cpp - This is a scaled-down version of ObjectTemplate from 
// sharedObject library in order to avoid nasty dependencies.
//
// jeff grills
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#include "sharedTemplateDefinition/FirstSharedTemplateDefinition.h"
#include "ObjectTemplate.h"
#include "sharedFile/Iff.h"


//-----------------------------------------------------------------

template <> ObjectTemplateList::CreateDataResourceMap *ObjectTemplateList::ms_bindings = nullptr;
template <> ObjectTemplateList::LoadedDataResourceMap *ObjectTemplateList::ms_loaded = nullptr;

// ======================================================================

/**
 * Construct an ObjectTemplate.
 * 
 * @param newName  name of the ObjectTemplate
 *
 */
ObjectTemplate::ObjectTemplate(const std::string & filename) : 
	DataResource(filename.c_str()),
	m_baseData(0)
{
}	// ObjectTemplate::ObjectTemplate

// ----------------------------------------------------------------------
/**
 * Destroy an ObjectTemplate.
 */

ObjectTemplate::~ObjectTemplate(void)
{
	if (m_baseData != nullptr)
	{
		m_baseData->releaseReference();
		m_baseData = nullptr;
	}
}

// ----------------------------------------------------------------------

void ObjectTemplate::load(Iff &iff)
{
	UNREF (iff);
	DEBUG_FATAL (true, ("this function should not be called"));
}

// ----------------------------------------------------------------------

/**
 * Adds a reference on this template and also adds a reference on the 
 * template's base.
 */
void ObjectTemplate::addReference(void) const
{
	if (m_baseData != nullptr)
		m_baseData->addReference();

	DataResource::addReference();
}	// ObjectTemplate::addReference

// ----------------------------------------------------------------------
/**
 * Releases a reference on this template and also releases a reference on the 
 * template's base.
 */
void ObjectTemplate::releaseReference(void) const
{
	if (m_baseData != nullptr)
		m_baseData->releaseReference();
	
	DataResource::releaseReference();
}	// ObjectTemplate::releaseReference


// ======================================================================
