//========================================================================
//
// ServerTangibleObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerTangibleObjectTemplate.h"
#include "serverGame/TangibleObject.h"
#include "sharedFile/Iff.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
//@BEGIN TFD TEMPLATE REFS
#include "ServerArmorTemplate.h"
//@END TFD TEMPLATE REFS
#include <stdio.h>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool ServerTangibleObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerTangibleObjectTemplate::ServerTangibleObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerObjectTemplate(filename)
	,m_triggerVolumesLoaded(false)
	,m_triggerVolumesAppend(false)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerTangibleObjectTemplate::ServerTangibleObjectTemplate

/**
 * Class destructor.
 */
ServerTangibleObjectTemplate::~ServerTangibleObjectTemplate()
{
//@BEGIN TFD CLEANUP
	{
		std::vector<TriggerVolumeParam *>::iterator iter;
		for (iter = m_triggerVolumes.begin(); iter != m_triggerVolumes.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_triggerVolumes.clear();
	}
//@END TFD CLEANUP
}	// ServerTangibleObjectTemplate::~ServerTangibleObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerTangibleObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerTangibleObjectTemplate_tag, create);
}	// ServerTangibleObjectTemplate::registerMe

/**
 * Creates a ServerTangibleObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerTangibleObjectTemplate::create(const std::string & filename)
{
	return new ServerTangibleObjectTemplate(filename);
}	// ServerTangibleObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerTangibleObjectTemplate::getId(void) const
{
	return ServerTangibleObjectTemplate_tag;
}	// ServerTangibleObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerTangibleObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerTangibleObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerTangibleObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerTangibleObjectTemplate * base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerTangibleObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerTangibleObjectTemplate::createObject(void) const
{
	return new TangibleObject(this);
}	// ServerTangibleObjectTemplate::createObject

//@BEGIN TFD
const TriggerVolumeData ServerTangibleObjectTemplate::getTriggerVolumes(int index) const
{
	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_triggerVolumesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter triggerVolumes in template %s", DataResource::getName()));
			return DefaultTriggerVolumeData;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter triggerVolumes has not been defined in template %s!", DataResource::getName()));
			return base->getTriggerVolumes(index);
		}
	}

	if (m_triggerVolumesAppend && base != nullptr)
	{
		int baseCount = base->getTriggerVolumesCount();
		if (index < baseCount)
			return base->getTriggerVolumes(index);
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_triggerVolumes.size(), ("template param index out of range"));

	return m_triggerVolumes[index]->getValue();
}	// ServerTangibleObjectTemplate::getTriggerVolumes

size_t ServerTangibleObjectTemplate::getTriggerVolumesCount(void) const
{
	if (!m_triggerVolumesLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerTangibleObjectTemplate * base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getTriggerVolumesCount();
	}

	size_t count = m_triggerVolumes.size();

	// if we are extending our base template, add it's count
	if (m_triggerVolumesAppend && m_baseData != nullptr)
	{
		const ServerTangibleObjectTemplate * base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getTriggerVolumesCount();
	}

	return count;
}	// ServerTangibleObjectTemplate::getTriggerVolumesCount

ServerTangibleObjectTemplate::CombatSkeleton ServerTangibleObjectTemplate::getCombatSkeleton(bool testData) const
{
#ifdef _DEBUG
ServerTangibleObjectTemplate::CombatSkeleton testDataValue = static_cast<ServerTangibleObjectTemplate::CombatSkeleton>(0);
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCombatSkeleton(true);
#endif
	}

	if (!m_combatSkeleton.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter combatSkeleton in template %s", DataResource::getName()));
			return static_cast<CombatSkeleton>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter combatSkeleton has not been defined in template %s!", DataResource::getName()));
			return base->getCombatSkeleton();
		}
	}

	CombatSkeleton value = static_cast<CombatSkeleton>(m_combatSkeleton.getValue());
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerTangibleObjectTemplate::getCombatSkeleton

int ServerTangibleObjectTemplate::getMaxHitPoints(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxHitPoints(true);
#endif
	}

	if (!m_maxHitPoints.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxHitPoints in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxHitPoints has not been defined in template %s!", DataResource::getName()));
			return base->getMaxHitPoints();
		}
	}

	int value = m_maxHitPoints.getValue();
	char delta = m_maxHitPoints.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxHitPoints();
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
}	// ServerTangibleObjectTemplate::getMaxHitPoints

int ServerTangibleObjectTemplate::getMaxHitPointsMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxHitPointsMin(true);
#endif
	}

	if (!m_maxHitPoints.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxHitPoints in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxHitPoints has not been defined in template %s!", DataResource::getName()));
			return base->getMaxHitPointsMin();
		}
	}

	int value = m_maxHitPoints.getMinValue();
	char delta = m_maxHitPoints.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxHitPointsMin();
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
}	// ServerTangibleObjectTemplate::getMaxHitPointsMin

int ServerTangibleObjectTemplate::getMaxHitPointsMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxHitPointsMax(true);
#endif
	}

	if (!m_maxHitPoints.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxHitPoints in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxHitPoints has not been defined in template %s!", DataResource::getName()));
			return base->getMaxHitPointsMax();
		}
	}

	int value = m_maxHitPoints.getMaxValue();
	char delta = m_maxHitPoints.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxHitPointsMax();
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
}	// ServerTangibleObjectTemplate::getMaxHitPointsMax

const ServerArmorTemplate * ServerTangibleObjectTemplate::getArmor() const
{
	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_armor.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter armor in template %s", DataResource::getName()));
			return nullptr;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter armor has not been defined in template %s!", DataResource::getName()));
			return base->getArmor();
		}
	}

	const ServerArmorTemplate * returnValue = nullptr;
	const std::string & templateName = m_armor.getValue();
	if (!templateName.empty())
	{
		returnValue = dynamic_cast<const ServerArmorTemplate *>(ObjectTemplateList::fetch(templateName));
		if (returnValue == nullptr)
			WARNING_STRICT_FATAL(true, ("Error loading template %s",templateName.c_str()));
	}
	return returnValue;
}	// ServerTangibleObjectTemplate::getArmor

int ServerTangibleObjectTemplate::getInterestRadius(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getInterestRadius(true);
#endif
	}

	if (!m_interestRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter interestRadius in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter interestRadius has not been defined in template %s!", DataResource::getName()));
			return base->getInterestRadius();
		}
	}

	int value = m_interestRadius.getValue();
	char delta = m_interestRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getInterestRadius();
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
}	// ServerTangibleObjectTemplate::getInterestRadius

int ServerTangibleObjectTemplate::getInterestRadiusMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getInterestRadiusMin(true);
#endif
	}

	if (!m_interestRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter interestRadius in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter interestRadius has not been defined in template %s!", DataResource::getName()));
			return base->getInterestRadiusMin();
		}
	}

	int value = m_interestRadius.getMinValue();
	char delta = m_interestRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getInterestRadiusMin();
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
}	// ServerTangibleObjectTemplate::getInterestRadiusMin

int ServerTangibleObjectTemplate::getInterestRadiusMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getInterestRadiusMax(true);
#endif
	}

	if (!m_interestRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter interestRadius in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter interestRadius has not been defined in template %s!", DataResource::getName()));
			return base->getInterestRadiusMax();
		}
	}

	int value = m_interestRadius.getMaxValue();
	char delta = m_interestRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getInterestRadiusMax();
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
}	// ServerTangibleObjectTemplate::getInterestRadiusMax

int ServerTangibleObjectTemplate::getCount(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCount(true);
#endif
	}

	if (!m_count.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter count in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter count has not been defined in template %s!", DataResource::getName()));
			return base->getCount();
		}
	}

	int value = m_count.getValue();
	char delta = m_count.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCount();
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
}	// ServerTangibleObjectTemplate::getCount

int ServerTangibleObjectTemplate::getCountMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCountMin(true);
#endif
	}

	if (!m_count.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter count in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter count has not been defined in template %s!", DataResource::getName()));
			return base->getCountMin();
		}
	}

	int value = m_count.getMinValue();
	char delta = m_count.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCountMin();
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
}	// ServerTangibleObjectTemplate::getCountMin

int ServerTangibleObjectTemplate::getCountMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCountMax(true);
#endif
	}

	if (!m_count.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter count in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter count has not been defined in template %s!", DataResource::getName()));
			return base->getCountMax();
		}
	}

	int value = m_count.getMaxValue();
	char delta = m_count.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCountMax();
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
}	// ServerTangibleObjectTemplate::getCountMax

int ServerTangibleObjectTemplate::getCondition(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCondition(true);
#endif
	}

	if (!m_condition.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter condition in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter condition has not been defined in template %s!", DataResource::getName()));
			return base->getCondition();
		}
	}

	int value = m_condition.getValue();
	char delta = m_condition.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCondition();
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
}	// ServerTangibleObjectTemplate::getCondition

int ServerTangibleObjectTemplate::getConditionMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getConditionMin(true);
#endif
	}

	if (!m_condition.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter condition in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter condition has not been defined in template %s!", DataResource::getName()));
			return base->getConditionMin();
		}
	}

	int value = m_condition.getMinValue();
	char delta = m_condition.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getConditionMin();
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
}	// ServerTangibleObjectTemplate::getConditionMin

int ServerTangibleObjectTemplate::getConditionMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getConditionMax(true);
#endif
	}

	if (!m_condition.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter condition in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter condition has not been defined in template %s!", DataResource::getName()));
			return base->getConditionMax();
		}
	}

	int value = m_condition.getMaxValue();
	char delta = m_condition.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getConditionMax();
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
}	// ServerTangibleObjectTemplate::getConditionMax

bool ServerTangibleObjectTemplate::getWantSawAttackTriggers(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const ServerTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getWantSawAttackTriggers(true);
#endif
	}

	if (!m_wantSawAttackTriggers.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter wantSawAttackTriggers in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter wantSawAttackTriggers has not been defined in template %s!", DataResource::getName()));
			return base->getWantSawAttackTriggers();
		}
	}

	bool value = m_wantSawAttackTriggers.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerTangibleObjectTemplate::getWantSawAttackTriggers

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerTangibleObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getCombatSkeleton(true));
	IGNORE_RETURN(getMaxHitPointsMin(true));
	IGNORE_RETURN(getMaxHitPointsMax(true));
	IGNORE_RETURN(getInterestRadiusMin(true));
	IGNORE_RETURN(getInterestRadiusMax(true));
	IGNORE_RETURN(getCountMin(true));
	IGNORE_RETURN(getCountMax(true));
	IGNORE_RETURN(getConditionMin(true));
	IGNORE_RETURN(getConditionMax(true));
	IGNORE_RETURN(getWantSawAttackTriggers(true));
	ServerObjectTemplate::testValues();
}	// ServerTangibleObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerTangibleObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerTangibleObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,0,4))
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
		if (strcmp(paramName, "triggerVolumes") == 0)
		{
			std::vector<TriggerVolumeParam *>::iterator iter;
			for (iter = m_triggerVolumes.begin(); iter != m_triggerVolumes.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_triggerVolumes.clear();
			m_triggerVolumesAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				TriggerVolumeParam * newData = new TriggerVolumeParam;
				newData->loadFromIff(file);
				m_triggerVolumes.push_back(newData);
			}
			m_triggerVolumesLoaded = true;
		}
		else if (strcmp(paramName, "combatSkeleton") == 0)
			m_combatSkeleton.loadFromIff(file);
		else if (strcmp(paramName, "maxHitPoints") == 0)
			m_maxHitPoints.loadFromIff(file);
		else if (strcmp(paramName, "armor") == 0)
			m_armor.loadFromIff(file);
		else if (strcmp(paramName, "interestRadius") == 0)
			m_interestRadius.loadFromIff(file);
		else if (strcmp(paramName, "count") == 0)
			m_count.loadFromIff(file);
		else if (strcmp(paramName, "condition") == 0)
			m_condition.loadFromIff(file);
		else if (strcmp(paramName, "wantSawAttackTriggers") == 0)
			m_wantSawAttackTriggers.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerTangibleObjectTemplate::load

//@END TFD
