//========================================================================
//
// ServerManufactureInstallationObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerManufactureInstallationObjectTemplate.h"
#include "serverGame/ManufactureInstallationObject.h"
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

bool ServerManufactureInstallationObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerManufactureInstallationObjectTemplate::ServerManufactureInstallationObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerInstallationObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerManufactureInstallationObjectTemplate::ServerManufactureInstallationObjectTemplate

/**
 * Class destructor.
 */
ServerManufactureInstallationObjectTemplate::~ServerManufactureInstallationObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerManufactureInstallationObjectTemplate::~ServerManufactureInstallationObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerManufactureInstallationObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerManufactureInstallationObjectTemplate_tag, create);
}	// ServerManufactureInstallationObjectTemplate::registerMe

/**
 * Creates a ServerManufactureInstallationObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerManufactureInstallationObjectTemplate::create(const std::string & filename)
{
	return new ServerManufactureInstallationObjectTemplate(filename);
}	// ServerManufactureInstallationObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerManufactureInstallationObjectTemplate::getId(void) const
{
	return ServerManufactureInstallationObjectTemplate_tag;
}	// ServerManufactureInstallationObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerManufactureInstallationObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerManufactureInstallationObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerManufactureInstallationObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerManufactureInstallationObjectTemplate * base = dynamic_cast<const ServerManufactureInstallationObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerManufactureInstallationObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerManufactureInstallationObjectTemplate::createObject(void) const
{
	return new ManufactureInstallationObject(this);
}	// ServerManufactureInstallationObjectTemplate::createObject

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerManufactureInstallationObjectTemplate::testValues(void) const
{
	ServerInstallationObjectTemplate::testValues();
}	// ServerManufactureInstallationObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerManufactureInstallationObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerManufactureInstallationObjectTemplate_tag)
	{
		ServerInstallationObjectTemplate::load(file);
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
	ServerInstallationObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerManufactureInstallationObjectTemplate::load

//@END TFD
