//========================================================================
//
// ServerHarvesterInstallationObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerHarvesterInstallationObjectTemplate.h"
#include "serverGame/HarvesterInstallationObject.h"
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

bool ServerHarvesterInstallationObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerHarvesterInstallationObjectTemplate::ServerHarvesterInstallationObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerInstallationObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerHarvesterInstallationObjectTemplate::ServerHarvesterInstallationObjectTemplate

/**
 * Class destructor.
 */
ServerHarvesterInstallationObjectTemplate::~ServerHarvesterInstallationObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerHarvesterInstallationObjectTemplate::~ServerHarvesterInstallationObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerHarvesterInstallationObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerHarvesterInstallationObjectTemplate_tag, create);
}	// ServerHarvesterInstallationObjectTemplate::registerMe

/**
 * Creates a ServerHarvesterInstallationObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerHarvesterInstallationObjectTemplate::create(const std::string & filename)
{
	return new ServerHarvesterInstallationObjectTemplate(filename);
}	// ServerHarvesterInstallationObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerHarvesterInstallationObjectTemplate::getId(void) const
{
	return ServerHarvesterInstallationObjectTemplate_tag;
}	// ServerHarvesterInstallationObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerHarvesterInstallationObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerHarvesterInstallationObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerHarvesterInstallationObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerHarvesterInstallationObjectTemplate * base = dynamic_cast<const ServerHarvesterInstallationObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerHarvesterInstallationObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerHarvesterInstallationObjectTemplate::createObject(void) const
{
	return new HarvesterInstallationObject(this);
}	// ServerHarvesterInstallationObjectTemplate::createObject

//@BEGIN TFD
int ServerHarvesterInstallationObjectTemplate::getMaxExtractionRate(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerHarvesterInstallationObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerHarvesterInstallationObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxExtractionRate(true);
#endif
	}

	if (!m_maxExtractionRate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxExtractionRate in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxExtractionRate has not been defined in template %s!", DataResource::getName()));
			return base->getMaxExtractionRate();
		}
	}

	int value = m_maxExtractionRate.getValue();
	char delta = m_maxExtractionRate.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxExtractionRate();
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
}	// ServerHarvesterInstallationObjectTemplate::getMaxExtractionRate

int ServerHarvesterInstallationObjectTemplate::getMaxExtractionRateMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerHarvesterInstallationObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerHarvesterInstallationObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxExtractionRateMin(true);
#endif
	}

	if (!m_maxExtractionRate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxExtractionRate in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxExtractionRate has not been defined in template %s!", DataResource::getName()));
			return base->getMaxExtractionRateMin();
		}
	}

	int value = m_maxExtractionRate.getMinValue();
	char delta = m_maxExtractionRate.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxExtractionRateMin();
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
}	// ServerHarvesterInstallationObjectTemplate::getMaxExtractionRateMin

int ServerHarvesterInstallationObjectTemplate::getMaxExtractionRateMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerHarvesterInstallationObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerHarvesterInstallationObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxExtractionRateMax(true);
#endif
	}

	if (!m_maxExtractionRate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxExtractionRate in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxExtractionRate has not been defined in template %s!", DataResource::getName()));
			return base->getMaxExtractionRateMax();
		}
	}

	int value = m_maxExtractionRate.getMaxValue();
	char delta = m_maxExtractionRate.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxExtractionRateMax();
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
}	// ServerHarvesterInstallationObjectTemplate::getMaxExtractionRateMax

int ServerHarvesterInstallationObjectTemplate::getCurrentExtractionRate(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerHarvesterInstallationObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerHarvesterInstallationObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCurrentExtractionRate(true);
#endif
	}

	if (!m_currentExtractionRate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter currentExtractionRate in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter currentExtractionRate has not been defined in template %s!", DataResource::getName()));
			return base->getCurrentExtractionRate();
		}
	}

	int value = m_currentExtractionRate.getValue();
	char delta = m_currentExtractionRate.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCurrentExtractionRate();
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
}	// ServerHarvesterInstallationObjectTemplate::getCurrentExtractionRate

int ServerHarvesterInstallationObjectTemplate::getCurrentExtractionRateMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerHarvesterInstallationObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerHarvesterInstallationObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCurrentExtractionRateMin(true);
#endif
	}

	if (!m_currentExtractionRate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter currentExtractionRate in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter currentExtractionRate has not been defined in template %s!", DataResource::getName()));
			return base->getCurrentExtractionRateMin();
		}
	}

	int value = m_currentExtractionRate.getMinValue();
	char delta = m_currentExtractionRate.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCurrentExtractionRateMin();
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
}	// ServerHarvesterInstallationObjectTemplate::getCurrentExtractionRateMin

int ServerHarvesterInstallationObjectTemplate::getCurrentExtractionRateMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerHarvesterInstallationObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerHarvesterInstallationObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCurrentExtractionRateMax(true);
#endif
	}

	if (!m_currentExtractionRate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter currentExtractionRate in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter currentExtractionRate has not been defined in template %s!", DataResource::getName()));
			return base->getCurrentExtractionRateMax();
		}
	}

	int value = m_currentExtractionRate.getMaxValue();
	char delta = m_currentExtractionRate.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCurrentExtractionRateMax();
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
}	// ServerHarvesterInstallationObjectTemplate::getCurrentExtractionRateMax

int ServerHarvesterInstallationObjectTemplate::getMaxHopperSize(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerHarvesterInstallationObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerHarvesterInstallationObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxHopperSize(true);
#endif
	}

	if (!m_maxHopperSize.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxHopperSize in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxHopperSize has not been defined in template %s!", DataResource::getName()));
			return base->getMaxHopperSize();
		}
	}

	int value = m_maxHopperSize.getValue();
	char delta = m_maxHopperSize.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxHopperSize();
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
}	// ServerHarvesterInstallationObjectTemplate::getMaxHopperSize

int ServerHarvesterInstallationObjectTemplate::getMaxHopperSizeMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerHarvesterInstallationObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerHarvesterInstallationObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxHopperSizeMin(true);
#endif
	}

	if (!m_maxHopperSize.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxHopperSize in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxHopperSize has not been defined in template %s!", DataResource::getName()));
			return base->getMaxHopperSizeMin();
		}
	}

	int value = m_maxHopperSize.getMinValue();
	char delta = m_maxHopperSize.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxHopperSizeMin();
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
}	// ServerHarvesterInstallationObjectTemplate::getMaxHopperSizeMin

int ServerHarvesterInstallationObjectTemplate::getMaxHopperSizeMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerHarvesterInstallationObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerHarvesterInstallationObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxHopperSizeMax(true);
#endif
	}

	if (!m_maxHopperSize.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxHopperSize in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxHopperSize has not been defined in template %s!", DataResource::getName()));
			return base->getMaxHopperSizeMax();
		}
	}

	int value = m_maxHopperSize.getMaxValue();
	char delta = m_maxHopperSize.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxHopperSizeMax();
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
}	// ServerHarvesterInstallationObjectTemplate::getMaxHopperSizeMax

const std::string & ServerHarvesterInstallationObjectTemplate::getMasterClassName(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerHarvesterInstallationObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerHarvesterInstallationObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMasterClassName(true);
#endif
	}

	if (!m_masterClassName.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter masterClassName in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter masterClassName has not been defined in template %s!", DataResource::getName()));
			return base->getMasterClassName();
		}
	}

	const std::string & value = m_masterClassName.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerHarvesterInstallationObjectTemplate::getMasterClassName

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerHarvesterInstallationObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getMaxExtractionRateMin(true));
	IGNORE_RETURN(getMaxExtractionRateMax(true));
	IGNORE_RETURN(getCurrentExtractionRateMin(true));
	IGNORE_RETURN(getCurrentExtractionRateMax(true));
	IGNORE_RETURN(getMaxHopperSizeMin(true));
	IGNORE_RETURN(getMaxHopperSizeMax(true));
	IGNORE_RETURN(getMasterClassName(true));
	ServerInstallationObjectTemplate::testValues();
}	// ServerHarvesterInstallationObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerHarvesterInstallationObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerHarvesterInstallationObjectTemplate_tag)
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
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "maxExtractionRate") == 0)
			m_maxExtractionRate.loadFromIff(file);
		else if (strcmp(paramName, "currentExtractionRate") == 0)
			m_currentExtractionRate.loadFromIff(file);
		else if (strcmp(paramName, "maxHopperSize") == 0)
			m_maxHopperSize.loadFromIff(file);
		else if (strcmp(paramName, "masterClassName") == 0)
			m_masterClassName.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerInstallationObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerHarvesterInstallationObjectTemplate::load

//@END TFD
