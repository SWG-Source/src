//========================================================================
//
// ServerInstallationObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerInstallationObjectTemplate.h"
#include "serverGame/InstallationObject.h"
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

bool ServerInstallationObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerInstallationObjectTemplate::ServerInstallationObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerInstallationObjectTemplate::ServerInstallationObjectTemplate

/**
 * Class destructor.
 */
ServerInstallationObjectTemplate::~ServerInstallationObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerInstallationObjectTemplate::~ServerInstallationObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerInstallationObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerInstallationObjectTemplate_tag, create);
}	// ServerInstallationObjectTemplate::registerMe

/**
 * Creates a ServerInstallationObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerInstallationObjectTemplate::create(const std::string & filename)
{
	return new ServerInstallationObjectTemplate(filename);
}	// ServerInstallationObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerInstallationObjectTemplate::getId(void) const
{
	return ServerInstallationObjectTemplate_tag;
}	// ServerInstallationObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerInstallationObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerInstallationObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerInstallationObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerInstallationObjectTemplate * base = dynamic_cast<const ServerInstallationObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerInstallationObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerInstallationObjectTemplate::createObject(void) const
{
	return new InstallationObject(this);
}	// ServerInstallationObjectTemplate::createObject

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerInstallationObjectTemplate::testValues(void) const
{
	ServerTangibleObjectTemplate::testValues();
}	// ServerInstallationObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerInstallationObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerInstallationObjectTemplate_tag)
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
	UNREF(paramName);
	UNREF(paramCount);

	file.exitForm();
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerInstallationObjectTemplate::load

//@END TFD
