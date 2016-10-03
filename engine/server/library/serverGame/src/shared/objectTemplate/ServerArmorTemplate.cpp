//========================================================================
//
// ServerArmorTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerArmorTemplate.h"
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

bool ServerArmorTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerArmorTemplate::ServerArmorTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ObjectTemplate(filename)
	,m_specialProtectionLoaded(false)
	,m_specialProtectionAppend(false)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerArmorTemplate::ServerArmorTemplate

/**
 * Class destructor.
 */
ServerArmorTemplate::~ServerArmorTemplate()
{
//@BEGIN TFD CLEANUP
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_specialProtection.begin(); iter != m_specialProtection.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_specialProtection.clear();
	}
//@END TFD CLEANUP
}	// ServerArmorTemplate::~ServerArmorTemplate

/**
 * Static function used to register this template.
 */
void ServerArmorTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerArmorTemplate_tag, create);
}	// ServerArmorTemplate::registerMe

/**
 * Creates a ServerArmorTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerArmorTemplate::create(const std::string & filename)
{
	return new ServerArmorTemplate(filename);
}	// ServerArmorTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerArmorTemplate::getId(void) const
{
	return ServerArmorTemplate_tag;
}	// ServerArmorTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerArmorTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerArmorTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerArmorTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerArmorTemplate * base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerArmorTemplate::getHighestTemplateVersion

//@BEGIN TFD
ServerArmorTemplate::ArmorRating ServerArmorTemplate::getRating(bool testData) const
{
#ifdef _DEBUG
ServerArmorTemplate::ArmorRating testDataValue = static_cast<ServerArmorTemplate::ArmorRating>(0);
#else
UNREF(testData);
#endif

	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getRating(true);
#endif
	}

	if (!m_rating.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter rating in template %s", DataResource::getName()));
			return static_cast<ArmorRating>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter rating has not been defined in template %s!", DataResource::getName()));
			return base->getRating();
		}
	}

	ArmorRating value = static_cast<ArmorRating>(m_rating.getValue());
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerArmorTemplate::getRating

int ServerArmorTemplate::getIntegrity(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getIntegrity(true);
#endif
	}

	if (!m_integrity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter integrity in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter integrity has not been defined in template %s!", DataResource::getName()));
			return base->getIntegrity();
		}
	}

	int value = m_integrity.getValue();
	char delta = m_integrity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getIntegrity();
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
}	// ServerArmorTemplate::getIntegrity

int ServerArmorTemplate::getIntegrityMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getIntegrityMin(true);
#endif
	}

	if (!m_integrity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter integrity in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter integrity has not been defined in template %s!", DataResource::getName()));
			return base->getIntegrityMin();
		}
	}

	int value = m_integrity.getMinValue();
	char delta = m_integrity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getIntegrityMin();
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
}	// ServerArmorTemplate::getIntegrityMin

int ServerArmorTemplate::getIntegrityMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getIntegrityMax(true);
#endif
	}

	if (!m_integrity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter integrity in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter integrity has not been defined in template %s!", DataResource::getName()));
			return base->getIntegrityMax();
		}
	}

	int value = m_integrity.getMaxValue();
	char delta = m_integrity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getIntegrityMax();
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
}	// ServerArmorTemplate::getIntegrityMax

int ServerArmorTemplate::getEffectiveness(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getEffectiveness(true);
#endif
	}

	if (!m_effectiveness.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter effectiveness in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter effectiveness has not been defined in template %s!", DataResource::getName()));
			return base->getEffectiveness();
		}
	}

	int value = m_effectiveness.getValue();
	char delta = m_effectiveness.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getEffectiveness();
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
}	// ServerArmorTemplate::getEffectiveness

int ServerArmorTemplate::getEffectivenessMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getEffectivenessMin(true);
#endif
	}

	if (!m_effectiveness.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter effectiveness in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter effectiveness has not been defined in template %s!", DataResource::getName()));
			return base->getEffectivenessMin();
		}
	}

	int value = m_effectiveness.getMinValue();
	char delta = m_effectiveness.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getEffectivenessMin();
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
}	// ServerArmorTemplate::getEffectivenessMin

int ServerArmorTemplate::getEffectivenessMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getEffectivenessMax(true);
#endif
	}

	if (!m_effectiveness.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter effectiveness in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter effectiveness has not been defined in template %s!", DataResource::getName()));
			return base->getEffectivenessMax();
		}
	}

	int value = m_effectiveness.getMaxValue();
	char delta = m_effectiveness.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getEffectivenessMax();
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
}	// ServerArmorTemplate::getEffectivenessMax

void ServerArmorTemplate::getSpecialProtection(SpecialProtection &data, int index) const
{
	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
	}

	if (!m_specialProtectionLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter specialProtection in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter specialProtection has not been defined in template %s!", DataResource::getName()));
			base->getSpecialProtection(data, index);
			return;
		}
	}

	if (m_specialProtectionAppend && base != nullptr)
	{
		int baseCount = base->getSpecialProtectionCount();
		if (index < baseCount)
			{
				base->getSpecialProtection(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_specialProtection.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_specialProtection[index]).getValue();
	NOT_NULL(structTemplate);
	const _SpecialProtection *param = dynamic_cast<const _SpecialProtection *>(structTemplate);
	NOT_NULL(param);
	data.type = param->getType(m_versionOk);
	data.effectiveness = param->getEffectiveness(m_versionOk);
}	// ServerArmorTemplate::getSpecialProtection

void ServerArmorTemplate::getSpecialProtectionMin(SpecialProtection &data, int index) const
{
	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
	}

	if (!m_specialProtectionLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter specialProtection in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter specialProtection has not been defined in template %s!", DataResource::getName()));
			base->getSpecialProtectionMin(data, index);
			return;
		}
	}

	if (m_specialProtectionAppend && base != nullptr)
	{
		int baseCount = base->getSpecialProtectionCount();
		if (index < baseCount)
			{
				base->getSpecialProtectionMin(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_specialProtection.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_specialProtection[index]).getValue();
	NOT_NULL(structTemplate);
	const _SpecialProtection *param = dynamic_cast<const _SpecialProtection *>(structTemplate);
	NOT_NULL(param);
	data.type = param->getType(m_versionOk);
	data.effectiveness = param->getEffectivenessMin(m_versionOk);
}	// ServerArmorTemplate::getSpecialProtectionMin

void ServerArmorTemplate::getSpecialProtectionMax(SpecialProtection &data, int index) const
{
	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
	}

	if (!m_specialProtectionLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter specialProtection in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter specialProtection has not been defined in template %s!", DataResource::getName()));
			base->getSpecialProtectionMax(data, index);
			return;
		}
	}

	if (m_specialProtectionAppend && base != nullptr)
	{
		int baseCount = base->getSpecialProtectionCount();
		if (index < baseCount)
			{
				base->getSpecialProtectionMax(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_specialProtection.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_specialProtection[index]).getValue();
	NOT_NULL(structTemplate);
	const _SpecialProtection *param = dynamic_cast<const _SpecialProtection *>(structTemplate);
	NOT_NULL(param);
	data.type = param->getType(m_versionOk);
	data.effectiveness = param->getEffectivenessMax(m_versionOk);
}	// ServerArmorTemplate::getSpecialProtectionMax

size_t ServerArmorTemplate::getSpecialProtectionCount(void) const
{
	if (!m_specialProtectionLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerArmorTemplate * base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getSpecialProtectionCount();
	}

	size_t count = m_specialProtection.size();

	// if we are extending our base template, add it's count
	if (m_specialProtectionAppend && m_baseData != nullptr)
	{
		const ServerArmorTemplate * base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getSpecialProtectionCount();
	}

	return count;
}	// ServerArmorTemplate::getSpecialProtectionCount

int ServerArmorTemplate::getVulnerability(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getVulnerability(true);
#endif
	}

	if (!m_vulnerability.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter vulnerability in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter vulnerability has not been defined in template %s!", DataResource::getName()));
			return base->getVulnerability();
		}
	}

	int value = m_vulnerability.getValue();
	char delta = m_vulnerability.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getVulnerability();
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
}	// ServerArmorTemplate::getVulnerability

int ServerArmorTemplate::getVulnerabilityMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getVulnerabilityMin(true);
#endif
	}

	if (!m_vulnerability.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter vulnerability in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter vulnerability has not been defined in template %s!", DataResource::getName()));
			return base->getVulnerabilityMin();
		}
	}

	int value = m_vulnerability.getMinValue();
	char delta = m_vulnerability.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getVulnerabilityMin();
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
}	// ServerArmorTemplate::getVulnerabilityMin

int ServerArmorTemplate::getVulnerabilityMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getVulnerabilityMax(true);
#endif
	}

	if (!m_vulnerability.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter vulnerability in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter vulnerability has not been defined in template %s!", DataResource::getName()));
			return base->getVulnerabilityMax();
		}
	}

	int value = m_vulnerability.getMaxValue();
	char delta = m_vulnerability.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getVulnerabilityMax();
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
}	// ServerArmorTemplate::getVulnerabilityMax

int ServerArmorTemplate::getEncumbrance(int index) const
{
	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 3, ("template param index out of range"));
	if (!m_encumbrance[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter encumbrance in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter encumbrance has not been defined in template %s!", DataResource::getName()));
			return base->getEncumbrance(index);
		}
	}

	int value = m_encumbrance[index].getValue();
	char delta = m_encumbrance[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getEncumbrance(index);
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
}	// ServerArmorTemplate::getEncumbrance

int ServerArmorTemplate::getEncumbranceMin(int index) const
{
	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 3, ("template param index out of range"));
	if (!m_encumbrance[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter encumbrance in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter encumbrance has not been defined in template %s!", DataResource::getName()));
			return base->getEncumbranceMin(index);
		}
	}

	int value = m_encumbrance[index].getMinValue();
	char delta = m_encumbrance[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getEncumbranceMin(index);
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
}	// ServerArmorTemplate::getEncumbranceMin

int ServerArmorTemplate::getEncumbranceMax(int index) const
{
	const ServerArmorTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 3, ("template param index out of range"));
	if (!m_encumbrance[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter encumbrance in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter encumbrance has not been defined in template %s!", DataResource::getName()));
			return base->getEncumbranceMax(index);
		}
	}

	int value = m_encumbrance[index].getMaxValue();
	char delta = m_encumbrance[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getEncumbranceMax(index);
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
}	// ServerArmorTemplate::getEncumbranceMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerArmorTemplate::testValues(void) const
{
	IGNORE_RETURN(getRating(true));
	IGNORE_RETURN(getIntegrityMin(true));
	IGNORE_RETURN(getIntegrityMax(true));
	IGNORE_RETURN(getEffectivenessMin(true));
	IGNORE_RETURN(getEffectivenessMax(true));
	IGNORE_RETURN(getVulnerabilityMin(true));
	IGNORE_RETURN(getVulnerabilityMax(true));
}	// ServerArmorTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerArmorTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerArmorTemplate_tag)
	{
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
		if (strcmp(paramName, "rating") == 0)
			m_rating.loadFromIff(file);
		else if (strcmp(paramName, "integrity") == 0)
			m_integrity.loadFromIff(file);
		else if (strcmp(paramName, "effectiveness") == 0)
			m_effectiveness.loadFromIff(file);
		else if (strcmp(paramName, "specialProtection") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_specialProtection.begin(); iter != m_specialProtection.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_specialProtection.clear();
			m_specialProtectionAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_specialProtection.push_back(newData);
			}
			m_specialProtectionLoaded = true;
		}
		else if (strcmp(paramName, "vulnerability") == 0)
			m_vulnerability.loadFromIff(file);
		else if (strcmp(paramName, "encumbrance") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 3, ("Template %s: read array size of %d for array \"encumbrance\" of size 3, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 3 && j < listCount; ++j)
				m_encumbrance[j].loadFromIff(file);
			// if there are more params for encumbrance read and dump them
			for (; j < listCount; ++j)
			{
				IntegerParam dummy;
				dummy.loadFromIff(file);
			}
		}
		file.exitChunk(true);
	}

	file.exitForm();
	return;
}	// ServerArmorTemplate::load


//=============================================================================
// class ServerArmorTemplate::_SpecialProtection

/**
 * Class constructor.
 */
ServerArmorTemplate::_SpecialProtection::_SpecialProtection(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// ServerArmorTemplate::_SpecialProtection::_SpecialProtection

/**
 * Class destructor.
 */
ServerArmorTemplate::_SpecialProtection::~_SpecialProtection()
{
}	// ServerArmorTemplate::_SpecialProtection::~_SpecialProtection

/**
 * Static function used to register this template.
 */
void ServerArmorTemplate::_SpecialProtection::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_SpecialProtection_tag, create);
}	// ServerArmorTemplate::_SpecialProtection::registerMe

/**
 * Creates a ServerArmorTemplate::_SpecialProtection template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerArmorTemplate::_SpecialProtection::create(const std::string & filename)
{
	return new ServerArmorTemplate::_SpecialProtection(filename);
}	// ServerArmorTemplate::_SpecialProtection::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerArmorTemplate::_SpecialProtection::getId(void) const
{
	return _SpecialProtection_tag;
}	// ServerArmorTemplate::_SpecialProtection::getId

ServerArmorTemplate::DamageType ServerArmorTemplate::_SpecialProtection::getType(bool versionOk, bool testData) const
{
#ifdef _DEBUG
ServerArmorTemplate::DamageType testDataValue = static_cast<ServerArmorTemplate::DamageType>(0);
#else
UNREF(testData);
#endif

	const ServerArmorTemplate::_SpecialProtection * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate::_SpecialProtection *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getType(true);
#endif
	}

	if (!m_type.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter type in template %s", DataResource::getName()));
			return static_cast<DamageType>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter type has not been defined in template %s!", DataResource::getName()));
			return base->getType(versionOk);
		}
	}

	DamageType value = static_cast<DamageType>(m_type.getValue());
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerArmorTemplate::_SpecialProtection::getType

int ServerArmorTemplate::_SpecialProtection::getEffectiveness(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerArmorTemplate::_SpecialProtection * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate::_SpecialProtection *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getEffectiveness(true);
#endif
	}

	if (!m_effectiveness.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter effectiveness in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter effectiveness has not been defined in template %s!", DataResource::getName()));
			return base->getEffectiveness(versionOk);
		}
	}

	int value = m_effectiveness.getValue();
	char delta = m_effectiveness.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getEffectiveness(versionOk);
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
}	// ServerArmorTemplate::_SpecialProtection::getEffectiveness

int ServerArmorTemplate::_SpecialProtection::getEffectivenessMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerArmorTemplate::_SpecialProtection * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate::_SpecialProtection *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getEffectivenessMin(true);
#endif
	}

	if (!m_effectiveness.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter effectiveness in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter effectiveness has not been defined in template %s!", DataResource::getName()));
			return base->getEffectivenessMin(versionOk);
		}
	}

	int value = m_effectiveness.getMinValue();
	char delta = m_effectiveness.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getEffectivenessMin(versionOk);
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
}	// ServerArmorTemplate::_SpecialProtection::getEffectivenessMin

int ServerArmorTemplate::_SpecialProtection::getEffectivenessMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerArmorTemplate::_SpecialProtection * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerArmorTemplate::_SpecialProtection *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getEffectivenessMax(true);
#endif
	}

	if (!m_effectiveness.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter effectiveness in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter effectiveness has not been defined in template %s!", DataResource::getName()));
			return base->getEffectivenessMax(versionOk);
		}
	}

	int value = m_effectiveness.getMaxValue();
	char delta = m_effectiveness.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getEffectivenessMax(versionOk);
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
}	// ServerArmorTemplate::_SpecialProtection::getEffectivenessMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerArmorTemplate::_SpecialProtection::testValues(void) const
{
	IGNORE_RETURN(getType(true));
	IGNORE_RETURN(getEffectivenessMin(true));
	IGNORE_RETURN(getEffectivenessMax(true));
}	// ServerArmorTemplate::_SpecialProtection::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerArmorTemplate::_SpecialProtection::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "type") == 0)
			m_type.loadFromIff(file);
		else if (strcmp(paramName, "effectiveness") == 0)
			m_effectiveness.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerArmorTemplate::_SpecialProtection::load

//@END TFD
