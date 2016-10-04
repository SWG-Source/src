//========================================================================
//
// ServerPlayerQuestObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PlayerQuestObject.h"
#include "ServerPlayerQuestObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedMath/Vector.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS
#include <stdio.h>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool ServerPlayerQuestObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerPlayerQuestObjectTemplate::ServerPlayerQuestObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerPlayerQuestObjectTemplate::ServerPlayerQuestObjectTemplate

/**
 * Class destructor.
 */
ServerPlayerQuestObjectTemplate::~ServerPlayerQuestObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerPlayerQuestObjectTemplate::~ServerPlayerQuestObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerPlayerQuestObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerPlayerQuestObjectTemplate_tag, create);
}	// ServerPlayerQuestObjectTemplate::registerMe

/**
 * Creates a ServerPlayerQuestObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerPlayerQuestObjectTemplate::create(const std::string & filename)
{
	return new ServerPlayerQuestObjectTemplate(filename);
}	// ServerPlayerQuestObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerPlayerQuestObjectTemplate::getId(void) const
{
	return ServerPlayerQuestObjectTemplate_tag;
}	// ServerPlayerQuestObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerPlayerQuestObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerPlayerQuestObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerPlayerQuestObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerPlayerQuestObjectTemplate * base = dynamic_cast<const ServerPlayerQuestObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerPlayerQuestObjectTemplate::getHighestTemplateVersion

/**
* Creates a new object from this template.
*
* @return the object
*/
Object * ServerPlayerQuestObjectTemplate::createObject(void) const
{
	return new PlayerQuestObject(this);
}	// ServerMissionObjectTemplate::createObject


//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerPlayerQuestObjectTemplate::testValues(void) const
{
	ServerTangibleObjectTemplate::testValues();
}	// ServerPlayerQuestObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerPlayerQuestObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerPlayerQuestObjectTemplate_tag)
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
}	// ServerPlayerQuestObjectTemplate::load

//@END TFD
