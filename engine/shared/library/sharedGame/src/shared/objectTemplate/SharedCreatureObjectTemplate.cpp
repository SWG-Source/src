//========================================================================
//
// SharedCreatureObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedCreatureObjectTemplate.h"
#include "sharedFile/Iff.h"
#include "sharedGame/AssetCustomizationManager.h"
#include "sharedMath/Vector.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/MovementTable.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/BasicRangedIntCustomizationVariable.h"
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS
#include <algorithm>
#include <cstdio>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool SharedCreatureObjectTemplate::ms_allowDefaultTemplateParams = true;

typedef ::RangedIntCustomizationVariable GlobalRangedIntCustomizationVariableType;

/**
 * Class constructor.
 */
SharedCreatureObjectTemplate::SharedCreatureObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedTangibleObjectTemplate(filename)
	,m_versionOk(true)
//@END TFD INIT
{
	m_movementTable = 0;
}	// SharedCreatureObjectTemplate::SharedCreatureObjectTemplate

/**
 * Class destructor.
 */
SharedCreatureObjectTemplate::~SharedCreatureObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
	delete m_movementTable;
	m_movementTable = 0;
}	// SharedCreatureObjectTemplate::~SharedCreatureObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedCreatureObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedCreatureObjectTemplate_tag, create);
}	// SharedCreatureObjectTemplate::registerMe

/**
 * Creates a SharedCreatureObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedCreatureObjectTemplate::create(const std::string & filename)
{
	return new SharedCreatureObjectTemplate(filename);
}	// SharedCreatureObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedCreatureObjectTemplate::getId(void) const
{
	return SharedCreatureObjectTemplate_tag;
}	// SharedCreatureObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedCreatureObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedCreatureObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedCreatureObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedCreatureObjectTemplate * base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedCreatureObjectTemplate::getHighestTemplateVersion

void SharedCreatureObjectTemplate::postLoad()
{
	SharedTangibleObjectTemplate::postLoad();
	m_movementTable = new MovementTable(getMovementDatatable(), DataResource::getName());
}

// ----------------------------------------------------------------------

void SharedCreatureObjectTemplate::createCustomizationDataPropertyAsNeeded(Object &object, bool /* forceCreation */) const
{
	//-- Properties cannot be added while an object is in the world.  Some callers may be in the world, 
	//   so temporarily remove the object from the world if necessary.
	bool shouldBeInWorld = object.isInWorld();
	if (shouldBeInWorld)
		object.removeFromWorld();

	//-- Create the CustomizationDataProperty, add to Object property collection
	CustomizationDataProperty *const cdProperty = new CustomizationDataProperty(object);
	object.addProperty(*cdProperty);

	//-- Put object back in world if it was originally there.
	if (shouldBeInWorld)
		object.addToWorld();

	//-- Fetch the CustomizationData instance
	CustomizationData *const customizationData = cdProperty->fetchCustomizationData();
	if (customizationData)
	{
		//-- Create customization variables associated with this appearance.  Don't skip /shared_owner/
		//   variables since we are a creature.  The SharedTangibleObjectTemplate version of
		//   this function sets it to true.
		bool const skipSharedOwnerVariables = false;
		AssetCustomizationManager::addCustomizationVariablesForAsset(TemporaryCrcString(getAppearanceFilename().c_str(), true), *customizationData, skipSharedOwnerVariables);

		//-- set up mappings for any variables which need dependent mappings
 		int numVariableMappings = getCustomizationVariableMappingCount();
		for (int i = 0; i < numVariableMappings; ++i)
		{
			CustomizationVariableMapping localVariableMapping;
 			getCustomizationVariableMapping(localVariableMapping, i);
			GlobalRangedIntCustomizationVariableType *source = safe_cast<GlobalRangedIntCustomizationVariableType *>(customizationData->findVariable(localVariableMapping.sourceVariable));
 			if(source)
 			{
 				source->setDependentVariable(localVariableMapping.dependentVariable);
 			} 			
 		}
		
		//-- release local reference to the CustomizationData instance
		customizationData->release();
	}
}

// ----------------------------------------------------------------------

//@BEGIN TFD
SharedCreatureObjectTemplate::Gender SharedCreatureObjectTemplate::getGender() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_gender.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter gender in template %s", DataResource::getName()));
			return static_cast<Gender>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter gender has not been defined in template %s!", DataResource::getName()));
			return base->getGender();
		}
	}

	Gender value = static_cast<Gender>(m_gender.getValue());

	return value;
}	// SharedCreatureObjectTemplate::getGender

SharedCreatureObjectTemplate::Niche SharedCreatureObjectTemplate::getNiche() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_niche.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter niche in template %s", DataResource::getName()));
			return static_cast<Niche>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter niche has not been defined in template %s!", DataResource::getName()));
			return base->getNiche();
		}
	}

	Niche value = static_cast<Niche>(m_niche.getValue());

	return value;
}	// SharedCreatureObjectTemplate::getNiche

SharedCreatureObjectTemplate::Species SharedCreatureObjectTemplate::getSpecies() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_species.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter species in template %s", DataResource::getName()));
			return static_cast<Species>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter species has not been defined in template %s!", DataResource::getName()));
			return base->getSpecies();
		}
	}

	Species value = static_cast<Species>(m_species.getValue());

	return value;
}	// SharedCreatureObjectTemplate::getSpecies

SharedCreatureObjectTemplate::Race SharedCreatureObjectTemplate::getRace() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_race.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter race in template %s", DataResource::getName()));
			return static_cast<Race>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter race has not been defined in template %s!", DataResource::getName()));
			return base->getRace();
		}
	}

	Race value = static_cast<Race>(m_race.getValue());

	return value;
}	// SharedCreatureObjectTemplate::getRace

float SharedCreatureObjectTemplate::getAcceleration(MovementTypes index) const
{
	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(static_cast<int>(index) < 0 || static_cast<int>(index) >= 2, ("template param index out of range"));
	if (!m_acceleration[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter acceleration in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter acceleration has not been defined in template %s!", DataResource::getName()));
			return base->getAcceleration(index);
		}
	}

	float value = m_acceleration[index].getValue();
	char delta = m_acceleration[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAcceleration(index);
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
}	// SharedCreatureObjectTemplate::getAcceleration

float SharedCreatureObjectTemplate::getAccelerationMin(MovementTypes index) const
{
	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(static_cast<int>(index) < 0 || static_cast<int>(index) >= 2, ("template param index out of range"));
	if (!m_acceleration[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter acceleration in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter acceleration has not been defined in template %s!", DataResource::getName()));
			return base->getAccelerationMin(index);
		}
	}

	float value = m_acceleration[index].getMinValue();
	char delta = m_acceleration[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAccelerationMin(index);
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
}	// SharedCreatureObjectTemplate::getAccelerationMin

float SharedCreatureObjectTemplate::getAccelerationMax(MovementTypes index) const
{
	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(static_cast<int>(index) < 0 || static_cast<int>(index) >= 2, ("template param index out of range"));
	if (!m_acceleration[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter acceleration in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter acceleration has not been defined in template %s!", DataResource::getName()));
			return base->getAccelerationMax(index);
		}
	}

	float value = m_acceleration[index].getMaxValue();
	char delta = m_acceleration[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getAccelerationMax(index);
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
}	// SharedCreatureObjectTemplate::getAccelerationMax

float SharedCreatureObjectTemplate::getSpeed(MovementTypes index) const
{
	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(static_cast<int>(index) < 0 || static_cast<int>(index) >= 2, ("template param index out of range"));
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
}	// SharedCreatureObjectTemplate::getSpeed

float SharedCreatureObjectTemplate::getSpeedMin(MovementTypes index) const
{
	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(static_cast<int>(index) < 0 || static_cast<int>(index) >= 2, ("template param index out of range"));
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
}	// SharedCreatureObjectTemplate::getSpeedMin

float SharedCreatureObjectTemplate::getSpeedMax(MovementTypes index) const
{
	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(static_cast<int>(index) < 0 || static_cast<int>(index) >= 2, ("template param index out of range"));
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
}	// SharedCreatureObjectTemplate::getSpeedMax

float SharedCreatureObjectTemplate::getTurnRate(MovementTypes index) const
{
	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(static_cast<int>(index) < 0 || static_cast<int>(index) >= 2, ("template param index out of range"));
	if (!m_turnRate[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter turnRate in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter turnRate has not been defined in template %s!", DataResource::getName()));
			return base->getTurnRate(index);
		}
	}

	float value = m_turnRate[index].getValue();
	char delta = m_turnRate[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTurnRate(index);
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
}	// SharedCreatureObjectTemplate::getTurnRate

float SharedCreatureObjectTemplate::getTurnRateMin(MovementTypes index) const
{
	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(static_cast<int>(index) < 0 || static_cast<int>(index) >= 2, ("template param index out of range"));
	if (!m_turnRate[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter turnRate in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter turnRate has not been defined in template %s!", DataResource::getName()));
			return base->getTurnRateMin(index);
		}
	}

	float value = m_turnRate[index].getMinValue();
	char delta = m_turnRate[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTurnRateMin(index);
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
}	// SharedCreatureObjectTemplate::getTurnRateMin

float SharedCreatureObjectTemplate::getTurnRateMax(MovementTypes index) const
{
	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(static_cast<int>(index) < 0 || static_cast<int>(index) >= 2, ("template param index out of range"));
	if (!m_turnRate[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter turnRate in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter turnRate has not been defined in template %s!", DataResource::getName()));
			return base->getTurnRateMax(index);
		}
	}

	float value = m_turnRate[index].getMaxValue();
	char delta = m_turnRate[index].getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getTurnRateMax(index);
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
}	// SharedCreatureObjectTemplate::getTurnRateMax

const std::string & SharedCreatureObjectTemplate::getAnimationMapFilename() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_animationMapFilename.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter animationMapFilename in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter animationMapFilename has not been defined in template %s!", DataResource::getName()));
			return base->getAnimationMapFilename();
		}
	}

	const std::string & value = m_animationMapFilename.getValue();

	return value;
}	// SharedCreatureObjectTemplate::getAnimationMapFilename

float SharedCreatureObjectTemplate::getSlopeModAngle() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_slopeModAngle.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slopeModAngle in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slopeModAngle has not been defined in template %s!", DataResource::getName()));
			return base->getSlopeModAngle();
		}
	}

	float value = m_slopeModAngle.getValue();
	char delta = m_slopeModAngle.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSlopeModAngle();
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
}	// SharedCreatureObjectTemplate::getSlopeModAngle

float SharedCreatureObjectTemplate::getSlopeModAngleMin() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_slopeModAngle.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slopeModAngle in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slopeModAngle has not been defined in template %s!", DataResource::getName()));
			return base->getSlopeModAngleMin();
		}
	}

	float value = m_slopeModAngle.getMinValue();
	char delta = m_slopeModAngle.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSlopeModAngleMin();
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
}	// SharedCreatureObjectTemplate::getSlopeModAngleMin

float SharedCreatureObjectTemplate::getSlopeModAngleMax() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_slopeModAngle.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slopeModAngle in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slopeModAngle has not been defined in template %s!", DataResource::getName()));
			return base->getSlopeModAngleMax();
		}
	}

	float value = m_slopeModAngle.getMaxValue();
	char delta = m_slopeModAngle.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSlopeModAngleMax();
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
}	// SharedCreatureObjectTemplate::getSlopeModAngleMax

float SharedCreatureObjectTemplate::getSlopeModPercent() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_slopeModPercent.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slopeModPercent in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slopeModPercent has not been defined in template %s!", DataResource::getName()));
			return base->getSlopeModPercent();
		}
	}

	float value = m_slopeModPercent.getValue();
	char delta = m_slopeModPercent.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSlopeModPercent();
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
}	// SharedCreatureObjectTemplate::getSlopeModPercent

float SharedCreatureObjectTemplate::getSlopeModPercentMin() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_slopeModPercent.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slopeModPercent in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slopeModPercent has not been defined in template %s!", DataResource::getName()));
			return base->getSlopeModPercentMin();
		}
	}

	float value = m_slopeModPercent.getMinValue();
	char delta = m_slopeModPercent.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSlopeModPercentMin();
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
}	// SharedCreatureObjectTemplate::getSlopeModPercentMin

float SharedCreatureObjectTemplate::getSlopeModPercentMax() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_slopeModPercent.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slopeModPercent in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slopeModPercent has not been defined in template %s!", DataResource::getName()));
			return base->getSlopeModPercentMax();
		}
	}

	float value = m_slopeModPercent.getMaxValue();
	char delta = m_slopeModPercent.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSlopeModPercentMax();
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
}	// SharedCreatureObjectTemplate::getSlopeModPercentMax

float SharedCreatureObjectTemplate::getWaterModPercent() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_waterModPercent.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter waterModPercent in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter waterModPercent has not been defined in template %s!", DataResource::getName()));
			return base->getWaterModPercent();
		}
	}

	float value = m_waterModPercent.getValue();
	char delta = m_waterModPercent.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getWaterModPercent();
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
}	// SharedCreatureObjectTemplate::getWaterModPercent

float SharedCreatureObjectTemplate::getWaterModPercentMin() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_waterModPercent.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter waterModPercent in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter waterModPercent has not been defined in template %s!", DataResource::getName()));
			return base->getWaterModPercentMin();
		}
	}

	float value = m_waterModPercent.getMinValue();
	char delta = m_waterModPercent.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getWaterModPercentMin();
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
}	// SharedCreatureObjectTemplate::getWaterModPercentMin

float SharedCreatureObjectTemplate::getWaterModPercentMax() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_waterModPercent.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter waterModPercent in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter waterModPercent has not been defined in template %s!", DataResource::getName()));
			return base->getWaterModPercentMax();
		}
	}

	float value = m_waterModPercent.getMaxValue();
	char delta = m_waterModPercent.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getWaterModPercentMax();
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
}	// SharedCreatureObjectTemplate::getWaterModPercentMax

float SharedCreatureObjectTemplate::getStepHeight() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_stepHeight.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter stepHeight in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter stepHeight has not been defined in template %s!", DataResource::getName()));
			return base->getStepHeight();
		}
	}

	float value = m_stepHeight.getValue();
	char delta = m_stepHeight.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getStepHeight();
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
}	// SharedCreatureObjectTemplate::getStepHeight

float SharedCreatureObjectTemplate::getStepHeightMin() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_stepHeight.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter stepHeight in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter stepHeight has not been defined in template %s!", DataResource::getName()));
			return base->getStepHeightMin();
		}
	}

	float value = m_stepHeight.getMinValue();
	char delta = m_stepHeight.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getStepHeightMin();
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
}	// SharedCreatureObjectTemplate::getStepHeightMin

float SharedCreatureObjectTemplate::getStepHeightMax() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_stepHeight.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter stepHeight in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter stepHeight has not been defined in template %s!", DataResource::getName()));
			return base->getStepHeightMax();
		}
	}

	float value = m_stepHeight.getMaxValue();
	char delta = m_stepHeight.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getStepHeightMax();
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
}	// SharedCreatureObjectTemplate::getStepHeightMax

float SharedCreatureObjectTemplate::getCollisionHeight() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionHeight.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionHeight in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionHeight has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionHeight();
		}
	}

	float value = m_collisionHeight.getValue();
	char delta = m_collisionHeight.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionHeight();
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
}	// SharedCreatureObjectTemplate::getCollisionHeight

float SharedCreatureObjectTemplate::getCollisionHeightMin() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionHeight.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionHeight in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionHeight has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionHeightMin();
		}
	}

	float value = m_collisionHeight.getMinValue();
	char delta = m_collisionHeight.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionHeightMin();
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
}	// SharedCreatureObjectTemplate::getCollisionHeightMin

float SharedCreatureObjectTemplate::getCollisionHeightMax() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionHeight.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionHeight in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionHeight has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionHeightMax();
		}
	}

	float value = m_collisionHeight.getMaxValue();
	char delta = m_collisionHeight.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionHeightMax();
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
}	// SharedCreatureObjectTemplate::getCollisionHeightMax

float SharedCreatureObjectTemplate::getCollisionRadius() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionRadius has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionRadius();
		}
	}

	float value = m_collisionRadius.getValue();
	char delta = m_collisionRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionRadius();
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
}	// SharedCreatureObjectTemplate::getCollisionRadius

float SharedCreatureObjectTemplate::getCollisionRadiusMin() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionRadius has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionRadiusMin();
		}
	}

	float value = m_collisionRadius.getMinValue();
	char delta = m_collisionRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionRadiusMin();
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
}	// SharedCreatureObjectTemplate::getCollisionRadiusMin

float SharedCreatureObjectTemplate::getCollisionRadiusMax() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionRadius.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionRadius in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionRadius has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionRadiusMax();
		}
	}

	float value = m_collisionRadius.getMaxValue();
	char delta = m_collisionRadius.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionRadiusMax();
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
}	// SharedCreatureObjectTemplate::getCollisionRadiusMax

const std::string & SharedCreatureObjectTemplate::getMovementDatatable() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_movementDatatable.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter movementDatatable in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter movementDatatable has not been defined in template %s!", DataResource::getName()));
			return base->getMovementDatatable();
		}
	}

	const std::string & value = m_movementDatatable.getValue();

	return value;
}	// SharedCreatureObjectTemplate::getMovementDatatable

bool SharedCreatureObjectTemplate::getPostureAlignToTerrain(Postures index) const
{
	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	DEBUG_FATAL(index < 0 || index >= 15, ("template param index out of range"));
	if (!m_postureAlignToTerrain[index].isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter postureAlignToTerrain in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter postureAlignToTerrain has not been defined in template %s!", DataResource::getName()));
			return base->getPostureAlignToTerrain(index);
		}
	}

	bool value = m_postureAlignToTerrain[index].getValue();
	return value;
}	// SharedCreatureObjectTemplate::getPostureAlignToTerrain

float SharedCreatureObjectTemplate::getSwimHeight() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_swimHeight.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter swimHeight in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter swimHeight has not been defined in template %s!", DataResource::getName()));
			return base->getSwimHeight();
		}
	}

	float value = m_swimHeight.getValue();
	char delta = m_swimHeight.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSwimHeight();
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
}	// SharedCreatureObjectTemplate::getSwimHeight

float SharedCreatureObjectTemplate::getSwimHeightMin() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_swimHeight.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter swimHeight in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter swimHeight has not been defined in template %s!", DataResource::getName()));
			return base->getSwimHeightMin();
		}
	}

	float value = m_swimHeight.getMinValue();
	char delta = m_swimHeight.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSwimHeightMin();
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
}	// SharedCreatureObjectTemplate::getSwimHeightMin

float SharedCreatureObjectTemplate::getSwimHeightMax() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_swimHeight.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter swimHeight in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter swimHeight has not been defined in template %s!", DataResource::getName()));
			return base->getSwimHeightMax();
		}
	}

	float value = m_swimHeight.getMaxValue();
	char delta = m_swimHeight.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getSwimHeightMax();
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
}	// SharedCreatureObjectTemplate::getSwimHeightMax

float SharedCreatureObjectTemplate::getWarpTolerance() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_warpTolerance.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter warpTolerance in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter warpTolerance has not been defined in template %s!", DataResource::getName()));
			return base->getWarpTolerance();
		}
	}

	float value = m_warpTolerance.getValue();
	char delta = m_warpTolerance.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getWarpTolerance();
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
}	// SharedCreatureObjectTemplate::getWarpTolerance

float SharedCreatureObjectTemplate::getWarpToleranceMin() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_warpTolerance.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter warpTolerance in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter warpTolerance has not been defined in template %s!", DataResource::getName()));
			return base->getWarpToleranceMin();
		}
	}

	float value = m_warpTolerance.getMinValue();
	char delta = m_warpTolerance.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getWarpToleranceMin();
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
}	// SharedCreatureObjectTemplate::getWarpToleranceMin

float SharedCreatureObjectTemplate::getWarpToleranceMax() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_warpTolerance.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter warpTolerance in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter warpTolerance has not been defined in template %s!", DataResource::getName()));
			return base->getWarpToleranceMax();
		}
	}

	float value = m_warpTolerance.getMaxValue();
	char delta = m_warpTolerance.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getWarpToleranceMax();
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
}	// SharedCreatureObjectTemplate::getWarpToleranceMax

float SharedCreatureObjectTemplate::getCollisionOffsetX() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionOffsetX.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionOffsetX in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionOffsetX has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionOffsetX();
		}
	}

	float value = m_collisionOffsetX.getValue();
	char delta = m_collisionOffsetX.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionOffsetX();
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
}	// SharedCreatureObjectTemplate::getCollisionOffsetX

float SharedCreatureObjectTemplate::getCollisionOffsetXMin() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionOffsetX.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionOffsetX in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionOffsetX has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionOffsetXMin();
		}
	}

	float value = m_collisionOffsetX.getMinValue();
	char delta = m_collisionOffsetX.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionOffsetXMin();
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
}	// SharedCreatureObjectTemplate::getCollisionOffsetXMin

float SharedCreatureObjectTemplate::getCollisionOffsetXMax() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionOffsetX.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionOffsetX in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionOffsetX has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionOffsetXMax();
		}
	}

	float value = m_collisionOffsetX.getMaxValue();
	char delta = m_collisionOffsetX.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionOffsetXMax();
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
}	// SharedCreatureObjectTemplate::getCollisionOffsetXMax

float SharedCreatureObjectTemplate::getCollisionOffsetZ() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionOffsetZ.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionOffsetZ in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionOffsetZ has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionOffsetZ();
		}
	}

	float value = m_collisionOffsetZ.getValue();
	char delta = m_collisionOffsetZ.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionOffsetZ();
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
}	// SharedCreatureObjectTemplate::getCollisionOffsetZ

float SharedCreatureObjectTemplate::getCollisionOffsetZMin() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionOffsetZ.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionOffsetZ in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionOffsetZ has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionOffsetZMin();
		}
	}

	float value = m_collisionOffsetZ.getMinValue();
	char delta = m_collisionOffsetZ.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionOffsetZMin();
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
}	// SharedCreatureObjectTemplate::getCollisionOffsetZMin

float SharedCreatureObjectTemplate::getCollisionOffsetZMax() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionOffsetZ.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionOffsetZ in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionOffsetZ has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionOffsetZMax();
		}
	}

	float value = m_collisionOffsetZ.getMaxValue();
	char delta = m_collisionOffsetZ.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionOffsetZMax();
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
}	// SharedCreatureObjectTemplate::getCollisionOffsetZMax

float SharedCreatureObjectTemplate::getCollisionLength() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionLength.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionLength in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionLength has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionLength();
		}
	}

	float value = m_collisionLength.getValue();
	char delta = m_collisionLength.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionLength();
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
}	// SharedCreatureObjectTemplate::getCollisionLength

float SharedCreatureObjectTemplate::getCollisionLengthMin() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionLength.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionLength in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionLength has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionLengthMin();
		}
	}

	float value = m_collisionLength.getMinValue();
	char delta = m_collisionLength.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionLengthMin();
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
}	// SharedCreatureObjectTemplate::getCollisionLengthMin

float SharedCreatureObjectTemplate::getCollisionLengthMax() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_collisionLength.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter collisionLength in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter collisionLength has not been defined in template %s!", DataResource::getName()));
			return base->getCollisionLengthMax();
		}
	}

	float value = m_collisionLength.getMaxValue();
	char delta = m_collisionLength.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCollisionLengthMax();
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
}	// SharedCreatureObjectTemplate::getCollisionLengthMax

float SharedCreatureObjectTemplate::getCameraHeight() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_cameraHeight.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter cameraHeight in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter cameraHeight has not been defined in template %s!", DataResource::getName()));
			return base->getCameraHeight();
		}
	}

	float value = m_cameraHeight.getValue();
	char delta = m_cameraHeight.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCameraHeight();
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
}	// SharedCreatureObjectTemplate::getCameraHeight

float SharedCreatureObjectTemplate::getCameraHeightMin() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_cameraHeight.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter cameraHeight in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter cameraHeight has not been defined in template %s!", DataResource::getName()));
			return base->getCameraHeightMin();
		}
	}

	float value = m_cameraHeight.getMinValue();
	char delta = m_cameraHeight.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCameraHeightMin();
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
}	// SharedCreatureObjectTemplate::getCameraHeightMin

float SharedCreatureObjectTemplate::getCameraHeightMax() const
{


	const SharedCreatureObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
	}

	if (!m_cameraHeight.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter cameraHeight in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter cameraHeight has not been defined in template %s!", DataResource::getName()));
			return base->getCameraHeightMax();
		}
	}

	float value = m_cameraHeight.getMaxValue();
	char delta = m_cameraHeight.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getCameraHeightMax();
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
}	// SharedCreatureObjectTemplate::getCameraHeightMax


/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedCreatureObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedCreatureObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,1,3))
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
		if (strcmp(paramName, "gender") == 0)
			m_gender.loadFromIff(file);
		else if (strcmp(paramName, "niche") == 0)
			m_niche.loadFromIff(file);
		else if (strcmp(paramName, "species") == 0)
			m_species.loadFromIff(file);
		else if (strcmp(paramName, "race") == 0)
			m_race.loadFromIff(file);
		else if (strcmp(paramName, "acceleration") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 2, ("Template %s: read array size of %d for array \"acceleration\" of size 2, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 2 && j < listCount; ++j)
				m_acceleration[j].loadFromIff(file);
			// if there are more params for acceleration read and dump them
			for (; j < listCount; ++j)
			{
				FloatParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "speed") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 2, ("Template %s: read array size of %d for array \"speed\" of size 2, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 2 && j < listCount; ++j)
				m_speed[j].loadFromIff(file);
			// if there are more params for speed read and dump them
			for (; j < listCount; ++j)
			{
				FloatParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "turnRate") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 2, ("Template %s: read array size of %d for array \"turnRate\" of size 2, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 2 && j < listCount; ++j)
				m_turnRate[j].loadFromIff(file);
			// if there are more params for turnRate read and dump them
			for (; j < listCount; ++j)
			{
				FloatParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "animationMapFilename") == 0)
			m_animationMapFilename.loadFromIff(file);
		else if (strcmp(paramName, "slopeModAngle") == 0)
			m_slopeModAngle.loadFromIff(file);
		else if (strcmp(paramName, "slopeModPercent") == 0)
			m_slopeModPercent.loadFromIff(file);
		else if (strcmp(paramName, "waterModPercent") == 0)
			m_waterModPercent.loadFromIff(file);
		else if (strcmp(paramName, "stepHeight") == 0)
			m_stepHeight.loadFromIff(file);
		else if (strcmp(paramName, "collisionHeight") == 0)
			m_collisionHeight.loadFromIff(file);
		else if (strcmp(paramName, "collisionRadius") == 0)
			m_collisionRadius.loadFromIff(file);
		else if (strcmp(paramName, "movementDatatable") == 0)
			m_movementDatatable.loadFromIff(file);
		else if (strcmp(paramName, "postureAlignToTerrain") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 15, ("Template %s: read array size of %d for array \"postureAlignToTerrain\" of size 15, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 15 && j < listCount; ++j)
				m_postureAlignToTerrain[j].loadFromIff(file);
			// if there are more params for postureAlignToTerrain read and dump them
			for (; j < listCount; ++j)
			{
				BoolParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "swimHeight") == 0)
			m_swimHeight.loadFromIff(file);
		else if (strcmp(paramName, "warpTolerance") == 0)
			m_warpTolerance.loadFromIff(file);
		else if (strcmp(paramName, "collisionOffsetX") == 0)
			m_collisionOffsetX.loadFromIff(file);
		else if (strcmp(paramName, "collisionOffsetZ") == 0)
			m_collisionOffsetZ.loadFromIff(file);
		else if (strcmp(paramName, "collisionLength") == 0)
			m_collisionLength.loadFromIff(file);
		else if (strcmp(paramName, "cameraHeight") == 0)
			m_cameraHeight.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedTangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedCreatureObjectTemplate::load

//@END TFD

