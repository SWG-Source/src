//========================================================================
//
// SharedVehicleObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedVehicleObjectTemplate.h"
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

bool SharedVehicleObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedVehicleObjectTemplate::SharedVehicleObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedVehicleObjectTemplate::SharedVehicleObjectTemplate

/**
 * Class destructor.
 */
SharedVehicleObjectTemplate::~SharedVehicleObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedVehicleObjectTemplate::~SharedVehicleObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedVehicleObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedVehicleObjectTemplate_tag, create);
}	// SharedVehicleObjectTemplate::registerMe

/**
 * Creates a SharedVehicleObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedVehicleObjectTemplate::create(const std::string & filename)
{
	return new SharedVehicleObjectTemplate(filename);
}	// SharedVehicleObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedVehicleObjectTemplate::getId(void) const
{
	return SharedVehicleObjectTemplate_tag;
}	// SharedVehicleObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedVehicleObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedVehicleObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedVehicleObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedVehicleObjectTemplate * base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedVehicleObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
float SharedVehicleObjectTemplate::getSpeed(MovementTypes index) const
{
	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 5, ("template param index out of range"));
	if (!m_speed[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter speed in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter speed has not been defined in template %s!", DataResource::getName()));
			return base->getSpeed(index);
		}
	}

	float value = m_speed[index].getValue();
	char delta = m_speed[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSpeed(index);
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
}	// SharedVehicleObjectTemplate::getSpeed

float SharedVehicleObjectTemplate::getSpeedMin(MovementTypes index) const
{
	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 5, ("template param index out of range"));
	if (!m_speed[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter speed in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter speed has not been defined in template %s!", DataResource::getName()));
			return base->getSpeedMin(index);
		}
	}

	float value = m_speed[index].getMinValue();
	char delta = m_speed[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSpeedMin(index);
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
}	// SharedVehicleObjectTemplate::getSpeedMin

float SharedVehicleObjectTemplate::getSpeedMax(MovementTypes index) const
{
	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 5, ("template param index out of range"));
	if (!m_speed[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter speed in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter speed has not been defined in template %s!", DataResource::getName()));
			return base->getSpeedMax(index);
		}
	}

	float value = m_speed[index].getMaxValue();
	char delta = m_speed[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSpeedMax(index);
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
}	// SharedVehicleObjectTemplate::getSpeedMax

float SharedVehicleObjectTemplate::getSlopeAversion(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getSlopeAversion(true);
#endif
	}

	if (!m_slopeAversion.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slopeAversion in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slopeAversion has not been defined in template %s!", DataResource::getName()));
			return base->getSlopeAversion();
		}
	}

	float value = m_slopeAversion.getValue();
	char delta = m_slopeAversion.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSlopeAversion();
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
}	// SharedVehicleObjectTemplate::getSlopeAversion

float SharedVehicleObjectTemplate::getSlopeAversionMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getSlopeAversionMin(true);
#endif
	}

	if (!m_slopeAversion.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slopeAversion in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slopeAversion has not been defined in template %s!", DataResource::getName()));
			return base->getSlopeAversionMin();
		}
	}

	float value = m_slopeAversion.getMinValue();
	char delta = m_slopeAversion.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSlopeAversionMin();
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
}	// SharedVehicleObjectTemplate::getSlopeAversionMin

float SharedVehicleObjectTemplate::getSlopeAversionMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getSlopeAversionMax(true);
#endif
	}

	if (!m_slopeAversion.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slopeAversion in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slopeAversion has not been defined in template %s!", DataResource::getName()));
			return base->getSlopeAversionMax();
		}
	}

	float value = m_slopeAversion.getMaxValue();
	char delta = m_slopeAversion.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSlopeAversionMax();
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
}	// SharedVehicleObjectTemplate::getSlopeAversionMax

float SharedVehicleObjectTemplate::getHoverValue(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getHoverValue(true);
#endif
	}

	if (!m_hoverValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter hoverValue in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter hoverValue has not been defined in template %s!", DataResource::getName()));
			return base->getHoverValue();
		}
	}

	float value = m_hoverValue.getValue();
	char delta = m_hoverValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getHoverValue();
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
}	// SharedVehicleObjectTemplate::getHoverValue

float SharedVehicleObjectTemplate::getHoverValueMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getHoverValueMin(true);
#endif
	}

	if (!m_hoverValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter hoverValue in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter hoverValue has not been defined in template %s!", DataResource::getName()));
			return base->getHoverValueMin();
		}
	}

	float value = m_hoverValue.getMinValue();
	char delta = m_hoverValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getHoverValueMin();
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
}	// SharedVehicleObjectTemplate::getHoverValueMin

float SharedVehicleObjectTemplate::getHoverValueMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getHoverValueMax(true);
#endif
	}

	if (!m_hoverValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter hoverValue in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter hoverValue has not been defined in template %s!", DataResource::getName()));
			return base->getHoverValueMax();
		}
	}

	float value = m_hoverValue.getMaxValue();
	char delta = m_hoverValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getHoverValueMax();
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
}	// SharedVehicleObjectTemplate::getHoverValueMax

float SharedVehicleObjectTemplate::getTurnRate(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTurnRate(true);
#endif
	}

	if (!m_turnRate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter turnRate in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter turnRate has not been defined in template %s!", DataResource::getName()));
			return base->getTurnRate();
		}
	}

	float value = m_turnRate.getValue();
	char delta = m_turnRate.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTurnRate();
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
}	// SharedVehicleObjectTemplate::getTurnRate

float SharedVehicleObjectTemplate::getTurnRateMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTurnRateMin(true);
#endif
	}

	if (!m_turnRate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter turnRate in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter turnRate has not been defined in template %s!", DataResource::getName()));
			return base->getTurnRateMin();
		}
	}

	float value = m_turnRate.getMinValue();
	char delta = m_turnRate.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTurnRateMin();
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
}	// SharedVehicleObjectTemplate::getTurnRateMin

float SharedVehicleObjectTemplate::getTurnRateMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTurnRateMax(true);
#endif
	}

	if (!m_turnRate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter turnRate in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter turnRate has not been defined in template %s!", DataResource::getName()));
			return base->getTurnRateMax();
		}
	}

	float value = m_turnRate.getMaxValue();
	char delta = m_turnRate.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTurnRateMax();
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
}	// SharedVehicleObjectTemplate::getTurnRateMax

float SharedVehicleObjectTemplate::getMaxVelocity(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxVelocity(true);
#endif
	}

	if (!m_maxVelocity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxVelocity in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxVelocity has not been defined in template %s!", DataResource::getName()));
			return base->getMaxVelocity();
		}
	}

	float value = m_maxVelocity.getValue();
	char delta = m_maxVelocity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxVelocity();
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
}	// SharedVehicleObjectTemplate::getMaxVelocity

float SharedVehicleObjectTemplate::getMaxVelocityMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxVelocityMin(true);
#endif
	}

	if (!m_maxVelocity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxVelocity in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxVelocity has not been defined in template %s!", DataResource::getName()));
			return base->getMaxVelocityMin();
		}
	}

	float value = m_maxVelocity.getMinValue();
	char delta = m_maxVelocity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxVelocityMin();
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
}	// SharedVehicleObjectTemplate::getMaxVelocityMin

float SharedVehicleObjectTemplate::getMaxVelocityMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxVelocityMax(true);
#endif
	}

	if (!m_maxVelocity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxVelocity in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxVelocity has not been defined in template %s!", DataResource::getName()));
			return base->getMaxVelocityMax();
		}
	}

	float value = m_maxVelocity.getMaxValue();
	char delta = m_maxVelocity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxVelocityMax();
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
}	// SharedVehicleObjectTemplate::getMaxVelocityMax

float SharedVehicleObjectTemplate::getAcceleration(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getAcceleration(true);
#endif
	}

	if (!m_acceleration.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter acceleration in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter acceleration has not been defined in template %s!", DataResource::getName()));
			return base->getAcceleration();
		}
	}

	float value = m_acceleration.getValue();
	char delta = m_acceleration.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAcceleration();
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
}	// SharedVehicleObjectTemplate::getAcceleration

float SharedVehicleObjectTemplate::getAccelerationMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getAccelerationMin(true);
#endif
	}

	if (!m_acceleration.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter acceleration in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter acceleration has not been defined in template %s!", DataResource::getName()));
			return base->getAccelerationMin();
		}
	}

	float value = m_acceleration.getMinValue();
	char delta = m_acceleration.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAccelerationMin();
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
}	// SharedVehicleObjectTemplate::getAccelerationMin

float SharedVehicleObjectTemplate::getAccelerationMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getAccelerationMax(true);
#endif
	}

	if (!m_acceleration.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter acceleration in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter acceleration has not been defined in template %s!", DataResource::getName()));
			return base->getAccelerationMax();
		}
	}

	float value = m_acceleration.getMaxValue();
	char delta = m_acceleration.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAccelerationMax();
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
}	// SharedVehicleObjectTemplate::getAccelerationMax

float SharedVehicleObjectTemplate::getBraking(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getBraking(true);
#endif
	}

	if (!m_braking.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter braking in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter braking has not been defined in template %s!", DataResource::getName()));
			return base->getBraking();
		}
	}

	float value = m_braking.getValue();
	char delta = m_braking.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getBraking();
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
}	// SharedVehicleObjectTemplate::getBraking

float SharedVehicleObjectTemplate::getBrakingMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getBrakingMin(true);
#endif
	}

	if (!m_braking.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter braking in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter braking has not been defined in template %s!", DataResource::getName()));
			return base->getBrakingMin();
		}
	}

	float value = m_braking.getMinValue();
	char delta = m_braking.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getBrakingMin();
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
}	// SharedVehicleObjectTemplate::getBrakingMin

float SharedVehicleObjectTemplate::getBrakingMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const SharedVehicleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedVehicleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getBrakingMax(true);
#endif
	}

	if (!m_braking.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter braking in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter braking has not been defined in template %s!", DataResource::getName()));
			return base->getBrakingMax();
		}
	}

	float value = m_braking.getMaxValue();
	char delta = m_braking.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getBrakingMax();
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
}	// SharedVehicleObjectTemplate::getBrakingMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedVehicleObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getSlopeAversionMin(true));
	IGNORE_RETURN(getSlopeAversionMax(true));
	IGNORE_RETURN(getHoverValueMin(true));
	IGNORE_RETURN(getHoverValueMax(true));
	IGNORE_RETURN(getTurnRateMin(true));
	IGNORE_RETURN(getTurnRateMax(true));
	IGNORE_RETURN(getMaxVelocityMin(true));
	IGNORE_RETURN(getMaxVelocityMax(true));
	IGNORE_RETURN(getAccelerationMin(true));
	IGNORE_RETURN(getAccelerationMax(true));
	IGNORE_RETURN(getBrakingMin(true));
	IGNORE_RETURN(getBrakingMax(true));
	SharedTangibleObjectTemplate::testValues();
}	// SharedVehicleObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedVehicleObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedVehicleObjectTemplate_tag)
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
		if (strcmp(paramName, "speed") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 5, ("Template %s: read array size of %d for array \"speed\" of size 5, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 5 && j < listCount; ++j)
				m_speed[j].loadFromIff(file);
			// if there are more params for speed read and dump them
			for (; j < listCount; ++j)
			{
				FloatParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "slopeAversion") == 0)
			m_slopeAversion.loadFromIff(file);
		else if (strcmp(paramName, "hoverValue") == 0)
			m_hoverValue.loadFromIff(file);
		else if (strcmp(paramName, "turnRate") == 0)
			m_turnRate.loadFromIff(file);
		else if (strcmp(paramName, "maxVelocity") == 0)
			m_maxVelocity.loadFromIff(file);
		else if (strcmp(paramName, "acceleration") == 0)
			m_acceleration.loadFromIff(file);
		else if (strcmp(paramName, "braking") == 0)
			m_braking.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedVehicleObjectTemplate::load

//@END TFD
