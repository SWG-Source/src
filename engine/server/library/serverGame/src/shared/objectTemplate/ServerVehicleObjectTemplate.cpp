//========================================================================
//
// ServerVehicleObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerVehicleObjectTemplate.h"
#include "serverGame/VehicleObject.h"
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

bool ServerVehicleObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerVehicleObjectTemplate::ServerVehicleObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerVehicleObjectTemplate::ServerVehicleObjectTemplate

/**
 * Class destructor.
 */
ServerVehicleObjectTemplate::~ServerVehicleObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerVehicleObjectTemplate::~ServerVehicleObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerVehicleObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerVehicleObjectTemplate_tag, create);
}	// ServerVehicleObjectTemplate::registerMe

/**
 * Creates a ServerVehicleObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerVehicleObjectTemplate::create(const std::string & filename)
{
	return new ServerVehicleObjectTemplate(filename);
}	// ServerVehicleObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerVehicleObjectTemplate::getId(void) const
{
	return ServerVehicleObjectTemplate_tag;
}	// ServerVehicleObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerVehicleObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerVehicleObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerVehicleObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerVehicleObjectTemplate * base = dynamic_cast<const ServerVehicleObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerVehicleObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerVehicleObjectTemplate::createObject(void) const
{
	return new VehicleObject(this);
}	// ServerVehicleObjectTemplate::createObject

//@BEGIN TFD
const std::string & ServerVehicleObjectTemplate::getFuelType(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getFuelType(true);
#endif
	}

	if (!m_fuelType.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter fuelType in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter fuelType has not been defined in template %s!", DataResource::getName()));
			return base->getFuelType();
		}
	}

	const std::string & value = m_fuelType.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerVehicleObjectTemplate::getFuelType

float ServerVehicleObjectTemplate::getCurrentFuel(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCurrentFuel(true);
#endif
	}

	if (!m_currentFuel.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter currentFuel in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter currentFuel has not been defined in template %s!", DataResource::getName()));
			return base->getCurrentFuel();
		}
	}

	float value = m_currentFuel.getValue();
	char delta = m_currentFuel.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCurrentFuel();
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
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerVehicleObjectTemplate::getCurrentFuel

float ServerVehicleObjectTemplate::getCurrentFuelMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCurrentFuelMin(true);
#endif
	}

	if (!m_currentFuel.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter currentFuel in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter currentFuel has not been defined in template %s!", DataResource::getName()));
			return base->getCurrentFuelMin();
		}
	}

	float value = m_currentFuel.getMinValue();
	char delta = m_currentFuel.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCurrentFuelMin();
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
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerVehicleObjectTemplate::getCurrentFuelMin

float ServerVehicleObjectTemplate::getCurrentFuelMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCurrentFuelMax(true);
#endif
	}

	if (!m_currentFuel.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter currentFuel in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter currentFuel has not been defined in template %s!", DataResource::getName()));
			return base->getCurrentFuelMax();
		}
	}

	float value = m_currentFuel.getMaxValue();
	char delta = m_currentFuel.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCurrentFuelMax();
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
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerVehicleObjectTemplate::getCurrentFuelMax

float ServerVehicleObjectTemplate::getMaxFuel(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxFuel(true);
#endif
	}

	if (!m_maxFuel.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxFuel in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxFuel has not been defined in template %s!", DataResource::getName()));
			return base->getMaxFuel();
		}
	}

	float value = m_maxFuel.getValue();
	char delta = m_maxFuel.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxFuel();
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
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerVehicleObjectTemplate::getMaxFuel

float ServerVehicleObjectTemplate::getMaxFuelMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxFuelMin(true);
#endif
	}

	if (!m_maxFuel.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxFuel in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxFuel has not been defined in template %s!", DataResource::getName()));
			return base->getMaxFuelMin();
		}
	}

	float value = m_maxFuel.getMinValue();
	char delta = m_maxFuel.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxFuelMin();
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
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerVehicleObjectTemplate::getMaxFuelMin

float ServerVehicleObjectTemplate::getMaxFuelMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxFuelMax(true);
#endif
	}

	if (!m_maxFuel.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxFuel in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxFuel has not been defined in template %s!", DataResource::getName()));
			return base->getMaxFuelMax();
		}
	}

	float value = m_maxFuel.getMaxValue();
	char delta = m_maxFuel.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxFuelMax();
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
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerVehicleObjectTemplate::getMaxFuelMax

float ServerVehicleObjectTemplate::getConsumpsion(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getConsumpsion(true);
#endif
	}

	if (!m_consumpsion.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter consumpsion in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter consumpsion has not been defined in template %s!", DataResource::getName()));
			return base->getConsumpsion();
		}
	}

	float value = m_consumpsion.getValue();
	char delta = m_consumpsion.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getConsumpsion();
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
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerVehicleObjectTemplate::getConsumpsion

float ServerVehicleObjectTemplate::getConsumpsionMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getConsumpsionMin(true);
#endif
	}

	if (!m_consumpsion.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter consumpsion in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter consumpsion has not been defined in template %s!", DataResource::getName()));
			return base->getConsumpsionMin();
		}
	}

	float value = m_consumpsion.getMinValue();
	char delta = m_consumpsion.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getConsumpsionMin();
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
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerVehicleObjectTemplate::getConsumpsionMin

float ServerVehicleObjectTemplate::getConsumpsionMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getConsumpsionMax(true);
#endif
	}

	if (!m_consumpsion.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter consumpsion in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter consumpsion has not been defined in template %s!", DataResource::getName()));
			return base->getConsumpsionMax();
		}
	}

	float value = m_consumpsion.getMaxValue();
	char delta = m_consumpsion.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getConsumpsionMax();
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
			value = baseValue + static_cast<float>(baseValue * (value / 100.0f));
		else if (delta == '_')
			value = baseValue - static_cast<float>(baseValue * (value / 100.0f));
	}
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerVehicleObjectTemplate::getConsumpsionMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerVehicleObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getFuelType(true));
	IGNORE_RETURN(getCurrentFuelMin(true));
	IGNORE_RETURN(getCurrentFuelMax(true));
	IGNORE_RETURN(getMaxFuelMin(true));
	IGNORE_RETURN(getMaxFuelMax(true));
	IGNORE_RETURN(getConsumpsionMin(true));
	IGNORE_RETURN(getConsumpsionMax(true));
	ServerTangibleObjectTemplate::testValues();
}	// ServerVehicleObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerVehicleObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerVehicleObjectTemplate_tag)
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
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "fuelType") == 0)
			m_fuelType.loadFromIff(file);
		else if (strcmp(paramName, "currentFuel") == 0)
			m_currentFuel.loadFromIff(file);
		else if (strcmp(paramName, "maxFuel") == 0)
			m_maxFuel.loadFromIff(file);
		else if (strcmp(paramName, "consumpsion") == 0)
			m_consumpsion.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerVehicleObjectTemplate::load

//@END TFD
