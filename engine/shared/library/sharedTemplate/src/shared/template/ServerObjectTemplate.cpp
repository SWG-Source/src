//========================================================================
//
// ServerObjectTemplate.cpp
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
#include "ServerObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerObjectTemplate::ServerObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: TpfTemplate(filename)
	,m_scriptsLoaded(false)
	,m_scriptsAppend(false)
	,m_visibleFlagsLoaded(false)
	,m_visibleFlagsAppend(false)
	,m_deleteFlagsLoaded(false)
	,m_deleteFlagsAppend(false)
	,m_moveFlagsLoaded(false)
	,m_moveFlagsAppend(false)
	,m_contentsLoaded(false)
	,m_contentsAppend(false)
	,m_xpPointsLoaded(false)
	,m_xpPointsAppend(false)
//@END TFD INIT
{
}	// ServerObjectTemplate::ServerObjectTemplate

/**
 * Class destructor.
 */
ServerObjectTemplate::~ServerObjectTemplate()
{
//@BEGIN TFD CLEANUP
	{
		std::vector<StringParam *>::iterator iter;
		for (iter = m_scripts.begin(); iter != m_scripts.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_scripts.clear();
	}
	{
		std::vector<CompilerIntegerParam *>::iterator iter;
		for (iter = m_visibleFlags.begin(); iter != m_visibleFlags.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_visibleFlags.clear();
	}
	{
		std::vector<CompilerIntegerParam *>::iterator iter;
		for (iter = m_deleteFlags.begin(); iter != m_deleteFlags.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_deleteFlags.clear();
	}
	{
		std::vector<CompilerIntegerParam *>::iterator iter;
		for (iter = m_moveFlags.begin(); iter != m_moveFlags.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_moveFlags.clear();
	}
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_contents.begin(); iter != m_contents.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_contents.clear();
	}
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_xpPoints.begin(); iter != m_xpPoints.end(); ++iter)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_xpPoints.clear();
	}
//@END TFD CLEANUP
}	// ServerObjectTemplate::~ServerObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerObjectTemplate_tag, create);
}	// ServerObjectTemplate::registerMe

/**
 * Creates a ServerObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerObjectTemplate::create(const std::string & filename)
{
	return new ServerObjectTemplate(filename);
}	// ServerObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerObjectTemplate::getId(void) const
{
	return ServerObjectTemplate_tag;
}	// ServerObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerObjectTemplate * base = dynamic_cast<const ServerObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "volume") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_volume;
		}
		fprintf(stderr, "trying to access single-parameter \"volume\" as an array\n");
	}
	else if (strcmp(name, "visibleFlags") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_visibleFlags.size()))
			return m_visibleFlags[index];
		if (index == static_cast<int>(m_visibleFlags.size()))
		{
			CompilerIntegerParam *temp = new CompilerIntegerParam();
			m_visibleFlags.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"visibleFlags\" out of bounds\n");
	}
	else if (strcmp(name, "deleteFlags") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_deleteFlags.size()))
			return m_deleteFlags[index];
		if (index == static_cast<int>(m_deleteFlags.size()))
		{
			CompilerIntegerParam *temp = new CompilerIntegerParam();
			m_deleteFlags.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"deleteFlags\" out of bounds\n");
	}
	else if (strcmp(name, "moveFlags") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_moveFlags.size()))
			return m_moveFlags[index];
		if (index == static_cast<int>(m_moveFlags.size()))
		{
			CompilerIntegerParam *temp = new CompilerIntegerParam();
			m_moveFlags.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"moveFlags\" out of bounds\n");
	}
	else if (strcmp(name, "tintIndex") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_tintIndex;
		}
		fprintf(stderr, "trying to access single-parameter \"tintIndex\" as an array\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerObjectTemplate::getCompilerIntegerParam

FloatParam * ServerObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "complexity") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_complexity;
		}
		fprintf(stderr, "trying to access single-parameter \"complexity\" as an array\n");
	}
	else if (strcmp(name, "updateRanges") == 0)
	{
		if (index >= 0 && index < 3)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_updateRanges[index];
		}
		fprintf(stderr, "index for parameter \"updateRanges\" out of bounds\n");
	}
	else
		return TpfTemplate::getFloatParam(name, deepCheck, index);
	return nullptr;
}	//ServerObjectTemplate::getFloatParam

BoolParam * ServerObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "invulnerable") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_invulnerable;
		}
		fprintf(stderr, "trying to access single-parameter \"invulnerable\" as an array\n");
	}
	else if (strcmp(name, "persistByDefault") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_persistByDefault;
		}
		fprintf(stderr, "trying to access single-parameter \"persistByDefault\" as an array\n");
	}
	else if (strcmp(name, "persistContents") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_persistContents;
		}
		fprintf(stderr, "trying to access single-parameter \"persistContents\" as an array\n");
	}
	else
		return TpfTemplate::getBoolParam(name, deepCheck, index);
	return nullptr;
}	//ServerObjectTemplate::getBoolParam

StringParam * ServerObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "sharedTemplate") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_sharedTemplate;
		}
		fprintf(stderr, "trying to access single-parameter \"sharedTemplate\" as an array\n");
	}
	else if (strcmp(name, "scripts") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_scripts.size()))
			return m_scripts[index];
		if (index == static_cast<int>(m_scripts.size()))
		{
			StringParam *temp = new StringParam();
			m_scripts.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"scripts\" out of bounds\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//ServerObjectTemplate::getStringParam

StringIdParam * ServerObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerObjectTemplate::getStringIdParam

VectorParam * ServerObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//ServerObjectTemplate::getVectorParam

DynamicVariableParam * ServerObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "objvars") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getDynamicVariableParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_objvars;
		}
		fprintf(stderr, "trying to access single-parameter \"objvars\" as an array\n");
	}
	else
		return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
	return nullptr;
}	//ServerObjectTemplate::getDynamicVariableParam

StructParamOT * ServerObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "contents") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_contents.size()))
			return m_contents[index];
		if (index == static_cast<int>(m_contents.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_contents.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"contents\" out of bounds\n");
	}
	else if (strcmp(name, "xpPoints") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_xpPoints.size()))
			return m_xpPoints[index];
		if (index == static_cast<int>(m_xpPoints.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_xpPoints.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"xpPoints\" out of bounds\n");
	}
	else
		return TpfTemplate::getStructParamOT(name, deepCheck, index);
	return nullptr;
}	//ServerObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerObjectTemplate::getTriggerVolumeParam

void ServerObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	if (strcmp(name, "contents") == 0)
		param.setValue(new _Contents(""));
	else if (strcmp(name, "xpPoints") == 0)
		param.setValue(new _Xp(""));
	else
		TpfTemplate::initStructParamOT(param, name);
}	// ServerObjectTemplate::initStructParamOT

void ServerObjectTemplate::setAsEmptyList(const char *name)
{
	if (strcmp(name, "scripts") == 0)
	{
		m_scripts.clear();
		m_scriptsLoaded = true;
	}
	else if (strcmp(name, "visibleFlags") == 0)
	{
		m_visibleFlags.clear();
		m_visibleFlagsLoaded = true;
	}
	else if (strcmp(name, "deleteFlags") == 0)
	{
		m_deleteFlags.clear();
		m_deleteFlagsLoaded = true;
	}
	else if (strcmp(name, "moveFlags") == 0)
	{
		m_moveFlags.clear();
		m_moveFlagsLoaded = true;
	}
	else if (strcmp(name, "contents") == 0)
	{
		m_contents.clear();
		m_contentsLoaded = true;
	}
	else if (strcmp(name, "xpPoints") == 0)
	{
		m_xpPoints.clear();
		m_xpPointsLoaded = true;
	}
	else
		TpfTemplate::setAsEmptyList(name);
}	// ServerObjectTemplate::setAsEmptyList

void ServerObjectTemplate::setAppend(const char *name)
{
	if (strcmp(name, "scripts") == 0)
		m_scriptsAppend = true;
	else if (strcmp(name, "visibleFlags") == 0)
		m_visibleFlagsAppend = true;
	else if (strcmp(name, "deleteFlags") == 0)
		m_deleteFlagsAppend = true;
	else if (strcmp(name, "moveFlags") == 0)
		m_moveFlagsAppend = true;
	else if (strcmp(name, "contents") == 0)
		m_contentsAppend = true;
	else if (strcmp(name, "xpPoints") == 0)
		m_xpPointsAppend = true;
	else
		TpfTemplate::setAppend(name);
}	// ServerObjectTemplate::setAppend

bool ServerObjectTemplate::isAppend(const char *name) const
{
	if (strcmp(name, "scripts") == 0)
		return m_scriptsAppend;
	else if (strcmp(name, "visibleFlags") == 0)
		return m_visibleFlagsAppend;
	else if (strcmp(name, "deleteFlags") == 0)
		return m_deleteFlagsAppend;
	else if (strcmp(name, "moveFlags") == 0)
		return m_moveFlagsAppend;
	else if (strcmp(name, "contents") == 0)
		return m_contentsAppend;
	else if (strcmp(name, "xpPoints") == 0)
		return m_xpPointsAppend;
	else
		return TpfTemplate::isAppend(name);
}	// ServerObjectTemplate::isAppend


int ServerObjectTemplate::getListLength(const char *name) const
{
	if (strcmp(name, "scripts") == 0)
	{
		return m_scripts.size();
	}
	else if (strcmp(name, "visibleFlags") == 0)
	{
		return m_visibleFlags.size();
	}
	else if (strcmp(name, "deleteFlags") == 0)
	{
		return m_deleteFlags.size();
	}
	else if (strcmp(name, "moveFlags") == 0)
	{
		return m_moveFlags.size();
	}
	else if (strcmp(name, "updateRanges") == 0)
	{
		return sizeof(m_updateRanges) / sizeof(FloatParam);
	}
	else if (strcmp(name, "contents") == 0)
	{
		return m_contents.size();
	}
	else if (strcmp(name, "xpPoints") == 0)
	{
		return m_xpPoints.size();
	}
	else
		return TpfTemplate::getListLength(name);
}	// ServerObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerObjectTemplate_tag)
	{
		TpfTemplate::load(file);
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
	if (getHighestTemplateVersion() != TAG(0,0,1,1))
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
		if (strcmp(paramName, "sharedTemplate") == 0)
			m_sharedTemplate.loadFromIff(file);
		else if (strcmp(paramName, "scripts") == 0)
		{
			std::vector<StringParam *>::iterator iter;
			for (iter = m_scripts.begin(); iter != m_scripts.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_scripts.clear();
			m_scriptsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringParam * newData = new StringParam;
				newData->loadFromIff(file);
				m_scripts.push_back(newData);
			}
			m_scriptsLoaded = true;
		}
		else if (strcmp(paramName, "objvars") == 0)
			m_objvars.loadFromIff(file);
		else if (strcmp(paramName, "volume") == 0)
			m_volume.loadFromIff(file);
		else if (strcmp(paramName, "visibleFlags") == 0)
		{
			std::vector<CompilerIntegerParam *>::iterator iter;
			for (iter = m_visibleFlags.begin(); iter != m_visibleFlags.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_visibleFlags.clear();
			m_visibleFlagsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				CompilerIntegerParam * newData = new CompilerIntegerParam;
				newData->loadFromIff(file);
				m_visibleFlags.push_back(newData);
			}
			m_visibleFlagsLoaded = true;
		}
		else if (strcmp(paramName, "deleteFlags") == 0)
		{
			std::vector<CompilerIntegerParam *>::iterator iter;
			for (iter = m_deleteFlags.begin(); iter != m_deleteFlags.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_deleteFlags.clear();
			m_deleteFlagsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				CompilerIntegerParam * newData = new CompilerIntegerParam;
				newData->loadFromIff(file);
				m_deleteFlags.push_back(newData);
			}
			m_deleteFlagsLoaded = true;
		}
		else if (strcmp(paramName, "moveFlags") == 0)
		{
			std::vector<CompilerIntegerParam *>::iterator iter;
			for (iter = m_moveFlags.begin(); iter != m_moveFlags.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_moveFlags.clear();
			m_moveFlagsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				CompilerIntegerParam * newData = new CompilerIntegerParam;
				newData->loadFromIff(file);
				m_moveFlags.push_back(newData);
			}
			m_moveFlagsLoaded = true;
		}
		else if (strcmp(paramName, "invulnerable") == 0)
			m_invulnerable.loadFromIff(file);
		else if (strcmp(paramName, "complexity") == 0)
			m_complexity.loadFromIff(file);
		else if (strcmp(paramName, "tintIndex") == 0)
			m_tintIndex.loadFromIff(file);
		else if (strcmp(paramName, "updateRanges") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 3, ("Template %s: read array size of %d for array \"updateRanges\" of size 3, reading values anyway", file.getFileName(), listCount));
			int j;
			for (j = 0; j < 3 && j < listCount; ++j)
				m_updateRanges[j].loadFromIff(file);
			// if there are more params for updateRanges read and dump them
			for (; j < listCount; ++j)
			{
				FloatParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "contents") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_contents.begin(); iter != m_contents.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_contents.clear();
			m_contentsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_contents.push_back(newData);
			}
			m_contentsLoaded = true;
		}
		else if (strcmp(paramName, "xpPoints") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_xpPoints.begin(); iter != m_xpPoints.end(); ++iter)
			{
				delete *iter;
				*iter = nullptr;
			}
			m_xpPoints.clear();
			m_xpPointsAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_xpPoints.push_back(newData);
			}
			m_xpPointsLoaded = true;
		}
		else if (strcmp(paramName, "persistByDefault") == 0)
			m_persistByDefault.loadFromIff(file);
		else if (strcmp(paramName, "persistContents") == 0)
			m_persistContents.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	TpfTemplate::load(file);
	file.exitForm();
	return;
}	// ServerObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,1,1));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	// save sharedTemplate
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("sharedTemplate");
	m_sharedTemplate.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_scriptsLoaded)
	{
		// mark the list as empty and extending the base list
		m_scriptsAppend = true;
	}
	// save scripts
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("scripts");
	file.insertChunkData(&m_scriptsAppend, sizeof(bool));
	count = m_scripts.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_scripts[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save objvars
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("objvars");
	m_objvars.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save volume
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("volume");
	m_volume.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_visibleFlagsLoaded)
	{
		// mark the list as empty and extending the base list
		m_visibleFlagsAppend = true;
	}
	// save visibleFlags
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("visibleFlags");
	file.insertChunkData(&m_visibleFlagsAppend, sizeof(bool));
	count = m_visibleFlags.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_visibleFlags[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_deleteFlagsLoaded)
	{
		// mark the list as empty and extending the base list
		m_deleteFlagsAppend = true;
	}
	// save deleteFlags
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("deleteFlags");
	file.insertChunkData(&m_deleteFlagsAppend, sizeof(bool));
	count = m_deleteFlags.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_deleteFlags[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_moveFlagsLoaded)
	{
		// mark the list as empty and extending the base list
		m_moveFlagsAppend = true;
	}
	// save moveFlags
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("moveFlags");
	file.insertChunkData(&m_moveFlagsAppend, sizeof(bool));
	count = m_moveFlags.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_moveFlags[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save invulnerable
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("invulnerable");
	m_invulnerable.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save complexity
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("complexity");
	m_complexity.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save tintIndex
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("tintIndex");
	m_tintIndex.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save updateRanges
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("updateRanges");
	count = 3;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 3; ++i)
		m_updateRanges[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_contentsLoaded)
	{
		// mark the list as empty and extending the base list
		m_contentsAppend = true;
	}
	// save contents
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("contents");
	file.insertChunkData(&m_contentsAppend, sizeof(bool));
	count = m_contents.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_contents[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_xpPointsLoaded)
	{
		// mark the list as empty and extending the base list
		m_xpPointsAppend = true;
	}
	// save xpPoints
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("xpPoints");
	file.insertChunkData(&m_xpPointsAppend, sizeof(bool));
	count = m_xpPoints.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_xpPoints[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save persistByDefault
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("persistByDefault");
	m_persistByDefault.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save persistContents
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("persistContents");
	m_persistContents.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	TpfTemplate::save(file);
	file.exitForm();
}	// ServerObjectTemplate::save


//=============================================================================
// class ServerObjectTemplate::_AttribMod

/**
 * Class constructor.
 */
ServerObjectTemplate::_AttribMod::_AttribMod(const std::string & filename)
	: TpfTemplate(filename)
{
}	// ServerObjectTemplate::_AttribMod::_AttribMod

/**
 * Class destructor.
 */
ServerObjectTemplate::_AttribMod::~_AttribMod()
{
}	// ServerObjectTemplate::_AttribMod::~_AttribMod

/**
 * Static function used to register this template.
 */
void ServerObjectTemplate::_AttribMod::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_AttribMod_tag, create);
}	// ServerObjectTemplate::_AttribMod::registerMe

/**
 * Creates a ServerObjectTemplate::_AttribMod template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerObjectTemplate::_AttribMod::create(const std::string & filename)
{
	return new ServerObjectTemplate::_AttribMod(filename);
}	// ServerObjectTemplate::_AttribMod::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerObjectTemplate::_AttribMod::getId(void) const
{
	return _AttribMod_tag;
}	// ServerObjectTemplate::_AttribMod::getId

CompilerIntegerParam * ServerObjectTemplate::_AttribMod::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "target") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_target;
		}
		fprintf(stderr, "trying to access single-parameter \"target\" as an array\n");
	}
	else if (strcmp(name, "value") == 0)
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
}	//ServerObjectTemplate::_AttribMod::getCompilerIntegerParam

FloatParam * ServerObjectTemplate::_AttribMod::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "time") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_time;
		}
		fprintf(stderr, "trying to access single-parameter \"time\" as an array\n");
	}
	else if (strcmp(name, "timeAtValue") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_timeAtValue;
		}
		fprintf(stderr, "trying to access single-parameter \"timeAtValue\" as an array\n");
	}
	else if (strcmp(name, "decay") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_decay;
		}
		fprintf(stderr, "trying to access single-parameter \"decay\" as an array\n");
	}
	else
		return TpfTemplate::getFloatParam(name, deepCheck, index);
	return nullptr;
}	//ServerObjectTemplate::_AttribMod::getFloatParam

BoolParam * ServerObjectTemplate::_AttribMod::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//ServerObjectTemplate::_AttribMod::getBoolParam

StringParam * ServerObjectTemplate::_AttribMod::getStringParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringParam(name, deepCheck, index);
}	//ServerObjectTemplate::_AttribMod::getStringParam

StringIdParam * ServerObjectTemplate::_AttribMod::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerObjectTemplate::_AttribMod::getStringIdParam

VectorParam * ServerObjectTemplate::_AttribMod::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//ServerObjectTemplate::_AttribMod::getVectorParam

DynamicVariableParam * ServerObjectTemplate::_AttribMod::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerObjectTemplate::_AttribMod::getDynamicVariableParam

StructParamOT * ServerObjectTemplate::_AttribMod::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerObjectTemplate::_AttribMod::getStructParamOT

TriggerVolumeParam * ServerObjectTemplate::_AttribMod::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerObjectTemplate::_AttribMod::getTriggerVolumeParam

void ServerObjectTemplate::_AttribMod::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// ServerObjectTemplate::_AttribMod::initStructParamOT

void ServerObjectTemplate::_AttribMod::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// ServerObjectTemplate::_AttribMod::setAsEmptyList

void ServerObjectTemplate::_AttribMod::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// ServerObjectTemplate::_AttribMod::setAppend

bool ServerObjectTemplate::_AttribMod::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// ServerObjectTemplate::_AttribMod::isAppend


int ServerObjectTemplate::_AttribMod::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// ServerObjectTemplate::_AttribMod::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerObjectTemplate::_AttribMod::load(Iff &file)
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
		if (strcmp(paramName, "target") == 0)
			m_target.loadFromIff(file);
		else if (strcmp(paramName, "value") == 0)
			m_value.loadFromIff(file);
		else if (strcmp(paramName, "time") == 0)
			m_time.loadFromIff(file);
		else if (strcmp(paramName, "timeAtValue") == 0)
			m_timeAtValue.loadFromIff(file);
		else if (strcmp(paramName, "decay") == 0)
			m_decay.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerObjectTemplate::_AttribMod::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerObjectTemplate::_AttribMod::save(Iff &file)
{
int count;

	file.insertForm(_AttribMod_tag);

	int paramCount = 0;

	// save target
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("target");
	m_target.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save value
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("value");
	m_value.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save time
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("time");
	m_time.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save timeAtValue
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("timeAtValue");
	m_timeAtValue.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save decay
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("decay");
	m_decay.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// ServerObjectTemplate::_AttribMod::save


//=============================================================================
// class ServerObjectTemplate::_Contents

/**
 * Class constructor.
 */
ServerObjectTemplate::_Contents::_Contents(const std::string & filename)
	: TpfTemplate(filename)
{
}	// ServerObjectTemplate::_Contents::_Contents

/**
 * Class destructor.
 */
ServerObjectTemplate::_Contents::~_Contents()
{
}	// ServerObjectTemplate::_Contents::~_Contents

/**
 * Static function used to register this template.
 */
void ServerObjectTemplate::_Contents::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_Contents_tag, create);
}	// ServerObjectTemplate::_Contents::registerMe

/**
 * Creates a ServerObjectTemplate::_Contents template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerObjectTemplate::_Contents::create(const std::string & filename)
{
	return new ServerObjectTemplate::_Contents(filename);
}	// ServerObjectTemplate::_Contents::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerObjectTemplate::_Contents::getId(void) const
{
	return _Contents_tag;
}	// ServerObjectTemplate::_Contents::getId

CompilerIntegerParam * ServerObjectTemplate::_Contents::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Contents::getCompilerIntegerParam

FloatParam * ServerObjectTemplate::_Contents::getFloatParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getFloatParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Contents::getFloatParam

BoolParam * ServerObjectTemplate::_Contents::getBoolParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "equipObject") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_equipObject;
		}
		fprintf(stderr, "trying to access single-parameter \"equipObject\" as an array\n");
	}
	else
		return TpfTemplate::getBoolParam(name, deepCheck, index);
	return nullptr;
}	//ServerObjectTemplate::_Contents::getBoolParam

StringParam * ServerObjectTemplate::_Contents::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "slotName") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_slotName;
		}
		fprintf(stderr, "trying to access single-parameter \"slotName\" as an array\n");
	}
	else if (strcmp(name, "content") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_content;
		}
		fprintf(stderr, "trying to access single-parameter \"content\" as an array\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return nullptr;
}	//ServerObjectTemplate::_Contents::getStringParam

StringIdParam * ServerObjectTemplate::_Contents::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Contents::getStringIdParam

VectorParam * ServerObjectTemplate::_Contents::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Contents::getVectorParam

DynamicVariableParam * ServerObjectTemplate::_Contents::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Contents::getDynamicVariableParam

StructParamOT * ServerObjectTemplate::_Contents::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerObjectTemplate::_Contents::getStructParamOT

TriggerVolumeParam * ServerObjectTemplate::_Contents::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Contents::getTriggerVolumeParam

void ServerObjectTemplate::_Contents::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// ServerObjectTemplate::_Contents::initStructParamOT

void ServerObjectTemplate::_Contents::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// ServerObjectTemplate::_Contents::setAsEmptyList

void ServerObjectTemplate::_Contents::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// ServerObjectTemplate::_Contents::setAppend

bool ServerObjectTemplate::_Contents::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// ServerObjectTemplate::_Contents::isAppend


int ServerObjectTemplate::_Contents::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// ServerObjectTemplate::_Contents::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerObjectTemplate::_Contents::load(Iff &file)
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
		if (strcmp(paramName, "slotName") == 0)
			m_slotName.loadFromIff(file);
		else if (strcmp(paramName, "equipObject") == 0)
			m_equipObject.loadFromIff(file);
		else if (strcmp(paramName, "content") == 0)
			m_content.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerObjectTemplate::_Contents::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerObjectTemplate::_Contents::save(Iff &file)
{
int count;

	file.insertForm(_Contents_tag);

	int paramCount = 0;

	// save slotName
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("slotName");
	m_slotName.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save equipObject
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("equipObject");
	m_equipObject.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save content
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("content");
	m_content.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// ServerObjectTemplate::_Contents::save


//=============================================================================
// class ServerObjectTemplate::_MentalStateMod

/**
 * Class constructor.
 */
ServerObjectTemplate::_MentalStateMod::_MentalStateMod(const std::string & filename)
	: TpfTemplate(filename)
{
}	// ServerObjectTemplate::_MentalStateMod::_MentalStateMod

/**
 * Class destructor.
 */
ServerObjectTemplate::_MentalStateMod::~_MentalStateMod()
{
}	// ServerObjectTemplate::_MentalStateMod::~_MentalStateMod

/**
 * Static function used to register this template.
 */
void ServerObjectTemplate::_MentalStateMod::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_MentalStateMod_tag, create);
}	// ServerObjectTemplate::_MentalStateMod::registerMe

/**
 * Creates a ServerObjectTemplate::_MentalStateMod template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerObjectTemplate::_MentalStateMod::create(const std::string & filename)
{
	return new ServerObjectTemplate::_MentalStateMod(filename);
}	// ServerObjectTemplate::_MentalStateMod::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerObjectTemplate::_MentalStateMod::getId(void) const
{
	return _MentalStateMod_tag;
}	// ServerObjectTemplate::_MentalStateMod::getId

CompilerIntegerParam * ServerObjectTemplate::_MentalStateMod::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "target") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_target;
		}
		fprintf(stderr, "trying to access single-parameter \"target\" as an array\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerObjectTemplate::_MentalStateMod::getCompilerIntegerParam

FloatParam * ServerObjectTemplate::_MentalStateMod::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "value") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_value;
		}
		fprintf(stderr, "trying to access single-parameter \"value\" as an array\n");
	}
	else if (strcmp(name, "time") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_time;
		}
		fprintf(stderr, "trying to access single-parameter \"time\" as an array\n");
	}
	else if (strcmp(name, "timeAtValue") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_timeAtValue;
		}
		fprintf(stderr, "trying to access single-parameter \"timeAtValue\" as an array\n");
	}
	else if (strcmp(name, "decay") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_decay;
		}
		fprintf(stderr, "trying to access single-parameter \"decay\" as an array\n");
	}
	else
		return TpfTemplate::getFloatParam(name, deepCheck, index);
	return nullptr;
}	//ServerObjectTemplate::_MentalStateMod::getFloatParam

BoolParam * ServerObjectTemplate::_MentalStateMod::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//ServerObjectTemplate::_MentalStateMod::getBoolParam

StringParam * ServerObjectTemplate::_MentalStateMod::getStringParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringParam(name, deepCheck, index);
}	//ServerObjectTemplate::_MentalStateMod::getStringParam

StringIdParam * ServerObjectTemplate::_MentalStateMod::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerObjectTemplate::_MentalStateMod::getStringIdParam

VectorParam * ServerObjectTemplate::_MentalStateMod::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//ServerObjectTemplate::_MentalStateMod::getVectorParam

DynamicVariableParam * ServerObjectTemplate::_MentalStateMod::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerObjectTemplate::_MentalStateMod::getDynamicVariableParam

StructParamOT * ServerObjectTemplate::_MentalStateMod::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerObjectTemplate::_MentalStateMod::getStructParamOT

TriggerVolumeParam * ServerObjectTemplate::_MentalStateMod::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerObjectTemplate::_MentalStateMod::getTriggerVolumeParam

void ServerObjectTemplate::_MentalStateMod::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// ServerObjectTemplate::_MentalStateMod::initStructParamOT

void ServerObjectTemplate::_MentalStateMod::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// ServerObjectTemplate::_MentalStateMod::setAsEmptyList

void ServerObjectTemplate::_MentalStateMod::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// ServerObjectTemplate::_MentalStateMod::setAppend

bool ServerObjectTemplate::_MentalStateMod::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// ServerObjectTemplate::_MentalStateMod::isAppend


int ServerObjectTemplate::_MentalStateMod::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// ServerObjectTemplate::_MentalStateMod::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerObjectTemplate::_MentalStateMod::load(Iff &file)
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
		if (strcmp(paramName, "target") == 0)
			m_target.loadFromIff(file);
		else if (strcmp(paramName, "value") == 0)
			m_value.loadFromIff(file);
		else if (strcmp(paramName, "time") == 0)
			m_time.loadFromIff(file);
		else if (strcmp(paramName, "timeAtValue") == 0)
			m_timeAtValue.loadFromIff(file);
		else if (strcmp(paramName, "decay") == 0)
			m_decay.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerObjectTemplate::_MentalStateMod::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerObjectTemplate::_MentalStateMod::save(Iff &file)
{
int count;

	file.insertForm(_MentalStateMod_tag);

	int paramCount = 0;

	// save target
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("target");
	m_target.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save value
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("value");
	m_value.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save time
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("time");
	m_time.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save timeAtValue
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("timeAtValue");
	m_timeAtValue.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save decay
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("decay");
	m_decay.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// ServerObjectTemplate::_MentalStateMod::save


//=============================================================================
// class ServerObjectTemplate::_Xp

/**
 * Class constructor.
 */
ServerObjectTemplate::_Xp::_Xp(const std::string & filename)
	: TpfTemplate(filename)
{
}	// ServerObjectTemplate::_Xp::_Xp

/**
 * Class destructor.
 */
ServerObjectTemplate::_Xp::~_Xp()
{
}	// ServerObjectTemplate::_Xp::~_Xp

/**
 * Static function used to register this template.
 */
void ServerObjectTemplate::_Xp::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_Xp_tag, create);
}	// ServerObjectTemplate::_Xp::registerMe

/**
 * Creates a ServerObjectTemplate::_Xp template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerObjectTemplate::_Xp::create(const std::string & filename)
{
	return new ServerObjectTemplate::_Xp(filename);
}	// ServerObjectTemplate::_Xp::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerObjectTemplate::_Xp::getId(void) const
{
	return _Xp_tag;
}	// ServerObjectTemplate::_Xp::getId

CompilerIntegerParam * ServerObjectTemplate::_Xp::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "type") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_type;
		}
		fprintf(stderr, "trying to access single-parameter \"type\" as an array\n");
	}
	else if (strcmp(name, "level") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_level;
		}
		fprintf(stderr, "trying to access single-parameter \"level\" as an array\n");
	}
	else if (strcmp(name, "value") == 0)
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
}	//ServerObjectTemplate::_Xp::getCompilerIntegerParam

FloatParam * ServerObjectTemplate::_Xp::getFloatParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getFloatParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Xp::getFloatParam

BoolParam * ServerObjectTemplate::_Xp::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Xp::getBoolParam

StringParam * ServerObjectTemplate::_Xp::getStringParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Xp::getStringParam

StringIdParam * ServerObjectTemplate::_Xp::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Xp::getStringIdParam

VectorParam * ServerObjectTemplate::_Xp::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Xp::getVectorParam

DynamicVariableParam * ServerObjectTemplate::_Xp::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Xp::getDynamicVariableParam

StructParamOT * ServerObjectTemplate::_Xp::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerObjectTemplate::_Xp::getStructParamOT

TriggerVolumeParam * ServerObjectTemplate::_Xp::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerObjectTemplate::_Xp::getTriggerVolumeParam

void ServerObjectTemplate::_Xp::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// ServerObjectTemplate::_Xp::initStructParamOT

void ServerObjectTemplate::_Xp::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// ServerObjectTemplate::_Xp::setAsEmptyList

void ServerObjectTemplate::_Xp::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// ServerObjectTemplate::_Xp::setAppend

bool ServerObjectTemplate::_Xp::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// ServerObjectTemplate::_Xp::isAppend


int ServerObjectTemplate::_Xp::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// ServerObjectTemplate::_Xp::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerObjectTemplate::_Xp::load(Iff &file)
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
		else if (strcmp(paramName, "level") == 0)
			m_level.loadFromIff(file);
		else if (strcmp(paramName, "value") == 0)
			m_value.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerObjectTemplate::_Xp::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerObjectTemplate::_Xp::save(Iff &file)
{
int count;

	file.insertForm(_Xp_tag);

	int paramCount = 0;

	// save type
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("type");
	m_type.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save level
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("level");
	m_level.saveToIff(file);
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
}	// ServerObjectTemplate::_Xp::save

//@END TFD
