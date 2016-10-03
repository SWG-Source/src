//========================================================================
//
// ServerBattlefieldMarkerObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerBattlefieldMarkerObjectTemplate.h"
#include "serverGame/BattlefieldMarkerObject.h"
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

bool ServerBattlefieldMarkerObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerBattlefieldMarkerObjectTemplate::ServerBattlefieldMarkerObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerBattlefieldMarkerObjectTemplate::ServerBattlefieldMarkerObjectTemplate

/**
 * Class destructor.
 */
ServerBattlefieldMarkerObjectTemplate::~ServerBattlefieldMarkerObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerBattlefieldMarkerObjectTemplate::~ServerBattlefieldMarkerObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerBattlefieldMarkerObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerBattlefieldMarkerObjectTemplate_tag, create);
}	// ServerBattlefieldMarkerObjectTemplate::registerMe

/**
 * Creates a ServerBattlefieldMarkerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerBattlefieldMarkerObjectTemplate::create(const std::string & filename)
{
	return new ServerBattlefieldMarkerObjectTemplate(filename);
}	// ServerBattlefieldMarkerObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerBattlefieldMarkerObjectTemplate::getId(void) const
{
	return ServerBattlefieldMarkerObjectTemplate_tag;
}	// ServerBattlefieldMarkerObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerBattlefieldMarkerObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerBattlefieldMarkerObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerBattlefieldMarkerObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerBattlefieldMarkerObjectTemplate * base = dynamic_cast<const ServerBattlefieldMarkerObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerBattlefieldMarkerObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerBattlefieldMarkerObjectTemplate::createObject(void) const
{
	return new BattlefieldMarkerObject(this);
}	// ServerBattlefieldMarkerObjectTemplate::createObject

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerBattlefieldMarkerObjectTemplate::testValues(void) const
{
	ServerTangibleObjectTemplate::testValues();
}	// ServerBattlefieldMarkerObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerBattlefieldMarkerObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerBattlefieldMarkerObjectTemplate_tag)
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
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerBattlefieldMarkerObjectTemplate::load

//@END TFD
