//========================================================================
//
// ServerWeaponObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerWeaponObjectTemplate.h"
#include "serverGame/WeaponObject.h"
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

bool ServerWeaponObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerWeaponObjectTemplate::ServerWeaponObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerWeaponObjectTemplate::ServerWeaponObjectTemplate

/**
 * Class destructor.
 */
ServerWeaponObjectTemplate::~ServerWeaponObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// ServerWeaponObjectTemplate::~ServerWeaponObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerWeaponObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerWeaponObjectTemplate_tag, create);
}	// ServerWeaponObjectTemplate::registerMe

/**
 * Creates a ServerWeaponObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerWeaponObjectTemplate::create(const std::string & filename)
{
	return new ServerWeaponObjectTemplate(filename);
}	// ServerWeaponObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerWeaponObjectTemplate::getId(void) const
{
	return ServerWeaponObjectTemplate_tag;
}	// ServerWeaponObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerWeaponObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerWeaponObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerWeaponObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const ServerWeaponObjectTemplate * base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerWeaponObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerWeaponObjectTemplate::createObject(void) const
{
	return new WeaponObject(this);
}	// ServerWeaponObjectTemplate::createObject

//@BEGIN TFD
ServerWeaponObjectTemplate::WeaponType ServerWeaponObjectTemplate::getWeaponType() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_weaponType.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter weaponType in template %s", DataResource::getName()));
			return static_cast<WeaponType>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter weaponType has not been defined in template %s!", DataResource::getName()));
			return base->getWeaponType();
		}
	}

	WeaponType value = static_cast<WeaponType>(m_weaponType.getValue());

	return value;
}	// ServerWeaponObjectTemplate::getWeaponType

ServerWeaponObjectTemplate::AttackType ServerWeaponObjectTemplate::getAttackType() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_attackType.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attackType in template %s", DataResource::getName()));
			return static_cast<AttackType>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attackType has not been defined in template %s!", DataResource::getName()));
			return base->getAttackType();
		}
	}

	AttackType value = static_cast<AttackType>(m_attackType.getValue());

	return value;
}	// ServerWeaponObjectTemplate::getAttackType

ServerWeaponObjectTemplate::DamageType ServerWeaponObjectTemplate::getDamageType() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_damageType.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter damageType in template %s", DataResource::getName()));
			return static_cast<DamageType>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter damageType has not been defined in template %s!", DataResource::getName()));
			return base->getDamageType();
		}
	}

	DamageType value = static_cast<DamageType>(m_damageType.getValue());

	return value;
}	// ServerWeaponObjectTemplate::getDamageType

ServerWeaponObjectTemplate::DamageType ServerWeaponObjectTemplate::getElementalType() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_elementalType.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter elementalType in template %s", DataResource::getName()));
			return static_cast<DamageType>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter elementalType has not been defined in template %s!", DataResource::getName()));
			return base->getElementalType();
		}
	}

	DamageType value = static_cast<DamageType>(m_elementalType.getValue());

	return value;
}	// ServerWeaponObjectTemplate::getElementalType

int ServerWeaponObjectTemplate::getElementalValue() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_elementalValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter elementalValue in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter elementalValue has not been defined in template %s!", DataResource::getName()));
			return base->getElementalValue();
		}
	}

	int value = m_elementalValue.getValue();
	char delta = m_elementalValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getElementalValue();
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
}	// ServerWeaponObjectTemplate::getElementalValue

int ServerWeaponObjectTemplate::getElementalValueMin() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_elementalValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter elementalValue in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter elementalValue has not been defined in template %s!", DataResource::getName()));
			return base->getElementalValueMin();
		}
	}

	int value = m_elementalValue.getMinValue();
	char delta = m_elementalValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getElementalValueMin();
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
}	// ServerWeaponObjectTemplate::getElementalValueMin

int ServerWeaponObjectTemplate::getElementalValueMax() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_elementalValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter elementalValue in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter elementalValue has not been defined in template %s!", DataResource::getName()));
			return base->getElementalValueMax();
		}
	}

	int value = m_elementalValue.getMaxValue();
	char delta = m_elementalValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getElementalValueMax();
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
}	// ServerWeaponObjectTemplate::getElementalValueMax

int ServerWeaponObjectTemplate::getMinDamageAmount() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_minDamageAmount.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minDamageAmount in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minDamageAmount has not been defined in template %s!", DataResource::getName()));
			return base->getMinDamageAmount();
		}
	}

	int value = m_minDamageAmount.getValue();
	char delta = m_minDamageAmount.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinDamageAmount();
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
}	// ServerWeaponObjectTemplate::getMinDamageAmount

int ServerWeaponObjectTemplate::getMinDamageAmountMin() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_minDamageAmount.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minDamageAmount in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minDamageAmount has not been defined in template %s!", DataResource::getName()));
			return base->getMinDamageAmountMin();
		}
	}

	int value = m_minDamageAmount.getMinValue();
	char delta = m_minDamageAmount.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinDamageAmountMin();
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
}	// ServerWeaponObjectTemplate::getMinDamageAmountMin

int ServerWeaponObjectTemplate::getMinDamageAmountMax() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_minDamageAmount.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minDamageAmount in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minDamageAmount has not been defined in template %s!", DataResource::getName()));
			return base->getMinDamageAmountMax();
		}
	}

	int value = m_minDamageAmount.getMaxValue();
	char delta = m_minDamageAmount.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinDamageAmountMax();
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
}	// ServerWeaponObjectTemplate::getMinDamageAmountMax

int ServerWeaponObjectTemplate::getMaxDamageAmount() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_maxDamageAmount.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxDamageAmount in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxDamageAmount has not been defined in template %s!", DataResource::getName()));
			return base->getMaxDamageAmount();
		}
	}

	int value = m_maxDamageAmount.getValue();
	char delta = m_maxDamageAmount.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxDamageAmount();
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
}	// ServerWeaponObjectTemplate::getMaxDamageAmount

int ServerWeaponObjectTemplate::getMaxDamageAmountMin() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_maxDamageAmount.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxDamageAmount in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxDamageAmount has not been defined in template %s!", DataResource::getName()));
			return base->getMaxDamageAmountMin();
		}
	}

	int value = m_maxDamageAmount.getMinValue();
	char delta = m_maxDamageAmount.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxDamageAmountMin();
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
}	// ServerWeaponObjectTemplate::getMaxDamageAmountMin

int ServerWeaponObjectTemplate::getMaxDamageAmountMax() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_maxDamageAmount.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxDamageAmount in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxDamageAmount has not been defined in template %s!", DataResource::getName()));
			return base->getMaxDamageAmountMax();
		}
	}

	int value = m_maxDamageAmount.getMaxValue();
	char delta = m_maxDamageAmount.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxDamageAmountMax();
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
}	// ServerWeaponObjectTemplate::getMaxDamageAmountMax

float ServerWeaponObjectTemplate::getAttackSpeed() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_attackSpeed.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attackSpeed in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attackSpeed has not been defined in template %s!", DataResource::getName()));
			return base->getAttackSpeed();
		}
	}

	float value = m_attackSpeed.getValue();
	char delta = m_attackSpeed.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAttackSpeed();
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
}	// ServerWeaponObjectTemplate::getAttackSpeed

float ServerWeaponObjectTemplate::getAttackSpeedMin() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_attackSpeed.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attackSpeed in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attackSpeed has not been defined in template %s!", DataResource::getName()));
			return base->getAttackSpeedMin();
		}
	}

	float value = m_attackSpeed.getMinValue();
	char delta = m_attackSpeed.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAttackSpeedMin();
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
}	// ServerWeaponObjectTemplate::getAttackSpeedMin

float ServerWeaponObjectTemplate::getAttackSpeedMax() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_attackSpeed.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attackSpeed in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attackSpeed has not been defined in template %s!", DataResource::getName()));
			return base->getAttackSpeedMax();
		}
	}

	float value = m_attackSpeed.getMaxValue();
	char delta = m_attackSpeed.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAttackSpeedMax();
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
}	// ServerWeaponObjectTemplate::getAttackSpeedMax

float ServerWeaponObjectTemplate::getAudibleRange() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_audibleRange.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter audibleRange in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter audibleRange has not been defined in template %s!", DataResource::getName()));
			return base->getAudibleRange();
		}
	}

	float value = m_audibleRange.getValue();
	char delta = m_audibleRange.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAudibleRange();
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
}	// ServerWeaponObjectTemplate::getAudibleRange

float ServerWeaponObjectTemplate::getAudibleRangeMin() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_audibleRange.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter audibleRange in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter audibleRange has not been defined in template %s!", DataResource::getName()));
			return base->getAudibleRangeMin();
		}
	}

	float value = m_audibleRange.getMinValue();
	char delta = m_audibleRange.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAudibleRangeMin();
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
}	// ServerWeaponObjectTemplate::getAudibleRangeMin

float ServerWeaponObjectTemplate::getAudibleRangeMax() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_audibleRange.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter audibleRange in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter audibleRange has not been defined in template %s!", DataResource::getName()));
			return base->getAudibleRangeMax();
		}
	}

	float value = m_audibleRange.getMaxValue();
	char delta = m_audibleRange.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAudibleRangeMax();
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
}	// ServerWeaponObjectTemplate::getAudibleRangeMax

float ServerWeaponObjectTemplate::getMinRange() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_minRange.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minRange in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minRange has not been defined in template %s!", DataResource::getName()));
			return base->getMinRange();
		}
	}

	float value = m_minRange.getValue();
	char delta = m_minRange.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinRange();
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
}	// ServerWeaponObjectTemplate::getMinRange

float ServerWeaponObjectTemplate::getMinRangeMin() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_minRange.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minRange in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minRange has not been defined in template %s!", DataResource::getName()));
			return base->getMinRangeMin();
		}
	}

	float value = m_minRange.getMinValue();
	char delta = m_minRange.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinRangeMin();
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
}	// ServerWeaponObjectTemplate::getMinRangeMin

float ServerWeaponObjectTemplate::getMinRangeMax() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_minRange.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minRange in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minRange has not been defined in template %s!", DataResource::getName()));
			return base->getMinRangeMax();
		}
	}

	float value = m_minRange.getMaxValue();
	char delta = m_minRange.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinRangeMax();
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
}	// ServerWeaponObjectTemplate::getMinRangeMax

float ServerWeaponObjectTemplate::getMaxRange() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_maxRange.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxRange in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxRange has not been defined in template %s!", DataResource::getName()));
			return base->getMaxRange();
		}
	}

	float value = m_maxRange.getValue();
	char delta = m_maxRange.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxRange();
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
}	// ServerWeaponObjectTemplate::getMaxRange

float ServerWeaponObjectTemplate::getMaxRangeMin() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_maxRange.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxRange in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxRange has not been defined in template %s!", DataResource::getName()));
			return base->getMaxRangeMin();
		}
	}

	float value = m_maxRange.getMinValue();
	char delta = m_maxRange.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxRangeMin();
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
}	// ServerWeaponObjectTemplate::getMaxRangeMin

float ServerWeaponObjectTemplate::getMaxRangeMax() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_maxRange.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxRange in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxRange has not been defined in template %s!", DataResource::getName()));
			return base->getMaxRangeMax();
		}
	}

	float value = m_maxRange.getMaxValue();
	char delta = m_maxRange.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxRangeMax();
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
}	// ServerWeaponObjectTemplate::getMaxRangeMax

float ServerWeaponObjectTemplate::getDamageRadius() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_damageRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter damageRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter damageRadius has not been defined in template %s!", DataResource::getName()));
			return base->getDamageRadius();
		}
	}

	float value = m_damageRadius.getValue();
	char delta = m_damageRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDamageRadius();
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
}	// ServerWeaponObjectTemplate::getDamageRadius

float ServerWeaponObjectTemplate::getDamageRadiusMin() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_damageRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter damageRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter damageRadius has not been defined in template %s!", DataResource::getName()));
			return base->getDamageRadiusMin();
		}
	}

	float value = m_damageRadius.getMinValue();
	char delta = m_damageRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDamageRadiusMin();
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
}	// ServerWeaponObjectTemplate::getDamageRadiusMin

float ServerWeaponObjectTemplate::getDamageRadiusMax() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_damageRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter damageRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter damageRadius has not been defined in template %s!", DataResource::getName()));
			return base->getDamageRadiusMax();
		}
	}

	float value = m_damageRadius.getMaxValue();
	char delta = m_damageRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDamageRadiusMax();
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
}	// ServerWeaponObjectTemplate::getDamageRadiusMax

float ServerWeaponObjectTemplate::getWoundChance() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_woundChance.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter woundChance in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter woundChance has not been defined in template %s!", DataResource::getName()));
			return base->getWoundChance();
		}
	}

	float value = m_woundChance.getValue();
	char delta = m_woundChance.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getWoundChance();
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
}	// ServerWeaponObjectTemplate::getWoundChance

float ServerWeaponObjectTemplate::getWoundChanceMin() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_woundChance.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter woundChance in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter woundChance has not been defined in template %s!", DataResource::getName()));
			return base->getWoundChanceMin();
		}
	}

	float value = m_woundChance.getMinValue();
	char delta = m_woundChance.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getWoundChanceMin();
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
}	// ServerWeaponObjectTemplate::getWoundChanceMin

float ServerWeaponObjectTemplate::getWoundChanceMax() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_woundChance.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter woundChance in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter woundChance has not been defined in template %s!", DataResource::getName()));
			return base->getWoundChanceMax();
		}
	}

	float value = m_woundChance.getMaxValue();
	char delta = m_woundChance.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getWoundChanceMax();
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
}	// ServerWeaponObjectTemplate::getWoundChanceMax

int ServerWeaponObjectTemplate::getAttackCost() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_attackCost.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attackCost in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attackCost has not been defined in template %s!", DataResource::getName()));
			return base->getAttackCost();
		}
	}

	int value = m_attackCost.getValue();
	char delta = m_attackCost.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAttackCost();
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
}	// ServerWeaponObjectTemplate::getAttackCost

int ServerWeaponObjectTemplate::getAttackCostMin() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_attackCost.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attackCost in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attackCost has not been defined in template %s!", DataResource::getName()));
			return base->getAttackCostMin();
		}
	}

	int value = m_attackCost.getMinValue();
	char delta = m_attackCost.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAttackCostMin();
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
}	// ServerWeaponObjectTemplate::getAttackCostMin

int ServerWeaponObjectTemplate::getAttackCostMax() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_attackCost.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attackCost in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attackCost has not been defined in template %s!", DataResource::getName()));
			return base->getAttackCostMax();
		}
	}

	int value = m_attackCost.getMaxValue();
	char delta = m_attackCost.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAttackCostMax();
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
}	// ServerWeaponObjectTemplate::getAttackCostMax

int ServerWeaponObjectTemplate::getAccuracy() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_accuracy.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter accuracy in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter accuracy has not been defined in template %s!", DataResource::getName()));
			return base->getAccuracy();
		}
	}

	int value = m_accuracy.getValue();
	char delta = m_accuracy.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAccuracy();
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
}	// ServerWeaponObjectTemplate::getAccuracy

int ServerWeaponObjectTemplate::getAccuracyMin() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_accuracy.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter accuracy in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter accuracy has not been defined in template %s!", DataResource::getName()));
			return base->getAccuracyMin();
		}
	}

	int value = m_accuracy.getMinValue();
	char delta = m_accuracy.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAccuracyMin();
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
}	// ServerWeaponObjectTemplate::getAccuracyMin

int ServerWeaponObjectTemplate::getAccuracyMax() const
{


	const ServerWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_accuracy.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter accuracy in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter accuracy has not been defined in template %s!", DataResource::getName()));
			return base->getAccuracyMax();
		}
	}

	int value = m_accuracy.getMaxValue();
	char delta = m_accuracy.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAccuracyMax();
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
}	// ServerWeaponObjectTemplate::getAccuracyMax


/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerWeaponObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerWeaponObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,1,1))
	{
		
			
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
		if (strcmp(paramName, "weaponType") == 0)
			m_weaponType.loadFromIff(file);
		else if (strcmp(paramName, "attackType") == 0)
			m_attackType.loadFromIff(file);
		else if (strcmp(paramName, "damageType") == 0)
			m_damageType.loadFromIff(file);
		else if (strcmp(paramName, "elementalType") == 0)
			m_elementalType.loadFromIff(file);
		else if (strcmp(paramName, "elementalValue") == 0)
			m_elementalValue.loadFromIff(file);
		else if (strcmp(paramName, "minDamageAmount") == 0)
			m_minDamageAmount.loadFromIff(file);
		else if (strcmp(paramName, "maxDamageAmount") == 0)
			m_maxDamageAmount.loadFromIff(file);
		else if (strcmp(paramName, "attackSpeed") == 0)
			m_attackSpeed.loadFromIff(file);
		else if (strcmp(paramName, "audibleRange") == 0)
			m_audibleRange.loadFromIff(file);
		else if (strcmp(paramName, "minRange") == 0)
			m_minRange.loadFromIff(file);
		else if (strcmp(paramName, "maxRange") == 0)
			m_maxRange.loadFromIff(file);
		else if (strcmp(paramName, "damageRadius") == 0)
			m_damageRadius.loadFromIff(file);
		else if (strcmp(paramName, "woundChance") == 0)
			m_woundChance.loadFromIff(file);
		else if (strcmp(paramName, "attackCost") == 0)
			m_attackCost.loadFromIff(file);
		else if (strcmp(paramName, "accuracy") == 0)
			m_accuracy.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerWeaponObjectTemplate::load

//@END TFD
