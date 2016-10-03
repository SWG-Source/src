//========================================================================
//
// ServerCreatureObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerCreatureObjectTemplate.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/NameManager.h"
#include "sharedFile/Iff.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/NameGenerator.h"
#include "sharedNetworkMessages/NameErrors.h"
//@BEGIN TFD TEMPLATE REFS
#include "ServerWeaponObjectTemplate.h"
//@END TFD TEMPLATE REFS
#include <stdio.h>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool ServerCreatureObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerCreatureObjectTemplate::ServerCreatureObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
	,m_attribModsLoaded(false)
	,m_attribModsAppend(false)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerCreatureObjectTemplate::ServerCreatureObjectTemplate

/**
 * Class destructor.
 */
ServerCreatureObjectTemplate::~ServerCreatureObjectTemplate()
{
//@BEGIN TFD CLEANUP
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_attribMods.begin(); iter != m_attribMods.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_attribMods.clear();
	}
//@END TFD CLEANUP
}	// ServerCreatureObjectTemplate::~ServerCreatureObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerCreatureObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerCreatureObjectTemplate_tag, create);
}	// ServerCreatureObjectTemplate::registerMe

/**
 * Creates a ServerCreatureObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerCreatureObjectTemplate::create(const std::string & filename)
{
	return new ServerCreatureObjectTemplate(filename);
}	// ServerCreatureObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerCreatureObjectTemplate::getId(void) const
{
	return ServerCreatureObjectTemplate_tag;
}	// ServerCreatureObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerCreatureObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerCreatureObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerCreatureObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerCreatureObjectTemplate * base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerCreatureObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerCreatureObjectTemplate::createObject(void) const
{
	return new CreatureObject(this);
}	// ServerCreatureObjectTemplate::createObject

//@BEGIN TFD
const ServerWeaponObjectTemplate * ServerCreatureObjectTemplate::getDefaultWeapon() const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_defaultWeapon.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter defaultWeapon in template %s", DataResource::getName()));
			return nullptr;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter defaultWeapon has not been defined in template %s!", DataResource::getName()));
			return base->getDefaultWeapon();
		}
	}

	const ServerWeaponObjectTemplate * returnValue = nullptr;
	const std::string & templateName = m_defaultWeapon.getValue();
	if (!templateName.empty())
	{
		returnValue = dynamic_cast<const ServerWeaponObjectTemplate *>(ObjectTemplateList::fetch(templateName));
		if (returnValue == nullptr)
			WARNING_STRICT_FATAL(true, ("Error loading template %s",templateName.c_str()));
	}
	return returnValue;
}	// ServerCreatureObjectTemplate::getDefaultWeapon

int ServerCreatureObjectTemplate::getAttributes(Attributes index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 6, ("template param index out of range"));
	if (!m_attributes[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attributes in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attributes has not been defined in template %s!", DataResource::getName()));
			return base->getAttributes(index);
		}
	}

	int value = m_attributes[index].getValue();
	char delta = m_attributes[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAttributes(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getAttributes

int ServerCreatureObjectTemplate::getAttributesMin(Attributes index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 6, ("template param index out of range"));
	if (!m_attributes[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attributes in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attributes has not been defined in template %s!", DataResource::getName()));
			return base->getAttributesMin(index);
		}
	}

	int value = m_attributes[index].getMinValue();
	char delta = m_attributes[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAttributesMin(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getAttributesMin

int ServerCreatureObjectTemplate::getAttributesMax(Attributes index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 6, ("template param index out of range"));
	if (!m_attributes[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attributes in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attributes has not been defined in template %s!", DataResource::getName()));
			return base->getAttributesMax(index);
		}
	}

	int value = m_attributes[index].getMaxValue();
	char delta = m_attributes[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAttributesMax(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getAttributesMax

int ServerCreatureObjectTemplate::getMinAttributes(Attributes index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 6, ("template param index out of range"));
	if (!m_minAttributes[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minAttributes in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minAttributes has not been defined in template %s!", DataResource::getName()));
			return base->getMinAttributes(index);
		}
	}

	int value = m_minAttributes[index].getValue();
	char delta = m_minAttributes[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinAttributes(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getMinAttributes

int ServerCreatureObjectTemplate::getMinAttributesMin(Attributes index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 6, ("template param index out of range"));
	if (!m_minAttributes[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minAttributes in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minAttributes has not been defined in template %s!", DataResource::getName()));
			return base->getMinAttributesMin(index);
		}
	}

	int value = m_minAttributes[index].getMinValue();
	char delta = m_minAttributes[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinAttributesMin(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getMinAttributesMin

int ServerCreatureObjectTemplate::getMinAttributesMax(Attributes index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 6, ("template param index out of range"));
	if (!m_minAttributes[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minAttributes in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minAttributes has not been defined in template %s!", DataResource::getName()));
			return base->getMinAttributesMax(index);
		}
	}

	int value = m_minAttributes[index].getMaxValue();
	char delta = m_minAttributes[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinAttributesMax(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getMinAttributesMax

int ServerCreatureObjectTemplate::getMaxAttributes(Attributes index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 6, ("template param index out of range"));
	if (!m_maxAttributes[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxAttributes in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxAttributes has not been defined in template %s!", DataResource::getName()));
			return base->getMaxAttributes(index);
		}
	}

	int value = m_maxAttributes[index].getValue();
	char delta = m_maxAttributes[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxAttributes(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getMaxAttributes

int ServerCreatureObjectTemplate::getMaxAttributesMin(Attributes index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 6, ("template param index out of range"));
	if (!m_maxAttributes[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxAttributes in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxAttributes has not been defined in template %s!", DataResource::getName()));
			return base->getMaxAttributesMin(index);
		}
	}

	int value = m_maxAttributes[index].getMinValue();
	char delta = m_maxAttributes[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxAttributesMin(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getMaxAttributesMin

int ServerCreatureObjectTemplate::getMaxAttributesMax(Attributes index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 6, ("template param index out of range"));
	if (!m_maxAttributes[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxAttributes in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxAttributes has not been defined in template %s!", DataResource::getName()));
			return base->getMaxAttributesMax(index);
		}
	}

	int value = m_maxAttributes[index].getMaxValue();
	char delta = m_maxAttributes[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxAttributesMax(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getMaxAttributesMax

float ServerCreatureObjectTemplate::getMinDrainModifier(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMinDrainModifier(true);
#endif
	}

	if (!m_minDrainModifier.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minDrainModifier in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minDrainModifier has not been defined in template %s!", DataResource::getName()));
			return base->getMinDrainModifier();
		}
	}

	float value = m_minDrainModifier.getValue();
	char delta = m_minDrainModifier.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinDrainModifier();
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
}	// ServerCreatureObjectTemplate::getMinDrainModifier

float ServerCreatureObjectTemplate::getMinDrainModifierMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMinDrainModifierMin(true);
#endif
	}

	if (!m_minDrainModifier.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minDrainModifier in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minDrainModifier has not been defined in template %s!", DataResource::getName()));
			return base->getMinDrainModifierMin();
		}
	}

	float value = m_minDrainModifier.getMinValue();
	char delta = m_minDrainModifier.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinDrainModifierMin();
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
}	// ServerCreatureObjectTemplate::getMinDrainModifierMin

float ServerCreatureObjectTemplate::getMinDrainModifierMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMinDrainModifierMax(true);
#endif
	}

	if (!m_minDrainModifier.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minDrainModifier in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minDrainModifier has not been defined in template %s!", DataResource::getName()));
			return base->getMinDrainModifierMax();
		}
	}

	float value = m_minDrainModifier.getMaxValue();
	char delta = m_minDrainModifier.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinDrainModifierMax();
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
}	// ServerCreatureObjectTemplate::getMinDrainModifierMax

float ServerCreatureObjectTemplate::getMaxDrainModifier(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxDrainModifier(true);
#endif
	}

	if (!m_maxDrainModifier.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxDrainModifier in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxDrainModifier has not been defined in template %s!", DataResource::getName()));
			return base->getMaxDrainModifier();
		}
	}

	float value = m_maxDrainModifier.getValue();
	char delta = m_maxDrainModifier.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxDrainModifier();
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
}	// ServerCreatureObjectTemplate::getMaxDrainModifier

float ServerCreatureObjectTemplate::getMaxDrainModifierMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxDrainModifierMin(true);
#endif
	}

	if (!m_maxDrainModifier.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxDrainModifier in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxDrainModifier has not been defined in template %s!", DataResource::getName()));
			return base->getMaxDrainModifierMin();
		}
	}

	float value = m_maxDrainModifier.getMinValue();
	char delta = m_maxDrainModifier.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxDrainModifierMin();
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
}	// ServerCreatureObjectTemplate::getMaxDrainModifierMin

float ServerCreatureObjectTemplate::getMaxDrainModifierMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxDrainModifierMax(true);
#endif
	}

	if (!m_maxDrainModifier.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxDrainModifier in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxDrainModifier has not been defined in template %s!", DataResource::getName()));
			return base->getMaxDrainModifierMax();
		}
	}

	float value = m_maxDrainModifier.getMaxValue();
	char delta = m_maxDrainModifier.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxDrainModifierMax();
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
}	// ServerCreatureObjectTemplate::getMaxDrainModifierMax

float ServerCreatureObjectTemplate::getMinFaucetModifier(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMinFaucetModifier(true);
#endif
	}

	if (!m_minFaucetModifier.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minFaucetModifier in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minFaucetModifier has not been defined in template %s!", DataResource::getName()));
			return base->getMinFaucetModifier();
		}
	}

	float value = m_minFaucetModifier.getValue();
	char delta = m_minFaucetModifier.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinFaucetModifier();
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
}	// ServerCreatureObjectTemplate::getMinFaucetModifier

float ServerCreatureObjectTemplate::getMinFaucetModifierMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMinFaucetModifierMin(true);
#endif
	}

	if (!m_minFaucetModifier.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minFaucetModifier in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minFaucetModifier has not been defined in template %s!", DataResource::getName()));
			return base->getMinFaucetModifierMin();
		}
	}

	float value = m_minFaucetModifier.getMinValue();
	char delta = m_minFaucetModifier.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinFaucetModifierMin();
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
}	// ServerCreatureObjectTemplate::getMinFaucetModifierMin

float ServerCreatureObjectTemplate::getMinFaucetModifierMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMinFaucetModifierMax(true);
#endif
	}

	if (!m_minFaucetModifier.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minFaucetModifier in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minFaucetModifier has not been defined in template %s!", DataResource::getName()));
			return base->getMinFaucetModifierMax();
		}
	}

	float value = m_minFaucetModifier.getMaxValue();
	char delta = m_minFaucetModifier.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinFaucetModifierMax();
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
}	// ServerCreatureObjectTemplate::getMinFaucetModifierMax

float ServerCreatureObjectTemplate::getMaxFaucetModifier(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxFaucetModifier(true);
#endif
	}

	if (!m_maxFaucetModifier.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxFaucetModifier in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxFaucetModifier has not been defined in template %s!", DataResource::getName()));
			return base->getMaxFaucetModifier();
		}
	}

	float value = m_maxFaucetModifier.getValue();
	char delta = m_maxFaucetModifier.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxFaucetModifier();
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
}	// ServerCreatureObjectTemplate::getMaxFaucetModifier

float ServerCreatureObjectTemplate::getMaxFaucetModifierMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxFaucetModifierMin(true);
#endif
	}

	if (!m_maxFaucetModifier.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxFaucetModifier in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxFaucetModifier has not been defined in template %s!", DataResource::getName()));
			return base->getMaxFaucetModifierMin();
		}
	}

	float value = m_maxFaucetModifier.getMinValue();
	char delta = m_maxFaucetModifier.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxFaucetModifierMin();
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
}	// ServerCreatureObjectTemplate::getMaxFaucetModifierMin

float ServerCreatureObjectTemplate::getMaxFaucetModifierMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxFaucetModifierMax(true);
#endif
	}

	if (!m_maxFaucetModifier.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxFaucetModifier in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxFaucetModifier has not been defined in template %s!", DataResource::getName()));
			return base->getMaxFaucetModifierMax();
		}
	}

	float value = m_maxFaucetModifier.getMaxValue();
	char delta = m_maxFaucetModifier.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxFaucetModifierMax();
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
}	// ServerCreatureObjectTemplate::getMaxFaucetModifierMax

void ServerCreatureObjectTemplate::getAttribMods(AttribMod &data, int index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_attribModsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attribMods in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attribMods has not been defined in template %s!", DataResource::getName()));
			base->getAttribMods(data, index);
			return;
		}
	}

	if (m_attribModsAppend && base != nullptr)
	{
		int baseCount = base->getAttribModsCount();
		if (index < baseCount)
			{
				base->getAttribMods(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_attribMods.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_attribMods[index]).getValue();
	NOT_NULL(structTemplate);
	const _AttribMod *param = dynamic_cast<const _AttribMod *>(structTemplate);
	NOT_NULL(param);
	data.target = param->getTarget(m_versionOk);
	data.value = param->getValue(m_versionOk);
	data.time = param->getTime(m_versionOk);
	data.timeAtValue = param->getTimeAtValue(m_versionOk);
	data.decay = param->getDecay(m_versionOk);
}	// ServerCreatureObjectTemplate::getAttribMods

void ServerCreatureObjectTemplate::getAttribModsMin(AttribMod &data, int index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_attribModsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attribMods in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attribMods has not been defined in template %s!", DataResource::getName()));
			base->getAttribModsMin(data, index);
			return;
		}
	}

	if (m_attribModsAppend && base != nullptr)
	{
		int baseCount = base->getAttribModsCount();
		if (index < baseCount)
			{
				base->getAttribModsMin(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_attribMods.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_attribMods[index]).getValue();
	NOT_NULL(structTemplate);
	const _AttribMod *param = dynamic_cast<const _AttribMod *>(structTemplate);
	NOT_NULL(param);
	data.target = param->getTarget(m_versionOk);
	data.value = param->getValueMin(m_versionOk);
	data.time = param->getTimeMin(m_versionOk);
	data.timeAtValue = param->getTimeAtValueMin(m_versionOk);
	data.decay = param->getDecayMin(m_versionOk);
}	// ServerCreatureObjectTemplate::getAttribModsMin

void ServerCreatureObjectTemplate::getAttribModsMax(AttribMod &data, int index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_attribModsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attribMods in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attribMods has not been defined in template %s!", DataResource::getName()));
			base->getAttribModsMax(data, index);
			return;
		}
	}

	if (m_attribModsAppend && base != nullptr)
	{
		int baseCount = base->getAttribModsCount();
		if (index < baseCount)
			{
				base->getAttribModsMax(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_attribMods.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_attribMods[index]).getValue();
	NOT_NULL(structTemplate);
	const _AttribMod *param = dynamic_cast<const _AttribMod *>(structTemplate);
	NOT_NULL(param);
	data.target = param->getTarget(m_versionOk);
	data.value = param->getValueMax(m_versionOk);
	data.time = param->getTimeMax(m_versionOk);
	data.timeAtValue = param->getTimeAtValueMax(m_versionOk);
	data.decay = param->getDecayMax(m_versionOk);
}	// ServerCreatureObjectTemplate::getAttribModsMax

size_t ServerCreatureObjectTemplate::getAttribModsCount(void) const
{
	if (!m_attribModsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerCreatureObjectTemplate * base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getAttribModsCount();
	}

	size_t count = m_attribMods.size();

	// if we are extending our base template, add it's count
	if (m_attribModsAppend && m_baseData != nullptr)
	{
		const ServerCreatureObjectTemplate * base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getAttribModsCount();
	}

	return count;
}	// ServerCreatureObjectTemplate::getAttribModsCount

int ServerCreatureObjectTemplate::getShockWounds(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getShockWounds(true);
#endif
	}

	if (!m_shockWounds.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter shockWounds in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter shockWounds has not been defined in template %s!", DataResource::getName()));
			return base->getShockWounds();
		}
	}

	int value = m_shockWounds.getValue();
	char delta = m_shockWounds.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getShockWounds();
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
}	// ServerCreatureObjectTemplate::getShockWounds

int ServerCreatureObjectTemplate::getShockWoundsMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getShockWoundsMin(true);
#endif
	}

	if (!m_shockWounds.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter shockWounds in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter shockWounds has not been defined in template %s!", DataResource::getName()));
			return base->getShockWoundsMin();
		}
	}

	int value = m_shockWounds.getMinValue();
	char delta = m_shockWounds.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getShockWoundsMin();
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
}	// ServerCreatureObjectTemplate::getShockWoundsMin

int ServerCreatureObjectTemplate::getShockWoundsMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getShockWoundsMax(true);
#endif
	}

	if (!m_shockWounds.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter shockWounds in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter shockWounds has not been defined in template %s!", DataResource::getName()));
			return base->getShockWoundsMax();
		}
	}

	int value = m_shockWounds.getMaxValue();
	char delta = m_shockWounds.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getShockWoundsMax();
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
}	// ServerCreatureObjectTemplate::getShockWoundsMax

bool ServerCreatureObjectTemplate::getCanCreateAvatar(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCanCreateAvatar(true);
#endif
	}

	if (!m_canCreateAvatar.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter canCreateAvatar in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter canCreateAvatar has not been defined in template %s!", DataResource::getName()));
			return base->getCanCreateAvatar();
		}
	}

	bool value = m_canCreateAvatar.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerCreatureObjectTemplate::getCanCreateAvatar

const std::string & ServerCreatureObjectTemplate::getNameGeneratorType(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getNameGeneratorType(true);
#endif
	}

	if (!m_nameGeneratorType.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter nameGeneratorType in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter nameGeneratorType has not been defined in template %s!", DataResource::getName()));
			return base->getNameGeneratorType();
		}
	}

	const std::string & value = m_nameGeneratorType.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerCreatureObjectTemplate::getNameGeneratorType

float ServerCreatureObjectTemplate::getApproachTriggerRange(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getApproachTriggerRange(true);
#endif
	}

	if (!m_approachTriggerRange.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter approachTriggerRange in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter approachTriggerRange has not been defined in template %s!", DataResource::getName()));
			return base->getApproachTriggerRange();
		}
	}

	float value = m_approachTriggerRange.getValue();
	char delta = m_approachTriggerRange.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getApproachTriggerRange();
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
}	// ServerCreatureObjectTemplate::getApproachTriggerRange

float ServerCreatureObjectTemplate::getApproachTriggerRangeMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getApproachTriggerRangeMin(true);
#endif
	}

	if (!m_approachTriggerRange.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter approachTriggerRange in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter approachTriggerRange has not been defined in template %s!", DataResource::getName()));
			return base->getApproachTriggerRangeMin();
		}
	}

	float value = m_approachTriggerRange.getMinValue();
	char delta = m_approachTriggerRange.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getApproachTriggerRangeMin();
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
}	// ServerCreatureObjectTemplate::getApproachTriggerRangeMin

float ServerCreatureObjectTemplate::getApproachTriggerRangeMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getApproachTriggerRangeMax(true);
#endif
	}

	if (!m_approachTriggerRange.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter approachTriggerRange in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter approachTriggerRange has not been defined in template %s!", DataResource::getName()));
			return base->getApproachTriggerRangeMax();
		}
	}

	float value = m_approachTriggerRange.getMaxValue();
	char delta = m_approachTriggerRange.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getApproachTriggerRangeMax();
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
}	// ServerCreatureObjectTemplate::getApproachTriggerRangeMax

float ServerCreatureObjectTemplate::getMaxMentalStates(MentalStates index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 4, ("template param index out of range"));
	if (!m_maxMentalStates[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxMentalStates in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxMentalStates has not been defined in template %s!", DataResource::getName()));
			return base->getMaxMentalStates(index);
		}
	}

	float value = m_maxMentalStates[index].getValue();
	char delta = m_maxMentalStates[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxMentalStates(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getMaxMentalStates

float ServerCreatureObjectTemplate::getMaxMentalStatesMin(MentalStates index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 4, ("template param index out of range"));
	if (!m_maxMentalStates[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxMentalStates in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxMentalStates has not been defined in template %s!", DataResource::getName()));
			return base->getMaxMentalStatesMin(index);
		}
	}

	float value = m_maxMentalStates[index].getMinValue();
	char delta = m_maxMentalStates[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxMentalStatesMin(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getMaxMentalStatesMin

float ServerCreatureObjectTemplate::getMaxMentalStatesMax(MentalStates index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 4, ("template param index out of range"));
	if (!m_maxMentalStates[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxMentalStates in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxMentalStates has not been defined in template %s!", DataResource::getName()));
			return base->getMaxMentalStatesMax(index);
		}
	}

	float value = m_maxMentalStates[index].getMaxValue();
	char delta = m_maxMentalStates[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxMentalStatesMax(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getMaxMentalStatesMax

float ServerCreatureObjectTemplate::getMentalStatesDecay(MentalStates index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 4, ("template param index out of range"));
	if (!m_mentalStatesDecay[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter mentalStatesDecay in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter mentalStatesDecay has not been defined in template %s!", DataResource::getName()));
			return base->getMentalStatesDecay(index);
		}
	}

	float value = m_mentalStatesDecay[index].getValue();
	char delta = m_mentalStatesDecay[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMentalStatesDecay(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getMentalStatesDecay

float ServerCreatureObjectTemplate::getMentalStatesDecayMin(MentalStates index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 4, ("template param index out of range"));
	if (!m_mentalStatesDecay[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter mentalStatesDecay in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter mentalStatesDecay has not been defined in template %s!", DataResource::getName()));
			return base->getMentalStatesDecayMin(index);
		}
	}

	float value = m_mentalStatesDecay[index].getMinValue();
	char delta = m_mentalStatesDecay[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMentalStatesDecayMin(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getMentalStatesDecayMin

float ServerCreatureObjectTemplate::getMentalStatesDecayMax(MentalStates index) const
{
	const ServerCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 4, ("template param index out of range"));
	if (!m_mentalStatesDecay[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter mentalStatesDecay in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter mentalStatesDecay has not been defined in template %s!", DataResource::getName()));
			return base->getMentalStatesDecayMax(index);
		}
	}

	float value = m_mentalStatesDecay[index].getMaxValue();
	char delta = m_mentalStatesDecay[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMentalStatesDecayMax(index);
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
	return value;
}	// ServerCreatureObjectTemplate::getMentalStatesDecayMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerCreatureObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getMinDrainModifierMin(true));
	IGNORE_RETURN(getMinDrainModifierMax(true));
	IGNORE_RETURN(getMaxDrainModifierMin(true));
	IGNORE_RETURN(getMaxDrainModifierMax(true));
	IGNORE_RETURN(getMinFaucetModifierMin(true));
	IGNORE_RETURN(getMinFaucetModifierMax(true));
	IGNORE_RETURN(getMaxFaucetModifierMin(true));
	IGNORE_RETURN(getMaxFaucetModifierMax(true));
	IGNORE_RETURN(getShockWoundsMin(true));
	IGNORE_RETURN(getShockWoundsMax(true));
	IGNORE_RETURN(getCanCreateAvatar(true));
	IGNORE_RETURN(getNameGeneratorType(true));
	IGNORE_RETURN(getApproachTriggerRangeMin(true));
	IGNORE_RETURN(getApproachTriggerRangeMax(true));
	ServerTangibleObjectTemplate::testValues();
}	// ServerCreatureObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerCreatureObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerCreatureObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,0,5))
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
		if (strcmp(paramName, "defaultWeapon") == 0)
			m_defaultWeapon.loadFromIff(file);
		else if (strcmp(paramName, "attributes") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 6, ("Template %s: read array size of %d for array \"attributes\" of size 6, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 6 && j < listCount; ++j)
				m_attributes[j].loadFromIff(file);
			// if there are more params for attributes read and dump them
			for (; j < listCount; ++j)
			{
				IntegerParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "minAttributes") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 6, ("Template %s: read array size of %d for array \"minAttributes\" of size 6, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 6 && j < listCount; ++j)
				m_minAttributes[j].loadFromIff(file);
			// if there are more params for minAttributes read and dump them
			for (; j < listCount; ++j)
			{
				IntegerParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "maxAttributes") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 6, ("Template %s: read array size of %d for array \"maxAttributes\" of size 6, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 6 && j < listCount; ++j)
				m_maxAttributes[j].loadFromIff(file);
			// if there are more params for maxAttributes read and dump them
			for (; j < listCount; ++j)
			{
				IntegerParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "minDrainModifier") == 0)
			m_minDrainModifier.loadFromIff(file);
		else if (strcmp(paramName, "maxDrainModifier") == 0)
			m_maxDrainModifier.loadFromIff(file);
		else if (strcmp(paramName, "minFaucetModifier") == 0)
			m_minFaucetModifier.loadFromIff(file);
		else if (strcmp(paramName, "maxFaucetModifier") == 0)
			m_maxFaucetModifier.loadFromIff(file);
		else if (strcmp(paramName, "attribMods") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_attribMods.begin(); iter != m_attribMods.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_attribMods.clear();
			m_attribModsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_attribMods.push_back(newData);
			}
			m_attribModsLoaded = true;
		}
		else if (strcmp(paramName, "shockWounds") == 0)
			m_shockWounds.loadFromIff(file);
		else if (strcmp(paramName, "canCreateAvatar") == 0)
			m_canCreateAvatar.loadFromIff(file);
		else if (strcmp(paramName, "nameGeneratorType") == 0)
			m_nameGeneratorType.loadFromIff(file);
		else if (strcmp(paramName, "approachTriggerRange") == 0)
			m_approachTriggerRange.loadFromIff(file);
		else if (strcmp(paramName, "maxMentalStates") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 4, ("Template %s: read array size of %d for array \"maxMentalStates\" of size 4, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 4 && j < listCount; ++j)
				m_maxMentalStates[j].loadFromIff(file);
			// if there are more params for maxMentalStates read and dump them
			for (; j < listCount; ++j)
			{
				FloatParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "mentalStatesDecay") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 4, ("Template %s: read array size of %d for array \"mentalStatesDecay\" of size 4, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 4 && j < listCount; ++j)
				m_mentalStatesDecay[j].loadFromIff(file);
			// if there are more params for mentalStatesDecay read and dump them
			for (; j < listCount; ++j)
			{
				FloatParam dummy;
				dummy.loadFromIff(file);
			}
		}
		file.exitChunk(true);
	}

	file.exitForm();
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerCreatureObjectTemplate::load

//@END TFD

/*
Unicode::String ServerCreatureObjectTemplate::generateRandomName(StringId &o_errorMessage) const
{
	o_errorMessage = NameErrors::nameApproved;
	return NameManager::getInstance().generateRandomName("name",getNameGeneratorType());
}

const StringId &ServerCreatureObjectTemplate::verifyName(const Unicode::String &name) const
{
	if (NameManager::getInstance().isNameAppropriate("name",getNameGeneratorType(),name))
		return NameErrors::nameApproved;
	else
		return NameErrors::nameDeclinedRaciallyInappropriate;
}
*/

void ServerCreatureObjectTemplate::postLoad()
{
	ServerTangibleObjectTemplate::postLoad();
}
