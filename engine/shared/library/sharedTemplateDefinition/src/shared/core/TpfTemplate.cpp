//========================================================================
//
// TpfTemplate.cpp - adds extra functionality to Template for the compiler
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedTemplateDefinition/FirstSharedTemplateDefinition.h"
#include "sharedTemplateDefinition/TpfTemplate.h"

#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/TpfFile.h"


//========================================================================
// member functions

/**
 * Class constructor.
 */
TpfTemplate::TpfTemplate(const std::string & filename) :
	ObjectTemplate(filename),
	m_parentFile(nullptr),
	m_baseTemplateName(),
	m_baseTemplateFile(nullptr)
{
}	// TpfTemplate::TpfTemplate

/**
 * Class destructor.
 */
TpfTemplate::~TpfTemplate()
{
	m_parentFile = nullptr;
	m_baseTemplateName.clear();
	if (m_baseTemplateFile != nullptr)
	{
		delete m_baseTemplateFile;
		m_baseTemplateFile = nullptr;
	}
}	// TpfTemplate::~TpfTemplate

/**
 * Sets the name of the base template for this template.
 *
 * @param name		the base template name
 *
 * @return 0 on success, error code on error
 */
int TpfTemplate::setBaseTemplateName(const std::string & name)
{
	m_baseTemplateName = name;
	
	if (m_parentFile == nullptr)
		return -1;

	// load the base template
	if (m_baseTemplateFile != nullptr)
	{
		delete m_baseTemplateFile;
		m_baseTemplateFile = nullptr;
	}

	Filename baseTemplateFileName(nullptr, nullptr, name.c_str(), TEMPLATE_EXTENSION);
	Filename sourceTpfPath(nullptr, m_parentFile->getTpfPath().c_str(), nullptr, nullptr);
	Filename tempPath(baseTemplateFileName);

	tempPath.setPath(sourceTpfPath.getPath().c_str());
	tempPath.appendPath(baseTemplateFileName);
	tempPath.setDrive(m_parentFile->m_fp.getFilename().getDrive().c_str());

	File fp;
	int i = 0;
	while (!fp.exists(tempPath) && i < MAX_DIRECTORY_DEPTH)
	{
		sourceTpfPath.appendPath(NEXT_HIGHER_PATH);
		tempPath.setPath(sourceTpfPath.getPath().c_str());
		tempPath.appendPath(baseTemplateFileName);
		++i;
	}
	if (i == MAX_DIRECTORY_DEPTH)
		return -1;

	m_baseTemplateFile = new TpfFile;
	return m_baseTemplateFile->loadTemplate(tempPath);
}	// TpfTemplate::setBaseTemplateName

/**
 * Returns the template this template is derived from.
 *
 * @return the base template
 */
TpfTemplate * TpfTemplate::getBaseTemplate(void) const
{
	if (m_baseTemplateFile != nullptr)
		return m_baseTemplateFile->getTemplate();
	return nullptr;
}	// TpfTemplate::getBaseTemplate

/**
 * Saves the template to an iff file.
 *
 * @param file			file to save to
 */
void TpfTemplate::save(Iff &file)
{
	UNREF(file);
}	// TpfTemplate::save

/**
 * Parameter accessor function for derived classes.
 *
 * @param name			name of the parameter to get
 * @param deepcheck		flag to check base template if the parameter isn't loaded
 * @param index			array index of the parameter
 *
 * @return the parameter, or nullptr if not found
 */
CompilerIntegerParam *TpfTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	UNREF(name);
	UNREF(deepCheck);
	UNREF(index);
	return nullptr;
}	// TpfTemplate::getCompilerIntegerParam

/**
 * Parameter accessor function for derived classes.
 *
 * @param name			name of the parameter to get
 * @param deepcheck		flag to check base template if the parameter isn't loaded
 * @param index			array index of the parameter
 *
 * @return the parameter, or nullptr if not found
 */
FloatParam *TpfTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	UNREF(name);
	UNREF(deepCheck);
	UNREF(index);
	return nullptr;
}	// TpfTemplate::getFloatParam

/**
 * Parameter accessor function for derived classes.
 *
 * @param name			name of the parameter to get
 * @param deepcheck		flag to check base template if the parameter isn't loaded
 * @param index			array index of the parameter
 *
 * @return the parameter, or nullptr if not found
 */
BoolParam *TpfTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	UNREF(name);
	UNREF(deepCheck);
	UNREF(index);
	return nullptr;
}	// TpfTemplate::getBoolParam

/**
 * Parameter accessor function for derived classes.
 *
 * @param name			name of the parameter to get
 * @param deepcheck		flag to check base template if the parameter isn't loaded
 * @param index			array index of the parameter
 *
 * @return the parameter, or nullptr if not found
 */
StringParam *TpfTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	UNREF(name);
	UNREF(deepCheck);
	UNREF(index);
	return nullptr;
}	// TpfTemplate::getStringParam

/**
 * Parameter accessor function for derived classes.
 *
 * @param name			name of the parameter to get
 * @param deepcheck		flag to check base template if the parameter isn't loaded
 * @param index			array index of the parameter
 *
 * @return the parameter, or nullptr if not found
 */
StringIdParam *TpfTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	UNREF(name);
	UNREF(deepCheck);
	UNREF(index);
	return nullptr;
}	// TpfTemplate::getStringIdParam

/**
 * Parameter accessor function for derived classes.
 *
 * @param name			name of the parameter to get
 * @param deepcheck		flag to check base template if the parameter isn't loaded
 * @param index			array index of the parameter
 *
 * @return the parameter, or nullptr if not found
 */
VectorParam *TpfTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	UNREF(name);
	UNREF(deepCheck);
	UNREF(index);
	return nullptr;
}	// TpfTemplate::getVectorParam

/**
 * Parameter accessor function for derived classes.
 *
 * @param name			name of the parameter to get
 * @param deepcheck		flag to check base template if the parameter isn't loaded
 * @param index			array index of the parameter
 *
 * @return the parameter, or nullptr if not found
 */
DynamicVariableParam *TpfTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	UNREF(name);
	UNREF(deepCheck);
	UNREF(index);
	return nullptr;
}	// TpfTemplate::getDynamicVariableParam

/**
 * Parameter accessor function for derived classes.
 *
 * @param name			name of the parameter to get
 * @param deepcheck		flag to check base template if the parameter isn't loaded
 * @param index			array index of the parameter
 *
 * @return the parameter, or nullptr if not found
 */
StructParamOT *TpfTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	UNREF(name);
	UNREF(deepCheck);
	UNREF(index);
	return nullptr;
}	// TpfTemplate::getStructParamOT

/**
 * Parameter accessor function for derived classes.
 *
 * @param name			name of the parameter to get
 * @param deepcheck		flag to check base template if the parameter isn't loaded
 * @param index			array index of the parameter
 *
 * @return the parameter, or nullptr if not found
 */
TriggerVolumeParam *TpfTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	UNREF(name);
	UNREF(deepCheck);
	UNREF(index);
	return nullptr;
}	// TpfTemplate::getTriggerVolumeParam

/**
 * Initializes a structure param with the right structure type.
 *
 * @param param		the structure param to initialize
 * @param name		name of the parameter to get
 */
void TpfTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	UNREF(param);
	UNREF(name);
}	// TpfTemplate::getStructParamOT

/**
 * Marks a list parameter as being an empty list.
 *
 * @param name		name of the parameter to set
 */
void TpfTemplate::setAsEmptyList(const char *name)
{
	UNREF(name);
}	// TpfTemplate::setAsEmptyList

/**
 * Sets the append flag for a list parameter.
 *
 * @param name		the parameter name
 */
void TpfTemplate::setAppend(const char *name)
{
	UNREF(name);
}	// TpfTemplate::setAppend

/**
 * Returns if the append flag is set for a list parameter.
 *
 * @param name		the parameter name
 *
 * @return the flag
 */
bool TpfTemplate::isAppend(const char *name) const
{
	UNREF(name);
	return false;
}	// TpfTemplate::isAppend

/**
 * Get the length of items in a list parameter.
 *
 * @param name		the parameter name
 *
 * @return number of items in the parameter list, or -1
 * if the item is not a list
 */
int TpfTemplate::getListLength(const char *name) const
{
	return -1;
	UNREF(name);
}	// TpfTemplate::getListLength

/**
 * Adds a param to the loaded list.
 *
 * @param name		the parameter name
 * @param index		the array index of the parameter
 */
void TpfTemplate::addLoadedParam(const std::string & name, int index)
{
	std::string indexedName;
	nameAndIndexToName(name, index, indexedName);
	m_loadedParams.insert(indexedName);
}	// TpfTemplate::addLoadedParam

/**
 * Tests if a param is loaded.
 *
 * @param name		the parameter name
 * @param index		the array index of the parameter
 *
 * @return true if the parameter is loaded, false if not
 */
bool TpfTemplate::isParamLoaded(const std::string & name, bool deepCheck, int index) const
{
	std::string indexedName;
	nameAndIndexToName(name, index, indexedName);
	return isParamLoadedLocal(indexedName, deepCheck);
}	// TpfTemplate::isParamLoaded

/**
 * Adds a param to the pure virtual list.
 *
 * @param name		the parameter name
 * @param index		the array index of the parameter
 */
void TpfTemplate::addPureVirtualParam(const std::string & name, int index)
{
	std::string indexedName;
	nameAndIndexToName(name, index, indexedName);
	m_pureVirtualParams.insert(indexedName);
}	// TpfTemplate::addPureVirtualParam

/**
 * Tests if a param is pure virtual (defined by @derived).
 *
 * @param name		the parameter name
 * @param index		the array index of the parameter
 *
 * @return true if the parameter is pure virtual, false if not
 */
bool TpfTemplate::isParamPureVirtual(const std::string & name, bool deepCheck, int index) const
{
	std::string indexedName;
	nameAndIndexToName(name, index, indexedName);
	return isParamPureVirtualLocal(indexedName, deepCheck);
}	// TpfTemplate::isParamPureVirtual

/**
 * Converts a parameter name and index to a single name.
 *
 * @param name			the paramter name
 * @param index			the parameter array index
 * @param indexedName	string for the modified name
 */
void TpfTemplate::nameAndIndexToName(const std::string & name, int index, 
	std::string & indexedName) const
{
	char buffer[16];
	_itoa(index, buffer, 10);
	indexedName = name + "[";
	indexedName += buffer;
}	// TpfTemplate::nameAndIndexToName

/**
 * Tests if a param is loaded.
 *
 * @param name     the parameter name
 *
 * @return true if the parameter is loaded, false if not
 */
bool TpfTemplate::isParamLoadedLocal(const std::string &name, bool deepCheck) const
{
	bool result = false;
	std::set<std::string>::const_iterator i = m_loadedParams.find(name);

	if (i != m_loadedParams.end())
	{
		result = true;
	}
	else if (deepCheck && getBaseTemplate() != nullptr)
	{
		result = getBaseTemplate()->isParamLoadedLocal(name);
	}

	return result;
}	// TpfTemplate::isParamLoadedLocal

/**
 * Tests if a param is pure virtual (defined by @derived).
 *
 * @param name     the parameter name
 *
 * @return true if the parameter is pure virtual, false if not
 */
bool TpfTemplate::isParamPureVirtualLocal(const std::string &name, bool deepCheck) const
{
	bool result = false;

	std::set<std::string>::const_iterator i = m_pureVirtualParams.find(name);

	if (i != m_pureVirtualParams.end())
	{
		result = true;
	}
	else if (deepCheck && (getBaseTemplate() != nullptr))
	{
		result = getBaseTemplate()->isParamPureVirtualLocal(name);
	}

	return result;
}	// TpfTemplate::isParamPureVirtualLocal
