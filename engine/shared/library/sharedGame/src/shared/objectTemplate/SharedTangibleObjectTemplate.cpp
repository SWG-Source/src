//========================================================================
//
// SharedTangibleObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedTangibleObjectTemplate.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFile/Iff.h"
#include "sharedGame/AssetCustomizationManager.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedMath/PaletteArgbList.h"
#include "sharedObject/BasicRangedIntCustomizationVariable.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/StructureFootprint.h"
#include "sharedObject/ObjectTemplateList.h"
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS
#include <algorithm>
#include <cstdio>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool SharedTangibleObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedTangibleObjectTemplate::SharedTangibleObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedObjectTemplate(filename)
	,m_paletteColorCustomizationVariablesLoaded(false)
	,m_paletteColorCustomizationVariablesAppend(false)
	,m_rangedIntCustomizationVariablesLoaded(false)
	,m_rangedIntCustomizationVariablesAppend(false)
	,m_constStringCustomizationVariablesLoaded(false)
	,m_constStringCustomizationVariablesAppend(false)
	,m_socketDestinationsLoaded(false)
	,m_socketDestinationsAppend(false)
	,m_certificationsRequiredLoaded(false)
	,m_certificationsRequiredAppend(false)
	,m_customizationVariableMappingLoaded(false)
	,m_customizationVariableMappingAppend(false)
	,m_versionOk(true)
//@END TFD INIT
	,m_structureFootprint (0)
{
}	// SharedTangibleObjectTemplate::SharedTangibleObjectTemplate

/**
 * Class destructor.
 */
SharedTangibleObjectTemplate::~SharedTangibleObjectTemplate()
{
//@BEGIN TFD CLEANUP
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_paletteColorCustomizationVariables.begin(); iter != m_paletteColorCustomizationVariables.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_paletteColorCustomizationVariables.clear();
	}
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_rangedIntCustomizationVariables.begin(); iter != m_rangedIntCustomizationVariables.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_rangedIntCustomizationVariables.clear();
	}
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_constStringCustomizationVariables.begin(); iter != m_constStringCustomizationVariables.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_constStringCustomizationVariables.clear();
	}
	{
		std::vector<IntegerParam *>::iterator iter;
		for (iter = m_socketDestinations.begin(); iter != m_socketDestinations.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_socketDestinations.clear();
	}
	{
		std::vector<StringParam *>::iterator iter;
		for (iter = m_certificationsRequired.begin(); iter != m_certificationsRequired.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_certificationsRequired.clear();
	}
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_customizationVariableMapping.begin(); iter != m_customizationVariableMapping.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_customizationVariableMapping.clear();
	}
//@END TFD CLEANUP
	
	if (m_structureFootprint)
	{
		delete m_structureFootprint;
		m_structureFootprint = 0;
	}
}	// SharedTangibleObjectTemplate::~SharedTangibleObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedTangibleObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedTangibleObjectTemplate_tag, create);
}	// SharedTangibleObjectTemplate::registerMe

/**
 * Creates a SharedTangibleObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedTangibleObjectTemplate::create(const std::string & filename)
{
	return new SharedTangibleObjectTemplate(filename);
}	// SharedTangibleObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedTangibleObjectTemplate::getId(void) const
{
	return SharedTangibleObjectTemplate_tag;
}	// SharedTangibleObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedTangibleObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedTangibleObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedTangibleObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedTangibleObjectTemplate::getHighestTemplateVersion

/**
 * Perform any post load initialization on the object
 */
void SharedTangibleObjectTemplate::postLoad()
{
	SharedObjectTemplate::postLoad ();

	//-- load the structure footprint
	if (getStructureFootprintFileName ().length () != 0)
	{
		m_structureFootprint = new StructureFootprint ();
		m_structureFootprint->load (getStructureFootprintFileName ().c_str ());
	}
}

/**
 * Returns the structure footprint
 *
 * @return the structure footprint
 */
const StructureFootprint* SharedTangibleObjectTemplate::getStructureFootprint () const
{
	return m_structureFootprint;
}

/**
 * Create a CustomizationDataProperty and associate it with the specified Object.
 *
 * This function will not add a CustomizationDataProperty if there are no
 * customization variables associated with the template or its base data template.
 *
 * The CustomizationData within the property will be filled with
 * the variable and its default for each customization variable
 * exported by this ObjectTemplate instance.
 *
 * The implementation makes use of the append-style base data class
 * mechanism for the customization variable lists.
 *
 * The forceCreation feature is added to allow the caller to insist that a CustomizationData
 * property be added to the Object.  The client needs this functionality for wearables ---
 * even if the wearable doesn't own any customization variables, it most likely needs to
 * link and inherit customization data from its owner (the Object on which the wearable is
 * worn).
 *
 * @param object         the Object instance with which to create and associate the
 *                       CustomizationDataProperty.
 * @param forceCreation  if true, will force a CustomizationData property to
 *                       be created even if there are no customization variable
 *                       declarations.
 */
void SharedTangibleObjectTemplate::createCustomizationDataPropertyAsNeeded(Object &object, bool /* forceCreation */) const
{
#if 1
	//-- Properties cannot be added while an object is in the world.  Some callers may be in the world, 
	//   so temporarily remove the object from the world if necessary.
	bool shouldBeInWorld = object.isInWorld();
	if (shouldBeInWorld)
		object.removeFromWorld();

	//-- create the CustomizationDataProperty, add to Object property collection
	CustomizationDataProperty *const cdProperty = new CustomizationDataProperty(object);
	object.addProperty(*cdProperty);

	//-- Put object back in world if it was originally there.
	if (shouldBeInWorld)
		object.addToWorld();

	//-- fetch the CustomizationData instance
	CustomizationData *const customizationData = cdProperty->fetchCustomizationData();
	if (customizationData)
	{
		//-- Create customization variables associated with this appearance.  Skip /shared_owner/
		//   variables since we're not a creature.  The SharedCreatureObjectTemplate overrides
		//   this function and sets it to false.
		bool const skipSharedOwnerVariables = true;
		AssetCustomizationManager::addCustomizationVariablesForAsset(TemporaryCrcString(getAppearanceFilename().c_str(), true), *customizationData, skipSharedOwnerVariables);

		
#ifdef _DEBUG
		//-- set up mappings for any variables which need dependent mappings
 		int numVariableMappings = getCustomizationVariableMappingCount();
		if(numVariableMappings != 0)
		{
			//If the object is a wearable, then the dependent variable, which is probably shared, will not exist at the moment the dependency
			//is set up.  For this reason, make sure that you're really doing what you want.
			DEBUG_WARNING(true, ("Generally, CustomizationVariableMapping should not be set on Wearables or other non-Creature tangibles."));
		}
#endif

		//-- release local reference to the CustomizationData instance
		customizationData->release();
	}

#else
	// @todo -TRF- remove this code as soon as asset customization manager is working properly.
	const size_t paletteColorCount = getPaletteColorCustomizationVariablesCount();
	const size_t rangedIntCount    = getRangedIntCustomizationVariablesCount();

	//-- Return if caller isn't forcing customization data creation and no customization variables are declared for the ObjectTemplate.
	const bool makeCustomizationData = forceCreation || (paletteColorCount > 0) || (rangedIntCount > 0);
	if (!makeCustomizationData)
		return;

	//-- Properties cannot be added while an object is in the world.  Some callers may be in the world, 
	//   so temporarily remove the object from the world if necessary.
	bool shouldBeInWorld = object.isInWorld();
	if (shouldBeInWorld)
		object.removeFromWorld();

	//-- create the CustomizationDataProperty, add to Object property collection
	CustomizationDataProperty *const cdProperty = new CustomizationDataProperty(object);
	object.addProperty(*cdProperty);

	//-- Put object back in world if it was originally there.
	if (shouldBeInWorld)
		object.addToWorld();

	//-- fetch the CustomizationData instance
	CustomizationData *const customizationData = cdProperty->fetchCustomizationData();

	//-- create PaletteColorCustomizationVariable variables as specified
	{
		SharedTangibleObjectTemplate::PaletteColorCustomizationVariable  variableData;

		for (int i = 0; i < static_cast<int>(paletteColorCount); ++i)
		{
			//-- get the palette variable data
			getPaletteColorCustomizationVariables(variableData, i);

			//-- fetch the palette
			const PaletteArgb *const palette = PaletteArgbList::fetch(TemporaryCrcString(variableData.palettePathName.c_str(), true));
			if (!palette)
			{
				DEBUG_WARNING(true, ("failed to retrieve color palette [%s] for [%s], skipping variable [%s].\n", variableData.palettePathName.c_str(), getName (), variableData.variableName.c_str()));
				continue;
			}

			//-- create the variable, add to CustomizationData
			::PaletteColorCustomizationVariable * const palColorVar = new ::PaletteColorCustomizationVariable(palette, variableData.defaultPaletteIndex);

			if (variableData.defaultPaletteIndex != palColorVar->getValue ())
				DEBUG_WARNING (true, ("Error loading PaletteColorCustomizationVariable [%s] for [%s]", variableData.variableName.c_str(), getName ()));

			customizationData->addVariableTakeOwnership(variableData.variableName, palColorVar);

			//-- release local palette reference
			palette->release();
		}
	}

	//-- create BasicRangedIntCustomizationVariable variables as specified
	{
		SharedTangibleObjectTemplate::RangedIntCustomizationVariable  variableData;

		for (int i = 0; i < static_cast<int>(rangedIntCount); ++i)
		{
			//-- get the palette variable data
			getRangedIntCustomizationVariables(variableData, i);

			//-- create the variable, add to CustomizationData
			customizationData->addVariableTakeOwnership(variableData.variableName, new ::BasicRangedIntCustomizationVariable(variableData.minValueInclusive, variableData.defaultValue, variableData.maxValueExclusive));
		}
	}
	//-- release local reference to the CustomizationData instance
	customizationData->release();
#endif
}

//@BEGIN TFD
void SharedTangibleObjectTemplate::getPaletteColorCustomizationVariables(PaletteColorCustomizationVariable &data, int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_paletteColorCustomizationVariablesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter paletteColorCustomizationVariables in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter paletteColorCustomizationVariables has not been defined in template %s!", DataResource::getName()));
			base->getPaletteColorCustomizationVariables(data, index);
			return;
		}
	}

	if (m_paletteColorCustomizationVariablesAppend && base != nullptr)
	{
		int baseCount = base->getPaletteColorCustomizationVariablesCount();
		if (index < baseCount)
			{
				base->getPaletteColorCustomizationVariables(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_paletteColorCustomizationVariables.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_paletteColorCustomizationVariables[index]).getValue();
	NOT_NULL(structTemplate);
	const _PaletteColorCustomizationVariable *param = dynamic_cast<const _PaletteColorCustomizationVariable *>(structTemplate);
	NOT_NULL(param);
	data.variableName = param->getVariableName(m_versionOk);
	data.palettePathName = param->getPalettePathName(m_versionOk);
	data.defaultPaletteIndex = param->getDefaultPaletteIndex(m_versionOk);
}	// SharedTangibleObjectTemplate::getPaletteColorCustomizationVariables

void SharedTangibleObjectTemplate::getPaletteColorCustomizationVariablesMin(PaletteColorCustomizationVariable &data, int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_paletteColorCustomizationVariablesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter paletteColorCustomizationVariables in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter paletteColorCustomizationVariables has not been defined in template %s!", DataResource::getName()));
			base->getPaletteColorCustomizationVariablesMin(data, index);
			return;
		}
	}

	if (m_paletteColorCustomizationVariablesAppend && base != nullptr)
	{
		int baseCount = base->getPaletteColorCustomizationVariablesCount();
		if (index < baseCount)
			{
				base->getPaletteColorCustomizationVariablesMin(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_paletteColorCustomizationVariables.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_paletteColorCustomizationVariables[index]).getValue();
	NOT_NULL(structTemplate);
	const _PaletteColorCustomizationVariable *param = dynamic_cast<const _PaletteColorCustomizationVariable *>(structTemplate);
	NOT_NULL(param);
	data.variableName = param->getVariableName(m_versionOk);
	data.palettePathName = param->getPalettePathName(m_versionOk);
	data.defaultPaletteIndex = param->getDefaultPaletteIndexMin(m_versionOk);
}	// SharedTangibleObjectTemplate::getPaletteColorCustomizationVariablesMin

void SharedTangibleObjectTemplate::getPaletteColorCustomizationVariablesMax(PaletteColorCustomizationVariable &data, int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_paletteColorCustomizationVariablesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter paletteColorCustomizationVariables in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter paletteColorCustomizationVariables has not been defined in template %s!", DataResource::getName()));
			base->getPaletteColorCustomizationVariablesMax(data, index);
			return;
		}
	}

	if (m_paletteColorCustomizationVariablesAppend && base != nullptr)
	{
		int baseCount = base->getPaletteColorCustomizationVariablesCount();
		if (index < baseCount)
			{
				base->getPaletteColorCustomizationVariablesMax(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_paletteColorCustomizationVariables.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_paletteColorCustomizationVariables[index]).getValue();
	NOT_NULL(structTemplate);
	const _PaletteColorCustomizationVariable *param = dynamic_cast<const _PaletteColorCustomizationVariable *>(structTemplate);
	NOT_NULL(param);
	data.variableName = param->getVariableName(m_versionOk);
	data.palettePathName = param->getPalettePathName(m_versionOk);
	data.defaultPaletteIndex = param->getDefaultPaletteIndexMax(m_versionOk);
}	// SharedTangibleObjectTemplate::getPaletteColorCustomizationVariablesMax

size_t SharedTangibleObjectTemplate::getPaletteColorCustomizationVariablesCount(void) const
{
	if (!m_paletteColorCustomizationVariablesLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getPaletteColorCustomizationVariablesCount();
	}

	size_t count = m_paletteColorCustomizationVariables.size();

	// if we are extending our base template, add it's count
	if (m_paletteColorCustomizationVariablesAppend && m_baseData != nullptr)
	{
		const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getPaletteColorCustomizationVariablesCount();
	}

	return count;
}	// SharedTangibleObjectTemplate::getPaletteColorCustomizationVariablesCount

void SharedTangibleObjectTemplate::getRangedIntCustomizationVariables(RangedIntCustomizationVariable &data, int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_rangedIntCustomizationVariablesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter rangedIntCustomizationVariables in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter rangedIntCustomizationVariables has not been defined in template %s!", DataResource::getName()));
			base->getRangedIntCustomizationVariables(data, index);
			return;
		}
	}

	if (m_rangedIntCustomizationVariablesAppend && base != nullptr)
	{
		int baseCount = base->getRangedIntCustomizationVariablesCount();
		if (index < baseCount)
			{
				base->getRangedIntCustomizationVariables(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_rangedIntCustomizationVariables.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_rangedIntCustomizationVariables[index]).getValue();
	NOT_NULL(structTemplate);
	const _RangedIntCustomizationVariable *param = dynamic_cast<const _RangedIntCustomizationVariable *>(structTemplate);
	NOT_NULL(param);
	data.variableName = param->getVariableName(m_versionOk);
	data.minValueInclusive = param->getMinValueInclusive(m_versionOk);
	data.defaultValue = param->getDefaultValue(m_versionOk);
	data.maxValueExclusive = param->getMaxValueExclusive(m_versionOk);
}	// SharedTangibleObjectTemplate::getRangedIntCustomizationVariables

void SharedTangibleObjectTemplate::getRangedIntCustomizationVariablesMin(RangedIntCustomizationVariable &data, int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_rangedIntCustomizationVariablesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter rangedIntCustomizationVariables in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter rangedIntCustomizationVariables has not been defined in template %s!", DataResource::getName()));
			base->getRangedIntCustomizationVariablesMin(data, index);
			return;
		}
	}

	if (m_rangedIntCustomizationVariablesAppend && base != nullptr)
	{
		int baseCount = base->getRangedIntCustomizationVariablesCount();
		if (index < baseCount)
			{
				base->getRangedIntCustomizationVariablesMin(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_rangedIntCustomizationVariables.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_rangedIntCustomizationVariables[index]).getValue();
	NOT_NULL(structTemplate);
	const _RangedIntCustomizationVariable *param = dynamic_cast<const _RangedIntCustomizationVariable *>(structTemplate);
	NOT_NULL(param);
	data.variableName = param->getVariableName(m_versionOk);
	data.minValueInclusive = param->getMinValueInclusiveMin(m_versionOk);
	data.defaultValue = param->getDefaultValueMin(m_versionOk);
	data.maxValueExclusive = param->getMaxValueExclusiveMin(m_versionOk);
}	// SharedTangibleObjectTemplate::getRangedIntCustomizationVariablesMin

void SharedTangibleObjectTemplate::getRangedIntCustomizationVariablesMax(RangedIntCustomizationVariable &data, int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_rangedIntCustomizationVariablesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter rangedIntCustomizationVariables in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter rangedIntCustomizationVariables has not been defined in template %s!", DataResource::getName()));
			base->getRangedIntCustomizationVariablesMax(data, index);
			return;
		}
	}

	if (m_rangedIntCustomizationVariablesAppend && base != nullptr)
	{
		int baseCount = base->getRangedIntCustomizationVariablesCount();
		if (index < baseCount)
			{
				base->getRangedIntCustomizationVariablesMax(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_rangedIntCustomizationVariables.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_rangedIntCustomizationVariables[index]).getValue();
	NOT_NULL(structTemplate);
	const _RangedIntCustomizationVariable *param = dynamic_cast<const _RangedIntCustomizationVariable *>(structTemplate);
	NOT_NULL(param);
	data.variableName = param->getVariableName(m_versionOk);
	data.minValueInclusive = param->getMinValueInclusiveMax(m_versionOk);
	data.defaultValue = param->getDefaultValueMax(m_versionOk);
	data.maxValueExclusive = param->getMaxValueExclusiveMax(m_versionOk);
}	// SharedTangibleObjectTemplate::getRangedIntCustomizationVariablesMax

size_t SharedTangibleObjectTemplate::getRangedIntCustomizationVariablesCount(void) const
{
	if (!m_rangedIntCustomizationVariablesLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getRangedIntCustomizationVariablesCount();
	}

	size_t count = m_rangedIntCustomizationVariables.size();

	// if we are extending our base template, add it's count
	if (m_rangedIntCustomizationVariablesAppend && m_baseData != nullptr)
	{
		const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getRangedIntCustomizationVariablesCount();
	}

	return count;
}	// SharedTangibleObjectTemplate::getRangedIntCustomizationVariablesCount

void SharedTangibleObjectTemplate::getConstStringCustomizationVariables(ConstStringCustomizationVariable &data, int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_constStringCustomizationVariablesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter constStringCustomizationVariables in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter constStringCustomizationVariables has not been defined in template %s!", DataResource::getName()));
			base->getConstStringCustomizationVariables(data, index);
			return;
		}
	}

	if (m_constStringCustomizationVariablesAppend && base != nullptr)
	{
		int baseCount = base->getConstStringCustomizationVariablesCount();
		if (index < baseCount)
			{
				base->getConstStringCustomizationVariables(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_constStringCustomizationVariables.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_constStringCustomizationVariables[index]).getValue();
	NOT_NULL(structTemplate);
	const _ConstStringCustomizationVariable *param = dynamic_cast<const _ConstStringCustomizationVariable *>(structTemplate);
	NOT_NULL(param);
	data.variableName = param->getVariableName(m_versionOk);
	data.constValue = param->getConstValue(m_versionOk);
}	// SharedTangibleObjectTemplate::getConstStringCustomizationVariables

void SharedTangibleObjectTemplate::getConstStringCustomizationVariablesMin(ConstStringCustomizationVariable &data, int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_constStringCustomizationVariablesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter constStringCustomizationVariables in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter constStringCustomizationVariables has not been defined in template %s!", DataResource::getName()));
			base->getConstStringCustomizationVariablesMin(data, index);
			return;
		}
	}

	if (m_constStringCustomizationVariablesAppend && base != nullptr)
	{
		int baseCount = base->getConstStringCustomizationVariablesCount();
		if (index < baseCount)
			{
				base->getConstStringCustomizationVariablesMin(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_constStringCustomizationVariables.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_constStringCustomizationVariables[index]).getValue();
	NOT_NULL(structTemplate);
	const _ConstStringCustomizationVariable *param = dynamic_cast<const _ConstStringCustomizationVariable *>(structTemplate);
	NOT_NULL(param);
	data.variableName = param->getVariableName(m_versionOk);
	data.constValue = param->getConstValue(m_versionOk);
}	// SharedTangibleObjectTemplate::getConstStringCustomizationVariablesMin

void SharedTangibleObjectTemplate::getConstStringCustomizationVariablesMax(ConstStringCustomizationVariable &data, int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_constStringCustomizationVariablesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter constStringCustomizationVariables in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter constStringCustomizationVariables has not been defined in template %s!", DataResource::getName()));
			base->getConstStringCustomizationVariablesMax(data, index);
			return;
		}
	}

	if (m_constStringCustomizationVariablesAppend && base != nullptr)
	{
		int baseCount = base->getConstStringCustomizationVariablesCount();
		if (index < baseCount)
			{
				base->getConstStringCustomizationVariablesMax(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_constStringCustomizationVariables.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_constStringCustomizationVariables[index]).getValue();
	NOT_NULL(structTemplate);
	const _ConstStringCustomizationVariable *param = dynamic_cast<const _ConstStringCustomizationVariable *>(structTemplate);
	NOT_NULL(param);
	data.variableName = param->getVariableName(m_versionOk);
	data.constValue = param->getConstValue(m_versionOk);
}	// SharedTangibleObjectTemplate::getConstStringCustomizationVariablesMax

size_t SharedTangibleObjectTemplate::getConstStringCustomizationVariablesCount(void) const
{
	if (!m_constStringCustomizationVariablesLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getConstStringCustomizationVariablesCount();
	}

	size_t count = m_constStringCustomizationVariables.size();

	// if we are extending our base template, add it's count
	if (m_constStringCustomizationVariablesAppend && m_baseData != nullptr)
	{
		const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getConstStringCustomizationVariablesCount();
	}

	return count;
}	// SharedTangibleObjectTemplate::getConstStringCustomizationVariablesCount

SharedTangibleObjectTemplate::GameObjectType SharedTangibleObjectTemplate::getSocketDestinations(int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_socketDestinationsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter socketDestinations in template %s", DataResource::getName()));
			return static_cast<GameObjectType>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter socketDestinations has not been defined in template %s!", DataResource::getName()));
			return base->getSocketDestinations(index);
		}
	}

	if (m_socketDestinationsAppend && base != nullptr)
	{
		int baseCount = base->getSocketDestinationsCount();
		if (index < baseCount)
			return base->getSocketDestinations(index);
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_socketDestinations.size(), ("template param index out of range"));
	return static_cast<GameObjectType>(m_socketDestinations[index]->getValue());
}	// SharedTangibleObjectTemplate::getSocketDestinations

size_t SharedTangibleObjectTemplate::getSocketDestinationsCount(void) const
{
	if (!m_socketDestinationsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getSocketDestinationsCount();
	}

	size_t count = m_socketDestinations.size();

	// if we are extending our base template, add it's count
	if (m_socketDestinationsAppend && m_baseData != nullptr)
	{
		const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getSocketDestinationsCount();
	}

	return count;
}	// SharedTangibleObjectTemplate::getSocketDestinationsCount

const std::string & SharedTangibleObjectTemplate::getStructureFootprintFileName(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getStructureFootprintFileName(true);
#endif
	}

	if (!m_structureFootprintFileName.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter structureFootprintFileName in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter structureFootprintFileName has not been defined in template %s!", DataResource::getName()));
			return base->getStructureFootprintFileName();
		}
	}

	const std::string & value = m_structureFootprintFileName.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedTangibleObjectTemplate::getStructureFootprintFileName

bool SharedTangibleObjectTemplate::getUseStructureFootprintOutline(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getUseStructureFootprintOutline(true);
#endif
	}

	if (!m_useStructureFootprintOutline.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter useStructureFootprintOutline in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter useStructureFootprintOutline has not been defined in template %s!", DataResource::getName()));
			return base->getUseStructureFootprintOutline();
		}
	}

	bool value = m_useStructureFootprintOutline.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedTangibleObjectTemplate::getUseStructureFootprintOutline

bool SharedTangibleObjectTemplate::getTargetable(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getTargetable(true);
#endif
	}

	if (!m_targetable.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter targetable in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter targetable has not been defined in template %s!", DataResource::getName()));
			return base->getTargetable();
		}
	}

	bool value = m_targetable.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedTangibleObjectTemplate::getTargetable

const std::string & SharedTangibleObjectTemplate::getCertificationsRequired(int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_certificationsRequiredLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter certificationsRequired in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter certificationsRequired has not been defined in template %s!", DataResource::getName()));
			return base->getCertificationsRequired(index);
		}
	}

	if (m_certificationsRequiredAppend && base != nullptr)
	{
		int baseCount = base->getCertificationsRequiredCount();
		if (index < baseCount)
			return base->getCertificationsRequired(index);
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_certificationsRequired.size(), ("template param index out of range"));
	const std::string & value = m_certificationsRequired[index]->getValue();
	return value;
}	// SharedTangibleObjectTemplate::getCertificationsRequired

size_t SharedTangibleObjectTemplate::getCertificationsRequiredCount(void) const
{
	if (!m_certificationsRequiredLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getCertificationsRequiredCount();
	}

	size_t count = m_certificationsRequired.size();

	// if we are extending our base template, add it's count
	if (m_certificationsRequiredAppend && m_baseData != nullptr)
	{
		const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getCertificationsRequiredCount();
	}

	return count;
}	// SharedTangibleObjectTemplate::getCertificationsRequiredCount

void SharedTangibleObjectTemplate::getCustomizationVariableMapping(CustomizationVariableMapping &data, int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_customizationVariableMappingLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter customizationVariableMapping in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter customizationVariableMapping has not been defined in template %s!", DataResource::getName()));
			base->getCustomizationVariableMapping(data, index);
			return;
		}
	}

	if (m_customizationVariableMappingAppend && base != nullptr)
	{
		int baseCount = base->getCustomizationVariableMappingCount();
		if (index < baseCount)
			{
				base->getCustomizationVariableMapping(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_customizationVariableMapping.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_customizationVariableMapping[index]).getValue();
	NOT_NULL(structTemplate);
	const _CustomizationVariableMapping *param = dynamic_cast<const _CustomizationVariableMapping *>(structTemplate);
	NOT_NULL(param);
	data.sourceVariable = param->getSourceVariable(m_versionOk);
	data.dependentVariable = param->getDependentVariable(m_versionOk);
}	// SharedTangibleObjectTemplate::getCustomizationVariableMapping

void SharedTangibleObjectTemplate::getCustomizationVariableMappingMin(CustomizationVariableMapping &data, int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_customizationVariableMappingLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter customizationVariableMapping in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter customizationVariableMapping has not been defined in template %s!", DataResource::getName()));
			base->getCustomizationVariableMappingMin(data, index);
			return;
		}
	}

	if (m_customizationVariableMappingAppend && base != nullptr)
	{
		int baseCount = base->getCustomizationVariableMappingCount();
		if (index < baseCount)
			{
				base->getCustomizationVariableMappingMin(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_customizationVariableMapping.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_customizationVariableMapping[index]).getValue();
	NOT_NULL(structTemplate);
	const _CustomizationVariableMapping *param = dynamic_cast<const _CustomizationVariableMapping *>(structTemplate);
	NOT_NULL(param);
	data.sourceVariable = param->getSourceVariable(m_versionOk);
	data.dependentVariable = param->getDependentVariable(m_versionOk);
}	// SharedTangibleObjectTemplate::getCustomizationVariableMappingMin

void SharedTangibleObjectTemplate::getCustomizationVariableMappingMax(CustomizationVariableMapping &data, int index) const
{
	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	}

	if (!m_customizationVariableMappingLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter customizationVariableMapping in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter customizationVariableMapping has not been defined in template %s!", DataResource::getName()));
			base->getCustomizationVariableMappingMax(data, index);
			return;
		}
	}

	if (m_customizationVariableMappingAppend && base != nullptr)
	{
		int baseCount = base->getCustomizationVariableMappingCount();
		if (index < baseCount)
			{
				base->getCustomizationVariableMappingMax(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_customizationVariableMapping.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_customizationVariableMapping[index]).getValue();
	NOT_NULL(structTemplate);
	const _CustomizationVariableMapping *param = dynamic_cast<const _CustomizationVariableMapping *>(structTemplate);
	NOT_NULL(param);
	data.sourceVariable = param->getSourceVariable(m_versionOk);
	data.dependentVariable = param->getDependentVariable(m_versionOk);
}	// SharedTangibleObjectTemplate::getCustomizationVariableMappingMax

size_t SharedTangibleObjectTemplate::getCustomizationVariableMappingCount(void) const
{
	if (!m_customizationVariableMappingLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getCustomizationVariableMappingCount();
	}

	size_t count = m_customizationVariableMapping.size();

	// if we are extending our base template, add it's count
	if (m_customizationVariableMappingAppend && m_baseData != nullptr)
	{
		const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getCustomizationVariableMappingCount();
	}

	return count;
}	// SharedTangibleObjectTemplate::getCustomizationVariableMappingCount

SharedTangibleObjectTemplate::ClientVisabilityFlags SharedTangibleObjectTemplate::getClientVisabilityFlag(bool testData) const
{
#ifdef _DEBUG
SharedTangibleObjectTemplate::ClientVisabilityFlags testDataValue = static_cast<SharedTangibleObjectTemplate::ClientVisabilityFlags>(0);
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getClientVisabilityFlag(true);
#endif
	}

	if (!m_clientVisabilityFlag.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter clientVisabilityFlag in template %s", DataResource::getName()));
			return static_cast<ClientVisabilityFlags>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter clientVisabilityFlag has not been defined in template %s!", DataResource::getName()));
			return base->getClientVisabilityFlag();
		}
	}

	ClientVisabilityFlags value = static_cast<ClientVisabilityFlags>(m_clientVisabilityFlag.getValue());
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedTangibleObjectTemplate::getClientVisabilityFlag

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedTangibleObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getStructureFootprintFileName(true));
	IGNORE_RETURN(getUseStructureFootprintOutline(true));
	IGNORE_RETURN(getTargetable(true));
	IGNORE_RETURN(getClientVisabilityFlag(true));
	SharedObjectTemplate::testValues();
}	// SharedTangibleObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedTangibleObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedTangibleObjectTemplate_tag)
	{
		SharedObjectTemplate::load(file);
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
	if (getHighestTemplateVersion() != TAG(0,0,1,0))
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
		if (strcmp(paramName, "paletteColorCustomizationVariables") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_paletteColorCustomizationVariables.begin(); iter != m_paletteColorCustomizationVariables.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_paletteColorCustomizationVariables.clear();
			m_paletteColorCustomizationVariablesAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_paletteColorCustomizationVariables.push_back(newData);
			}
			m_paletteColorCustomizationVariablesLoaded = true;
		}
		else if (strcmp(paramName, "rangedIntCustomizationVariables") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_rangedIntCustomizationVariables.begin(); iter != m_rangedIntCustomizationVariables.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_rangedIntCustomizationVariables.clear();
			m_rangedIntCustomizationVariablesAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_rangedIntCustomizationVariables.push_back(newData);
			}
			m_rangedIntCustomizationVariablesLoaded = true;
		}
		else if (strcmp(paramName, "constStringCustomizationVariables") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_constStringCustomizationVariables.begin(); iter != m_constStringCustomizationVariables.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_constStringCustomizationVariables.clear();
			m_constStringCustomizationVariablesAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_constStringCustomizationVariables.push_back(newData);
			}
			m_constStringCustomizationVariablesLoaded = true;
		}
		else if (strcmp(paramName, "socketDestinations") == 0)
		{
			std::vector<IntegerParam *>::iterator iter;
			for (iter = m_socketDestinations.begin(); iter != m_socketDestinations.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_socketDestinations.clear();
			m_socketDestinationsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				IntegerParam * newData = new IntegerParam;
				newData->loadFromIff(file);
				m_socketDestinations.push_back(newData);
			}
			m_socketDestinationsLoaded = true;
		}
		else if (strcmp(paramName, "structureFootprintFileName") == 0)
			m_structureFootprintFileName.loadFromIff(file);
		else if (strcmp(paramName, "useStructureFootprintOutline") == 0)
			m_useStructureFootprintOutline.loadFromIff(file);
		else if (strcmp(paramName, "targetable") == 0)
			m_targetable.loadFromIff(file);
		else if (strcmp(paramName, "certificationsRequired") == 0)
		{
			std::vector<StringParam *>::iterator iter;
			for (iter = m_certificationsRequired.begin(); iter != m_certificationsRequired.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_certificationsRequired.clear();
			m_certificationsRequiredAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringParam * newData = new StringParam;
				newData->loadFromIff(file);
				m_certificationsRequired.push_back(newData);
			}
			m_certificationsRequiredLoaded = true;
		}
		else if (strcmp(paramName, "customizationVariableMapping") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_customizationVariableMapping.begin(); iter != m_customizationVariableMapping.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_customizationVariableMapping.clear();
			m_customizationVariableMappingAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_customizationVariableMapping.push_back(newData);
			}
			m_customizationVariableMappingLoaded = true;
		}
		else if (strcmp(paramName, "clientVisabilityFlag") == 0)
			m_clientVisabilityFlag.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedTangibleObjectTemplate::load


//=============================================================================
// class SharedTangibleObjectTemplate::_ConstStringCustomizationVariable

/**
 * Class constructor.
 */
SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::_ConstStringCustomizationVariable(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::_ConstStringCustomizationVariable

/**
 * Class destructor.
 */
SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::~_ConstStringCustomizationVariable()
{
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::~_ConstStringCustomizationVariable

/**
 * Static function used to register this template.
 */
void SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_ConstStringCustomizationVariable_tag, create);
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::registerMe

/**
 * Creates a SharedTangibleObjectTemplate::_ConstStringCustomizationVariable template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::create(const std::string & filename)
{
	return new SharedTangibleObjectTemplate::_ConstStringCustomizationVariable(filename);
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getId(void) const
{
	return _ConstStringCustomizationVariable_tag;
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getId

const std::string & SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getVariableName(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_ConstStringCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_ConstStringCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getVariableName(true);
#endif
	}

	if (!m_variableName.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter variableName in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter variableName has not been defined in template %s!", DataResource::getName()));
			return base->getVariableName(versionOk);
		}
	}

	const std::string & value = m_variableName.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getVariableName

const std::string & SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getConstValue(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_ConstStringCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_ConstStringCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getConstValue(true);
#endif
	}

	if (!m_constValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter constValue in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter constValue has not been defined in template %s!", DataResource::getName()));
			return base->getConstValue(versionOk);
		}
	}

	const std::string & value = m_constValue.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getConstValue

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::testValues(void) const
{
	IGNORE_RETURN(getVariableName(true));
	IGNORE_RETURN(getConstValue(true));
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::load(Iff &file)
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
		if (strcmp(paramName, "variableName") == 0)
			m_variableName.loadFromIff(file);
		else if (strcmp(paramName, "constValue") == 0)
			m_constValue.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::load


//=============================================================================
// class SharedTangibleObjectTemplate::_CustomizationVariableMapping

/**
 * Class constructor.
 */
SharedTangibleObjectTemplate::_CustomizationVariableMapping::_CustomizationVariableMapping(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::_CustomizationVariableMapping

/**
 * Class destructor.
 */
SharedTangibleObjectTemplate::_CustomizationVariableMapping::~_CustomizationVariableMapping()
{
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::~_CustomizationVariableMapping

/**
 * Static function used to register this template.
 */
void SharedTangibleObjectTemplate::_CustomizationVariableMapping::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_CustomizationVariableMapping_tag, create);
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::registerMe

/**
 * Creates a SharedTangibleObjectTemplate::_CustomizationVariableMapping template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedTangibleObjectTemplate::_CustomizationVariableMapping::create(const std::string & filename)
{
	return new SharedTangibleObjectTemplate::_CustomizationVariableMapping(filename);
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedTangibleObjectTemplate::_CustomizationVariableMapping::getId(void) const
{
	return _CustomizationVariableMapping_tag;
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::getId

const std::string & SharedTangibleObjectTemplate::_CustomizationVariableMapping::getSourceVariable(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_CustomizationVariableMapping * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_CustomizationVariableMapping *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getSourceVariable(true);
#endif
	}

	if (!m_sourceVariable.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter sourceVariable in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter sourceVariable has not been defined in template %s!", DataResource::getName()));
			return base->getSourceVariable(versionOk);
		}
	}

	const std::string & value = m_sourceVariable.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::getSourceVariable

const std::string & SharedTangibleObjectTemplate::_CustomizationVariableMapping::getDependentVariable(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_CustomizationVariableMapping * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_CustomizationVariableMapping *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDependentVariable(true);
#endif
	}

	if (!m_dependentVariable.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter dependentVariable in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter dependentVariable has not been defined in template %s!", DataResource::getName()));
			return base->getDependentVariable(versionOk);
		}
	}

	const std::string & value = m_dependentVariable.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::getDependentVariable

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedTangibleObjectTemplate::_CustomizationVariableMapping::testValues(void) const
{
	IGNORE_RETURN(getSourceVariable(true));
	IGNORE_RETURN(getDependentVariable(true));
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedTangibleObjectTemplate::_CustomizationVariableMapping::load(Iff &file)
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
		if (strcmp(paramName, "sourceVariable") == 0)
			m_sourceVariable.loadFromIff(file);
		else if (strcmp(paramName, "dependentVariable") == 0)
			m_dependentVariable.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::load


//=============================================================================
// class SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable

/**
 * Class constructor.
 */
SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::_PaletteColorCustomizationVariable(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::_PaletteColorCustomizationVariable

/**
 * Class destructor.
 */
SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::~_PaletteColorCustomizationVariable()
{
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::~_PaletteColorCustomizationVariable

/**
 * Static function used to register this template.
 */
void SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_PaletteColorCustomizationVariable_tag, create);
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::registerMe

/**
 * Creates a SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::create(const std::string & filename)
{
	return new SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable(filename);
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getId(void) const
{
	return _PaletteColorCustomizationVariable_tag;
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getId

const std::string & SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getVariableName(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getVariableName(true);
#endif
	}

	if (!m_variableName.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter variableName in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter variableName has not been defined in template %s!", DataResource::getName()));
			return base->getVariableName(versionOk);
		}
	}

	const std::string & value = m_variableName.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getVariableName

const std::string & SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getPalettePathName(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getPalettePathName(true);
#endif
	}

	if (!m_palettePathName.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter palettePathName in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter palettePathName has not been defined in template %s!", DataResource::getName()));
			return base->getPalettePathName(versionOk);
		}
	}

	const std::string & value = m_palettePathName.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getPalettePathName

int SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getDefaultPaletteIndex(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDefaultPaletteIndex(true);
#endif
	}

	if (!m_defaultPaletteIndex.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter defaultPaletteIndex in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter defaultPaletteIndex has not been defined in template %s!", DataResource::getName()));
			return base->getDefaultPaletteIndex(versionOk);
		}
	}

	int value = m_defaultPaletteIndex.getValue();
	char delta = m_defaultPaletteIndex.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDefaultPaletteIndex(versionOk);
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
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getDefaultPaletteIndex

int SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getDefaultPaletteIndexMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDefaultPaletteIndexMin(true);
#endif
	}

	if (!m_defaultPaletteIndex.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter defaultPaletteIndex in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter defaultPaletteIndex has not been defined in template %s!", DataResource::getName()));
			return base->getDefaultPaletteIndexMin(versionOk);
		}
	}

	int value = m_defaultPaletteIndex.getMinValue();
	char delta = m_defaultPaletteIndex.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDefaultPaletteIndexMin(versionOk);
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
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getDefaultPaletteIndexMin

int SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getDefaultPaletteIndexMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDefaultPaletteIndexMax(true);
#endif
	}

	if (!m_defaultPaletteIndex.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter defaultPaletteIndex in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter defaultPaletteIndex has not been defined in template %s!", DataResource::getName()));
			return base->getDefaultPaletteIndexMax(versionOk);
		}
	}

	int value = m_defaultPaletteIndex.getMaxValue();
	char delta = m_defaultPaletteIndex.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDefaultPaletteIndexMax(versionOk);
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
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getDefaultPaletteIndexMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::testValues(void) const
{
	IGNORE_RETURN(getVariableName(true));
	IGNORE_RETURN(getPalettePathName(true));
	IGNORE_RETURN(getDefaultPaletteIndexMin(true));
	IGNORE_RETURN(getDefaultPaletteIndexMax(true));
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::load(Iff &file)
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
		if (strcmp(paramName, "variableName") == 0)
			m_variableName.loadFromIff(file);
		else if (strcmp(paramName, "palettePathName") == 0)
			m_palettePathName.loadFromIff(file);
		else if (strcmp(paramName, "defaultPaletteIndex") == 0)
			m_defaultPaletteIndex.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::load


//=============================================================================
// class SharedTangibleObjectTemplate::_RangedIntCustomizationVariable

/**
 * Class constructor.
 */
SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::_RangedIntCustomizationVariable(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::_RangedIntCustomizationVariable

/**
 * Class destructor.
 */
SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::~_RangedIntCustomizationVariable()
{
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::~_RangedIntCustomizationVariable

/**
 * Static function used to register this template.
 */
void SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_RangedIntCustomizationVariable_tag, create);
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::registerMe

/**
 * Creates a SharedTangibleObjectTemplate::_RangedIntCustomizationVariable template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::create(const std::string & filename)
{
	return new SharedTangibleObjectTemplate::_RangedIntCustomizationVariable(filename);
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getId(void) const
{
	return _RangedIntCustomizationVariable_tag;
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getId

const std::string & SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getVariableName(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getVariableName(true);
#endif
	}

	if (!m_variableName.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter variableName in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter variableName has not been defined in template %s!", DataResource::getName()));
			return base->getVariableName(versionOk);
		}
	}

	const std::string & value = m_variableName.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getVariableName

int SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getMinValueInclusive(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMinValueInclusive(true);
#endif
	}

	if (!m_minValueInclusive.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minValueInclusive in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minValueInclusive has not been defined in template %s!", DataResource::getName()));
			return base->getMinValueInclusive(versionOk);
		}
	}

	int value = m_minValueInclusive.getValue();
	char delta = m_minValueInclusive.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinValueInclusive(versionOk);
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
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getMinValueInclusive

int SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getMinValueInclusiveMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMinValueInclusiveMin(true);
#endif
	}

	if (!m_minValueInclusive.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minValueInclusive in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minValueInclusive has not been defined in template %s!", DataResource::getName()));
			return base->getMinValueInclusiveMin(versionOk);
		}
	}

	int value = m_minValueInclusive.getMinValue();
	char delta = m_minValueInclusive.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinValueInclusiveMin(versionOk);
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
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getMinValueInclusiveMin

int SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getMinValueInclusiveMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMinValueInclusiveMax(true);
#endif
	}

	if (!m_minValueInclusive.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter minValueInclusive in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter minValueInclusive has not been defined in template %s!", DataResource::getName()));
			return base->getMinValueInclusiveMax(versionOk);
		}
	}

	int value = m_minValueInclusive.getMaxValue();
	char delta = m_minValueInclusive.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMinValueInclusiveMax(versionOk);
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
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getMinValueInclusiveMax

int SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getDefaultValue(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDefaultValue(true);
#endif
	}

	if (!m_defaultValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter defaultValue in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter defaultValue has not been defined in template %s!", DataResource::getName()));
			return base->getDefaultValue(versionOk);
		}
	}

	int value = m_defaultValue.getValue();
	char delta = m_defaultValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDefaultValue(versionOk);
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
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getDefaultValue

int SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getDefaultValueMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDefaultValueMin(true);
#endif
	}

	if (!m_defaultValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter defaultValue in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter defaultValue has not been defined in template %s!", DataResource::getName()));
			return base->getDefaultValueMin(versionOk);
		}
	}

	int value = m_defaultValue.getMinValue();
	char delta = m_defaultValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDefaultValueMin(versionOk);
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
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getDefaultValueMin

int SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getDefaultValueMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDefaultValueMax(true);
#endif
	}

	if (!m_defaultValue.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter defaultValue in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter defaultValue has not been defined in template %s!", DataResource::getName()));
			return base->getDefaultValueMax(versionOk);
		}
	}

	int value = m_defaultValue.getMaxValue();
	char delta = m_defaultValue.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getDefaultValueMax(versionOk);
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
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getDefaultValueMax

int SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getMaxValueExclusive(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxValueExclusive(true);
#endif
	}

	if (!m_maxValueExclusive.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxValueExclusive in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxValueExclusive has not been defined in template %s!", DataResource::getName()));
			return base->getMaxValueExclusive(versionOk);
		}
	}

	int value = m_maxValueExclusive.getValue();
	char delta = m_maxValueExclusive.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxValueExclusive(versionOk);
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
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getMaxValueExclusive

int SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getMaxValueExclusiveMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxValueExclusiveMin(true);
#endif
	}

	if (!m_maxValueExclusive.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxValueExclusive in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxValueExclusive has not been defined in template %s!", DataResource::getName()));
			return base->getMaxValueExclusiveMin(versionOk);
		}
	}

	int value = m_maxValueExclusive.getMinValue();
	char delta = m_maxValueExclusive.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxValueExclusiveMin(versionOk);
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
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getMaxValueExclusiveMin

int SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getMaxValueExclusiveMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedTangibleObjectTemplate::_RangedIntCustomizationVariable *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getMaxValueExclusiveMax(true);
#endif
	}

	if (!m_maxValueExclusive.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter maxValueExclusive in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter maxValueExclusive has not been defined in template %s!", DataResource::getName()));
			return base->getMaxValueExclusiveMax(versionOk);
		}
	}

	int value = m_maxValueExclusive.getMaxValue();
	char delta = m_maxValueExclusive.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getMaxValueExclusiveMax(versionOk);
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
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getMaxValueExclusiveMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::testValues(void) const
{
	IGNORE_RETURN(getVariableName(true));
	IGNORE_RETURN(getMinValueInclusiveMin(true));
	IGNORE_RETURN(getMinValueInclusiveMax(true));
	IGNORE_RETURN(getDefaultValueMin(true));
	IGNORE_RETURN(getDefaultValueMax(true));
	IGNORE_RETURN(getMaxValueExclusiveMin(true));
	IGNORE_RETURN(getMaxValueExclusiveMax(true));
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::load(Iff &file)
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
		if (strcmp(paramName, "variableName") == 0)
			m_variableName.loadFromIff(file);
		else if (strcmp(paramName, "minValueInclusive") == 0)
			m_minValueInclusive.loadFromIff(file);
		else if (strcmp(paramName, "defaultValue") == 0)
			m_defaultValue.loadFromIff(file);
		else if (strcmp(paramName, "maxValueExclusive") == 0)
			m_maxValueExclusive.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::load

//@END TFD
