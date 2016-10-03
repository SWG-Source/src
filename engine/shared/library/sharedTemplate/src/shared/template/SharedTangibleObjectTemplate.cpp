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

#include "sharedTemplate/FirstSharedTemplate.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TemplateGlobals.h"
#include "SharedTangibleObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



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
//@END TFD INIT
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
		std::vector<CompilerIntegerParam *>::iterator iter;
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
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedTangibleObjectTemplate * base = dynamic_cast<const SharedTangibleObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedTangibleObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedTangibleObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "socketDestinations") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_socketDestinations.size()))
			return m_socketDestinations[index];
		if (index == static_cast<int>(m_socketDestinations.size()))
		{
			CompilerIntegerParam *temp = new CompilerIntegerParam();
			m_socketDestinations.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"socketDestinations\" out of bounds\n");
	}
	else if (strcmp(name, "clientVisabilityFlag") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_clientVisabilityFlag;
		}
		fprintf(stderr, "trying to access single-parameter \"clientVisabilityFlag\" as an array\n");
	}
	else
		return SharedObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//SharedTangibleObjectTemplate::getCompilerIntegerParam

FloatParam * SharedTangibleObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::getFloatParam

BoolParam * SharedTangibleObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "useStructureFootprintOutline") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_useStructureFootprintOutline;
		}
		fprintf(stderr, "trying to access single-parameter \"useStructureFootprintOutline\" as an array\n");
	}
	else if (strcmp(name, "targetable") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_targetable;
		}
		fprintf(stderr, "trying to access single-parameter \"targetable\" as an array\n");
	}
	else
		return SharedObjectTemplate::getBoolParam(name, deepCheck, index);
	return nullptr;
}	//SharedTangibleObjectTemplate::getBoolParam

StringParam * SharedTangibleObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "structureFootprintFileName") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_structureFootprintFileName;
		}
		fprintf(stderr, "trying to access single-parameter \"structureFootprintFileName\" as an array\n");
	}
	else if (strcmp(name, "certificationsRequired") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_certificationsRequired.size()))
			return m_certificationsRequired[index];
		if (index == static_cast<int>(m_certificationsRequired.size()))
		{
			StringParam *temp = new StringParam();
			m_certificationsRequired.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"certificationsRequired\" out of bounds\n");
	}
	else
		return SharedObjectTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//SharedTangibleObjectTemplate::getStringParam

StringIdParam * SharedTangibleObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::getStringIdParam

VectorParam * SharedTangibleObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::getVectorParam

DynamicVariableParam * SharedTangibleObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::getDynamicVariableParam

StructParamOT * SharedTangibleObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "paletteColorCustomizationVariables") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_paletteColorCustomizationVariables.size()))
			return m_paletteColorCustomizationVariables[index];
		if (index == static_cast<int>(m_paletteColorCustomizationVariables.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_paletteColorCustomizationVariables.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"paletteColorCustomizationVariables\" out of bounds\n");
	}
	else if (strcmp(name, "rangedIntCustomizationVariables") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_rangedIntCustomizationVariables.size()))
			return m_rangedIntCustomizationVariables[index];
		if (index == static_cast<int>(m_rangedIntCustomizationVariables.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_rangedIntCustomizationVariables.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"rangedIntCustomizationVariables\" out of bounds\n");
	}
	else if (strcmp(name, "constStringCustomizationVariables") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_constStringCustomizationVariables.size()))
			return m_constStringCustomizationVariables[index];
		if (index == static_cast<int>(m_constStringCustomizationVariables.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_constStringCustomizationVariables.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"constStringCustomizationVariables\" out of bounds\n");
	}
	else if (strcmp(name, "customizationVariableMapping") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_customizationVariableMapping.size()))
			return m_customizationVariableMapping[index];
		if (index == static_cast<int>(m_customizationVariableMapping.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_customizationVariableMapping.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"customizationVariableMapping\" out of bounds\n");
	}
	else
		return SharedObjectTemplate::getStructParamOT(name, deepCheck, index);
	return nullptr;
}	//SharedTangibleObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedTangibleObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::getTriggerVolumeParam

void SharedTangibleObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	if (strcmp(name, "paletteColorCustomizationVariables") == 0)
		param.setValue(new _PaletteColorCustomizationVariable(""));
	else if (strcmp(name, "rangedIntCustomizationVariables") == 0)
		param.setValue(new _RangedIntCustomizationVariable(""));
	else if (strcmp(name, "constStringCustomizationVariables") == 0)
		param.setValue(new _ConstStringCustomizationVariable(""));
	else if (strcmp(name, "customizationVariableMapping") == 0)
		param.setValue(new _CustomizationVariableMapping(""));
	else
		SharedObjectTemplate::initStructParamOT(param, name);
}	// SharedTangibleObjectTemplate::initStructParamOT

void SharedTangibleObjectTemplate::setAsEmptyList(const char *name)
{
	if (strcmp(name, "paletteColorCustomizationVariables") == 0)
	{
		m_paletteColorCustomizationVariables.clear();
		m_paletteColorCustomizationVariablesLoaded = true;
	}
	else if (strcmp(name, "rangedIntCustomizationVariables") == 0)
	{
		m_rangedIntCustomizationVariables.clear();
		m_rangedIntCustomizationVariablesLoaded = true;
	}
	else if (strcmp(name, "constStringCustomizationVariables") == 0)
	{
		m_constStringCustomizationVariables.clear();
		m_constStringCustomizationVariablesLoaded = true;
	}
	else if (strcmp(name, "socketDestinations") == 0)
	{
		m_socketDestinations.clear();
		m_socketDestinationsLoaded = true;
	}
	else if (strcmp(name, "certificationsRequired") == 0)
	{
		m_certificationsRequired.clear();
		m_certificationsRequiredLoaded = true;
	}
	else if (strcmp(name, "customizationVariableMapping") == 0)
	{
		m_customizationVariableMapping.clear();
		m_customizationVariableMappingLoaded = true;
	}
	else
		SharedObjectTemplate::setAsEmptyList(name);
}	// SharedTangibleObjectTemplate::setAsEmptyList

void SharedTangibleObjectTemplate::setAppend(const char *name)
{
	if (strcmp(name, "paletteColorCustomizationVariables") == 0)
		m_paletteColorCustomizationVariablesAppend = true;
	else if (strcmp(name, "rangedIntCustomizationVariables") == 0)
		m_rangedIntCustomizationVariablesAppend = true;
	else if (strcmp(name, "constStringCustomizationVariables") == 0)
		m_constStringCustomizationVariablesAppend = true;
	else if (strcmp(name, "socketDestinations") == 0)
		m_socketDestinationsAppend = true;
	else if (strcmp(name, "certificationsRequired") == 0)
		m_certificationsRequiredAppend = true;
	else if (strcmp(name, "customizationVariableMapping") == 0)
		m_customizationVariableMappingAppend = true;
	else
		SharedObjectTemplate::setAppend(name);
}	// SharedTangibleObjectTemplate::setAppend

bool SharedTangibleObjectTemplate::isAppend(const char *name) const
{
	if (strcmp(name, "paletteColorCustomizationVariables") == 0)
		return m_paletteColorCustomizationVariablesAppend;
	else if (strcmp(name, "rangedIntCustomizationVariables") == 0)
		return m_rangedIntCustomizationVariablesAppend;
	else if (strcmp(name, "constStringCustomizationVariables") == 0)
		return m_constStringCustomizationVariablesAppend;
	else if (strcmp(name, "socketDestinations") == 0)
		return m_socketDestinationsAppend;
	else if (strcmp(name, "certificationsRequired") == 0)
		return m_certificationsRequiredAppend;
	else if (strcmp(name, "customizationVariableMapping") == 0)
		return m_customizationVariableMappingAppend;
	else
		return SharedObjectTemplate::isAppend(name);
}	// SharedTangibleObjectTemplate::isAppend


int SharedTangibleObjectTemplate::getListLength(const char *name) const
{
	if (strcmp(name, "paletteColorCustomizationVariables") == 0)
	{
		return m_paletteColorCustomizationVariables.size();
	}
	else if (strcmp(name, "rangedIntCustomizationVariables") == 0)
	{
		return m_rangedIntCustomizationVariables.size();
	}
	else if (strcmp(name, "constStringCustomizationVariables") == 0)
	{
		return m_constStringCustomizationVariables.size();
	}
	else if (strcmp(name, "socketDestinations") == 0)
	{
		return m_socketDestinations.size();
	}
	else if (strcmp(name, "certificationsRequired") == 0)
	{
		return m_certificationsRequired.size();
	}
	else if (strcmp(name, "customizationVariableMapping") == 0)
	{
		return m_customizationVariableMapping.size();
	}
	else
		return SharedObjectTemplate::getListLength(name);
}	// SharedTangibleObjectTemplate::getListLength

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
			std::vector<CompilerIntegerParam *>::iterator iter;
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
				CompilerIntegerParam * newData = new CompilerIntegerParam;
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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedTangibleObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedTangibleObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,1,0));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	if (!m_paletteColorCustomizationVariablesLoaded)
	{
		// mark the list as empty and extending the base list
		m_paletteColorCustomizationVariablesAppend = true;
	}
	// save paletteColorCustomizationVariables
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("paletteColorCustomizationVariables");
	file.insertChunkData(&m_paletteColorCustomizationVariablesAppend, sizeof(bool));
	count = m_paletteColorCustomizationVariables.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_paletteColorCustomizationVariables[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_rangedIntCustomizationVariablesLoaded)
	{
		// mark the list as empty and extending the base list
		m_rangedIntCustomizationVariablesAppend = true;
	}
	// save rangedIntCustomizationVariables
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("rangedIntCustomizationVariables");
	file.insertChunkData(&m_rangedIntCustomizationVariablesAppend, sizeof(bool));
	count = m_rangedIntCustomizationVariables.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_rangedIntCustomizationVariables[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_constStringCustomizationVariablesLoaded)
	{
		// mark the list as empty and extending the base list
		m_constStringCustomizationVariablesAppend = true;
	}
	// save constStringCustomizationVariables
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("constStringCustomizationVariables");
	file.insertChunkData(&m_constStringCustomizationVariablesAppend, sizeof(bool));
	count = m_constStringCustomizationVariables.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_constStringCustomizationVariables[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_socketDestinationsLoaded)
	{
		// mark the list as empty and extending the base list
		m_socketDestinationsAppend = true;
	}
	// save socketDestinations
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("socketDestinations");
	file.insertChunkData(&m_socketDestinationsAppend, sizeof(bool));
	count = m_socketDestinations.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_socketDestinations[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save structureFootprintFileName
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("structureFootprintFileName");
	m_structureFootprintFileName.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save useStructureFootprintOutline
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("useStructureFootprintOutline");
	m_useStructureFootprintOutline.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save targetable
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("targetable");
	m_targetable.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_certificationsRequiredLoaded)
	{
		// mark the list as empty and extending the base list
		m_certificationsRequiredAppend = true;
	}
	// save certificationsRequired
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("certificationsRequired");
	file.insertChunkData(&m_certificationsRequiredAppend, sizeof(bool));
	count = m_certificationsRequired.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_certificationsRequired[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_customizationVariableMappingLoaded)
	{
		// mark the list as empty and extending the base list
		m_customizationVariableMappingAppend = true;
	}
	// save customizationVariableMapping
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("customizationVariableMapping");
	file.insertChunkData(&m_customizationVariableMappingAppend, sizeof(bool));
	count = m_customizationVariableMapping.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_customizationVariableMapping[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save clientVisabilityFlag
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("clientVisabilityFlag");
	m_clientVisabilityFlag.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	SharedObjectTemplate::save(file);
	file.exitForm();
}	// SharedTangibleObjectTemplate::save


//=============================================================================
// class SharedTangibleObjectTemplate::_ConstStringCustomizationVariable

/**
 * Class constructor.
 */
SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::_ConstStringCustomizationVariable(const std::string & filename)
	: TpfTemplate(filename)
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

CompilerIntegerParam * SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getCompilerIntegerParam

FloatParam * SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getFloatParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getFloatParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getFloatParam

BoolParam * SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getBoolParam

StringParam * SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "variableName") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_variableName;
		}
		fprintf(stderr, "trying to access single-parameter \"variableName\" as an array\n");
	}
	else if (strcmp(name, "constValue") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_constValue;
		}
		fprintf(stderr, "trying to access single-parameter \"constValue\" as an array\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getStringParam

StringIdParam * SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getStringIdParam

VectorParam * SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getVectorParam

DynamicVariableParam * SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getDynamicVariableParam

StructParamOT * SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getStructParamOT

TriggerVolumeParam * SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getTriggerVolumeParam

void SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::initStructParamOT

void SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::setAsEmptyList

void SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::setAppend

bool SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::isAppend


int SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::getListLength

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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::save(Iff &file)
{
int count;

	file.insertForm(_ConstStringCustomizationVariable_tag);

	int paramCount = 0;

	// save variableName
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("variableName");
	m_variableName.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save constValue
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("constValue");
	m_constValue.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::save


//=============================================================================
// class SharedTangibleObjectTemplate::_CustomizationVariableMapping

/**
 * Class constructor.
 */
SharedTangibleObjectTemplate::_CustomizationVariableMapping::_CustomizationVariableMapping(const std::string & filename)
	: TpfTemplate(filename)
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

CompilerIntegerParam * SharedTangibleObjectTemplate::_CustomizationVariableMapping::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_CustomizationVariableMapping::getCompilerIntegerParam

FloatParam * SharedTangibleObjectTemplate::_CustomizationVariableMapping::getFloatParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getFloatParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_CustomizationVariableMapping::getFloatParam

BoolParam * SharedTangibleObjectTemplate::_CustomizationVariableMapping::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_CustomizationVariableMapping::getBoolParam

StringParam * SharedTangibleObjectTemplate::_CustomizationVariableMapping::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "sourceVariable") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_sourceVariable;
		}
		fprintf(stderr, "trying to access single-parameter \"sourceVariable\" as an array\n");
	}
	else if (strcmp(name, "dependentVariable") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_dependentVariable;
		}
		fprintf(stderr, "trying to access single-parameter \"dependentVariable\" as an array\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//SharedTangibleObjectTemplate::_CustomizationVariableMapping::getStringParam

StringIdParam * SharedTangibleObjectTemplate::_CustomizationVariableMapping::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_CustomizationVariableMapping::getStringIdParam

VectorParam * SharedTangibleObjectTemplate::_CustomizationVariableMapping::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_CustomizationVariableMapping::getVectorParam

DynamicVariableParam * SharedTangibleObjectTemplate::_CustomizationVariableMapping::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_CustomizationVariableMapping::getDynamicVariableParam

StructParamOT * SharedTangibleObjectTemplate::_CustomizationVariableMapping::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_CustomizationVariableMapping::getStructParamOT

TriggerVolumeParam * SharedTangibleObjectTemplate::_CustomizationVariableMapping::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_CustomizationVariableMapping::getTriggerVolumeParam

void SharedTangibleObjectTemplate::_CustomizationVariableMapping::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::initStructParamOT

void SharedTangibleObjectTemplate::_CustomizationVariableMapping::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::setAsEmptyList

void SharedTangibleObjectTemplate::_CustomizationVariableMapping::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::setAppend

bool SharedTangibleObjectTemplate::_CustomizationVariableMapping::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::isAppend


int SharedTangibleObjectTemplate::_CustomizationVariableMapping::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::getListLength

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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedTangibleObjectTemplate::_CustomizationVariableMapping::save(Iff &file)
{
int count;

	file.insertForm(_CustomizationVariableMapping_tag);

	int paramCount = 0;

	// save sourceVariable
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("sourceVariable");
	m_sourceVariable.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save dependentVariable
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("dependentVariable");
	m_dependentVariable.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// SharedTangibleObjectTemplate::_CustomizationVariableMapping::save


//=============================================================================
// class SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable

/**
 * Class constructor.
 */
SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::_PaletteColorCustomizationVariable(const std::string & filename)
	: TpfTemplate(filename)
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

CompilerIntegerParam * SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "defaultPaletteIndex") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_defaultPaletteIndex;
		}
		fprintf(stderr, "trying to access single-parameter \"defaultPaletteIndex\" as an array\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getCompilerIntegerParam

FloatParam * SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getFloatParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getFloatParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getFloatParam

BoolParam * SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getBoolParam

StringParam * SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "variableName") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_variableName;
		}
		fprintf(stderr, "trying to access single-parameter \"variableName\" as an array\n");
	}
	else if (strcmp(name, "palettePathName") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_palettePathName;
		}
		fprintf(stderr, "trying to access single-parameter \"palettePathName\" as an array\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getStringParam

StringIdParam * SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getStringIdParam

VectorParam * SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getVectorParam

DynamicVariableParam * SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getDynamicVariableParam

StructParamOT * SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getStructParamOT

TriggerVolumeParam * SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getTriggerVolumeParam

void SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::initStructParamOT

void SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::setAsEmptyList

void SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::setAppend

bool SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::isAppend


int SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::getListLength

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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::save(Iff &file)
{
int count;

	file.insertForm(_PaletteColorCustomizationVariable_tag);

	int paramCount = 0;

	// save variableName
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("variableName");
	m_variableName.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save palettePathName
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("palettePathName");
	m_palettePathName.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save defaultPaletteIndex
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("defaultPaletteIndex");
	m_defaultPaletteIndex.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::save


//=============================================================================
// class SharedTangibleObjectTemplate::_RangedIntCustomizationVariable

/**
 * Class constructor.
 */
SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::_RangedIntCustomizationVariable(const std::string & filename)
	: TpfTemplate(filename)
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

CompilerIntegerParam * SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "minValueInclusive") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_minValueInclusive;
		}
		fprintf(stderr, "trying to access single-parameter \"minValueInclusive\" as an array\n");
	}
	else if (strcmp(name, "defaultValue") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_defaultValue;
		}
		fprintf(stderr, "trying to access single-parameter \"defaultValue\" as an array\n");
	}
	else if (strcmp(name, "maxValueExclusive") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_maxValueExclusive;
		}
		fprintf(stderr, "trying to access single-parameter \"maxValueExclusive\" as an array\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getCompilerIntegerParam

FloatParam * SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getFloatParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getFloatParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getFloatParam

BoolParam * SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getBoolParam

StringParam * SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "variableName") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_variableName;
		}
		fprintf(stderr, "trying to access single-parameter \"variableName\" as an array\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getStringParam

StringIdParam * SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getStringIdParam

VectorParam * SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getVectorParam

DynamicVariableParam * SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getDynamicVariableParam

StructParamOT * SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getStructParamOT

TriggerVolumeParam * SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getTriggerVolumeParam

void SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::initStructParamOT

void SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::setAsEmptyList

void SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::setAppend

bool SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::isAppend


int SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::getListLength

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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::save(Iff &file)
{
int count;

	file.insertForm(_RangedIntCustomizationVariable_tag);

	int paramCount = 0;

	// save variableName
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("variableName");
	m_variableName.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save minValueInclusive
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("minValueInclusive");
	m_minValueInclusive.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save defaultValue
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("defaultValue");
	m_defaultValue.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save maxValueExclusive
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("maxValueExclusive");
	m_maxValueExclusive.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::save

//@END TFD
