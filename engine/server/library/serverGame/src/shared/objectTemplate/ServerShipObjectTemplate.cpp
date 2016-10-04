//========================================================================
//
// ServerShipObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerShipObjectTemplate.h"
#include "serverGame/ShipObject.h"
#include "sharedDebug/DataLint.h"
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

bool ServerShipObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerShipObjectTemplate::ServerShipObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerShipObjectTemplate::ServerShipObjectTemplate

/**
 * Class destructor.
 */
ServerShipObjectTemplate::~ServerShipObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerShipObjectTemplate::~ServerShipObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerShipObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerShipObjectTemplate_tag, create);
}	// ServerShipObjectTemplate::registerMe

/**
 * Creates a ServerShipObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerShipObjectTemplate::create(const std::string & filename)
{
	return new ServerShipObjectTemplate(filename);
}	// ServerShipObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerShipObjectTemplate::getId(void) const
{
	return ServerShipObjectTemplate_tag;
}	// ServerShipObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerShipObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerShipObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerShipObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerShipObjectTemplate * base = dynamic_cast<const ServerShipObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerShipObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerShipObjectTemplate::createObject(void) const
{
	return new ShipObject(this);
}	// ServerShipObjectTemplate::createObject

//@BEGIN TFD
const std::string & ServerShipObjectTemplate::getShipType(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerShipObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerShipObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getShipType(true);
#endif
	}

	if (!m_shipType.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter shipType in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter shipType has not been defined in template %s!", DataResource::getName()));
			return base->getShipType();
		}
	}

	const std::string & value = m_shipType.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerShipObjectTemplate::getShipType

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerShipObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getShipType(true));
	ServerTangibleObjectTemplate::testValues();
}	// ServerShipObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerShipObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerShipObjectTemplate_tag)
	{
		ServerTangibleObjectTemplate::load(file);
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
		if (strcmp(paramName, "shipType") == 0)
			m_shipType.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerShipObjectTemplate::load

//@END TFD
