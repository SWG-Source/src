//========================================================================
//
// SharedFactoryObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedFactoryObjectTemplate.h"
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

bool SharedFactoryObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedFactoryObjectTemplate::SharedFactoryObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedFactoryObjectTemplate::SharedFactoryObjectTemplate

/**
 * Class destructor.
 */
SharedFactoryObjectTemplate::~SharedFactoryObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedFactoryObjectTemplate::~SharedFactoryObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedFactoryObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedFactoryObjectTemplate_tag, create);
}	// SharedFactoryObjectTemplate::registerMe

/**
 * Creates a SharedFactoryObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedFactoryObjectTemplate::create(const std::string & filename)
{
	return new SharedFactoryObjectTemplate(filename);
}	// SharedFactoryObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedFactoryObjectTemplate::getId(void) const
{
	return SharedFactoryObjectTemplate_tag;
}	// SharedFactoryObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedFactoryObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedFactoryObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedFactoryObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedFactoryObjectTemplate * base = dynamic_cast<const SharedFactoryObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedFactoryObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedFactoryObjectTemplate::testValues(void) const
{
	SharedTangibleObjectTemplate::testValues();
}	// SharedFactoryObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedFactoryObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedFactoryObjectTemplate_tag)
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
}	// SharedFactoryObjectTemplate::load

//@END TFD
