//========================================================================
//
// ServerUniverseObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerUniverseObjectTemplate.h"
#include "sharedFile/Iff.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "serverGame/UniverseObject.h"
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS
#include <stdio.h>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool ServerUniverseObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerUniverseObjectTemplate::ServerUniverseObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerUniverseObjectTemplate::ServerUniverseObjectTemplate

/**
 * Class destructor.
 */
ServerUniverseObjectTemplate::~ServerUniverseObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerUniverseObjectTemplate::~ServerUniverseObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerUniverseObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerUniverseObjectTemplate_tag, create);
}	// ServerUniverseObjectTemplate::registerMe

/**
 * Creates a ServerUniverseObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerUniverseObjectTemplate::create(const std::string & filename)
{
	return new ServerUniverseObjectTemplate(filename);
}	// ServerUniverseObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerUniverseObjectTemplate::getId(void) const
{
	return ServerUniverseObjectTemplate_tag;
}	// ServerUniverseObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerUniverseObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerUniverseObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerUniverseObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerUniverseObjectTemplate * base = dynamic_cast<const ServerUniverseObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerUniverseObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerUniverseObjectTemplate::createObject(void) const
{
	return new UniverseObject(this);
}	// ServerUniverseObjectTemplate::createObject

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerUniverseObjectTemplate::testValues(void) const
{
	ServerObjectTemplate::testValues();
}	// ServerUniverseObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerUniverseObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerUniverseObjectTemplate_tag)
	{
		ServerObjectTemplate::load(file);
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
	ServerObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerUniverseObjectTemplate::load

//@END TFD
