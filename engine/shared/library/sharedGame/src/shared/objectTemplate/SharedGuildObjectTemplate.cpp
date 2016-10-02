//========================================================================
//
// SharedGuildObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedGuildObjectTemplate.h"
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

bool SharedGuildObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedGuildObjectTemplate::SharedGuildObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedUniverseObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedGuildObjectTemplate::SharedGuildObjectTemplate

/**
 * Class destructor.
 */
SharedGuildObjectTemplate::~SharedGuildObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedGuildObjectTemplate::~SharedGuildObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedGuildObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedGuildObjectTemplate_tag, create);
}	// SharedGuildObjectTemplate::registerMe

/**
 * Creates a SharedGuildObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedGuildObjectTemplate::create(const std::string & filename)
{
	return new SharedGuildObjectTemplate(filename);
}	// SharedGuildObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedGuildObjectTemplate::getId(void) const
{
	return SharedGuildObjectTemplate_tag;
}	// SharedGuildObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedGuildObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedGuildObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedGuildObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedGuildObjectTemplate * base = dynamic_cast<const SharedGuildObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedGuildObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedGuildObjectTemplate::testValues(void) const
{
	SharedUniverseObjectTemplate::testValues();
}	// SharedGuildObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedGuildObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedGuildObjectTemplate_tag)
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
}	// SharedGuildObjectTemplate::load

//@END TFD
