//========================================================================
//
// ServerCellObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerCellObjectTemplate.h"
#include "serverGame/CellObject.h"
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

bool ServerCellObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerCellObjectTemplate::ServerCellObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerCellObjectTemplate::ServerCellObjectTemplate

/**
 * Class destructor.
 */
ServerCellObjectTemplate::~ServerCellObjectTemplate()
{
	if (m_baseData)
	{
		DEBUG_REPORT_LOG(true,("Released m_baseData.\n"));
		m_baseData->releaseReference();
		m_baseData=0;
	}
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerCellObjectTemplate::~ServerCellObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerCellObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerCellObjectTemplate_tag, create);
}	// ServerCellObjectTemplate::registerMe

/**
 * Creates a ServerCellObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerCellObjectTemplate::create(const std::string & filename)
{
	return new ServerCellObjectTemplate(filename);
}	// ServerCellObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerCellObjectTemplate::getId(void) const
{
	return ServerCellObjectTemplate_tag;
}	// ServerCellObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerCellObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerCellObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerCellObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerCellObjectTemplate * base = dynamic_cast<const ServerCellObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerCellObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerCellObjectTemplate::createObject(void) const
{
	return new CellObject(this);
}	// ServerCellObjectTemplate::createObject

//@BEGIN TFD
#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerCellObjectTemplate::testValues(void) const
{
	ServerObjectTemplate::testValues();
}	// ServerCellObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerCellObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerCellObjectTemplate_tag)
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
}	// ServerCellObjectTemplate::load

//@END TFD
