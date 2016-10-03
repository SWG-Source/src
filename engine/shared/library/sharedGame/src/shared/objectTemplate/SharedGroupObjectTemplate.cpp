//========================================================================
//
// SharedGroupObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedGroupObjectTemplate.h"
#include "sharedFile/Iff.h"
#include "sharedMath/Vector.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS
#include <algorithm>
#include <cstdio>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool SharedGroupObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedGroupObjectTemplate::SharedGroupObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedUniverseObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedGroupObjectTemplate::SharedGroupObjectTemplate

/**
 * Class destructor.
 */
SharedGroupObjectTemplate::~SharedGroupObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedGroupObjectTemplate::~SharedGroupObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedGroupObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedGroupObjectTemplate_tag, create);
}	// SharedGroupObjectTemplate::registerMe

/**
 * Creates a SharedGroupObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedGroupObjectTemplate::create(const std::string & filename)
{
	return new SharedGroupObjectTemplate(filename);
}	// SharedGroupObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedGroupObjectTemplate::getId(void) const
{
	return SharedGroupObjectTemplate_tag;
}	// SharedGroupObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedGroupObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedGroupObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedGroupObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedGroupObjectTemplate * base = dynamic_cast<const SharedGroupObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedGroupObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedGroupObjectTemplate::testValues(void) const
{
	SharedUniverseObjectTemplate::testValues();
}	// SharedGroupObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedGroupObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedGroupObjectTemplate_tag)
	{
		SharedUniverseObjectTemplate::load(file);
		return;
	}

	file.enterForm();
	m_templateVersion = file.getCurrentName();
	if (m_templateVersion == TAG(D,E,R,V))
	{
		file.enterForm();
		file.enterChunk();
		std::string baseFilename;
		file.read_string(baseFilename);
		file.exitChunk();
		const ObjectTemplate *base = ObjectTemplateList::fetch(baseFilename);
		DEBUG_WARNING(base == nullptr, ("was unable to load base template %s", baseFilename.c_str()));
		if (m_baseData == base && base != nullptr)
			base->releaseReference();
		else
		{
			if (m_baseData != nullptr)
				m_baseData->releaseReference();
			m_baseData = base;
		}
		file.exitForm();
		m_templateVersion = file.getCurrentName();
	}
	if (getHighestTemplateVersion() != TAG(0,0,0,0))
	{
		if (DataLint::isEnabled())
			DEBUG_WARNING(true, ("template %s version out of date", file.getFileName()));
		m_versionOk = false;
	}

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	UNREF(paramName);
	UNREF(paramCount);

	file.exitForm();
	SharedUniverseObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedGroupObjectTemplate::load

//@END TFD
