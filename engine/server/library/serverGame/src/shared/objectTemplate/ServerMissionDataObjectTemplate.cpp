//========================================================================
//
// ServerMissionDataObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerMissionDataObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedMath/Vector.h"
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS
#include <stdio.h>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool ServerMissionDataObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerMissionDataObjectTemplate::ServerMissionDataObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerIntangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerMissionDataObjectTemplate::ServerMissionDataObjectTemplate

/**
 * Class destructor.
 */
ServerMissionDataObjectTemplate::~ServerMissionDataObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerMissionDataObjectTemplate::~ServerMissionDataObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerMissionDataObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerMissionDataObjectTemplate_tag, create);
}	// ServerMissionDataObjectTemplate::registerMe

/**
 * Creates a ServerMissionDataObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerMissionDataObjectTemplate::create(const std::string & filename)
{
	return new ServerMissionDataObjectTemplate(filename);
}	// ServerMissionDataObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerMissionDataObjectTemplate::getId(void) const
{
	return ServerMissionDataObjectTemplate_tag;
}	// ServerMissionDataObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerMissionDataObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerMissionDataObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerMissionDataObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const ServerMissionDataObjectTemplate * base = dynamic_cast<const ServerMissionDataObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerMissionDataObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerMissionDataObjectTemplate::testValues(void) const
{
	ServerIntangibleObjectTemplate::testValues();
}	// ServerMissionDataObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerMissionDataObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerMissionDataObjectTemplate_tag)
	{
		ServerIntangibleObjectTemplate::load(file);
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
	ServerIntangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerMissionDataObjectTemplate::load

//@END TFD
