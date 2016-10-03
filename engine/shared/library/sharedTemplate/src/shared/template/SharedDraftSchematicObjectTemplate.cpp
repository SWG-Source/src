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

#include "sharedTemplate/FirstSharedTemplate.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TemplateGlobals.h"
#include "SharedDraftSchematicObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



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
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedDraftSchematicObjectTemplate * base = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedDraftSchematicObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedDraftSchematicObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::getCompilerIntegerParam

FloatParam * SharedDraftSchematicObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::getFloatParam

BoolParam * SharedDraftSchematicObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::getBoolParam

StringParam * SharedDraftSchematicObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "craftedSharedTemplate") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_craftedSharedTemplate;
		}
		fprintf(stderr, "trying to access single-parameter \"craftedSharedTemplate\" as an array\n");
	}
	else
		return SharedIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//SharedDraftSchematicObjectTemplate::getStringParam

StringIdParam * SharedDraftSchematicObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::getStringIdParam

VectorParam * SharedDraftSchematicObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::getVectorParam

DynamicVariableParam * SharedDraftSchematicObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::getDynamicVariableParam

StructParamOT * SharedDraftSchematicObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "slots") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_slots.size()))
			return m_slots[index];
		if (index == static_cast<int>(m_slots.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_slots.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"slots\" out of bounds\n");
	}
	else if (strcmp(name, "attributes") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_attributes.size()))
			return m_attributes[index];
		if (index == static_cast<int>(m_attributes.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_attributes.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"attributes\" out of bounds\n");
	}
	else
		return SharedIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
	return nullptr;
}	//SharedDraftSchematicObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedDraftSchematicObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::getTriggerVolumeParam

void SharedDraftSchematicObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	if (strcmp(name, "slots") == 0)
		param.setValue(new _IngredientSlot(""));
	else if (strcmp(name, "attributes") == 0)
		param.setValue(new _SchematicAttribute(""));
	else
		SharedIntangibleObjectTemplate::initStructParamOT(param, name);
}	// SharedDraftSchematicObjectTemplate::initStructParamOT

void SharedDraftSchematicObjectTemplate::setAsEmptyList(const char *name)
{
	if (strcmp(name, "slots") == 0)
	{
		m_slots.clear();
		m_slotsLoaded = true;
	}
	else if (strcmp(name, "attributes") == 0)
	{
		m_attributes.clear();
		m_attributesLoaded = true;
	}
	else
		SharedIntangibleObjectTemplate::setAsEmptyList(name);
}	// SharedDraftSchematicObjectTemplate::setAsEmptyList

void SharedDraftSchematicObjectTemplate::setAppend(const char *name)
{
	if (strcmp(name, "slots") == 0)
		m_slotsAppend = true;
	else if (strcmp(name, "attributes") == 0)
		m_attributesAppend = true;
	else
		SharedIntangibleObjectTemplate::setAppend(name);
}	// SharedDraftSchematicObjectTemplate::setAppend

bool SharedDraftSchematicObjectTemplate::isAppend(const char *name) const
{
	if (strcmp(name, "slots") == 0)
		return m_slotsAppend;
	else if (strcmp(name, "attributes") == 0)
		return m_attributesAppend;
	else
		return SharedIntangibleObjectTemplate::isAppend(name);
}	// SharedDraftSchematicObjectTemplate::isAppend


int SharedDraftSchematicObjectTemplate::getListLength(const char *name) const
{
	if (strcmp(name, "slots") == 0)
	{
		return m_slots.size();
	}
	else if (strcmp(name, "attributes") == 0)
	{
		return m_attributes.size();
	}
	else
		return SharedIntangibleObjectTemplate::getListLength(name);
}	// SharedDraftSchematicObjectTemplate::getListLength

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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedDraftSchematicObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedDraftSchematicObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,0,3));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	if (!m_slotsLoaded)
	{
		// mark the list as empty and extending the base list
		m_slotsAppend = true;
	}
	// save slots
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("slots");
	file.insertChunkData(&m_slotsAppend, sizeof(bool));
	count = m_slots.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_slots[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_attributesLoaded)
	{
		// mark the list as empty and extending the base list
		m_attributesAppend = true;
	}
	// save attributes
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("attributes");
	file.insertChunkData(&m_attributesAppend, sizeof(bool));
	count = m_attributes.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_attributes[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save craftedSharedTemplate
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("craftedSharedTemplate");
	m_craftedSharedTemplate.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	SharedIntangibleObjectTemplate::save(file);
	file.exitForm();
}	// SharedDraftSchematicObjectTemplate::save


//=============================================================================
// class SharedDraftSchematicObjectTemplate::_IngredientSlot

/**
 * Class constructor.
 */
SharedDraftSchematicObjectTemplate::_IngredientSlot::_IngredientSlot(const std::string & filename)
	: TpfTemplate(filename)
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

CompilerIntegerParam * SharedDraftSchematicObjectTemplate::_IngredientSlot::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_IngredientSlot::getCompilerIntegerParam

FloatParam * SharedDraftSchematicObjectTemplate::_IngredientSlot::getFloatParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getFloatParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_IngredientSlot::getFloatParam

BoolParam * SharedDraftSchematicObjectTemplate::_IngredientSlot::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_IngredientSlot::getBoolParam

StringParam * SharedDraftSchematicObjectTemplate::_IngredientSlot::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "hardpoint") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_hardpoint;
		}
		fprintf(stderr, "trying to access single-parameter \"hardpoint\" as an array\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//SharedDraftSchematicObjectTemplate::_IngredientSlot::getStringParam

StringIdParam * SharedDraftSchematicObjectTemplate::_IngredientSlot::getStringIdParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "name") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_name;
		}
		fprintf(stderr, "trying to access single-parameter \"name\" as an array\n");
	}
	else
		return TpfTemplate::getStringIdParam(name, deepCheck, index);
	return nullptr;
}	//SharedDraftSchematicObjectTemplate::_IngredientSlot::getStringIdParam

VectorParam * SharedDraftSchematicObjectTemplate::_IngredientSlot::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_IngredientSlot::getVectorParam

DynamicVariableParam * SharedDraftSchematicObjectTemplate::_IngredientSlot::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_IngredientSlot::getDynamicVariableParam

StructParamOT * SharedDraftSchematicObjectTemplate::_IngredientSlot::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_IngredientSlot::getStructParamOT

TriggerVolumeParam * SharedDraftSchematicObjectTemplate::_IngredientSlot::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_IngredientSlot::getTriggerVolumeParam

void SharedDraftSchematicObjectTemplate::_IngredientSlot::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::initStructParamOT

void SharedDraftSchematicObjectTemplate::_IngredientSlot::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::setAsEmptyList

void SharedDraftSchematicObjectTemplate::_IngredientSlot::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::setAppend

bool SharedDraftSchematicObjectTemplate::_IngredientSlot::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::isAppend


int SharedDraftSchematicObjectTemplate::_IngredientSlot::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::getListLength

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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedDraftSchematicObjectTemplate::_IngredientSlot::save(Iff &file)
{
int count;

	file.insertForm(_IngredientSlot_tag);

	int paramCount = 0;

	// save name
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("name");
	m_name.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save hardpoint
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("hardpoint");
	m_hardpoint.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// SharedDraftSchematicObjectTemplate::_IngredientSlot::save


//=============================================================================
// class SharedDraftSchematicObjectTemplate::_SchematicAttribute

/**
 * Class constructor.
 */
SharedDraftSchematicObjectTemplate::_SchematicAttribute::_SchematicAttribute(const std::string & filename)
	: TpfTemplate(filename)
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

CompilerIntegerParam * SharedDraftSchematicObjectTemplate::_SchematicAttribute::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "value") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_value;
		}
		fprintf(stderr, "trying to access single-parameter \"value\" as an array\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//SharedDraftSchematicObjectTemplate::_SchematicAttribute::getCompilerIntegerParam

FloatParam * SharedDraftSchematicObjectTemplate::_SchematicAttribute::getFloatParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getFloatParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_SchematicAttribute::getFloatParam

BoolParam * SharedDraftSchematicObjectTemplate::_SchematicAttribute::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_SchematicAttribute::getBoolParam

StringParam * SharedDraftSchematicObjectTemplate::_SchematicAttribute::getStringParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_SchematicAttribute::getStringParam

StringIdParam * SharedDraftSchematicObjectTemplate::_SchematicAttribute::getStringIdParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "name") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_name;
		}
		fprintf(stderr, "trying to access single-parameter \"name\" as an array\n");
	}
	else if (strcmp(name, "experiment") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_experiment;
		}
		fprintf(stderr, "trying to access single-parameter \"experiment\" as an array\n");
	}
	else
		return TpfTemplate::getStringIdParam(name, deepCheck, index);
	return nullptr;
}	//SharedDraftSchematicObjectTemplate::_SchematicAttribute::getStringIdParam

VectorParam * SharedDraftSchematicObjectTemplate::_SchematicAttribute::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_SchematicAttribute::getVectorParam

DynamicVariableParam * SharedDraftSchematicObjectTemplate::_SchematicAttribute::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_SchematicAttribute::getDynamicVariableParam

StructParamOT * SharedDraftSchematicObjectTemplate::_SchematicAttribute::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_SchematicAttribute::getStructParamOT

TriggerVolumeParam * SharedDraftSchematicObjectTemplate::_SchematicAttribute::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedDraftSchematicObjectTemplate::_SchematicAttribute::getTriggerVolumeParam

void SharedDraftSchematicObjectTemplate::_SchematicAttribute::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::initStructParamOT

void SharedDraftSchematicObjectTemplate::_SchematicAttribute::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::setAsEmptyList

void SharedDraftSchematicObjectTemplate::_SchematicAttribute::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::setAppend

bool SharedDraftSchematicObjectTemplate::_SchematicAttribute::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::isAppend


int SharedDraftSchematicObjectTemplate::_SchematicAttribute::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::getListLength

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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedDraftSchematicObjectTemplate::_SchematicAttribute::save(Iff &file)
{
int count;

	file.insertForm(_SchematicAttribute_tag);

	int paramCount = 0;

	// save name
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("name");
	m_name.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save experiment
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("experiment");
	m_experiment.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save value
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("value");
	m_value.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// SharedDraftSchematicObjectTemplate::_SchematicAttribute::save

//@END TFD
