//========================================================================
//
// ServerStaticObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerStaticObjectTemplate.h"
#include "serverGame/StaticObject.h"
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

bool ServerStaticObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerStaticObjectTemplate::ServerStaticObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerStaticObjectTemplate::ServerStaticObjectTemplate

/**
 * Class destructor.
 */
ServerStaticObjectTemplate::~ServerStaticObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerStaticObjectTemplate::~ServerStaticObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerStaticObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerStaticObjectTemplate_tag, create);
}	// ServerStaticObjectTemplate::registerMe

/**
 * Creates a ServerStaticObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerStaticObjectTemplate::create(const std::string & filename)
{
	return new ServerStaticObjectTemplate(filename);
}	// ServerStaticObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerStaticObjectTemplate::getId(void) const
{
	return ServerStaticObjectTemplate_tag;
}	// ServerStaticObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerStaticObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerStaticObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerStaticObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerStaticObjectTemplate * base = dynamic_cast<const ServerStaticObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerStaticObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerStaticObjectTemplate::createObject(void) const
{
	return new StaticObject(this);
}	// ServerStaticObjectTemplate::createObject

//@BEGIN TFD
bool ServerStaticObjectTemplate::getClientOnlyBuildout(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const ServerStaticObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerStaticObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getClientOnlyBuildout(true);
#endif
	}

	if (!m_clientOnlyBuildout.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter clientOnlyBuildout in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter clientOnlyBuildout has not been defined in template %s!", DataResource::getName()));
			return base->getClientOnlyBuildout();
		}
	}

	bool value = m_clientOnlyBuildout.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerStaticObjectTemplate::getClientOnlyBuildout

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerStaticObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getClientOnlyBuildout(true));
	ServerObjectTemplate::testValues();
}	// ServerStaticObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerStaticObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerStaticObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,0,1))
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
		if (strcmp(paramName, "clientOnlyBuildout") == 0)
			m_clientOnlyBuildout.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerStaticObjectTemplate::load

//@END TFD
