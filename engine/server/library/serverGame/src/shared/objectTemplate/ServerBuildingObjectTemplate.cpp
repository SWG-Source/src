//========================================================================
//
// ServerBuildingObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerBuildingObjectTemplate.h"
#include "serverGame/BuildingObject.h"
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

bool ServerBuildingObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerBuildingObjectTemplate::ServerBuildingObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerBuildingObjectTemplate::ServerBuildingObjectTemplate

/**
 * Class destructor.
 */
ServerBuildingObjectTemplate::~ServerBuildingObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerBuildingObjectTemplate::~ServerBuildingObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerBuildingObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerBuildingObjectTemplate_tag, create);
}	// ServerBuildingObjectTemplate::registerMe

/**
 * Creates a ServerBuildingObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerBuildingObjectTemplate::create(const std::string & filename)
{
	return new ServerBuildingObjectTemplate(filename);
}	// ServerBuildingObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerBuildingObjectTemplate::getId(void) const
{
	return ServerBuildingObjectTemplate_tag;
}	// ServerBuildingObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerBuildingObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerBuildingObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerBuildingObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerBuildingObjectTemplate * base = dynamic_cast<const ServerBuildingObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerBuildingObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerBuildingObjectTemplate::createObject(void) const
{
	return new BuildingObject(this);
}	// ServerBuildingObjectTemplate::createObject

//@BEGIN TFD
int ServerBuildingObjectTemplate::getMaintenanceCost(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerBuildingObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerBuildingObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaintenanceCost(true);
#endif
	}

	if (!m_maintenanceCost.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maintenanceCost in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maintenanceCost has not been defined in template %s!", DataResource::getName()));
			return base->getMaintenanceCost();
		}
	}

	int value = m_maintenanceCost.getValue();
	char delta = m_maintenanceCost.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaintenanceCost();
			else if (ms_allowDefaultTemplateParams)
				DEBUG_WARNING(true, ("No base template for delta, using 0"));
			else
				NOT_NULL(base);
		}
		if (delta == '+')
			value = baseValue + value;
		else if (delta == '-')
			value = baseValue - value;
		else if (delta == '=')
			value = baseValue + static_cast<int>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<int>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerBuildingObjectTemplate::getMaintenanceCost

int ServerBuildingObjectTemplate::getMaintenanceCostMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerBuildingObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerBuildingObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaintenanceCostMin(true);
#endif
	}

	if (!m_maintenanceCost.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maintenanceCost in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maintenanceCost has not been defined in template %s!", DataResource::getName()));
			return base->getMaintenanceCostMin();
		}
	}

	int value = m_maintenanceCost.getMinValue();
	char delta = m_maintenanceCost.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaintenanceCostMin();
			else if (ms_allowDefaultTemplateParams)
				DEBUG_WARNING(true, ("No base template for delta, using 0"));
			else
				NOT_NULL(base);
		}
		if (delta == '+')
			value = baseValue + value;
		else if (delta == '-')
			value = baseValue - value;
		else if (delta == '=')
			value = baseValue + static_cast<int>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<int>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerBuildingObjectTemplate::getMaintenanceCostMin

int ServerBuildingObjectTemplate::getMaintenanceCostMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerBuildingObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerBuildingObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaintenanceCostMax(true);
#endif
	}

	if (!m_maintenanceCost.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maintenanceCost in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maintenanceCost has not been defined in template %s!", DataResource::getName()));
			return base->getMaintenanceCostMax();
		}
	}

	int value = m_maintenanceCost.getMaxValue();
	char delta = m_maintenanceCost.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaintenanceCostMax();
			else if (ms_allowDefaultTemplateParams)
				DEBUG_WARNING(true, ("No base template for delta, using 0"));
			else
				NOT_NULL(base);
		}
		if (delta == '+')
			value = baseValue + value;
		else if (delta == '-')
			value = baseValue - value;
		else if (delta == '=')
			value = baseValue + static_cast<int>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<int>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerBuildingObjectTemplate::getMaintenanceCostMax

bool ServerBuildingObjectTemplate::getIsPublic(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const ServerBuildingObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerBuildingObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getIsPublic(true);
#endif
	}

	if (!m_isPublic.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter isPublic in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter isPublic has not been defined in template %s!", DataResource::getName()));
			return base->getIsPublic();
		}
	}

	bool value = m_isPublic.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerBuildingObjectTemplate::getIsPublic

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerBuildingObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getMaintenanceCostMin(true));
	IGNORE_RETURN(getMaintenanceCostMax(true));
	IGNORE_RETURN(getIsPublic(true));
	ServerTangibleObjectTemplate::testValues();
}	// ServerBuildingObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerBuildingObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerBuildingObjectTemplate_tag)
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
		if (strcmp(paramName, "maintenanceCost") == 0)
			m_maintenanceCost.loadFromIff(file);
		else if (strcmp(paramName, "isPublic") == 0)
			m_isPublic.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerBuildingObjectTemplate::load

//@END TFD
