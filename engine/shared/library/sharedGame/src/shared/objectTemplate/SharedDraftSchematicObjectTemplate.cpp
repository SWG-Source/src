//========================================================================
//
// SharedDraftSchematicObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "SharedDraftSchematicObjectTemplate.h"
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

bool SharedDraftSchematicObjectTemplate::ms_allowDefaultTemplateParams = true;


/**
 * Class constructor.
 */
SharedDraftSchematicObjectTemplate::SharedDraftSchematicObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedIntangibleObjectTemplate(filename)
	,m_slotsLoaded(false)
	,m_slotsAppend(false)
	,m_attributesLoaded(false)
	,m_attributesAppend(false)
	,m_versionOk(true)
//@END TFD INIT
{
}	// SharedDraftSchematicObjectTemplate::SharedDraftSchematicObjectTemplate

/**
 * Class destructor.
 */
SharedDraftSchematicObjectTemplate::~SharedDraftSchematicObjectTemplate()
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
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_attributes.begin(); iter != m_attributes.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_attributes.clear();
	}
//@END TFD CLEANUP
}	// SharedDraftSchematicObjectTemplate::~SharedDraftSchematicObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedDraftSchematicObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedDraftSchematicObjectTemplate_tag, create);
}	// SharedDraftSchematicObjectTemplate::registerMe

/**
 * Creates a SharedDraftSchematicObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedDraftSchematicObjectTemplate::create(const std::string & filename)
{
	return new SharedDraftSchematicObjectTemplate(filename);
}	// SharedDraftSchematicObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedDraftSchematicObjectTemplate::getId(void) const
{
	return SharedDraftSchematicObjectTemplate_tag;
}	// SharedDraftSchematicObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedDraftSchematicObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedDraftSchematicObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedDraftSchematicObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == nullptr)
		return m_templateVersion;
	const SharedDraftSchematicObjectTemplate * base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
	if (base == nullptr)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedDraftSchematicObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
void SharedDraftSchematicObjectTemplate::getSlots(IngredientSlot &data, int index) const
{
	const SharedDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
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
	data.name = param->getName(m_versionOk);
	data.hardpoint = param->getHardpoint(m_versionOk);
}	// SharedDraftSchematicObjectTemplate::getSlots

void SharedDraftSchematicObjectTemplate::getSlotsMin(IngredientSlot &data, int index) const
{
	const SharedDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
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
	data.name = param->getName(m_versionOk);
	data.hardpoint = param->getHardpoint(m_versionOk);
}	// SharedDraftSchematicObjectTemplate::getSlotsMin

void SharedDraftSchematicObjectTemplate::getSlotsMax(IngredientSlot &data, int index) const
{
	const SharedDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
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
	data.name = param->getName(m_versionOk);
	data.hardpoint = param->getHardpoint(m_versionOk);
}	// SharedDraftSchematicObjectTemplate::getSlotsMax

size_t SharedDraftSchematicObjectTemplate::getSlotsCount(void) const
{
	if (!m_slotsLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const SharedDraftSchematicObjectTemplate * base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getSlotsCount();
	}

	size_t count = m_slots.size();

	// if we are extending our base template, add it's count
	if (m_slotsAppend && m_baseData != nullptr)
	{
		const SharedDraftSchematicObjectTemplate * base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getSlotsCount();
	}

	return count;
}	// SharedDraftSchematicObjectTemplate::getSlotsCount

void SharedDraftSchematicObjectTemplate::getAttributes(SchematicAttribute &data, int index) const
{
	const SharedDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_attributesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attributes in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attributes has not been defined in template %s!", DataResource::getName()));
			base->getAttributes(data, index);
			return;
		}
	}

	if (m_attributesAppend && base != nullptr)
	{
		int baseCount = base->getAttributesCount();
		if (index < baseCount)
			{
				base->getAttributes(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_attributes.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_attributes[index]).getValue();
	NOT_NULL(structTemplate);
	const _SchematicAttribute *param = dynamic_cast<const _SchematicAttribute *>(structTemplate);
	NOT_NULL(param);
	data.name = param->getName(m_versionOk);
	data.experiment = param->getExperiment(m_versionOk);
	data.value = param->getValue(m_versionOk);
}	// SharedDraftSchematicObjectTemplate::getAttributes

void SharedDraftSchematicObjectTemplate::getAttributesMin(SchematicAttribute &data, int index) const
{
	const SharedDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_attributesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attributes in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attributes has not been defined in template %s!", DataResource::getName()));
			base->getAttributesMin(data, index);
			return;
		}
	}

	if (m_attributesAppend && base != nullptr)
	{
		int baseCount = base->getAttributesCount();
		if (index < baseCount)
			{
				base->getAttributesMin(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_attributes.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_attributes[index]).getValue();
	NOT_NULL(structTemplate);
	const _SchematicAttribute *param = dynamic_cast<const _SchematicAttribute *>(structTemplate);
	NOT_NULL(param);
	data.name = param->getName(m_versionOk);
	data.experiment = param->getExperiment(m_versionOk);
	data.value = param->getValueMin(m_versionOk);
}	// SharedDraftSchematicObjectTemplate::getAttributesMin

void SharedDraftSchematicObjectTemplate::getAttributesMax(SchematicAttribute &data, int index) const
{
	const SharedDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
	}

	if (!m_attributesLoaded)
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter attributes in template %s", DataResource::getName()));
			return ;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter attributes has not been defined in template %s!", DataResource::getName()));
			base->getAttributesMax(data, index);
			return;
		}
	}

	if (m_attributesAppend && base != nullptr)
	{
		int baseCount = base->getAttributesCount();
		if (index < baseCount)
			{
				base->getAttributesMax(data, index);
				return;
			}
		index -= baseCount;
	}

	DEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_attributes.size(), ("template param index out of range"));
	const ObjectTemplate * structTemplate = (*m_attributes[index]).getValue();
	NOT_NULL(structTemplate);
	const _SchematicAttribute *param = dynamic_cast<const _SchematicAttribute *>(structTemplate);
	NOT_NULL(param);
	data.name = param->getName(m_versionOk);
	data.experiment = param->getExperiment(m_versionOk);
	data.value = param->getValueMax(m_versionOk);
}	// SharedDraftSchematicObjectTemplate::getAttributesMax

size_t SharedDraftSchematicObjectTemplate::getAttributesCount(void) const
{
	if (!m_attributesLoaded)
	{
		if (m_baseData == nullptr)
			return 0;
		const SharedDraftSchematicObjectTemplate * base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
		DEBUG_FATAL(base == nullptr, ("base template wrong type"));
		return base->getAttributesCount();
	}

	size_t count = m_attributes.size();

	// if we are extending our base template, add it's count
	if (m_attributesAppend && m_baseData != nullptr)
	{
		const SharedDraftSchematicObjectTemplate * base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
		if (base != nullptr)
			count += base->getAttributesCount();
	}

	return count;
}	// SharedDraftSchematicObjectTemplate::getAttributesCount

const std::string & SharedDraftSchematicObjectTemplate::getCraftedSharedTemplate(bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedDraftSchematicObjectTemplate * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getCraftedSharedTemplate(true);
#endif
	}

	if (!m_craftedSharedTemplate.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!m_versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter craftedSharedTemplate in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter craftedSharedTemplate has not been defined in template %s!", DataResource::getName()));
			return base->getCraftedSharedTemplate();
		}
	}

	const std::string & value = m_craftedSharedTemplate.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedDraftSchematicObjectTemplate::getCraftedSharedTemplate

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedDraftSchematicObjectTemplate::testValues(void) const
{
	IGNORE_RETURN(getCraftedSharedTemplate(true));
	SharedIntangibleObjectTemplate::testValues();
}	// SharedDraftSchematicObjectTemplate::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedDraftSchematicObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedDraftSchematicObjectTemplate_tag)
	{
		SharedIntangibleObjectTemplate::load(file);
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
	if (getHighestTemplateVersion() != TAG(0,0,0,3))
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
		if (strcmp(paramName, "slots") == 0)
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
		else if (strcmp(paramName, "attributes") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_attributes.begin(); iter != m_attributes.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_attributes.clear();
			m_attributesAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_attributes.push_back(newData);
			}
			m_attributesLoaded = true;
		}
		else if (strcmp(paramName, "craftedSharedTemplate") == 0)
			m_craftedSharedTemplate.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	SharedIntangibleObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedDraftSchematicObjectTemplate::load


//=============================================================================
// class SharedDraftSchematicObjectTemplate::_IngredientSlot

/**
 * Class constructor.
 */
SharedDraftSchematicObjectTemplate::_IngredientSlot::_IngredientSlot(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::_IngredientSlot

/**
 * Class destructor.
 */
SharedDraftSchematicObjectTemplate::_IngredientSlot::~_IngredientSlot()
{
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::~_IngredientSlot

/**
 * Static function used to register this template.
 */
void SharedDraftSchematicObjectTemplate::_IngredientSlot::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_IngredientSlot_tag, create);
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::registerMe

/**
 * Creates a SharedDraftSchematicObjectTemplate::_IngredientSlot template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedDraftSchematicObjectTemplate::_IngredientSlot::create(const std::string & filename)
{
	return new SharedDraftSchematicObjectTemplate::_IngredientSlot(filename);
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedDraftSchematicObjectTemplate::_IngredientSlot::getId(void) const
{
	return _IngredientSlot_tag;
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::getId

const StringId SharedDraftSchematicObjectTemplate::_IngredientSlot::getName(bool versionOk, bool testData) const
{
#ifdef _DEBUG
StringId testDataValue = DefaultStringId;
#else
UNREF(testData);
#endif

	const SharedDraftSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
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
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::getName

const std::string & SharedDraftSchematicObjectTemplate::_IngredientSlot::getHardpoint(bool versionOk, bool testData) const
{
#ifdef _DEBUG
std::string testDataValue = DefaultString;
#else
UNREF(testData);
#endif

	const SharedDraftSchematicObjectTemplate::_IngredientSlot * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate::_IngredientSlot *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getHardpoint(true);
#endif
	}

	if (!m_hardpoint.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter hardpoint in template %s", DataResource::getName()));
			return DefaultString;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter hardpoint has not been defined in template %s!", DataResource::getName()));
			return base->getHardpoint(versionOk);
		}
	}

	const std::string & value = m_hardpoint.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::getHardpoint

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedDraftSchematicObjectTemplate::_IngredientSlot::testValues(void) const
{
	IGNORE_RETURN(getName(true));
	IGNORE_RETURN(getHardpoint(true));
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedDraftSchematicObjectTemplate::_IngredientSlot::load(Iff &file)
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
		if (strcmp(paramName, "name") == 0)
			m_name.loadFromIff(file);
		else if (strcmp(paramName, "hardpoint") == 0)
			m_hardpoint.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::load


//=============================================================================
// class SharedDraftSchematicObjectTemplate::_SchematicAttribute

/**
 * Class constructor.
 */
SharedDraftSchematicObjectTemplate::_SchematicAttribute::_SchematicAttribute(const std::string & filename)
	: ObjectTemplate(filename)
{
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::_SchematicAttribute

/**
 * Class destructor.
 */
SharedDraftSchematicObjectTemplate::_SchematicAttribute::~_SchematicAttribute()
{
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::~_SchematicAttribute

/**
 * Static function used to register this template.
 */
void SharedDraftSchematicObjectTemplate::_SchematicAttribute::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_SchematicAttribute_tag, create);
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::registerMe

/**
 * Creates a SharedDraftSchematicObjectTemplate::_SchematicAttribute template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedDraftSchematicObjectTemplate::_SchematicAttribute::create(const std::string & filename)
{
	return new SharedDraftSchematicObjectTemplate::_SchematicAttribute(filename);
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedDraftSchematicObjectTemplate::_SchematicAttribute::getId(void) const
{
	return _SchematicAttribute_tag;
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::getId

const StringId SharedDraftSchematicObjectTemplate::_SchematicAttribute::getName(bool versionOk, bool testData) const
{
#ifdef _DEBUG
StringId testDataValue = DefaultStringId;
#else
UNREF(testData);
#endif

	const SharedDraftSchematicObjectTemplate::_SchematicAttribute * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate::_SchematicAttribute *>(m_baseData);
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
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::getName

const StringId SharedDraftSchematicObjectTemplate::_SchematicAttribute::getExperiment(bool versionOk, bool testData) const
{
#ifdef _DEBUG
StringId testDataValue = DefaultStringId;
#else
UNREF(testData);
#endif

	const SharedDraftSchematicObjectTemplate::_SchematicAttribute * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate::_SchematicAttribute *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getExperiment(true);
#endif
	}

	if (!m_experiment.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter experiment in template %s", DataResource::getName()));
			return DefaultStringId;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter experiment has not been defined in template %s!", DataResource::getName()));
			return base->getExperiment(versionOk);
		}
	}

	const StringId value = m_experiment.getValue();
#ifdef _DEBUG
	if (testData && base != nullptr)
	{
	}
#endif

	return value;
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::getExperiment

int SharedDraftSchematicObjectTemplate::_SchematicAttribute::getValue(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedDraftSchematicObjectTemplate::_SchematicAttribute * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate::_SchematicAttribute *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValue(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValue(versionOk);
		}
	}

	int value = m_value.getValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValue(versionOk);
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
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::getValue

int SharedDraftSchematicObjectTemplate::_SchematicAttribute::getValueMin(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedDraftSchematicObjectTemplate::_SchematicAttribute * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate::_SchematicAttribute *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValueMin(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValueMin(versionOk);
		}
	}

	int value = m_value.getMinValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValueMin(versionOk);
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
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::getValueMin

int SharedDraftSchematicObjectTemplate::_SchematicAttribute::getValueMax(bool versionOk, bool testData) const
{
#ifdef _DEBUG
int testDataValue = 0;
#else
UNREF(testData);
#endif

	const SharedDraftSchematicObjectTemplate::_SchematicAttribute * base = nullptr;
	if (m_baseData != nullptr)
	{
		base = dynamic_cast<const SharedDraftSchematicObjectTemplate::_SchematicAttribute *>(m_baseData);
#ifdef _DEBUG
		if (testData && base != nullptr)
			testDataValue = base->getValueMax(true);
#endif
	}

	if (!m_value.isLoaded())
	{
		if (ms_allowDefaultTemplateParams && /*!versionOk &&*/ base == nullptr)
		{
			DEBUG_WARNING(true, ("Returning default value for missing parameter value in template %s", DataResource::getName()));
			return 0;
		}
		else
		{
			DEBUG_FATAL(base == nullptr, ("Template parameter value has not been defined in template %s!", DataResource::getName()));
			return base->getValueMax(versionOk);
		}
	}

	int value = m_value.getMaxValue();
	char delta = m_value.getDeltaType();
	if (delta == '+' || delta == '-' || delta == '_' || delta == '=')
	{
		int baseValue = 0;
		if (m_baseData != nullptr)
		{
			if (base != nullptr)
				baseValue = base->getValueMax(versionOk);
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
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::getValueMax

#ifdef _DEBUG
/**
 * Special function used by datalint. Checks for duplicate values in base and derived templates.
 */
void SharedDraftSchematicObjectTemplate::_SchematicAttribute::testValues(void) const
{
	IGNORE_RETURN(getName(true));
	IGNORE_RETURN(getExperiment(true));
	IGNORE_RETURN(getValueMin(true));
	IGNORE_RETURN(getValueMax(true));
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::testValues
#endif

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedDraftSchematicObjectTemplate::_SchematicAttribute::load(Iff &file)
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
		if (strcmp(paramName, "name") == 0)
			m_name.loadFromIff(file);
		else if (strcmp(paramName, "experiment") == 0)
			m_experiment.loadFromIff(file);
		else if (strcmp(paramName, "value") == 0)
			m_value.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::load

//@END TFD

//----------------------------------------------------------------------

const StringId SharedDraftSchematicObjectTemplate::getCraftedName                 () const
{
	StringId sid = getObjectName ();

	if (sid.isInvalid ())
	{
		const SharedObjectTemplate * const sot = fetchCraftedSharedObjectTemplate ();
		if (sot)
		{
			sid = sot->getObjectName ();
			sot->releaseReference ();
		}
	}

	return sid;
}

//----------------------------------------------------------------------

const StringId SharedDraftSchematicObjectTemplate::getCraftedDetailedDescription  () const
{
	StringId sid = getDetailedDescription ();

	if (sid.isInvalid ())
	{
		const SharedObjectTemplate * const sot = fetchCraftedSharedObjectTemplate ();
		if (sot)
		{
			sid = sot->getDetailedDescription ();
			sot->releaseReference ();
		}
	}

	return sid;
}

//----------------------------------------------------------------------

const StringId SharedDraftSchematicObjectTemplate::getCraftedLookAtText           () const
{
	StringId sid = getLookAtText  ();

	if (sid.isInvalid ())
	{
		const SharedObjectTemplate * const sot = fetchCraftedSharedObjectTemplate ();
		if (sot)
		{
			sid = sot->getLookAtText ();
			sot->releaseReference ();
		}
	}

	return sid;
}

//----------------------------------------------------------------------

const std::string SharedDraftSchematicObjectTemplate::getCraftedAppearanceFilename   () const
{
	const std::string & app = getAppearanceFilename ();

	if (app.empty ())
	{
		const SharedObjectTemplate * const sot = fetchCraftedSharedObjectTemplate ();
		if (sot)
		{
			const std::string capp = sot->getAppearanceFilename ();
			sot->releaseReference ();
			return capp;
		}
	}

	return app;
}

//----------------------------------------------------------------------

const SharedObjectTemplate * SharedDraftSchematicObjectTemplate::fetchCraftedSharedObjectTemplate () const
{
	const std::string & sotName = getCraftedSharedTemplate ();
	const SharedObjectTemplate * const sot = safe_cast<const SharedObjectTemplate *>(ObjectTemplateList::fetch (sotName.c_str ()));

	if (!sot)
	{
		const char * const draftName = this->DataResource::getName ();
		WARNING (true, ("SharedDraftSchematicObjectTemplate [%s] could not load craftedSharedTemplate [%s]", draftName ? draftName : "", sotName.c_str ()));
	}

	return sot;
}

//----------------------------------------------------------------------

