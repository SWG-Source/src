// ======================================================================
//
// ObjectTemplate.cpp
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/ObjectTemplate.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplateList.h"

#include <string>

// ======================================================================

namespace ObjectTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	char  s_crashReportInfoConstructor[MAX_PATH * 2];
	char  s_crashReportInfoLoadFromIff[MAX_PATH * 2];
}

using namespace ObjectTemplateNamespace;

// ======================================================================

void ObjectTemplateNamespace::remove()
{
	CrashReportInformation::removeDynamicText(s_crashReportInfoConstructor);
	CrashReportInformation::removeDynamicText(s_crashReportInfoLoadFromIff);
}

// ======================================================================

void ObjectTemplate::install()
{
	s_crashReportInfoConstructor[0] = '\0';
	CrashReportInformation::addDynamicText(s_crashReportInfoConstructor);

	s_crashReportInfoLoadFromIff[0] = '\0';
	CrashReportInformation::addDynamicText(s_crashReportInfoLoadFromIff);

	ExitChain::add(remove, "ObjectTemplate");
}

// ======================================================================
/**
 * Construct an ObjectTemplate.
 * 
 * @param newName  name of the ObjectTemplate
 *
 */
ObjectTemplate::ObjectTemplate(const std::string & filename) : 
	DataResource(filename.c_str ()),
	m_baseData(0)
{
	//-- Track name of most recently loading object template name to give us more 
	//   data to work with when we receive crash info.
	IGNORE_RETURN(snprintf(s_crashReportInfoConstructor, sizeof(s_crashReportInfoConstructor) - 1, "ObjectTemplate_Constructor: %s\n", !filename.empty() ? filename.c_str() : "<nullptr>"));
	s_crashReportInfoConstructor[sizeof(s_crashReportInfoConstructor) - 1] = '\0';
}

// ----------------------------------------------------------------------
/**
 * Destroy an ObjectTemplate.
 */

ObjectTemplate::~ObjectTemplate()
{
	if (m_baseData != 0)
	{
		m_baseData->releaseReference();
		m_baseData = 0;
	}
}

// ----------------------------------------------------------------------

ServerObjectTemplate * ObjectTemplate::asServerObjectTemplate()
{
	return 0;
}

// ----------------------------------------------------------------------

ServerObjectTemplate const * ObjectTemplate::asServerObjectTemplate() const
{
	return 0;
}

// ----------------------------------------------------------------------

SharedObjectTemplate * ObjectTemplate::asSharedObjectTemplate()
{
	return 0;
}

// ----------------------------------------------------------------------

SharedObjectTemplate const * ObjectTemplate::asSharedObjectTemplate() const
{
	return 0;
}

// ----------------------------------------------------------------------

void ObjectTemplate::load(Iff & /*iff*/)
{
	DEBUG_FATAL (true, ("ObjectTemplate::load [%s] - this function should not be called", getName()));
}

// ----------------------------------------------------------------------
/**
 * Creates an object from an iff file.
 *
 * @param iffFilename		file name of iff file to read
 *
 * @return the new object
 */
Object *ObjectTemplate::createObject(const char *const fileName)
{
	const ObjectTemplate *const objectTemplate = ObjectTemplateList::fetch(fileName);
	if (objectTemplate)
	{
		Object *const object = objectTemplate->createObject();
		objectTemplate->releaseReference ();

		return object;
	}

	return 0;
}

// ----------------------------------------------------------------------
/**
 * Create a unique Object for this ObjectTemplate.
 * 
 * This routine should be overloaded in derived classes as appropriate.
 * 
 * Derived classes should not chain down to this routine.
 * 
 * @return An Object for the specified ObjectTemplate
 */

Object *ObjectTemplate::createObject() const
{
	return new Object(this, NetworkId::cms_invalid);
}

// ----------------------------------------------------------------------

/**
 * Adds a reference on this template and also adds a reference on the 
 * template's base.
 */
void ObjectTemplate::addReference() const
{
	if (m_baseData != nullptr)
		m_baseData->addReference();

	DataResource::addReference();
}

// ----------------------------------------------------------------------
/**
 * Releases a reference on this template and also releases a reference on the 
 * template's base.
 */
void ObjectTemplate::releaseReference() const
{
	if (m_baseData != nullptr)
		m_baseData->releaseReference();
	
	DataResource::releaseReference();
}


// ======================================================================

void ObjectTemplate::release() const
{
	ObjectTemplateList::release(*this);
}

// ----------------------------------------------------------------------

void ObjectTemplate::loadFromIff(Iff &iff)
{
	//-- Track name of most recently loading object template name to give us more 
	//   data to work with when we receive crash info.
	char const *const filename = iff.getFileName();
	IGNORE_RETURN(snprintf(s_crashReportInfoLoadFromIff, sizeof(s_crashReportInfoLoadFromIff) - 1, "ObjectTemplate_Iff: %s\n", (filename && *filename) ? filename : "<nullptr>"));
	s_crashReportInfoLoadFromIff[sizeof(s_crashReportInfoLoadFromIff) - 1] = '\0';

	preLoad();
	load(iff);
	postLoad();
}

// ----------------------------------------------------------------------

void ObjectTemplate::preLoad()
{
}

// ----------------------------------------------------------------------

void ObjectTemplate::postLoad()
{
}

// ----------------------------------------------------------------------

void ObjectTemplate::saveToIff(Iff & /*iff*/)
{
	DEBUG_WARNING(true, ("ObjectTemplate::saveToIff - unsupported"));
} //lint !e1762 // (Info -- Member function 'ObjectTemplate::saveToIff(Iff &)' could be made const) // @todo Steve, this most likely is true.  Save/write functions generally can be const since they're not modifying internal content.  If this is true for you, please fix this function and all derived versions.

//-----------------------------------------------------------------------

const bool ObjectTemplate::isLoaded(const std::string & fileName)
{
	return ObjectTemplateList::isLoaded(fileName);
}

// ----------------------------------------------------------------------

bool ObjectTemplate::derivesFrom (const std::string & potentialAncestorName) const
{
	if (!strcmp (getName (), potentialAncestorName.c_str ()))
		return true;

	if (m_baseData)
		return m_baseData->derivesFrom (potentialAncestorName);

	return false;
}

	
//----------------------------------------------------------------------

#ifdef _DEBUG
/**
 * Special function used by datalint. Derived classes will check their data as 
 * needed.
 */
void ObjectTemplate::testValues(void) const
{
}
#endif

// ======================================================================

