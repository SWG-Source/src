//========================================================================
//
// SharedStaticObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedStaticObjectTemplate.h"
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

bool SharedStaticObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedStaticObjectTemplate::SharedStaticObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedStaticObjectTemplate::SharedStaticObjectTemplate

/**
 * Class destructor.
 */
SharedStaticObjectTemplate::~SharedStaticObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedStaticObjectTemplate::~SharedStaticObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedStaticObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedStaticObjectTemplate_tag, create);
}	// SharedStaticObjectTemplate::registerMe

/**
 * Creates a SharedStaticObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedStaticObjectTemplate::create(const std::string & filename)
{
	return new SharedStaticObjectTemplate(filename);
}	// SharedStaticObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedStaticObjectTemplate::getId(void) const
{
	return SharedStaticObjectTemplate_tag;
}	// SharedStaticObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedStaticObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedStaticObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedStaticObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedStaticObjectTemplate * base = dynamic_cast<const SharedStaticObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedStaticObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedStaticObjectTemplate::testValues(void) const
{
	SharedObjectTemplate::testValues();
}	// SharedStaticObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedStaticObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedStaticObjectTemplate_tag)
	{
		SharedObjectTemplate::load(file);
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
	SharedObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedStaticObjectTemplate::load

//@END TFD
