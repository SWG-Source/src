//========================================================================
//
// SharedCellObjectTemplate.cpp
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
#include "SharedCellObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
SharedCellObjectTemplate::SharedCellObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: SharedObjectTemplate(filename)
//@END TFD INIT
{
}	// SharedCellObjectTemplate::SharedCellObjectTemplate

/**
 * Class destructor.
 */
SharedCellObjectTemplate::~SharedCellObjectTemplate()
{
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}	// SharedCellObjectTemplate::~SharedCellObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedCellObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(SharedCellObjectTemplate_tag, create);
}	// SharedCellObjectTemplate::registerMe

/**
 * Creates a SharedCellObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * SharedCellObjectTemplate::create(const std::string & filename)
{
	return new SharedCellObjectTemplate(filename);
}	// SharedCellObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedCellObjectTemplate::getId(void) const
{
	return SharedCellObjectTemplate_tag;
}	// SharedCellObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedCellObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // SharedCellObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedCellObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const SharedCellObjectTemplate * base = dynamic_cast<const SharedCellObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedCellObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * SharedCellObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//SharedCellObjectTemplate::getCompilerIntegerParam

FloatParam * SharedCellObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getFloatParam(name, deepCheck, index);
}	//SharedCellObjectTemplate::getFloatParam

BoolParam * SharedCellObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getBoolParam(name, deepCheck, index);
}	//SharedCellObjectTemplate::getBoolParam

StringParam * SharedCellObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStringParam(name, deepCheck, index);
}	//SharedCellObjectTemplate::getStringParam

StringIdParam * SharedCellObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStringIdParam(name, deepCheck, index);
}	//SharedCellObjectTemplate::getStringIdParam

VectorParam * SharedCellObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getVectorParam(name, deepCheck, index);
}	//SharedCellObjectTemplate::getVectorParam

DynamicVariableParam * SharedCellObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//SharedCellObjectTemplate::getDynamicVariableParam

StructParamOT * SharedCellObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getStructParamOT(name, deepCheck, index);
}	//SharedCellObjectTemplate::getStructParamOT

TriggerVolumeParam * SharedCellObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return SharedObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//SharedCellObjectTemplate::getTriggerVolumeParam

void SharedCellObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	SharedObjectTemplate::initStructParamOT(param, name);
}	// SharedCellObjectTemplate::initStructParamOT

void SharedCellObjectTemplate::setAsEmptyList(const char *name)
{
	SharedObjectTemplate::setAsEmptyList(name);
}	// SharedCellObjectTemplate::setAsEmptyList

void SharedCellObjectTemplate::setAppend(const char *name)
{
	SharedObjectTemplate::setAppend(name);
}	// SharedCellObjectTemplate::setAppend

bool SharedCellObjectTemplate::isAppend(const char *name) const
{
	return SharedObjectTemplate::isAppend(name);
}	// SharedCellObjectTemplate::isAppend


int SharedCellObjectTemplate::getListLength(const char *name) const
{
	return SharedObjectTemplate::getListLength(name);
}	// SharedCellObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedCellObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != SharedCellObjectTemplate_tag)
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
	if (getHighestTemplateVersion() != TAG(0,0,0,0))
	{
		if (DataLint::isEnabled())
			DEBUG_WARNING(true, ("template %s version out of date", file.getFileName()));
	}

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	UNREF(paramName);
	UNREF(paramCount);

	file.exitForm();
	SharedObjectTemplate::load(file);
	file.exitForm();
	return;
}	// SharedCellObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedCellObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(SharedCellObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,0,0));
	file.allowNonlinearFunctions();

	int paramCount = 0;


	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	SharedObjectTemplate::save(file);
	file.exitForm();
	UNREF(count);
}	// SharedCellObjectTemplate::save

//@END TFD
