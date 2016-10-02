//========================================================================
//
// ServerPlanetObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerPlanetObjectTemplate.h"
#include "serverGame/PlanetObject.h"
#include "sharedFile/Iff.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS
#include <stdio.h>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool ServerPlanetObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerPlanetObjectTemplate::ServerPlanetObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerUniverseObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerPlanetObjectTemplate::ServerPlanetObjectTemplate

/**
 * Class destructor.
 */
ServerPlanetObjectTemplate::~ServerPlanetObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerPlanetObjectTemplate::~ServerPlanetObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerPlanetObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerPlanetObjectTemplate_tag, create);
}	// ServerPlanetObjectTemplate::registerMe

/**
 * Creates a ServerPlanetObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerPlanetObjectTemplate::create(const std::string & filename)
{
	return new ServerPlanetObjectTemplate(filename);
}	// ServerPlanetObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerPlanetObjectTemplate::getId(void) const
{
	return ServerPlanetObjectTemplate_tag;
}	// ServerPlanetObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerPlanetObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerPlanetObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerPlanetObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerPlanetObjectTemplate * base = dynamic_cast<const ServerPlanetObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerPlanetObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerPlanetObjectTemplate::createObject(void) const
{
	return new PlanetObject(this);
}	// ServerPlanetObjectTemplate::createObject

//@BEGIN TFD
const std::string & ServerPlanetObjectTemplate::getPlanetName(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerPlanetObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerPlanetObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getPlanetName(true);
#endif
	}

	if (!m_planetName.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter planetName in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter planetName has not been defined in template %s!", DataResource::getName()));
			return base->getPlanetName();
		}
	}

	const std::string & value = m_planetName.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerPlanetObjectTemplate::getPlanetName

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerPlanetObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getPlanetName(true));
	ServerUniverseObjectTemplate::testValues();
}	// ServerPlanetObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerPlanetObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerPlanetObjectTemplate_tag)
	{
		ServerUniverseObjectTemplate::load(file);
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
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "planetName") == 0)
			m_planetName.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerUniverseObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerPlanetObjectTemplate::load

//@END TFD
