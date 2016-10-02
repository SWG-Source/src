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

#include "sharedTemplate/FirstSharedTemplate.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TemplateGlobals.h"
#include "ServerArmorTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerArmorTemplate::ServerArmorTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: TpfTemplate(filename)
	,m_specialProtectionLoaded(false)
	,m_specialProtectionAppend(false)
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
CompilerIntegerParam * ServerArmorTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "rating") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_rating;
		}
		fprintf(stderr, "trying to access single-parameter \"rating\" as an array\n");
	}
	else if (strcmp(name, "integrity") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_integrity;
		}
		fprintf(stderr, "trying to access single-parameter \"integrity\" as an array\n");
	}
	else if (strcmp(name, "effectiveness") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_effectiveness;
		}
		fprintf(stderr, "trying to access single-parameter \"effectiveness\" as an array\n");
	}
	else if (strcmp(name, "vulnerability") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_vulnerability;
		}
		fprintf(stderr, "trying to access single-parameter \"vulnerability\" as an array\n");
	}
	else if (strcmp(name, "encumbrance") == 0)
	{
		if (index >= 0 && index < 3)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_encumbrance[index];
		}
		fprintf(stderr, "index for parameter \"encumbrance\" out of bounds\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerArmorTemplate::getCompilerIntegerParam

FloatParam * ServerArmorTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getFloatParam(name, deepCheck, index);
}	//ServerArmorTemplate::getFloatParam

BoolParam * ServerArmorTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//ServerArmorTemplate::getBoolParam

StringParam * ServerArmorTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringParam(name, deepCheck, index);
}	//ServerArmorTemplate::getStringParam

StringIdParam * ServerArmorTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerArmorTemplate::getStringIdParam

VectorParam * ServerArmorTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//ServerArmorTemplate::getVectorParam

DynamicVariableParam * ServerArmorTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerArmorTemplate::getDynamicVariableParam

StructParamOT * ServerArmorTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "specialProtection") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_specialProtection.size()))
			return m_specialProtection[index];
		if (index == static_cast<int>(m_specialProtection.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_specialProtection.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"specialProtection\" out of bounds\n");
	}
	else
		return TpfTemplate::getStructParamOT(name, deepCheck, index);
	return nullptr;
}	//ServerArmorTemplate::getStructParamOT

TriggerVolumeParam * ServerArmorTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerArmorTemplate::getTriggerVolumeParam

void ServerArmorTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	if (strcmp(name, "specialProtection") == 0)
		param.setValue(new _SpecialProtection(""));
	else
		TpfTemplate::initStructParamOT(param, name);
}	// ServerArmorTemplate::initStructParamOT

void ServerArmorTemplate::setAsEmptyList(const char *name)
{
	if (strcmp(name, "specialProtection") == 0)
	{
		m_specialProtection.clear();
		m_specialProtectionLoaded = true;
	}
	else
		TpfTemplate::setAsEmptyList(name);
}	// ServerArmorTemplate::setAsEmptyList

void ServerArmorTemplate::setAppend(const char *name)
{
	if (strcmp(name, "specialProtection") == 0)
		m_specialProtectionAppend = true;
	else
		TpfTemplate::setAppend(name);
}	// ServerArmorTemplate::setAppend

bool ServerArmorTemplate::isAppend(const char *name) const
{
	if (strcmp(name, "specialProtection") == 0)
		return m_specialProtectionAppend;
	else
		return TpfTemplate::isAppend(name);
}	// ServerArmorTemplate::isAppend


int ServerArmorTemplate::getListLength(const char *name) const
{
	if (strcmp(name, "specialProtection") == 0)
	{
		return m_specialProtection.size();
	}
	else if (strcmp(name, "encumbrance") == 0)
	{
		return sizeof(m_encumbrance) / sizeof(CompilerIntegerParam);
	}
	else
		return TpfTemplate::getListLength(name);
}	// ServerArmorTemplate::getListLength

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
	if (getHighestTemplateVersion() != TAG(0,0,0,1))
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
				CompilerIntegerParam dummy;
				dummy.loadFromIff(file);
			}
		}
		file.exitChunk(true);
	}

	file.exitForm();
	TpfTemplate::load(file);
	file.exitForm();
	return;
}	// ServerArmorTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerArmorTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerArmorTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,0,1));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	// save rating
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("rating");
	m_rating.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save integrity
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("integrity");
	m_integrity.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save effectiveness
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("effectiveness");
	m_effectiveness.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_specialProtectionLoaded)
	{
		// mark the list as empty and extending the base list
		m_specialProtectionAppend = true;
	}
	// save specialProtection
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("specialProtection");
	file.insertChunkData(&m_specialProtectionAppend, sizeof(bool));
	count = m_specialProtection.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_specialProtection[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save vulnerability
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("vulnerability");
	m_vulnerability.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save encumbrance
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("encumbrance");
	count = 3;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 3; ++i)
		m_encumbrance[i].saveToIff(file);}
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
}	// ServerArmorTemplate::save


//=============================================================================
// class ServerArmorTemplate::_SpecialProtection

/**
 * Class constructor.
 */
ServerArmorTemplate::_SpecialProtection::_SpecialProtection(const std::string & filename)
	: TpfTemplate(filename)
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

CompilerIntegerParam * ServerArmorTemplate::_SpecialProtection::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
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
	else if (strcmp(name, "effectiveness") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != nullptr)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return nullptr;
			}
			return &m_effectiveness;
		}
		fprintf(stderr, "trying to access single-parameter \"effectiveness\" as an array\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return nullptr;
}	//ServerArmorTemplate::_SpecialProtection::getCompilerIntegerParam

FloatParam * ServerArmorTemplate::_SpecialProtection::getFloatParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getFloatParam(name, deepCheck, index);
}	//ServerArmorTemplate::_SpecialProtection::getFloatParam

BoolParam * ServerArmorTemplate::_SpecialProtection::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//ServerArmorTemplate::_SpecialProtection::getBoolParam

StringParam * ServerArmorTemplate::_SpecialProtection::getStringParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringParam(name, deepCheck, index);
}	//ServerArmorTemplate::_SpecialProtection::getStringParam

StringIdParam * ServerArmorTemplate::_SpecialProtection::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerArmorTemplate::_SpecialProtection::getStringIdParam

VectorParam * ServerArmorTemplate::_SpecialProtection::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//ServerArmorTemplate::_SpecialProtection::getVectorParam

DynamicVariableParam * ServerArmorTemplate::_SpecialProtection::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerArmorTemplate::_SpecialProtection::getDynamicVariableParam

StructParamOT * ServerArmorTemplate::_SpecialProtection::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerArmorTemplate::_SpecialProtection::getStructParamOT

TriggerVolumeParam * ServerArmorTemplate::_SpecialProtection::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerArmorTemplate::_SpecialProtection::getTriggerVolumeParam

void ServerArmorTemplate::_SpecialProtection::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// ServerArmorTemplate::_SpecialProtection::initStructParamOT

void ServerArmorTemplate::_SpecialProtection::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// ServerArmorTemplate::_SpecialProtection::setAsEmptyList

void ServerArmorTemplate::_SpecialProtection::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// ServerArmorTemplate::_SpecialProtection::setAppend

bool ServerArmorTemplate::_SpecialProtection::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// ServerArmorTemplate::_SpecialProtection::isAppend


int ServerArmorTemplate::_SpecialProtection::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// ServerArmorTemplate::_SpecialProtection::getListLength

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

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerArmorTemplate::_SpecialProtection::save(Iff &file)
{
int count;

	file.insertForm(_SpecialProtection_tag);

	int paramCount = 0;

	// save type
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("type");
	m_type.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save effectiveness
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("effectiveness");
	m_effectiveness.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// ServerArmorTemplate::_SpecialProtection::save

//@END TFD
