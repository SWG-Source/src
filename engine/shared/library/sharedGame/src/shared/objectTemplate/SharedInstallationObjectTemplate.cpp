//========================================================================
//
// SharedInstallationObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedInstallationObjectTemplate.h"
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

bool SharedInstallationObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedInstallationObjectTemplate::SharedInstallationObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedInstallationObjectTemplate::SharedInstallationObjectTemplate

/**
 * Class destructor.
 */
SharedInstallationObjectTemplate::~SharedInstallationObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedInstallationObjectTemplate::~SharedInstallationObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedInstallationObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedInstallationObjectTemplate_tag, create);
}	// SharedInstallationObjectTemplate::registerMe

/**
 * Creates a SharedInstallationObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedInstallationObjectTemplate::create(const std::string & filename)
{
	return new SharedInstallationObjectTemplate(filename);
}	// SharedInstallationObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedInstallationObjectTemplate::getId(void) const
{
	return SharedInstallationObjectTemplate_tag;
}	// SharedInstallationObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedInstallationObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedInstallationObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedInstallationObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedInstallationObjectTemplate * base = dynamic_cast<const SharedInstallationObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedInstallationObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedInstallationObjectTemplate::testValues(void) const
{
	SharedTangibleObjectTemplate::testValues();
}	// SharedInstallationObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedInstallationObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedInstallationObjectTemplate_tag)
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
}	// SharedInstallationObjectTemplate::load

//@END TFD
