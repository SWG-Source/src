//========================================================================
//
// ServerFactoryObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerFactoryObjectTemplate.h"
#include "serverGame/FactoryObject.h"
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

bool ServerFactoryObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerFactoryObjectTemplate::ServerFactoryObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerFactoryObjectTemplate::ServerFactoryObjectTemplate

/**
 * Class destructor.
 */
ServerFactoryObjectTemplate::~ServerFactoryObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerFactoryObjectTemplate::~ServerFactoryObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerFactoryObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerFactoryObjectTemplate_tag, create);
}	// ServerFactoryObjectTemplate::registerMe

/**
 * Creates a ServerFactoryObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerFactoryObjectTemplate::create(const std::string & filename)
{
	return new ServerFactoryObjectTemplate(filename);
}	// ServerFactoryObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerFactoryObjectTemplate::getId(void) const
{
	return ServerFactoryObjectTemplate_tag;
}	// ServerFactoryObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerFactoryObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerFactoryObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerFactoryObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerFactoryObjectTemplate * base = dynamic_cast<const ServerFactoryObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerFactoryObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerFactoryObjectTemplate::createObject(void) const
{
	return new FactoryObject(this);
}	// ServerFactoryObjectTemplate::createObject

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerFactoryObjectTemplate::testValues(void) const
{
	ServerTangibleObjectTemplate::testValues();
}	// ServerFactoryObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerFactoryObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerFactoryObjectTemplate_tag)
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
}	// ServerFactoryObjectTemplate::load

//@END TFD
