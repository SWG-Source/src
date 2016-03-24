//========================================================================
//
// SharedWeaponObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedWeaponObjectTemplate.h"
#include "sharedFile/Iff.h"
#include "sharedMath/Vector.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS
#include <algorithm>
#include <cstdio>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool SharedWeaponObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedWeaponObjectTemplate::SharedWeaponObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedWeaponObjectTemplate::SharedWeaponObjectTemplate

/**
 * Class destructor.
 */
SharedWeaponObjectTemplate::~SharedWeaponObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedWeaponObjectTemplate::~SharedWeaponObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedWeaponObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedWeaponObjectTemplate_tag, create);
}	// SharedWeaponObjectTemplate::registerMe

/**
 * Creates a SharedWeaponObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedWeaponObjectTemplate::create(const std::string & filename)
{
	return new SharedWeaponObjectTemplate(filename);
}	// SharedWeaponObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedWeaponObjectTemplate::getId(void) const
{
	return SharedWeaponObjectTemplate_tag;
}	// SharedWeaponObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedWeaponObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedWeaponObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedWeaponObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedWeaponObjectTemplate * base = dynamic_cast<const SharedWeaponObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedWeaponObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
const std::string & SharedWeaponObjectTemplate::getWeaponEffect() const
{


	const SharedWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_weaponEffect.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter weaponEffect in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter weaponEffect has not been defined in template %s!", DataResource::getName()));
			return base->getWeaponEffect();
		}
	}

	const std::string & value = m_weaponEffect.getValue();

	return value;
}	// SharedWeaponObjectTemplate::getWeaponEffect

int SharedWeaponObjectTemplate::getWeaponEffectIndex() const
{


	const SharedWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_weaponEffectIndex.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter weaponEffectIndex in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter weaponEffectIndex has not been defined in template %s!", DataResource::getName()));
			return base->getWeaponEffectIndex();
		}
	}

	int value = m_weaponEffectIndex.getValue();
	char delta = m_weaponEffectIndex.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getWeaponEffectIndex();
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
}	// SharedWeaponObjectTemplate::getWeaponEffectIndex

int SharedWeaponObjectTemplate::getWeaponEffectIndexMin() const
{


	const SharedWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_weaponEffectIndex.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter weaponEffectIndex in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter weaponEffectIndex has not been defined in template %s!", DataResource::getName()));
			return base->getWeaponEffectIndexMin();
		}
	}

	int value = m_weaponEffectIndex.getMinValue();
	char delta = m_weaponEffectIndex.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getWeaponEffectIndexMin();
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
}	// SharedWeaponObjectTemplate::getWeaponEffectIndexMin

int SharedWeaponObjectTemplate::getWeaponEffectIndexMax() const
{


	const SharedWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedWeaponObjectTemplate *>(m_baseData);
	}

	if (!m_weaponEffectIndex.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter weaponEffectIndex in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter weaponEffectIndex has not been defined in template %s!", DataResource::getName()));
			return base->getWeaponEffectIndexMax();
		}
	}

	int value = m_weaponEffectIndex.getMaxValue();
	char delta = m_weaponEffectIndex.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getWeaponEffectIndexMax();
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
}	// SharedWeaponObjectTemplate::getWeaponEffectIndexMax

SharedWeaponObjectTemplate::AttackType SharedWeaponObjectTemplate::getAttackType() const
{


	const SharedWeaponObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedWeaponObjectTemplate *>(m_baseData);
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
}	// SharedWeaponObjectTemplate::getAttackType


/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedWeaponObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedWeaponObjectTemplate_tag)
	{
		SharedTangibleObjectTemplate::load(file);
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
		if (strcmp(paramName, "weaponEffect") == 0)
			m_weaponEffect.loadFromIff(file);
		else if (strcmp(paramName, "weaponEffectIndex") == 0)
			m_weaponEffectIndex.loadFromIff(file);
		else if (strcmp(paramName, "attackType") == 0)
			m_attackType.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedWeaponObjectTemplate::load

//@END TFD
