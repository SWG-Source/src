//========================================================================
//
// SharedResourceContainerObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedResourceContainerObjectTemplate.h"
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

bool SharedResourceContainerObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate

/**
 * Class destructor.
 */
SharedResourceContainerObjectTemplate::~SharedResourceContainerObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedResourceContainerObjectTemplate::~SharedResourceContainerObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedResourceContainerObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedResourceContainerObjectTemplate_tag, create);
}	// SharedResourceContainerObjectTemplate::registerMe

/**
 * Creates a SharedResourceContainerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedResourceContainerObjectTemplate::create(const std::string & filename)
{
	return new SharedResourceContainerObjectTemplate(filename);
}	// SharedResourceContainerObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedResourceContainerObjectTemplate::getId(void) const
{
	return SharedResourceContainerObjectTemplate_tag;
}	// SharedResourceContainerObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedResourceContainerObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedResourceContainerObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedResourceContainerObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedResourceContainerObjectTemplate * base = dynamic_cast<const SharedResourceContainerObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedResourceContainerObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedResourceContainerObjectTemplate::testValues(void) const
{
	SharedTangibleObjectTemplate::testValues();
}	// SharedResourceContainerObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedResourceContainerObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedResourceContainerObjectTemplate_tag)
	{
		SharedTangibleObjectTemplate::load(file);
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
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedResourceContainerObjectTemplate::load

//@END TFD
