//========================================================================
//
// ServerDraftSchematicObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "ServerDraftSchematicObjectTemplate.h"
#include "serverGame/DraftSchematicObject.h"
#include "sharedFile/Iff.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
//@BEGIN TFD TEMPLATE REFS
#include "ServerFactoryObjectTemplate.h"
#include "ServerObjectTemplate.h"
//@END TFD TEMPLATE REFS
#include <stdio.h>

const std::string DefaultString("");
const StringId DefaultStringId("", 0);
const Vector DefaultVector(0,0,0);
const TriggerVolumeData DefaultTriggerVolumeData;

bool ServerDraftSchematicObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
ServerDraftSchematicObjectTemplate::ServerDraftSchematicObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: ServerIntangibleObjectTemplate(filename)
	,m_slotsLoaded(false)
	,m_slotsAppend(false)
	,m_skillCommandsLoaded(false)
	,m_skillCommandsAppend(false)
	,m_manufactureScriptsLoaded(false)
	,m_manufactureScriptsAppend(false)
	,m_versionOk(true)
//@END TFD INIT
{
}	// ServerDraftSchematicObjectTemplate::ServerDraftSchematicObjectTemplate

/**
 * Class destructor.
 */
ServerDraftSchematicObjectTemplate::~ServerDraftSchematicObjectTemplate()
{
//@BEGIN TFD CLEANUP
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_slots.begin(); iter != m_slots.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_slots.clear();
	}
	{
		std::vector<StringParam *>::iterator iter;
		for (iter = m_skillCommands.begin(); iter != m_skillCommands.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_skillCommands.clear();
	}
	{
		std::vector<StringParam *>::iterator iter;
		for (iter = m_manufactureScripts.begin(); iter != m_manufactureScripts.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_manufactureScripts.clear();
	}
//@END TFD CLEANUP
}	// ServerDraftSchematicObjectTemplate::~ServerDraftSchematicObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerDraftSchematicObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerDraftSchematicObjectTemplate_tag, create);
}	// ServerDraftSchematicObjectTemplate::registerMe

/**
 * Creates a ServerDraftSchematicObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerDraftSchematicObjectTemplate::create(const std::string & filename)
{
	return new ServerDraftSchematicObjectTemplate(filename);
}	// ServerDraftSchematicObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerDraftSchematicObjectTemplate::getId(void) const
{
	return ServerDraftSchematicObjectTemplate_tag;
}	// ServerDraftSchematicObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerDraftSchematicObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerDraftSchematicObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerDraftSchematicObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerDraftSchematicObjectTemplate * base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerDraftSchematicObjectTemplate::getHighestTemplateVersion

/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ServerDraftSchematicObjectTemplate::createObject(void) const
{
#ifdef _DEBUG
	if (DataLint::isEnabled())
	{
		return new DraftSchematicObject(this);
	}
#endif

	WARNING(true, ("Trying to create a draft schematic %s outside the draft "
		"schematic factory!\n", getName()));
	return NULL;
}	// ServerDraftSchematicObjectTemplate::createObject

/**
 * Called after the template data has been loaded. Verifies that the schematic has 
 * good data.
 */
void ServerDraftSchematicObjectTemplate::postLoad()
{
#ifdef _DEBUG
	// verify that all slot names are unique
	std::set<std::string> slotNames;

	IngredientSlot data;
	int count = getSlotsCount();
	for (int i = 0; i < count; ++i)
	{
		getSlots(data, i);
		if (slotNames.insert(data.name.getText()).second == false)
		{
			WARNING(true, ("Draft schematic %s has duplicate slot name %s",
				getName(), data.name.getText().c_str()));
		}
	}
#endif
}	// ServerDraftSchematicObjectTemplate::postLoad

//@BEGIN TFD
ServerDraftSchematicObjectTemplate::CraftingType ServerDraftSchematicObjectTemplate::getCategory(bool testData) const
{
#ifdef _DEBUG
ServerDraftSchematicObjectTemplate::CraftingType testDataValue = static_cast<ServerDraftSchematicObjectTemplate::CraftingType>(0);
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCategory(true);
#endif
	}

	if (!m_category.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter category in template %s", DataResource::getName()));
			return static_cast<CraftingType>(0);
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter category has not been defined in template %s!", DataResource::getName()));
			return base->getCategory();
		}
	}

	CraftingType value = static_cast<CraftingType>(m_category.getValue());
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerDraftSchematicObjectTemplate::getCategory

const ServerObjectTemplate * ServerDraftSchematicObjectTemplate::getCraftedObjectTemplate() const
{
	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_craftedObjectTemplate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter craftedObjectTemplate in template %s", DataResource::getName()));
			return nullptr;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter craftedObjectTemplate has not been defined in template %s!", DataResource::getName()));
			return base->getCraftedObjectTemplate();
		}
	}

	const ServerObjectTemplate * returnValue = nullptr;
	const std::string & templateName = m_craftedObjectTemplate.getValue();
	if (!templateName.empty())
	{
		returnValue = dynamic_cast<const ServerObjectTemplate *>(ObjectTemplateList::fetch(templateName));
		if (returnValue == nullptr)
			WARNING_STRICT_FATAL(true, ("Error loading template %s",templateName.c_str()));
	}
	return returnValue;
}	// ServerDraftSchematicObjectTemplate::getCraftedObjectTemplate

const ServerFactoryObjectTemplate * ServerDraftSchematicObjectTemplate::getCrateObjectTemplate() const
{
	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_crateObjectTemplate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter crateObjectTemplate in template %s", DataResource::getName()));
			return nullptr;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter crateObjectTemplate has not been defined in template %s!", DataResource::getName()));
			return base->getCrateObjectTemplate();
		}
	}

	const ServerFactoryObjectTemplate * returnValue = nullptr;
	const std::string & templateName = m_crateObjectTemplate.getValue();
	if (!templateName.empty())
	{
		returnValue = dynamic_cast<const ServerFactoryObjectTemplate *>(ObjectTemplateList::fetch(templateName));
		if (returnValue == nullptr)
			WARNING_STRICT_FATAL(true, ("Error loading template %s",templateName.c_str()));
	}
	return returnValue;
}	// ServerDraftSchematicObjectTemplate::getCrateObjectTemplate

void ServerDraftSchematicObjectTemplate::getSlots(IngredientSlot &data, int index) const
{
	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_slotsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slots in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slots has not been defined in template %s!", DataResource::getName()));
			base->getSlots(data, index);
			return;
		}
	}

	if (m_slotsAppend && base != nullptr)
	{
		int baseCount = base->getSlotsCount();
		if (index < baseCount)
			{
				base->getSlots(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_slots.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_slots[index]).getValue();
	NOT_NULL(structTemplate);
	const _IngredientSlot *param = dynamic_cast<const _IngredientSlot *>(structTemplate);
	NOT_NULL(param);
	data.optional = param->getOptional(m_versionOk);
	data.name = param->getName(m_versionOk);
	{
		for (size_t i = 0; i < param->getOptionsCount(); ++i)
		{
			data.options.push_back(Ingredient());
			param->getOptions(data.options.back(), i, m_versionOk);
		}
	}
	data.optionalSkillCommand = param->getOptionalSkillCommand(m_versionOk);
	data.complexity = param->getComplexity(m_versionOk);
	data.appearance = param->getAppearance(m_versionOk);
}	// ServerDraftSchematicObjectTemplate::getSlots

void ServerDraftSchematicObjectTemplate::getSlotsMin(IngredientSlot &data, int index) const
{
	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_slotsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slots in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slots has not been defined in template %s!", DataResource::getName()));
			base->getSlotsMin(data, index);
			return;
		}
	}

	if (m_slotsAppend && base != nullptr)
	{
		int baseCount = base->getSlotsCount();
		if (index < baseCount)
			{
				base->getSlotsMin(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_slots.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_slots[index]).getValue();
	NOT_NULL(structTemplate);
	const _IngredientSlot *param = dynamic_cast<const _IngredientSlot *>(structTemplate);
	NOT_NULL(param);
	data.optional = param->getOptional(m_versionOk);
	data.name = param->getName(m_versionOk);
	{
		for (size_t i = 0; i < param->getOptionsCount(); ++i)
		{
			data.options.push_back(Ingredient());
			param->getOptionsMin(data.options.back(), i, m_versionOk);
		}
	}
	data.optionalSkillCommand = param->getOptionalSkillCommand(m_versionOk);
	data.complexity = param->getComplexityMin(m_versionOk);
	data.appearance = param->getAppearance(m_versionOk);
}	// ServerDraftSchematicObjectTemplate::getSlotsMin

void ServerDraftSchematicObjectTemplate::getSlotsMax(IngredientSlot &data, int index) const
{
	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_slotsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter slots in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter slots has not been defined in template %s!", DataResource::getName()));
			base->getSlotsMax(data, index);
			return;
		}
	}

	if (m_slotsAppend && base != nullptr)
	{
		int baseCount = base->getSlotsCount();
		if (index < baseCount)
			{
				base->getSlotsMax(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_slots.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_slots[index]).getValue();
	NOT_NULL(structTemplate);
	const _IngredientSlot *param = dynamic_cast<const _IngredientSlot *>(structTemplate);
	NOT_NULL(param);
	data.optional = param->getOptional(m_versionOk);
	data.name = param->getName(m_versionOk);
	{
		for (size_t i = 0; i < param->getOptionsCount(); ++i)
		{
			data.options.push_back(Ingredient());
			param->getOptionsMax(data.options.back(), i, m_versionOk);
		}
	}
	data.optionalSkillCommand = param->getOptionalSkillCommand(m_versionOk);
	data.complexity = param->getComplexityMax(m_versionOk);
	data.appearance = param->getAppearance(m_versionOk);
}	// ServerDraftSchematicObjectTemplate::getSlotsMax

size_t ServerDraftSchematicObjectTemplate::getSlotsCount(void) const
{
	if (!m_slotsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerDraftSchematicObjectTemplate * base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getSlotsCount();
	}

	size_t count = m_slots.size();

	// if we are extending our base template, add it's count
	if (m_slotsAppend && m_baseData != nullptr)
	{
		const ServerDraftSchematicObjectTemplate * base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getSlotsCount();
	}

	return count;
}	// ServerDraftSchematicObjectTemplate::getSlotsCount

const std::string & ServerDraftSchematicObjectTemplate::getSkillCommands(int index) const
{
	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_skillCommandsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter skillCommands in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter skillCommands has not been defined in template %s!", DataResource::getName()));
			return base->getSkillCommands(index);
		}
	}

	if (m_skillCommandsAppend && base != nullptr)
	{
		int baseCount = base->getSkillCommandsCount();
		if (index < baseCount)
			return base->getSkillCommands(index);
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_skillCommands.size(), ("template param index out of range"));
	const std::string & value = m_skillCommands[index]->getValue();
	return value;
}	// ServerDraftSchematicObjectTemplate::getSkillCommands

size_t ServerDraftSchematicObjectTemplate::getSkillCommandsCount(void) const
{
	if (!m_skillCommandsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerDraftSchematicObjectTemplate * base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getSkillCommandsCount();
	}

	size_t count = m_skillCommands.size();

	// if we are extending our base template, add it's count
	if (m_skillCommandsAppend && m_baseData != nullptr)
	{
		const ServerDraftSchematicObjectTemplate * base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getSkillCommandsCount();
	}

	return count;
}	// ServerDraftSchematicObjectTemplate::getSkillCommandsCount

bool ServerDraftSchematicObjectTemplate::getDestroyIngredients(bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getDestroyIngredients(true);
#endif
	}

	if (!m_destroyIngredients.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter destroyIngredients in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter destroyIngredients has not been defined in template %s!", DataResource::getName()));
			return base->getDestroyIngredients();
		}
	}

	bool value = m_destroyIngredients.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerDraftSchematicObjectTemplate::getDestroyIngredients

const std::string & ServerDraftSchematicObjectTemplate::getManufactureScripts(int index) const
{
	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_manufactureScriptsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter manufactureScripts in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter manufactureScripts has not been defined in template %s!", DataResource::getName()));
			return base->getManufactureScripts(index);
		}
	}

	if (m_manufactureScriptsAppend && base != nullptr)
	{
		int baseCount = base->getManufactureScriptsCount();
		if (index < baseCount)
			return base->getManufactureScripts(index);
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_manufactureScripts.size(), ("template param index out of range"));
	const std::string & value = m_manufactureScripts[index]->getValue();
	return value;
}	// ServerDraftSchematicObjectTemplate::getManufactureScripts

size_t ServerDraftSchematicObjectTemplate::getManufactureScriptsCount(void) const
{
	if (!m_manufactureScriptsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerDraftSchematicObjectTemplate * base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getManufactureScriptsCount();
	}

	size_t count = m_manufactureScripts.size();

	// if we are extending our base template, add it's count
	if (m_manufactureScriptsAppend && m_baseData != nullptr)
	{
		const ServerDraftSchematicObjectTemplate * base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getManufactureScriptsCount();
	}

	return count;
}	// ServerDraftSchematicObjectTemplate::getManufactureScriptsCount

int ServerDraftSchematicObjectTemplate::getItemsPerContainer(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getItemsPerContainer(true);
#endif
	}

	if (!m_itemsPerContainer.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter itemsPerContainer in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter itemsPerContainer has not been defined in template %s!", DataResource::getName()));
			return base->getItemsPerContainer();
		}
	}

	int value = m_itemsPerContainer.getValue();
	char delta = m_itemsPerContainer.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getItemsPerContainer();
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
}	// ServerDraftSchematicObjectTemplate::getItemsPerContainer

int ServerDraftSchematicObjectTemplate::getItemsPerContainerMin(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getItemsPerContainerMin(true);
#endif
	}

	if (!m_itemsPerContainer.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter itemsPerContainer in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter itemsPerContainer has not been defined in template %s!", DataResource::getName()));
			return base->getItemsPerContainerMin();
		}
	}

	int value = m_itemsPerContainer.getMinValue();
	char delta = m_itemsPerContainer.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getItemsPerContainerMin();
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
}	// ServerDraftSchematicObjectTemplate::getItemsPerContainerMin

int ServerDraftSchematicObjectTemplate::getItemsPerContainerMax(bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getItemsPerContainerMax(true);
#endif
	}

	if (!m_itemsPerContainer.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter itemsPerContainer in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter itemsPerContainer has not been defined in template %s!", DataResource::getName()));
			return base->getItemsPerContainerMax();
		}
	}

	int value = m_itemsPerContainer.getMaxValue();
	char delta = m_itemsPerContainer.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getItemsPerContainerMax();
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
}	// ServerDraftSchematicObjectTemplate::getItemsPerContainerMax

float ServerDraftSchematicObjectTemplate::getManufactureTime(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getManufactureTime(true);
#endif
	}

	if (!m_manufactureTime.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter manufactureTime in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter manufactureTime has not been defined in template %s!", DataResource::getName()));
			return base->getManufactureTime();
		}
	}

	float value = m_manufactureTime.getValue();
	char delta = m_manufactureTime.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getManufactureTime();
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
}	// ServerDraftSchematicObjectTemplate::getManufactureTime

float ServerDraftSchematicObjectTemplate::getManufactureTimeMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getManufactureTimeMin(true);
#endif
	}

	if (!m_manufactureTime.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter manufactureTime in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter manufactureTime has not been defined in template %s!", DataResource::getName()));
			return base->getManufactureTimeMin();
		}
	}

	float value = m_manufactureTime.getMinValue();
	char delta = m_manufactureTime.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getManufactureTimeMin();
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
}	// ServerDraftSchematicObjectTemplate::getManufactureTimeMin

float ServerDraftSchematicObjectTemplate::getManufactureTimeMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getManufactureTimeMax(true);
#endif
	}

	if (!m_manufactureTime.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter manufactureTime in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter manufactureTime has not been defined in template %s!", DataResource::getName()));
			return base->getManufactureTimeMax();
		}
	}

	float value = m_manufactureTime.getMaxValue();
	char delta = m_manufactureTime.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getManufactureTimeMax();
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
}	// ServerDraftSchematicObjectTemplate::getManufactureTimeMax

float ServerDraftSchematicObjectTemplate::getPrototypeTime(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getPrototypeTime(true);
#endif
	}

	if (!m_prototypeTime.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter prototypeTime in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter prototypeTime has not been defined in template %s!", DataResource::getName()));
			return base->getPrototypeTime();
		}
	}

	float value = m_prototypeTime.getValue();
	char delta = m_prototypeTime.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getPrototypeTime();
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
}	// ServerDraftSchematicObjectTemplate::getPrototypeTime

float ServerDraftSchematicObjectTemplate::getPrototypeTimeMin(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getPrototypeTimeMin(true);
#endif
	}

	if (!m_prototypeTime.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter prototypeTime in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter prototypeTime has not been defined in template %s!", DataResource::getName()));
			return base->getPrototypeTimeMin();
		}
	}

	float value = m_prototypeTime.getMinValue();
	char delta = m_prototypeTime.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getPrototypeTimeMin();
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
}	// ServerDraftSchematicObjectTemplate::getPrototypeTimeMin

float ServerDraftSchematicObjectTemplate::getPrototypeTimeMax(bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getPrototypeTimeMax(true);
#endif
	}

	if (!m_prototypeTime.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter prototypeTime in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter prototypeTime has not been defined in template %s!", DataResource::getName()));
			return base->getPrototypeTimeMax();
		}
	}

	float value = m_prototypeTime.getMaxValue();
	char delta = m_prototypeTime.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getPrototypeTimeMax();
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
}	// ServerDraftSchematicObjectTemplate::getPrototypeTimeMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerDraftSchematicObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getCategory(true));
	IGNORE_RETURN(getDestroyIngredients(true));
	IGNORE_RETURN(getItemsPerContainerMin(true));
	IGNORE_RETURN(getItemsPerContainerMax(true));
	IGNORE_RETURN(getManufactureTimeMin(true));
	IGNORE_RETURN(getManufactureTimeMax(true));
	IGNORE_RETURN(getPrototypeTimeMin(true));
	IGNORE_RETURN(getPrototypeTimeMax(true));
	ServerIntangibleObjectTemplate::testValues();
}	// ServerDraftSchematicObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerDraftSchematicObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerDraftSchematicObjectTemplate_tag)
	{
		ServerIntangibleObjectTemplate::load(file);
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
	if (getHighestTemplateVersion() != TAG(0,0,0,7))
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
		if (strcmp(paramName, "category") == 0)
			m_category.loadFromIff(file);
		else if (strcmp(paramName, "craftedObjectTemplate") == 0)
			m_craftedObjectTemplate.loadFromIff(file);
		else if (strcmp(paramName, "crateObjectTemplate") == 0)
			m_crateObjectTemplate.loadFromIff(file);
		else if (strcmp(paramName, "slots") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_slots.begin(); iter != m_slots.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_slots.clear();
			m_slotsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_slots.push_back(newData);
			}
			m_slotsLoaded = true;
		}
		else if (strcmp(paramName, "skillCommands") == 0)
		{
			std::vector<StringParam *>::iterator iter;
			for (iter = m_skillCommands.begin(); iter != m_skillCommands.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_skillCommands.clear();
			m_skillCommandsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringParam * newData = new StringParam;
				newData->loadFromIff(file);
				m_skillCommands.push_back(newData);
			}
			m_skillCommandsLoaded = true;
		}
		else if (strcmp(paramName, "destroyIngredients") == 0)
			m_destroyIngredients.loadFromIff(file);
		else if (strcmp(paramName, "manufactureScripts") == 0)
		{
			std::vector<StringParam *>::iterator iter;
			for (iter = m_manufactureScripts.begin(); iter != m_manufactureScripts.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_manufactureScripts.clear();
			m_manufactureScriptsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringParam * newData = new StringParam;
				newData->loadFromIff(file);
				m_manufactureScripts.push_back(newData);
			}
			m_manufactureScriptsLoaded = true;
		}
		else if (strcmp(paramName, "itemsPerContainer") == 0)
			m_itemsPerContainer.loadFromIff(file);
		else if (strcmp(paramName, "manufactureTime") == 0)
			m_manufactureTime.loadFromIff(file);
		else if (strcmp(paramName, "prototypeTime") == 0)
			m_prototypeTime.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	ServerIntangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// ServerDraftSchematicObjectTemplate::load


//=============================================================================
// class ServerDraftSchematicObjectTemplate::_IngredientSlot

/**
 * Class constructor.
 */
ServerDraftSchematicObjectTemplate::_IngredientSlot::_IngredientSlot(const std::string & filename)
	: ObjectTemplate(filename)
	,m_optionsLoaded(false)
	,m_optionsAppend(false)
{
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::_IngredientSlot

/**
 * Class destructor.
 */
ServerDraftSchematicObjectTemplate::_IngredientSlot::~_IngredientSlot()
{
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_options.begin(); iter != m_options.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_options.clear();
	}
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::~_IngredientSlot

/**
 * Static function used to register this template.
 */
void ServerDraftSchematicObjectTemplate::_IngredientSlot::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_IngredientSlot_tag, create);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::registerMe

/**
 * Creates a ServerDraftSchematicObjectTemplate::_IngredientSlot template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerDraftSchematicObjectTemplate::_IngredientSlot::create(const std::string & filename)
{
	return new ServerDraftSchematicObjectTemplate::_IngredientSlot(filename);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerDraftSchematicObjectTemplate::_IngredientSlot::getId(void) const
{
	return _IngredientSlot_tag;
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getId

bool ServerDraftSchematicObjectTemplate::_IngredientSlot::getOptional(bool versionOk, bool testData) const
{
#ifdef _DEBUG
bool testDataValue = false;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getOptional(true);
#endif
	}

	if (!m_optional.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter optional in template %s", DataResource::getName()));
			return false;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter optional has not been defined in template %s!", DataResource::getName()));
			return base->getOptional(versionOk);
		}
	}

	bool value = m_optional.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getOptional

const StringId ServerDraftSchematicObjectTemplate::_IngredientSlot::getName(bool versionOk, bool testData) const
{
#ifdef _DEBUG
StringId testDataValue = DefaultStringId;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getName(true);
#endif
	}

	if (!m_name.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter name in template %s", DataResource::getName()));
			return DefaultStringId;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter name has not been defined in template %s!", DataResource::getName()));
			return base->getName(versionOk);
		}
	}

	const StringId value = m_name.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getName

void ServerDraftSchematicObjectTemplate::_IngredientSlot::getOptions(Ingredient &data, int index, bool versionOk) const
{
	const ServerDraftSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
	}

	if (!m_optionsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter options in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter options has not been defined in template %s!", DataResource::getName()));
			base->getOptions(data, index, versionOk);
			return;
		}
	}

	if (m_optionsAppend && base != nullptr)
	{
		int baseCount = base->getOptionsCount();
		if (index < baseCount)
			{
				base->getOptions(data, index, versionOk);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_options.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_options[index]).getValue();
	NOT_NULL(structTemplate);
	const _Ingredient *param = dynamic_cast<const _Ingredient *>(structTemplate);
	NOT_NULL(param);
	data.ingredientType = param->getIngredientType(versionOk);
	{
		for (size_t i = 0; i < param->getIngredientsCount(); ++i)
		{
			data.ingredients.push_back(SimpleIngredient());
			param->getIngredients(data.ingredients.back(), i, versionOk);
		}
	}
	data.complexity = param->getComplexity(versionOk);
	data.skillCommand = param->getSkillCommand(versionOk);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getOptions

void ServerDraftSchematicObjectTemplate::_IngredientSlot::getOptionsMin(Ingredient &data, int index, bool versionOk) const
{
	const ServerDraftSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
	}

	if (!m_optionsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter options in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter options has not been defined in template %s!", DataResource::getName()));
			base->getOptionsMin(data, index, versionOk);
			return;
		}
	}

	if (m_optionsAppend && base != nullptr)
	{
		int baseCount = base->getOptionsCount();
		if (index < baseCount)
			{
				base->getOptionsMin(data, index, versionOk);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_options.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_options[index]).getValue();
	NOT_NULL(structTemplate);
	const _Ingredient *param = dynamic_cast<const _Ingredient *>(structTemplate);
	NOT_NULL(param);
	data.ingredientType = param->getIngredientType(versionOk);
	{
		for (size_t i = 0; i < param->getIngredientsCount(); ++i)
		{
			data.ingredients.push_back(SimpleIngredient());
			param->getIngredientsMin(data.ingredients.back(), i, versionOk);
		}
	}
	data.complexity = param->getComplexityMin(versionOk);
	data.skillCommand = param->getSkillCommand(versionOk);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getOptionsMin

void ServerDraftSchematicObjectTemplate::_IngredientSlot::getOptionsMax(Ingredient &data, int index, bool versionOk) const
{
	const ServerDraftSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
	}

	if (!m_optionsLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter options in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter options has not been defined in template %s!", DataResource::getName()));
			base->getOptionsMax(data, index, versionOk);
			return;
		}
	}

	if (m_optionsAppend && base != nullptr)
	{
		int baseCount = base->getOptionsCount();
		if (index < baseCount)
			{
				base->getOptionsMax(data, index, versionOk);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_options.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_options[index]).getValue();
	NOT_NULL(structTemplate);
	const _Ingredient *param = dynamic_cast<const _Ingredient *>(structTemplate);
	NOT_NULL(param);
	data.ingredientType = param->getIngredientType(versionOk);
	{
		for (size_t i = 0; i < param->getIngredientsCount(); ++i)
		{
			data.ingredients.push_back(SimpleIngredient());
			param->getIngredientsMax(data.ingredients.back(), i, versionOk);
		}
	}
	data.complexity = param->getComplexityMax(versionOk);
	data.skillCommand = param->getSkillCommand(versionOk);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getOptionsMax

size_t ServerDraftSchematicObjectTemplate::_IngredientSlot::getOptionsCount(void) const
{
	if (!m_optionsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const ServerDraftSchematicObjectTemplate::_IngredientSlot * base = dynamic_cast<const ServerDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getOptionsCount();
	}

	size_t count = m_options.size();

	// if we are extending our base template, add it's count
	if (m_optionsAppend && m_baseData != nullptr)
	{
		const ServerDraftSchematicObjectTemplate::_IngredientSlot * base = dynamic_cast<const ServerDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
		if (base != nullptr)
			count += base->getOptionsCount();
	}

	return count;
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getOptionsCount

const std::string & ServerDraftSchematicObjectTemplate::_IngredientSlot::getOptionalSkillCommand(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getOptionalSkillCommand(true);
#endif
	}

	if (!m_optionalSkillCommand.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter optionalSkillCommand in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter optionalSkillCommand has not been defined in template %s!", DataResource::getName()));
			return base->getOptionalSkillCommand(versionOk);
		}
	}

	const std::string & value = m_optionalSkillCommand.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getOptionalSkillCommand

float ServerDraftSchematicObjectTemplate::_IngredientSlot::getComplexity(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getComplexity(true);
#endif
	}

	if (!m_complexity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter complexity in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter complexity has not been defined in template %s!", DataResource::getName()));
			return base->getComplexity(versionOk);
		}
	}

	float value = m_complexity.getValue();
	char delta = m_complexity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getComplexity(versionOk);
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
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getComplexity

float ServerDraftSchematicObjectTemplate::_IngredientSlot::getComplexityMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getComplexityMin(true);
#endif
	}

	if (!m_complexity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter complexity in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter complexity has not been defined in template %s!", DataResource::getName()));
			return base->getComplexityMin(versionOk);
		}
	}

	float value = m_complexity.getMinValue();
	char delta = m_complexity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getComplexityMin(versionOk);
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
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getComplexityMin

float ServerDraftSchematicObjectTemplate::_IngredientSlot::getComplexityMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
float testDataValue = 0.0f;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getComplexityMax(true);
#endif
	}

	if (!m_complexity.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter complexity in template %s", DataResource::getName()));
			return 0.0f;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter complexity has not been defined in template %s!", DataResource::getName()));
			return base->getComplexityMax(versionOk);
		}
	}

	float value = m_complexity.getMaxValue();
	char delta = m_complexity.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		float baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getComplexityMax(versionOk);
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
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getComplexityMax

const std::string & ServerDraftSchematicObjectTemplate::_IngredientSlot::getAppearance(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const ServerDraftSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const ServerDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getAppearance(true);
#endif
	}

	if (!m_appearance.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter appearance in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter appearance has not been defined in template %s!", DataResource::getName()));
			return base->getAppearance(versionOk);
		}
	}

	const std::string & value = m_appearance.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::getAppearance

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void ServerDraftSchematicObjectTemplate::_IngredientSlot::testValues(void) const
{
	IGNORE_RETURN(getOptional(true));
	IGNORE_RETURN(getName(true));
	IGNORE_RETURN(getOptionalSkillCommand(true));
	IGNORE_RETURN(getComplexityMin(true));
	IGNORE_RETURN(getComplexityMax(true));
	IGNORE_RETURN(getAppearance(true));
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerDraftSchematicObjectTemplate::_IngredientSlot::load(Iff &file)
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
		if (strcmp(paramName, "optional") == 0)
			m_optional.loadFromIff(file);
		else if (strcmp(paramName, "name") == 0)
			m_name.loadFromIff(file);
		else if (strcmp(paramName, "options") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_options.begin(); iter != m_options.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_options.clear();
			m_optionsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_options.push_back(newData);
			}
			m_optionsLoaded = true;
		}
		else if (strcmp(paramName, "optionalSkillCommand") == 0)
			m_optionalSkillCommand.loadFromIff(file);
		else if (strcmp(paramName, "complexity") == 0)
			m_complexity.loadFromIff(file);
		else if (strcmp(paramName, "appearance") == 0)
			m_appearance.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerDraftSchematicObjectTemplate::_IngredientSlot::load

//@END TFD
