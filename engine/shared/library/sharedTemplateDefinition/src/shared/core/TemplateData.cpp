//========================================================================
//
// TemplateData.cpp - data for a given version of a template
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedTemplateDefinition/FirstSharedTemplateDefinition.h"
#include "TemplateGlobals.h"
#include "File.h"
#include "TemplateDefinitionFile.h"
#include "TemplateData.h"
#include "TpfTemplate.h"

#include <cstdio>

//==============================================================================
// constants

// states when parsing a parameter
enum ParamState
{
	STATE_LIST,
	STATE_TYPE,
	STATE_NAME,
	STATE_LIMITS,
	STATE_DESCRIPTION
};

// tags for min/max getters
static const char * const MinMaxNames[] = {"", "Min", "Max"};

static const bool HasMinMax[] =
{
	false,
	false,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false
};

// map enum ParamType to access function return value
static const char * const PaddedDataMethodNames[] =
{
	nullptr,
	nullptr,
	"int                   ",
	"float                 ",
	"bool                  ",
	"const std::string &   ",
	"const StringId        ",
	"const Vector &        ",
	"void                  ",
	"const ObjectTemplate *",
	nullptr,
	nullptr,
	"const TriggerVolumeData &",
	"const std::string &   "
};
static const char * const UnpaddedDataMethodNames[] =
{
	nullptr,
	nullptr,
	"int",
	"float",
	"bool",
	"const std::string &",
	"const StringId",
	"const Vector &",
	nullptr,
	"const ObjectTemplate *",
	nullptr,
	nullptr,
	"const TriggerVolumeData &",
	"const std::string &"
};

// map enum ParamType to struct storage type
static const char * const PaddedDataStructNames[] =
{
	nullptr,
	nullptr,
	"int                   ",
	"float                 ",
	"bool                  ",
	"std::string           ",
	"StringId              ",
	"Vector                ",
	"DynamicVariableList   ",
	"const ObjectTemplate *",
	nullptr,
	nullptr,
	"TriggerVolumeData     ",
	"std::string           "
};
static const char * const UnpaddedDataStructNames[] =
{
	nullptr,
	nullptr,
	"int",
	"float",
	"bool",
	"std::string",
	"StringId",
	"Vector",
	"DynamicVariableList",
	"const ObjectTemplate *",
	nullptr,
	nullptr,
	"TriggerVolumeData",
	"std::string"
};

// map enum ParamType to TemplateParam type
static const char * const DataVariableNames[] =
{
	"",
	"",
	"IntegerParam",
	"FloatParam",
	"BoolParam",
	"StringParam",
	"StringIdParam",
	"VectorParam",
	"DynamicVariableParam",
	"StringParam",
	"IntegerParam",
	"StructParamOT",
	"TriggerVolumeParam",
	"StringParam"
};

static const char * const CompilerDataVariableNames[] =
{
	"",
	"",
	"CompilerIntegerParam",
	"FloatParam",
	"BoolParam",
	"StringParam",
	"StringIdParam",
	"VectorParam",
	"DynamicVariableParam",
	"StringParam",
	"CompilerIntegerParam",
	"StructParamOT",
	"TriggerVolumeParam",
	"StringParam"
};

static const char * const DefaultDataReturnValue[] =
{
	nullptr,
	nullptr,
	"0",
	"0.0f",
	"false",
	"DefaultString",
	"DefaultStringId",
	"DefaultVector",
	"",
	"nullptr",
	"(0)",
	"",
	"DefaultTriggerVolumeData",
	"DefaultString"
};

//==============================================================================
// class methods

/**
 * Class constructor.
 *
 * @param version		version number of this template data
 * @param parent		the tdf file this data was taken from
 */
TemplateData::TemplateData(int version, const TemplateDefinitionFile &parent) :
	m_fileParent(&parent),
	m_templateParent(nullptr),
	m_hasTemplateParam(false),
	m_hasDynamicVarParam(false),
	m_hasList(false),
//	m_name(parent.getTemplateName()),
	m_baseName(parent.getBaseName()),
	m_writeForCompilerFlag(false),
	m_bracketCount(0),
	m_parseState(STATE_PARAM),
	m_parameters(),
	m_parameterMap(),
	m_currentEnumList(nullptr),
	m_enumMap(),
	m_currentStruct(nullptr),
	m_structMap(),
	m_structList()
{
	m_structId.tag = NO_TAG;
	m_structId.tagString = NO_TAG_STRING;
	m_version.tag = version;
	char versionbuf[5];
	char buffer[16];
	sprintf(versionbuf, "%04d", version);
	sprintf(buffer, "TAG(%c,%c,%c,%c)", versionbuf[0], versionbuf[1],
		versionbuf[2], versionbuf[3]);
	m_version.tagString = buffer;
}	// TemplateData::TemplateData(int, const TemplateDefinitionFile &)

/**
 * Class constructor (for a structure in a template).
 *
 * @param parent	the template data this structure belongs to
 * @param name		the structure's name
 */
TemplateData::TemplateData(const TemplateData *parent, const std::string &name) :
	m_fileParent(nullptr),
	m_templateParent(parent),
	m_hasTemplateParam(false),
	m_hasDynamicVarParam(false),
	m_hasList(false),
	m_name(name),
	m_baseName(""),
	m_writeForCompilerFlag(false),
	m_version(parent->m_version),
	m_bracketCount(0),
	m_parseState(STATE_PARAM),
	m_parameters(),
	m_parameterMap(),
	m_currentEnumList(nullptr),
	m_enumMap(),
	m_currentStruct(nullptr),
	m_structMap(),
	m_structList()
{
	m_structId.tag = NO_TAG;
	m_structId.tagString = NO_TAG_STRING;
}	// TemplateData::TemplateData(const TemplateData &)

/**
 * Class destructor.
 */
TemplateData::~TemplateData()
{
	StructMap::iterator iter;
	for (iter = m_structMap.begin(); iter != m_structMap.end(); ++iter)
	{
		delete (*iter).second;
		(*iter).second = nullptr;
	}

	m_parameterMap.clear();
	m_parameters.clear();
	m_structMap.clear();
	m_structList.clear();
	m_currentEnumList = nullptr;
	m_currentStruct = nullptr;
}	// TemplateData::~TemplateData


//========================================================================
// access functions

/**
 * Returns the template name for this data.
 *
 * @return the template name
 */
const std::string TemplateData::getName(void) const
{
	if (m_fileParent != nullptr)
		return m_fileParent->getTemplateName();
	if (m_templateParent != nullptr)
		return m_templateParent->getName() + "::_" + m_name;
	return "";
}

/**
 * Returns the template's base template name.
 *
 * @return the base template name
 */
const std::string TemplateData::getBaseName(void) const
{
	if (m_fileParent != nullptr && !m_fileParent->getBaseName().empty())
		return m_fileParent->getBaseName();
//	if (m_templateParent != nullptr)
		return m_baseName;
//	return "";
}

/**
 * Returns the template's location (client, server, shared)
 *
 * @return the location
 */
TemplateLocation TemplateData::getTemplateLocation(void) const
{
	if (m_fileParent != nullptr)
		return m_fileParent->getTemplateLocation();
	if (m_templateParent != nullptr)
		return m_templateParent->getTemplateLocation();
	return LOC_NONE;
}	// TemplateData::getTemplateLocation


//========================================================================
// parsing functions

/**
 * Parses a line of a template definition.
 *
 * @param fp			file line was read from
 * @param buffer		the line to parse
 * @param tokenbuf		a buffer to store tokens in
 *
 * @return point in buffer where parsing stopped, or -1 on error
 */
const char * TemplateData::parseLine(const File &fp, const char *buffer,
	char *tokenbuf)
{
ParamState paramState = STATE_LIST;

	if (buffer == nullptr || *buffer == '\0')
		return nullptr;

	const char *line = buffer;

	if (m_parseState == STATE_ENUM)
		return parseEnum(fp, line, tokenbuf);
	else if (m_parseState == STATE_STRUCT)
		return parseStruct(fp, line, tokenbuf);

	Parameter parameter;
	parameter.type = TYPE_NONE;
	parameter.list_type = LIST_NONE;
	parameter.list_size = 1;
	parameter.min_int_limit = INT_MIN;
	parameter.min_float_limit = -FLT_MAX;
	parameter.max_int_limit = INT_MAX;
	parameter.max_float_limit = FLT_MAX;

	if (*buffer == '/' && *(buffer + 1) == '/')
	{
		parameter.type = TYPE_COMMENT;
		paramState = STATE_DESCRIPTION;
	}

	while (paramState != STATE_DESCRIPTION)
	{
		line = getNextWhitespaceToken(line, tokenbuf);
		if (*tokenbuf == '\0')
		{
			fp.printEolError();
			return CHAR_ERROR;
		}

		if (paramState == STATE_LIST)
		{
			// check for alternative options to defining a parameter
			if (strcmp(tokenbuf, "enum") == 0)
			{
				if (m_parseState == STATE_PARAM)
				{
					// get the enum name
					line = getNextWhitespaceToken(line, tokenbuf);
					if (*tokenbuf == '\0')
					{
						fp.printError("no enum name");
						return CHAR_ERROR;
					}
					if (!isalpha(*tokenbuf))
					{
						fp.printError("bad enum name");
						return CHAR_ERROR;
					}
					const EnumList * enumList = getEnumList(tokenbuf, true);
					if (enumList != nullptr)
					{
						fp.printError("enum already defined");
						return CHAR_ERROR;
					}
					m_currentEnumList = &(*m_enumMap.insert(std::make_pair(
						std::string(tokenbuf), EnumList())).first).second;
					m_parseState = STATE_ENUM;
					if (line != nullptr && *line != '\0')
						return parseEnum(fp, line, tokenbuf);
					return line;
				}
				fp.printError("unexpected keyword \"enum\"");
				return CHAR_ERROR;
			}
			else if (strcmp(tokenbuf, "struct") == 0)
			{
				if (m_parseState == STATE_PARAM)
				{
					// get the struct name
					line = getNextWhitespaceToken(line, tokenbuf);
					if (*tokenbuf == '\0')
					{
						fp.printError("no struct name");
						return CHAR_ERROR;
					}
					if (!isalpha(*tokenbuf))
					{
						fp.printError("bad struct name");
						return CHAR_ERROR;
					}
					StructMap::const_iterator structIter = m_structMap.find(tokenbuf);
					if (structIter != m_structMap.end())
					{
						fp.printError("struct already defined");
						return CHAR_ERROR;
					}
					m_currentStruct = new TemplateData(this, tokenbuf);
					m_currentStruct->m_name = tokenbuf;
					m_structMap.insert(std::make_pair(tokenbuf, m_currentStruct));
					m_structList.push_back(m_currentStruct);
					m_parseState = STATE_STRUCT;
					if (line != nullptr && *line != '\0')
						return parseStruct(fp, line, tokenbuf);
					return line;
				}
				fp.printError("unexpected keyword \"struct\"");
				return CHAR_ERROR;
			}
			// if we are a structure, the 1st item should be the structure id
			else if (strcmp(tokenbuf, "id") == 0 && m_templateParent != nullptr &&
				m_structId.tag == NO_TAG)
			{
				line = getNextToken(line, tokenbuf);
				if (strlen(tokenbuf) != 4)
				{
					fp.printError("struct id not 4 characters");
					return CHAR_ERROR;
				}
				m_structId.tag = ConvertStringToTag(tokenbuf);
				m_structId.tagString = ConvertStringToTagString(tokenbuf);
				return line;
			}
		}

		// if we are a structure, the 1st item should be the structure id
		if (m_templateParent != nullptr && m_structId.tag == NO_TAG)
		{
			fp.printError("struct id not defined");
			return CHAR_ERROR;
		}

		switch (paramState)
		{
			case STATE_LIST:
				if (strcmp(tokenbuf, "list") == 0)
				{
					// see what kind of list it is
					const char *tempLine = getNextWhitespaceToken(line, tokenbuf);
					if (*tokenbuf == '\0')
					{
						fp.printEolError();
						return CHAR_ERROR;
					}
					if (isdigit(*tokenbuf))
					{
						parameter.list_type = LIST_INT_ARRAY;
						parameter.list_size = atoi(tokenbuf);
						line = tempLine;
					}
					else if (strncmp(tokenbuf, "enumList", 8) == 0)
					{
						parameter.list_type = LIST_ENUM_ARRAY;
						parameter.enum_list_name = &tokenbuf[8];
						const EnumList * list = getEnumList(parameter.enum_list_name.c_str(), false);
						if (list == nullptr)
						{
							fp.printError("enum name not defined!");
							return CHAR_ERROR;
						}
						parameter.list_size = static_cast<int>(list->size());
						line = tempLine;
					}
					else
					{
						parameter.list_type = LIST_LIST;
						m_hasList = true;
					}
					paramState = STATE_TYPE;
					break;
				}
				// fall through to STATE_TYPE
			case STATE_TYPE:
				if (strcmp(tokenbuf, "int") == 0)
				{
					parameter.type = TYPE_INTEGER;
					parameter.min_int_limit = INT_MIN;
					parameter.max_int_limit = INT_MAX;
					paramState = STATE_LIMITS;
				}
				else if (strcmp(tokenbuf, "float") == 0)
				{
					parameter.type = TYPE_FLOAT;
					parameter.min_float_limit = -FLT_MAX;
					parameter.max_float_limit = FLT_MAX;
					paramState = STATE_LIMITS;
				}
				else if (strcmp(tokenbuf, "bool") == 0)
				{
					parameter.type = TYPE_BOOL;
					paramState = STATE_LIMITS;
				}
				else if (strcmp(tokenbuf, "string") == 0)
				{
					parameter.type = TYPE_STRING;
					paramState = STATE_NAME;
				}
				else if (strcmp(tokenbuf, "filename") == 0)
				{
					parameter.type = TYPE_FILENAME;
					paramState = STATE_NAME;
				}
				else if (strcmp(tokenbuf, "stringId") == 0)
				{
					parameter.type = TYPE_STRINGID;
					paramState = STATE_NAME;
				}
				else if (strcmp(tokenbuf, "vector") == 0)
				{
					parameter.type = TYPE_VECTOR;
					paramState = STATE_NAME;
				}
				else if (strcmp(tokenbuf, "objvar") == 0)
				{
					if (parameter.list_type == LIST_LIST)
					{
						fp.printError("objvar may not be part of a list");
						return CHAR_ERROR;
					}
					if (parameter.type == TYPE_DYNAMIC_VAR)
					{
						fp.printError("server-only data type");
						return CHAR_ERROR;
					}
					parameter.type = TYPE_DYNAMIC_VAR;
					m_hasDynamicVarParam = true;
					paramState = STATE_NAME;
				}
				else if (strncmp(tokenbuf, "template", 8) == 0)
				{
					parameter.type = TYPE_TEMPLATE;
					parameter.extendedName = &tokenbuf[8];
					m_hasTemplateParam = true;
					paramState = STATE_NAME;
				}
				else if (strncmp(tokenbuf, "enum", 4) == 0)
				{
					parameter.type = TYPE_ENUM;
					parameter.extendedName = &tokenbuf[4];
					if (getEnumList(&tokenbuf[4], false) == nullptr)
					{
						std::string errbuf = "enum type " + parameter.extendedName +
							" not defined";
						fp.printError(errbuf.c_str());
						return CHAR_ERROR;
					}
					paramState = STATE_NAME;
				}
				else if (strncmp(tokenbuf, "struct", 6) == 0)
				{
					parameter.type = TYPE_STRUCT;
					parameter.extendedName = &tokenbuf[6];
					if (getStruct(&tokenbuf[6]) == nullptr)
					{
						std::string errbuf = "struct " + parameter.extendedName +
							" not defined";
						fp.printError(errbuf.c_str());
						return CHAR_ERROR;
					}
					paramState = STATE_NAME;
				}
				else if (strcmp(tokenbuf, "triggerVolume") == 0)
				{
					parameter.type = TYPE_TRIGGER_VOLUME;
					parameter.min_float_limit = 0.0f;
					paramState = STATE_NAME;
				}
				else
				{
					if (paramState == STATE_LIST)
						fp.printError("Expected \"list\" or parameter type");
					else
						fp.printError("Expected parameter type");
					return CHAR_ERROR;
				}
				break;
			case STATE_LIMITS:
				if (isdigit(*tokenbuf) || *tokenbuf == '-' || (*tokenbuf == '.' && *(tokenbuf + 1) == '.'))
				{
					char tempBuf[64];
					const char *tempToken = tokenbuf;

					if (isdigit(*tempToken) || *tempToken == '-')
					{
						tempToken = getNextToken(tempToken, tempBuf);
						if (parameter.type == TYPE_INTEGER)
						{
							parameter.min_int_limit = strtol(tempBuf, nullptr, 10);
						}
						else
						{
							parameter.min_float_limit = static_cast<float>(
								strtod(tempBuf, nullptr));
						}
					}
					if (*tempToken == '.' && *(tempToken + 1) == '.')
					{
						tempToken = getNextToken(tempToken + 2, tempBuf);
						if (isdigit(*tempBuf) || *tempBuf == '-')
						{
							if (parameter.type == TYPE_INTEGER)
								parameter.max_int_limit = atoi(tempBuf);
							else
							{
								parameter.max_float_limit = static_cast<float>(
									atof(tempBuf));
							}
						}
					}
					paramState = STATE_NAME;
					break;
				}
				// fall through to STATE_NAME
			case STATE_NAME:
				if (*tokenbuf == '_')
				{
					fp.printError("parameter names may not begin with '_'");
					return CHAR_ERROR;
				}
				parameter.name = tokenbuf;
				paramState = STATE_DESCRIPTION;
				break;
			case STATE_DESCRIPTION:
			default:
				break;
		}
	}

	// anything left over in the line is the parameter description
	if (line != nullptr)
		parameter.description = line;

	m_parameters.push_back(parameter);
	m_parameterMap.insert(std::make_pair(parameter.name, parameter));

	return line;
}	// TemplateData::parseLine

/**
 * Finds the integral value associated with an enum value.
 *
 * @param enumValue		the enumeration value
 *
 * @return the value, or INVALID_ENUM_RESULT if not found
 */
int TemplateData::getEnumValue(const char * enumValue) const
{
	NOT_NULL(enumValue);

	EnumMap::const_iterator mapIter;
	for (mapIter = m_enumMap.begin(); mapIter != m_enumMap.end(); ++mapIter)
	{
		const EnumList &elist = (*mapIter).second;
		EnumList::const_iterator listIter;
		for (listIter = elist.begin(); listIter != elist.end(); ++listIter)
		{
			if ((*listIter).name == enumValue)
				return (*listIter).value;
		}
	}

	if (m_templateParent != nullptr)
		return m_templateParent->getEnumValue(enumValue);
	if (m_fileParent != nullptr)
	{
		const TemplateDefinitionFile * baseFile = m_fileParent->getBaseDefinitionFile();
		if (baseFile != nullptr)
		{
			return baseFile->getTemplateData(baseFile->getHighestVersion())->
				getEnumValue(enumValue);
		}
	}
	return INVALID_ENUM_RESULT;
}	// TemplateData::getEnumValue(const char *)

/**
 * Finds the integral value associated with an enum value.
 *
 * @param enumType		the enumeration type
 * @param enumValue		the enumeration value
 *
 * @return the value, or INVALID_ENUM_RESULT if not found
 */
int TemplateData::getEnumValue(const std::string & enumType,
	const char * enumValue) const
{
	NOT_NULL(enumValue);

	const EnumList *elist = getEnumList(enumType.c_str(), false);
	if (elist == nullptr)
		return INVALID_ENUM_RESULT;
	EnumList::const_iterator listIter;
	for (listIter = elist->begin(); listIter != elist->end(); ++listIter)
	{
		if ((*listIter).name == enumValue)
			return (*listIter).value;
	}
	return INVALID_ENUM_RESULT;
}	// TemplateData::getEnumValue(const std::string &, const char *)

/**
 * Tries to find an enum list definition for this template. If it can't find
 * one it will try to look for the definition in the template's base definition.
 *
 * @param name		the enum list name
 * @param define	flag that we are defining templates and should not look in base templates
 *
 * @return the enum list definition, or nullptr if not found
 */
const TemplateData::EnumList * TemplateData::getEnumList(const std::string & name,
	bool define) const
{
	EnumMap::const_iterator iter = m_enumMap.find(name);
	if (iter != m_enumMap.end())
		return &(*iter).second;
	if (m_templateParent != nullptr)
		return m_templateParent->getEnumList(name, define);
	if (!define && m_fileParent != nullptr && m_fileParent->getBaseDefinitionFile() != nullptr)
	{
		const TemplateData * baseData = m_fileParent->getBaseDefinitionFile()->
			getTemplateData(m_fileParent->getBaseDefinitionFile()->
			getHighestVersion());
		if (baseData != nullptr)
			return baseData->getEnumList(name, define);
	}
	return nullptr;
}	// TemplateData::getEnumList

/**
 * Tries to find a struct definition for this template. If it can't find one
 * it will try to look for the definition in the template's base definition.
 *
 * @param name		the enum list name
 *
 * @return the struct definition, or nullptr if not found
 */
const TemplateData * TemplateData::getStruct(const char *name) const
{
	NOT_NULL(name);

	StructMap::const_iterator iter = m_structMap.find(name);
	if (iter != m_structMap.end())
		return (*iter).second;
	if (m_templateParent != nullptr)
		return m_templateParent->getStruct(name);
	if (m_fileParent != nullptr && m_fileParent->getBaseDefinitionFile() != nullptr)
	{
		const TemplateData * baseData = m_fileParent->getBaseDefinitionFile()->
			getTemplateData(m_fileParent->getBaseDefinitionFile()->
			getHighestVersion());
		if (baseData != nullptr)
			return baseData->getStruct(name);
	}
	return nullptr;
}	// TemplateData::getStruct

/**
 * Returns the tdf file for this TemplateData
 *
 * @return the TemplateDefinitionFile, or nullptr if not there
 */

const TemplateDefinitionFile * TemplateData::getTdf() const
{
	return m_fileParent;
}

/**
 * Returns the tdf file for this TemplateData's first ancestor
 *
 * @return the TemplateDefinitionFile, or nullptr if not there
 */

const TemplateDefinitionFile * TemplateData::getTdfParent() const
{
	if(m_fileParent)
	{
		return m_fileParent->getBaseDefinitionFile();
	}
	else
	{
		return nullptr;
	}
}

/**
 * Returns the tdf file that contains the parameter
 *
 * @return the TemplateDefinitionFile, or nullptr if not found
 */

const TemplateDefinitionFile * TemplateData::getTdfForParameter(const char *parameterName) const
{
	if(m_fileParent != nullptr)
	{
		if(getParameter(parameterName))
		{
			return m_fileParent;
		}

		const TemplateDefinitionFile* ancestorTemplateDefinitionFile = m_fileParent->getBaseDefinitionFile();

		if(ancestorTemplateDefinitionFile != nullptr)
		{
			const TemplateData *ancestorTemplateData = ancestorTemplateDefinitionFile->getTemplateData(ancestorTemplateDefinitionFile->getHighestVersion());

			if(ancestorTemplateData != nullptr)
			{
				return ancestorTemplateData->getTdfForParameter(parameterName);
			}
		}
	}

	return nullptr;
}

/**
 * Parses a string and tries to convert it to an integer value.
 *
 * @patam fp			file to print errors out to
 * @param line			string to parse
 * @param endLine		pointer that will be set to where parsing stopped, or CHAR_ERROR on error
 * @param intbuf		buffer to be filled in with the string that represents the integer
 *
 * @return the integer that the string was parsed to
 */
int TemplateData::parseIntValue(const File &fp, const char * line, const char ** endLine,
	char * intbuf) const
{
	NOT_NULL(endLine);
	if (line == nullptr || *line == '\0' || intbuf == nullptr)
	{
		fp.printError("bad value passed to TemplateData::parseIntValue");
		*endLine = CHAR_ERROR;
		return 0;
	}

	int value = 0;
	const char * startLine = line;

	// check for negative number
	bool negative = false;
	if (*line == '-')
	{
		++line;
		negative = true;
		if (*line == '\0')
		{
			fp.printError("TemplateData::parseIntValue: expected value after '-'");
			*endLine = CHAR_ERROR;
			return 0;
		}
	}

	// an integer can be a normal value, a hex value starting with 0x, or an enum value
	// we can also have int value or'ed together with |
	if (isalpha(*line))
	{
		// see if it is an enum
		const char * tempLine = getNextToken(line, intbuf);
		value = getEnumValue(intbuf);
		if (value == INVALID_ENUM_RESULT)
		{
			fp.printError("TemplateData::parseIntValue: bad enum name");
			*endLine = CHAR_ERROR;
			return 0;
		}
		if (tempLine != nullptr)
			line = tempLine;
		else
			line += strlen(line);
	}
	else if (*line == '0' && *(line + 1) == 'x')
	{
		// a hex vale
		line += 2;
		char * tempLine;
		value = static_cast<int>(strtoul(line, &tempLine, 16));
		line = tempLine;
	}
	else if (isdigit(*line))
	{
		// a regular integer
		char * tempLine;
		value = static_cast<int>(strtol(line, &tempLine, 10));
		line = tempLine;
	}
	else
	{
		fp.printError("TemplateData::parseIntValue: data not an integer");
		*endLine = CHAR_ERROR;
		return 0;
	}

	if (negative)
		value = -value;

	// copy the int string to intbuf
	strncpy(intbuf, startLine, line - startLine);
	intbuf[line - startLine] = '\0';
	intbuf += strlen(intbuf);
	if (line != nullptr && *line == '\0')
		*endLine = nullptr;
	else
		*endLine = line;

	// check if we want to or another int
	const char * tempLine = getNextToken(line, intbuf);
	if (*intbuf == '|')
	{
		strcpy(intbuf, " | ");
		intbuf += strlen(intbuf);
		value |= parseIntValue(fp, tempLine, endLine, intbuf);
	}
	else
		*intbuf = '\0';

	return value;
}	// TemplateData::parseIntValue

/**
 * Parses a enumeration definition.
 *
 * @param fp			file line was read from
 * @param buffer		the line to parse
 * @param tokenbuf		a buffer to store tokens in
 *
 * @return point in buffer where parsing stopped, or -1 on error
 */
const char * TemplateData::parseEnum(const File &fp, const char *buffer,
	char *tokenbuf)
{
	NOT_NULL(buffer);
	NOT_NULL(tokenbuf);
	NOT_NULL(m_currentEnumList);

	const char *line = buffer;

	line = getNextToken(line, tokenbuf);
	if (*tokenbuf == '\0')
		return line;

	if (strcmp(tokenbuf, "{") == 0)
	{
		if (m_bracketCount == 0)
		{
			m_bracketCount = 1;
			return line;
		}
		fp.printError("unexpected {");
		return CHAR_ERROR;
	}
	else if (strcmp(tokenbuf, "}") == 0)
	{
		if (m_bracketCount == 1)
		{
			m_bracketCount = 0;
			m_currentEnumList = nullptr;
			m_parseState = STATE_PARAM;
			return line;
		}
	}
	else if (m_bracketCount == 0)
	{
		fp.printError("open bracket for enumeration not found");
		return CHAR_ERROR;
	}

	EnumData enumData;
	enumData.name = tokenbuf;
	if (line != nullptr && *line == '=')
	{

		line = getNextToken(line, tokenbuf);
		if (*tokenbuf == '\0')
		{
			fp.printError("expected value for enumeration after =");
			return CHAR_ERROR;
		}

		int value = parseIntValue(fp, line, &line, tokenbuf);
		if (line == CHAR_ERROR)
			return CHAR_ERROR;

//		if (!isinteger(tokenbuf))
//		{
//			fp.printError("expected integer value for enumeration");
//			return CHAR_ERROR;
//		}
		enumData.valueName = tokenbuf;
		enumData.value = value;
	}
	else if (m_currentEnumList->size() == 0)
	{
		enumData.value = 0;
	}
	else
	{
		enumData.value = m_currentEnumList->back().value + 1;
	}

	if (line != nullptr && *line == ',')
		line = getNextToken(line, tokenbuf);
	if (line != nullptr)
	{
		enumData.comment = line;
		line = nullptr;
	}

	m_currentEnumList->push_back(enumData);
	return line;
}	// TemplateData::parseEnum

/**
 * Parses a structure definition.
 *
 * @param fp			file line was read from
 * @param buffer		the line to parse
 * @param tokenbuf		a buffer to store tokens in
 *
 * @return point in buffer where parsing stopped, or -1 on error
 */
const char * TemplateData::parseStruct(const File &fp, const char *buffer,
	char *tokenbuf)
{
	NOT_NULL(buffer);
	NOT_NULL(tokenbuf);
	NOT_NULL(m_currentStruct);

	const char *line = buffer;

	line = getNextWhitespaceToken(line, tokenbuf);
	if (*tokenbuf == '\0')
		return line;

	if (strcmp(tokenbuf, "{") == 0)
	{
		if (m_bracketCount == 0)
			m_bracketCount = 1;
		else
		{
			fp.printError("unexpected {");
			return CHAR_ERROR;
		}
	}
	else if (strcmp(tokenbuf, "}") == 0)
	{
		if (m_bracketCount == 1)
		{
			m_bracketCount = 0;
			m_currentStruct = nullptr;
			m_parseState = STATE_PARAM;
			return line;
		}
		else
		{
			fp.printError("unexpected }");
			return CHAR_ERROR;
		}
	}
	else if (m_bracketCount == 0)
	{
		fp.printError("open bracket for struct not found");
		return CHAR_ERROR;
	}
	else
		line = buffer;

	return m_currentStruct->parseLine(fp, line, tokenbuf);
}	// TemplateData::parseStruct


//========================================================================
// template verification functions

/**
 * Verifies that a template has every necesssary member defined.
 *
 * @param tpfTemplate		the template to verify
 * @param file				the file the template is being read from
 *
 * @return true if the template has all it's members, false if some are missing
 */
bool TemplateData::verifyTemplate(const TpfTemplate *tpfTemplate, const File &file) const
{
bool result = true;
char buffer[256];

	NOT_NULL(tpfTemplate);

	const char * templateName = getFileParent()->getTemplateName().c_str();

	ParameterList::const_iterator iter;
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = *iter;
		if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
			continue;

		if (param.list_type == LIST_NONE)
		{
			if (tpfTemplate->isParamPureVirtual(param.name))
				continue;
			if (!tpfTemplate->isParamLoaded(param.name))
			{
				result = false;
				sprintf(buffer, "missing parameter %s from section @class "
					"%s", param.name.c_str(), templateName);
				file.printError(buffer);
			}
		}
		else if (param.list_type == LIST_LIST)
		{
			if (tpfTemplate->isParamPureVirtual(param.name))
				continue;
			if (!tpfTemplate->isParamLoaded(param.name))
			{
				result = false;
				sprintf(buffer, "missing parameter %s from section @class %s",
					param.name.c_str(), templateName);
				file.printError(buffer);
			}
		}
		else
		{
			for (int i = 0; i < param.list_size; ++i)
			{
				if (tpfTemplate->isParamPureVirtual(param.name, true, i))
					continue;

				if (!tpfTemplate->isParamLoaded(param.name, true, i))
				{
					result = false;
					if (param.list_type == LIST_INT_ARRAY)
					{
						sprintf(buffer, "missing parameter %s[%d] from section "
							"@class %s", param.name.c_str(), i, templateName);
					}
					else if (param.list_type == LIST_ENUM_ARRAY)
					{
						const EnumList * enumList = getEnumList(param.enum_list_name, false);
						FATAL(enumList == nullptr, ("Enum list %s missing",
							param.enum_list_name.c_str()));
						sprintf(buffer, "missing parameter %s[%s] from section "
							"@class %s", param.name.c_str(), enumList->at(i).name.c_str(),
							templateName);
					}
					file.printError(buffer);
				}
			}
		}
	}
	return result;
}	// TemplateData::verifyTemplate

/**
 * Verifies that a template has every necesssary member defined. If a parameter
 * is missing, writes out a default parameter to a file.
 *
 * @param tpfTemplate		the template to verify
 * @param file				the template file to write missing parameters to
 */
void TemplateData::updateTemplate(const TpfTemplate *tpfTemplate, File & file) const
{
	NOT_NULL(tpfTemplate);

	ParameterList::const_iterator iter;
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = *iter;
		if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
			continue;
		if (param.list_type == LIST_LIST || param.list_size <= 1)
		{
			if (!tpfTemplate->isParamLoaded(param.name))
			{
				writeParamTypeToDefaultTemplate(file, param);
				writeParameterDefault(file, param);
			}
		}
		else
		{
			for (int i = 0; i < param.list_size; ++i)
			{
				if (!tpfTemplate->isParamLoaded(param.name, true, i))
				{
					writeParamTypeToDefaultTemplate(file, param);
					writeParameterDefault(file, param, i);
				}
			}
		}
	}
}	// TemplateData::updateTemplate


//========================================================================
// template C++ writing functions

/**
 * Sets a flag saying whether we are writing C++ code for the template compiler
 * or the game.
 *
 * @param flag		if true, we are writing for the compiler
 */
void TemplateData::setWriteForCompiler(bool flag)
{
	m_writeForCompilerFlag = flag;
	if (flag && (m_baseName.empty() || getBaseName() == ROOT_TEMPLATE_NAME))
		m_baseName = COMPILER_ROOT_TEMPLATE_NAME;
	else if (!flag && (m_baseName.empty() || getBaseName() == COMPILER_ROOT_TEMPLATE_NAME))
		m_baseName = ROOT_TEMPLATE_NAME;
	for (StructMap::iterator iter = m_structMap.begin(); iter != m_structMap.end();
		++iter)
	{
		NOT_NULL((*iter).second);
		(*iter).second->setWriteForCompiler(flag);
	}
}	// TemplateData::setWriteForCompiler

/**
 * Writes code to register all the structures defined by this template.
 *
 * @param fp			the file to write to
 * @param leadInChars	text to write before the register line
 */
void TemplateData::writeRegisterTemplate(File &fp, const char * leadInChars) const
{
	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_INSTALL_BEGIN);

	if (leadInChars != nullptr)
		fp.print("%s", leadInChars);
	fp.print("%s::registerMe();\n", getName().c_str());

	// register any structures
	StructMap::const_iterator iter;
	for (iter = m_structMap.begin(); iter != m_structMap.end(); ++iter)
	{
		TemplateData *subStruct = (*iter).second;
		NOT_NULL(subStruct);
		subStruct->writeRegisterTemplate(fp, leadInChars);
	}

	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_INSTALL_END);
}	// TemplateData::writeRegisterTemplate

/**
 * Writes the parameters for a template param access method.
 *
 * @param fp			the file to write to
 * @param param			the parameter being accessed
 * @param headerParams	flag that we are writing params for a header file
 */
void TemplateData::writeMethodParams(File &fp, const Parameter &param,
	bool headerParams) const
{
std::vector<std::string> paramStrings;

	switch (param.type)
	{
		case TYPE_VECTOR:
			paramStrings.push_back("Vector &pos");
			break;
		case TYPE_DYNAMIC_VAR:
			paramStrings.push_back("DynamicVariableList &list");
			break;
		case TYPE_STRUCT:
			paramStrings.push_back(param.extendedName + " &data");
			break;
		default:
			break;
	}
	switch (param.list_type)
	{
		case LIST_NONE:
			break;
		case LIST_LIST:
		case LIST_INT_ARRAY:
			paramStrings.push_back("int index");
			break;
		case LIST_ENUM_ARRAY:
			paramStrings.push_back(param.enum_list_name + " index");
			break;
	}
	if (m_templateParent != nullptr)
		paramStrings.push_back("bool versionOk");
	if (param.list_type == LIST_NONE)
	{
		switch (param.type)
		{
			case TYPE_INTEGER:
			case TYPE_FLOAT:
			case TYPE_BOOL:
			case TYPE_STRING:
			case TYPE_STRINGID:
			case TYPE_FILENAME:
			case TYPE_TRIGGER_VOLUME:
			case TYPE_ENUM:
				if (headerParams)
					paramStrings.push_back("bool testData = false");
				else
					paramStrings.push_back("bool testData");
				break;
			default:
				break;
		}
	}

	// write the parameters passed to the method
	std::vector<std::string>::const_iterator iter;
	for (iter = paramStrings.begin(); iter != paramStrings.end(); ++iter)
	{
		if (iter != paramStrings.begin())
			fp.print(", ");
		fp.print("%s", (*iter).c_str());
	}
}	// TemplateData::writeMethodParams

/**
 * Writes class forward declarations if we have a parameter that returns a
 * template.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeHeaderFwdDecls(File &fp) const
{
std::set<std::string> names;

	fp.print("%s\n", TDF_TEMPLATE_REFS_BEGIN);

	getTemplateNames(names);

	if (names.size() != 0)
	{
		std::set<std::string>::const_iterator iter;
		for (iter = names.begin(); iter != names.end(); ++iter)
		{
			fp.print("class %s%s;\n", EnumLocationTypes[getTemplateLocation()],
				filenameLowerToUpper(*iter).c_str());
		}
	}

	fp.print("%s\n", TDF_TEMPLATE_REFS_END);
}	// TemplateData::writeHeaderFwdDecls

/**
 * Gets the names of any templates referenced by this template.
 *
 * @param names			set to be filled with template names used by this template
 */
void TemplateData::getTemplateNames(std::set<std::string> &names) const
{
	ParameterList::const_iterator iter;
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
			continue;

		// if the paramter is not the type we want to write, go to the next
		// parameter
		if (param.type == TYPE_TEMPLATE)
			names.insert(param.extendedName);
		else if (param.type == TYPE_STRUCT)
		{
			const TemplateData * structData = getStruct(param.extendedName.c_str());
			if (structData != nullptr)
				structData->getTemplateNames(names);
		}
	}
}	// TemplateData::getHeaderFwdDecls

/**
 * Writes out the template's parameters for a header.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeHeaderParams(File &fp) const
{
	if (m_writeForCompilerFlag)
	{
		writeCompilerHeaderParams(fp);
		return;
	}

	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_BEGIN);

	writeHeaderEnums(fp);
	writeHeaderStructs(fp);
	writeHeaderMethods(fp);
	writeHeaderVariables(fp);

	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_END);
}	// TemplateData::writeHeaderParams

/**
 * Writes out the template's parameters for a header for use in the template
 * compiler.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeCompilerHeaderParams(File &fp) const
{
	DEBUG_FATAL(!m_writeForCompilerFlag, ("write for compiler not enabled"));

	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_BEGIN);

	writeHeaderEnums(fp);
	writeHeaderStructs(fp);
	writeCompilerHeaderMethods(fp);
	writeHeaderVariables(fp);

	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_END);
}	// TemplateData::writeCompilerHeaderParams

/**
 * Writes out the template's enumeration definitions.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeHeaderEnums(File &fp) const
{
	if (m_enumMap.size() == 0)
		return;

	fp.print("public:\n");

	EnumMap::const_iterator mapIter;
	EnumList::const_iterator listIter;

	for (mapIter = m_enumMap.begin(); mapIter != m_enumMap.end(); ++mapIter)
	{
		fp.print("\tenum %s\n", (*mapIter).first.c_str());
		fp.print("\t{\n");
		const EnumList &enumList = (*mapIter).second;
		std::string last_name;
		for (listIter = enumList.begin(); listIter != enumList.end(); ++listIter)
		{
			const EnumData &data = *listIter;
			last_name = data.name.c_str();
			fp.print("\t\t%s", data.name.c_str());
			if (data.valueName.size() != 0)
				fp.print(" = %s", data.valueName.c_str());
			fp.print(",");
			if (data.comment.size() != 0)
				fp.print("\t\t// %s", data.comment.c_str());
			fp.print("\n");
		}
		fp.print("\t\t%s = %s,\n", ((*mapIter).first + std::string("_Last")).c_str(), last_name.c_str());
		fp.print("\t};\n\n");
	}
}	// TemplateData::writeHeaderEnums

/**
 * Writes out the template's structure definitions.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeHeaderStructs(File &fp) const
{
StructList::const_iterator iter;

	if (m_structList.size() == 0)
		return;

	// write public interface
	fp.print("public:\n");
	for (iter = m_structList.begin(); iter != m_structList.end(); ++iter)
	{
		const TemplateData *structData = *iter;
		NOT_NULL(structData);
		const char *structName = structData->m_name.c_str();
		fp.print("\tstruct %s\n", structName);
		fp.print("\t{\n");
		structData->writeStructDefinition(fp);
		if (structData->m_hasTemplateParam)
		{
			// the structure needs a constructor and destructor
			fp.print("\n");
			fp.print("\t\t %s(void);\n", structName);
			fp.print("\t\t %s(const %s & source);\n", structName, structName);
			fp.print("\t\t~%s();\n", structName);
			// write a dummy operator = for windows
			fp.print("#ifdef WIN32\n");
			fp.print("\t\t%s & operator =(const %s &);\n", structName, structName);
			fp.print("#endif\n");
		}
		if (structData->m_hasTemplateParam || structData->m_hasDynamicVarParam)
		{
			// the structure needs a (private) assignment operator
			fp.print("#ifdef LINUX\n");
			fp.print("\n");
			fp.print("\tprivate:\n");
			fp.print("\t\t%s & operator =(const %s &);\n", structName, structName);
			fp.print("#endif \n");
		}
		fp.print("\t};\n\n");
	}

	// write protected interface
	fp.print("protected:\n");
	for (iter = m_structList.begin(); iter != m_structList.end(); ++iter)
	{
		const TemplateData *structData = *iter;
		NOT_NULL(structData);
		const char *structName = structData->m_name.c_str();
		fp.print("\tclass _%s : public ", structName);
		if (isWritingForCompiler())
			fp.print("%s\n", COMPILER_ROOT_TEMPLATE_NAME);
		else
			fp.print("%s\n", ROOT_TEMPLATE_NAME);
		fp.print("\t{\n");
		// make the template class a friend
		fp.print("\t\tfriend class %s;\n", getName().c_str());
		// write tag definition
		fp.print("\tpublic:\n");
		fp.print("\t\tenum\n");
		fp.print("\t\t{\n");
		fp.print("\t\t\t_%s_tag = %s\n", structName, structData->m_structId.tagString.c_str());
		fp.print("\t\t};\n");
		fp.print("\n");
		// write constructor/destructor
		fp.print("\tpublic:\n");
		fp.print("\t\t         _%s(const std::string & filename);\n", structName);
		fp.print("\t\tvirtual ~_%s();\n", structName);
		fp.print("\n");
		fp.print("\tvirtual Tag getId(void) const;\n");
		fp.print("\n");
		// write functions and data
		structData->writeHeaderParams(fp);
		// write registration data
		fp.print("\tprivate:\n");
		fp.print("\t\tstatic void registerMe(void);\n");
		fp.print("\t\tstatic ObjectTemplate * create(const std::string & filename);\n");
		// write private copy constructors
		fp.print("\tprivate:\n");
		fp.print("\t\t_%s(const _%s &);\n", structName, structName);
		fp.print("\t\t_%s & operator =(const _%s &);\n", structName, structName);
		fp.print("\t};\n");
		fp.print("\tfriend class %s;\n\n", structData->getName().c_str());
	}
}	// TemplateData::writeHeaderStructs

/**
 * Writes out the template's parameters for a header.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeHeaderMethods(File &fp) const
{
std::string upperName;

	fp.print("public:\n");

	ParameterList::const_iterator iter;
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type == TYPE_COMMENT || param.type == TYPE_NONE)
			continue;

		// make a version of the parameter name with the first character upper case
		upperName = param.name;
		upperName[0] = static_cast<char>(toupper(upperName[0]));

		for (int i = 0; i < 3; ++i)
		{
			switch (param.type)
			{
				case TYPE_INTEGER:
				case TYPE_FLOAT:
				case TYPE_BOOL:
				case TYPE_STRING:
				case TYPE_FILENAME:
				case TYPE_STRINGID:
				case TYPE_TRIGGER_VOLUME:
					fp.print("\t%s get%s%s(", PaddedDataMethodNames[param.type],
						upperName.c_str(), MinMaxNames[i]);
					break;
				case TYPE_TEMPLATE:
					fp.print("\tconst %s%s * get%s(",
						EnumLocationTypes[getTemplateLocation()],
						filenameLowerToUpper(param.extendedName).c_str(),
						upperName.c_str());
					break;
				case TYPE_ENUM:
					fp.print("\t%s     get%s(", param.extendedName.c_str(),
						upperName.c_str());
					break;
				case TYPE_VECTOR:
					fp.print("\tvoid get%s(", upperName.c_str());
					break;
				case TYPE_DYNAMIC_VAR:
					fp.print("\t%s get%s(", PaddedDataMethodNames[param.type],
						upperName.c_str());
					break;
				case TYPE_STRUCT:
					fp.print("\tvoid              get%s%s(", upperName.c_str(),
						MinMaxNames[i]);
					break;
				default:
					break;
			}
			writeMethodParams(fp, param, true);
			fp.print(") const;\n");
			if (!HasMinMax[param.type])
				break;
		}

		if (param.list_type == LIST_LIST)
		{
			fp.print("\tsize_t            get%sCount(void) const;\n",
				upperName.c_str());
		}
	}
	fp.print("\n");
	fp.print("#ifdef _DEBUG\n");
	fp.print("public:\n");
	fp.print("\t// special code used by datalint\n");
	fp.print("\tvirtual void testValues(void) const;\n");
	fp.print("#endif\n");
	fp.print("\n");
	fp.print("protected:\n");
	fp.print("\tvirtual void load(Iff &file);\n");
	fp.print("\n");
}	// TemplateData::writeHeaderMethods

/**
 * Writes out the template's parameters for a header.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeCompilerHeaderMethods(File &fp) const
{
	DEBUG_FATAL(!m_writeForCompilerFlag, ("write for compiler not enabled"));

	fp.print("public:\n");
	fp.print("\tvirtual CompilerIntegerParam *getCompilerIntegerParam(const char *name, bool deepCheck = true, int index = 0);\n");
	fp.print("\tvirtual FloatParam *getFloatParam(const char *name, bool deepCheck = true, int index = 0);\n");
	fp.print("\tvirtual BoolParam *getBoolParam(const char *name, bool deepCheck = true, int index = 0);\n");
	fp.print("\tvirtual StringParam *getStringParam(const char *name, bool deepCheck = true, int index = 0);\n");
	fp.print("\tvirtual StringIdParam *getStringIdParam(const char *name, bool deepCheck = true, int index = 0);\n");
	fp.print("\tvirtual VectorParam *getVectorParam(const char *name, bool deepCheck = true, int index = 0);\n");
	fp.print("\tvirtual DynamicVariableParam *getDynamicVariableParam(const char *name, bool deepCheck = true, int index = 0);\n");
	fp.print("\tvirtual StructParamOT *getStructParamOT(const char *name, bool deepCheck = true, int index = 0);\n");
	fp.print("\tvirtual TriggerVolumeParam *getTriggerVolumeParam(const char *name, bool deepCheck = true, int index = 0);\n");
	fp.print("\tvirtual void initStructParamOT(StructParamOT &param, const char *name);\n");
	fp.print("\tvirtual void setAsEmptyList(const char *name);\n");
//	fp.print("\tvirtual bool isLoaded(const char *name, int index) const;\n");
	fp.print("\tvirtual void setAppend(const char *name);\n");
	fp.print("\tvirtual bool isAppend(const char *name) const;\n");
	fp.print("\tvirtual int getListLength(const char *name) const;\n");
	fp.print("\n");
	fp.print("protected:\n");
	fp.print("\tvirtual void load(Iff &file);\n");
	fp.print("\tvirtual void save(Iff &file);\n");
	fp.print("\n");
}	// TemplateData::writeCompilerHeaderMethods

/**
 * Writes out the template's parameters for a header.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeHeaderVariables(File &fp) const
{
	fp.print("private:\n");
	writeHeaderVariables(fp, m_parameters, "m_");
}	// TemplateData::writeHeaderVariables

/**
 * Writes out a list of parameters for a header.
 *
 * @param fp			the file to write to
 * @param list			the list to write
 * @param namePrefix	prefix to add to a parameter name
 */
void TemplateData::writeHeaderVariables(File &fp, const ParameterList &list,
	const char *namePrefix) const
{
// since comments are attached to the parameter following them, we need to keep
// a backlog of comments until we get to a parameter
typedef std::vector< const std::string * > CommentList;
CommentList commentList;
ParameterList::const_iterator iter;

	NOT_NULL(namePrefix);

	std::string extraTab = "";
	if (namePrefix[0] == '\0')
		extraTab = "\t";

	const char * const * variableNames = DataVariableNames;
	if (m_writeForCompilerFlag)
		variableNames = CompilerDataVariableNames;

	for (iter = list.begin(); iter != list.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type != TYPE_COMMENT)
		{
			// write out any queued comments
			CommentList::const_iterator citer;
			for (citer = commentList.begin(); citer != commentList.end(); ++citer)
				fp.print("\t%s\n", (*citer)->c_str());
			commentList.clear();
		}
		switch (param.type)
		{
			case TYPE_COMMENT:
				// queue the comment until we reach a parameter
				commentList.push_back(&param.description);
				break;
			case TYPE_INTEGER:
			case TYPE_ENUM:
			case TYPE_FLOAT:
			case TYPE_BOOL:
			case TYPE_STRING:
			case TYPE_STRINGID:
			case TYPE_FILENAME:
			case TYPE_VECTOR:
			case TYPE_DYNAMIC_VAR:
			case TYPE_TEMPLATE:
			case TYPE_STRUCT:
			case TYPE_TRIGGER_VOLUME:
				switch (param.list_type)
				{
					case LIST_NONE:
						fp.print("\t%s%s %s%s;", extraTab.c_str(),
							variableNames[param.type], namePrefix,
							param.name.c_str());
						break;
					case LIST_LIST:
						fp.print("\t%sstd::vector<%s *> %s%s;", extraTab.c_str(),
							variableNames[param.type], namePrefix,
							param.name.c_str());
						break;
					case LIST_INT_ARRAY:
					case LIST_ENUM_ARRAY:
						fp.print("\t%s%s %s%s[%d];", extraTab.c_str(),
							variableNames[param.type], namePrefix,
							param.name.c_str(), param.list_size);
						break;
					default:
						break;
				}
				break;
			case TYPE_NONE:
			default:
				break;
		}
		if (param.type != TYPE_COMMENT)
		{
			if (param.description.size() != 0)
				fp.print("\t\t// %s", param.description.c_str());
			fp.print("\n");
		}

		// add an "isloaded" flag for list parameters to see if we need to go
		// down in the hierarchy to get the data
		switch (param.type)
		{
			case TYPE_INTEGER:
			case TYPE_FLOAT:
			case TYPE_BOOL:
			case TYPE_STRING:
			case TYPE_STRINGID:
			case TYPE_FILENAME:
			case TYPE_VECTOR:
			case TYPE_DYNAMIC_VAR:
			case TYPE_ENUM:
			case TYPE_TEMPLATE:
			case TYPE_STRUCT:
			case TYPE_TRIGGER_VOLUME:
				if (param.list_type == LIST_LIST)
				{
					fp.print("\t%sbool m_%sLoaded;\n", extraTab.c_str(),
						param.name.c_str());
					fp.print("\t%sbool m_%sAppend;\n", extraTab.c_str(),
						param.name.c_str());
				}
				break;
			case TYPE_NONE:
			case TYPE_COMMENT:
			default:
				break;
		}
	}
}	// TemplateData::writeHeaderVariables

/**
 * Writes the public definition for a structure.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeStructDefinition(File &fp) const
{
// since comments are attached to the parameter following them, we need to keep
// a backlog of comments until we get to a parameter
typedef std::vector< const std::string * > CommentList;
CommentList commentList;
ParameterList::const_iterator iter;

	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type != TYPE_COMMENT)
		{
			// write out any queued comments
			CommentList::const_iterator citer;
			for (citer = commentList.begin(); citer != commentList.end(); ++citer)
				fp.print("\t\t%s\n", (*citer)->c_str());
			commentList.clear();
		}
		else
		{
			commentList.push_back(&param.description);
			continue;
		}

		switch (param.list_type)
		{
			case LIST_NONE:
			case LIST_INT_ARRAY:
			case LIST_ENUM_ARRAY:
				if (PaddedDataStructNames[param.type] != nullptr)
				{
					fp.print("\t\t%s %s", PaddedDataStructNames[param.type],
						param.name.c_str());
				}
				else if (param.type == TYPE_ENUM)
				{
					fp.print("\t\tenum %s %s", param.extendedName.c_str(),
						param.name.c_str());
				}
				else if (param.type == TYPE_STRUCT)
				{
					fp.print("\t\t%s %s", param.extendedName.c_str(),
						param.name.c_str());
				}
				if (param.list_type != LIST_NONE)
					fp.print("[%d]", param.list_size);
				fp.print(";\n");
				break;
			case LIST_LIST:
				fp.print("\t\tstd::vector<");
				if (UnpaddedDataStructNames[param.type] != nullptr)
					fp.print("%s", UnpaddedDataStructNames[param.type]);
				else if (param.type == TYPE_ENUM)
					fp.print("enum %s", param.extendedName.c_str());
				else if (param.type == TYPE_STRUCT)
					fp.print("%s", param.extendedName.c_str());
				fp.print("> %s;\n", param.name.c_str());
				break;
			default:
				break;
		}
	}
}	// TemplateData::writeStructDefinition

/**
 * Writes #includes for any templates used by this template.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeSourceTemplateIncludes(File &fp) const
{
std::set<std::string> names;

	fp.print("%s\n", TDF_TEMPLATE_REFS_BEGIN);

	getTemplateNames(names);

	if (names.size() != 0)
	{
		std::set<std::string>::const_iterator iter;
		for (iter = names.begin(); iter != names.end(); ++iter)
		{
			fp.print("#include \"%s%s.h\"\n", EnumLocationTypes[getTemplateLocation()],
				filenameLowerToUpper(*iter).c_str());
		}
	}

	fp.print("%s\n", TDF_TEMPLATE_REFS_END);
}	// TemplateData::writeSourceTemplateIncludes

/**
 * Writes constructor code to initialize the list loaded flags to false.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeSourceLoadedFlagInit(File &fp) const
{
ParameterList::const_iterator iter;

	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_INIT_BEGIN);

	fp.print("\t: %s(filename)\n", getBaseName().c_str());

	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.list_type != LIST_LIST)
			continue;

		fp.print("\t,");
		fp.print("m_%sLoaded(false)\n", param.name.c_str());
		fp.print("\t,m_%sAppend(false)\n", param.name.c_str());
	}
	if (m_templateParent == nullptr && !isWritingForCompiler())
	{
		fp.print("\t,");
		fp.print("m_versionOk(true)\n");
	}

	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_INIT_END);
}	// TemplateData::writeSourceLoadedFlagInit

/**
 * Writes constructor, destructor for structures (private).
 *
 * @param fp		the file to write to
 */
void TemplateData::writeSourceStructStart(File &fp) const
{
	if (m_templateParent == nullptr)
		return;

	const std::string & templateNameString = getName();
	const char *name = templateNameString.c_str();
	std::string::size_type colonPos = getName().rfind(':') + 1;

	fp.print("\n");
	fp.print("//=============================================================================\n");
	fp.print("// class %s\n", name);
	fp.print("\n");
	fp.print("/**\n");
	fp.print(" * Class constructor.\n");
	fp.print(" */\n");
	fp.print("%s::%s(const std::string & filename)\n", name, &name[colonPos]);
	writeSourceLoadedFlagInit(fp);
	fp.print("{\n");
	fp.print("}	// %s::%s\n", name, &name[colonPos]);
	fp.print("\n");
	fp.print("/**\n");
	fp.print(" * Class destructor.\n");
	fp.print(" */\n");
	fp.print("%s::~%s()\n", name, &name[colonPos]);
	fp.print("{\n");
	writeSourceCleanup(fp);
	fp.print("}	// %s::~%s\n", name, &name[colonPos]);
	fp.print("\n");
	fp.print("/**\n");
	fp.print(" * Static function used to register this template.\n");
	fp.print(" */\n");
	fp.print("void %s::registerMe(void)\n", name);
	fp.print("{\n");
	fp.print("\tObjectTemplateList::registerTemplate(_%s_tag, create);\n", m_name.c_str());
	fp.print("}	// %s::registerMe\n", name);
	fp.print("\n");
	fp.print("/**\n");
	fp.print(" * Creates a %s template.\n", name);
	fp.print(" *\n");
	fp.print(" * @return a new instance of the template\n");
	fp.print(" */\n");
	fp.print("ObjectTemplate * %s::create(const std::string & filename)\n", name);
	fp.print("{\n");
	fp.print("\treturn new %s(filename);\n", name);
	fp.print("}	// %s::create\n\n", name);
	fp.print("/**\n");
	fp.print(" * Returns the template id.\n");
	fp.print(" *\n");
	fp.print(" * @return the template id\n");
	fp.print(" */\n");
	fp.print("Tag %s::getId(void) const\n", name);
	fp.print("{\n");
	fp.print("\treturn _%s_tag;\n", m_name.c_str());
	fp.print("}	// %s::getId\n", name);
	fp.print("\n");
}	// TemplateData::writeSourceStructStart

/**
 * Writes constructor, destructor for structures (public).
 *
 * @param fp		the file to write to
 * @param name		the structure name
 */
void TemplateData::writeSourceStructStart(File &fp, const std::string &name) const
{
ParameterList::const_iterator iter;

	if (m_templateParent == nullptr || !m_hasTemplateParam)
		return;

	std::string className = m_templateParent->getName() + "::" + name;

	fp.print("\n");
	fp.print("//=============================================================================\n");
	fp.print("// struct %s\n", className.c_str());
	fp.print("\n");

	// write the default constructor

	fp.print("/**\n");
	fp.print(" * Struct constructor.\n");
	fp.print(" */\n");
	fp.print("%s::%s(void)\n", className.c_str(), name.c_str());
	fp.print("{\n");

	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type != TYPE_TEMPLATE)
			continue;

		const char *pname = param.name.c_str();
		switch (param.list_type)
		{
			case LIST_NONE:
				fp.print("\t%s = nullptr;\n", pname);
				break;
			case LIST_INT_ARRAY:
				fp.print("\tfor (int i = 0; i < %d; ++i)\n", param.list_size);
				fp.print("\t{\n");
				fp.print("\t\t%s[i] = nullptr;\n", pname);
				fp.print("\t}\n");
				break;
			case LIST_ENUM_ARRAY:
				fp.print("\tfor (int i = 0; i < %d; ++i)\n", param.list_size);
				fp.print("\t{\n");
				fp.print("\t\t%s[static_cast<%s>(i)] = nullptr;\n", pname,
					param.enum_list_name.c_str());
				fp.print("\t}\n");
				break;
			case LIST_LIST:
			default:
				break;
		}
	}
	fp.print("}	// %s::%s()\n", className.c_str(), name.c_str());
	fp.print("\n");

	// write the copy constructor

	fp.print("/**\n");
	fp.print(" * Struct copy constructor.\n");
	fp.print(" */\n");
	fp.print("%s::%s(const %s & source) :\n", className.c_str(), name.c_str(),
		className.c_str());
	bool firstParam = true;
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
			continue;

		const char *pname = param.name.c_str();
		if (!firstParam)
			fp.print(",\n");
		fp.print("\t%s(source.%s)", pname, pname);
		firstParam = false;
	}
	fp.print("\n{\n");

	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type != TYPE_TEMPLATE)
			continue;

		const char *pname = param.name.c_str();
		switch (param.list_type)
		{
			case LIST_NONE:
				fp.print("\tif (%s != nullptr)\n", pname);
				fp.print("\t\tconst_cast<ObjectTemplate *>(%s)->addReference();\n",
					pname);
				break;
			case LIST_INT_ARRAY:
				fp.print("\tfor (int i = 0; i < %d; ++i)\n", param.list_size);
				fp.print("\t{\n");
				fp.print("\t\tif (%s[i] != nullptr)\n", pname);
				fp.print("\t\t\tconst_cast<ObjectTemplate *>(%s[i])->addReference"
					"();\n", pname);
				fp.print("\t}\n");
				break;
			case LIST_ENUM_ARRAY:
				fp.print("\tfor (int i = 0; i < %d; ++i)\n", param.list_size);
				fp.print("\t{\n");
				fp.print("\t\tif (%s[static_cast<%s>(i)] != nullptr)\n", pname,
					param.enum_list_name.c_str());
				fp.print("\t\t\tconst_cast<ObjectTemplate *>(%s[static_cast<%s>"
					"(i)])->addReference();\n", pname, param.enum_list_name.c_str());
				fp.print("\t}\n");
				break;
			case LIST_LIST:
			default:
				break;
		}
	}
	fp.print("}	// %s::%s(const %s &)\n", className.c_str(), name.c_str(),
		className.c_str());
	fp.print("\n");

	// write the destructor

	fp.print("/**\n");
	fp.print(" * Struct destructor.\n");
	fp.print(" */\n");
	fp.print("%s::~%s()\n", className.c_str(), name.c_str());
	fp.print("{\n");

	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type != TYPE_TEMPLATE)
			continue;

		const char *pname = param.name.c_str();
		switch (param.list_type)
		{
			case LIST_NONE:
				fp.print("\tif (%s != nullptr)\n", pname);
				fp.print("\t{\n");
				fp.print("\t\t%s->releaseReference();\n", pname);
				fp.print("\t\t%s = nullptr;\n", pname);
				fp.print("\t}\n");
				break;
			case LIST_LIST:
			case LIST_INT_ARRAY:
				if (param.list_type == LIST_LIST)
				{
					fp.print("\tint count = %s.size();\n", pname);
					fp.print("\tfor (int i = 0; i < count; ++i)\n");
				}
				else
					fp.print("\tfor (int i = 0; i < %d; ++i)\n", param.list_size);
				fp.print("\t{\n");
				fp.print("\t\tif (%s[i] != nullptr)\n", pname);
				fp.print("\t\t{\n");
				fp.print("\t\t\t%s[i]->releaseReference();\n",
					pname);
				fp.print("\t\t\t%s[i] = nullptr;\n", pname);
				fp.print("\t\t}\n");
				fp.print("\t}\n");
				break;
			case LIST_ENUM_ARRAY:
				fp.print("\tfor (int i = 0; i < %d; ++i)\n", param.list_size);
				fp.print("\t{\n");
				fp.print("\t\tif (%s[static_cast<%s>(i)] != nullptr)\n", pname,
					param.enum_list_name.c_str());
				fp.print("\t\t{\n");
				fp.print("\t\t\t%s[static_cast<%s>(i)]->releaseReference();\n",
					pname, param.enum_list_name.c_str());
				fp.print("\t\t\t%s[static_cast<%s>(i)] = nullptr;\n", pname,
					param.enum_list_name.c_str());
				fp.print("\t\t}\n");
				fp.print("\t}\n");
				break;
			default:
				break;
		}
	}
	fp.print("}	// %s::~%s\n", className.c_str(), name.c_str());
	fp.print("\n");

	// write a dummy operator = for windows
	fp.print("#ifdef WIN32\n");
	fp.print("/**\n");
	fp.print(" * Struct destructor.\n");
	fp.print(" */\n");
	fp.print("%s & %s::operator =(const %s &)\n", className.c_str(),
		className.c_str(), className.c_str());
	fp.print("{\n");
	fp.print("\tDEBUG_FATAL(true, (\"operator = should not be called for template "
		"structures!\"));\n");
	fp.print("\treturn *this;\n");
	fp.print("} // %s::operator =\n", className.c_str());
	fp.print("#endif\n");
	fp.print("\n");
}	// TemplateData::writeSourceStructStart

/**
 * Writes the source code to load the template data.
 *
 * @param fp		the file to write to
 *
 * @return 0 on success, -1 on fail
 */
int TemplateData::writeSourceMethods(File &fp) const
{
int result;

	if (m_writeForCompilerFlag)
	{
		writeCompilerSourceMethods(fp);
		return 0;
	}

	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_BEGIN);

	result = writeSourceGetData(fp);
	if (result != 0)
		return result;
	writeSourceTestData(fp);
	writeSourceReadIff(fp);

	// write methods for any structures
	StructMap::const_iterator iter;
	for (iter = m_structMap.begin(); iter != m_structMap.end(); ++iter)
	{
		TemplateData *subStruct = (*iter).second;
		NOT_NULL(subStruct);
		if (subStruct->m_hasTemplateParam)
			subStruct->writeSourceStructStart(fp, (*iter).first);
		subStruct->writeSourceStructStart(fp);
		result = subStruct->writeSourceMethods(fp);
		if (result != 0)
			return result;
	}

	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_END);
	return 0;
}	// TemplateData::writeSourceMethods

/**
 * Writes the source code to load and save the template data for use in the
 * template compiler.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeCompilerSourceMethods(File &fp) const
{
	DEBUG_FATAL(!m_writeForCompilerFlag, ("write for compiler not enabled"));

	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_BEGIN);

	writeCompilerSourceAccessMethods(fp);
	writeSourceReadIff(fp);
	writeSourceWriteIff(fp);

	// write methods for any structures
	StructMap::const_iterator iter;
	for (iter = m_structMap.begin(); iter != m_structMap.end(); ++iter)
	{
		TemplateData *subStruct = (*iter).second;
		NOT_NULL(subStruct);
		subStruct->writeSourceStructStart(fp);
		subStruct->writeCompilerSourceMethods(fp);
	}

	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_END);
}	// TemplateData::writeCompilerSourceMethods

/**
 * Writes code to retrieve a parameter from a base template, or return the
 * default value if the template is out of date.
 *
 * @param fp				the file to write to
 * @param param				the parameter to get
 * @param minMaxString		string to use for min/max getters
 */
void TemplateData::writeSourceReturnBaseValue(File &fp, const Parameter &param,
	const std::string & minMaxString) const
{
	const char * realMinMaxString;
	if (HasMinMax[param.type])
		realMinMaxString = minMaxString.c_str();
	else
		realMinMaxString = "";

	std::string upperName = param.name;
	upperName[0] = static_cast<char>(toupper(upperName[0]));

	std::string indexString;
	if (param.type == TYPE_DYNAMIC_VAR)
	{
		if (param.list_type == LIST_NONE)
			indexString = "list";
		else
			indexString = "list, index";
	}
	else if (param.type == TYPE_STRUCT)
	{
		if (param.list_type == LIST_NONE)
			indexString = "data";
		else
			indexString = "data, index";
	}
	else if (param.list_type != LIST_NONE)
		indexString = "index";
	if (m_templateParent != nullptr)
	{
		if (!indexString.empty())
			indexString += ", ";
		indexString += "versionOk";
	}

	// write code to check if a paramter is loaded, and if not call the base
	// template's function
	if (param.list_type == LIST_NONE)
		fp.print("\tif (!m_%s.isLoaded())\n", param.name.c_str());
	else if (param.list_type == LIST_LIST)
		fp.print("\tif (!m_%sLoaded)\n", param.name.c_str());
	else
		fp.print("\tif (!m_%s[index].isLoaded())\n", param.name.c_str());

	fp.print("\t{\n");
	if (DefaultDataReturnValue[param.type] != nullptr)
	{
		fp.print("\t\tif (ms_allowDefaultTemplateParams && "
			"/*!%s &&*/ base == nullptr)\n", m_templateParent == nullptr ? "m_versionOk" :
			"versionOk");
		fp.print("\t\t{\n");
		fp.print("\t\t\tDEBUG_WARNING(true, (\"Returning default value for "
			"missing parameter %s in template %%s\", DataResource::getName()));\n",
			param.name.c_str());
		if (param.type == TYPE_ENUM)
		{
			fp.print("\t\t\treturn static_cast<%s>%s;\n",
				param.extendedName.c_str(), DefaultDataReturnValue[param.type]);
		}
		else
			fp.print("\t\t\treturn %s;\n", DefaultDataReturnValue[param.type]);
		fp.print("\t\t}\n");
		fp.print("\t\telse\n");
		fp.print("\t\t{\n");
		fp.print("\t\t\tDEBUG_FATAL(base == nullptr, (\"Template parameter %s has "
			"not been defined in template %%s!\", DataResource::getName()));\n",
			param.name.c_str());
		if (DefaultDataReturnValue[param.type][0] != '\0')
		{
			fp.print("\t\t\treturn base->get%s%s(%s);\n", upperName.c_str(),
				realMinMaxString, indexString.c_str());
		}
		else
		{
			fp.print("\t\t\tbase->get%s%s(%s);\n", upperName.c_str(),
				realMinMaxString, indexString.c_str());
			fp.print("\t\t\treturn;\n");
		}
		fp.print("\t\t}\n");
	}
	else
	{
		fp.print("\t\tNOT_NULL(base);\n");
		fp.print("\t\treturn base->get%s%s(%s);\n", upperName.c_str(),
			realMinMaxString, indexString.c_str());
	}
	fp.print("\t}\n\n");

	// if the param is a list and we are appending, check the index to see if
	// we need to get the base value instead of ours
	if (param.list_type == LIST_LIST)
	{
		fp.print("\tif (m_%sAppend && base != nullptr)\n", param.name.c_str());
		fp.print("\t{\n");
		fp.print("\t\tint baseCount = base->get%sCount();\n",
			upperName.c_str());
		fp.print("\t\tif (index < baseCount)\n");
		if (DefaultDataReturnValue[param.type][0] != '\0')
		{
			fp.print("\t\t\treturn base->get%s%s(%s);\n", upperName.c_str(),
				realMinMaxString, indexString.c_str());
		}
		else
		{
			fp.print("\t\t\t{\n");
			fp.print("\t\t\t\tbase->get%s%s(%s);\n", upperName.c_str(),
				realMinMaxString, indexString.c_str());
			fp.print("\t\t\t\treturn;\n");
			fp.print("\t\t\t}\n");
		}
		fp.print("\t\tindex -= baseCount;\n");
		fp.print("\t}\n\n");
	}
}	// TemplateData::writeSourceReturnBaseValue

/**
 * Writes functions to get parameters.
 *
 * @param fp		the file to write to
 *
 * @return 0 on success, -1 on fail
 */
int TemplateData::writeSourceGetData(File &fp) const
{
int result;

	ParameterList::const_iterator iter;
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
			continue;

		switch (param.type)
		{
			case TYPE_INTEGER:
			case TYPE_FLOAT:
			case TYPE_BOOL:
			case TYPE_STRING:
			case TYPE_STRINGID:
			case TYPE_FILENAME:
			case TYPE_TRIGGER_VOLUME:
				writeSourceGetGeneric(fp, param);
				break;
			case TYPE_VECTOR:
				writeSourceGetVector(fp, param);
				break;
			case TYPE_DYNAMIC_VAR:
				writeSourceGetDynamicVariable(fp, param);
				break;
			case TYPE_TEMPLATE:
				writeSourceGetTemplate(fp, param);
				break;
			case TYPE_ENUM:
				writeSourceGetEnum(fp, param);
				break;
			case TYPE_STRUCT:
				result = writeSourceGetStruct(fp, param);
				if (result != 0)
					return result;
				break;
			default:
				break;
		}
		if (param.list_type == LIST_LIST)
		{
			// write get count function
			const std::string & templateNameString = getName();
			const char *templateName = templateNameString.c_str();
			const char *pname = param.name.c_str();
			std::string upperName = param.name;
			upperName[0] = static_cast<char>(toupper(upperName[0]));

			fp.print("size_t %s::get%sCount(void) const\n",
				templateName, upperName.c_str());
			fp.print("{\n");
			fp.print("\tif (!m_%sLoaded)\n", pname);
			fp.print("\t{\n");
			fp.print("\t\tif (m_baseData == nullptr)\n");
			fp.print("\t\t\treturn 0;\n");
			fp.print("\t\tconst %s * base = dynamic_cast<const %s *>"
				"(m_baseData);\n", templateName, templateName);
			fp.print("\t\tDEBUG_FATAL(base == nullptr, (\"base template wrong "
				"type\"));\n");
			fp.print("\t\treturn base->get%sCount();\n", upperName.c_str());
			fp.print("\t}\n\n");
			fp.print("\tsize_t count = m_%s.size();\n\n", pname);
			fp.print("\t// if we are extending our base template, add it's count\n");
			fp.print("\tif (m_%sAppend && m_baseData != nullptr)\n", pname);
			fp.print("\t{\n");
			fp.print("\t\tconst %s * base = dynamic_cast<const %s *>(m_baseData);\n",
				templateName, templateName);
			fp.print("\t\tif (base != nullptr)\n");
			fp.print("\t\t\tcount += base->get%sCount();\n", upperName.c_str());
			fp.print("\t}\n\n");
			fp.print("\treturn count;\n");
			fp.print("}\t// %s::get%sCount\n", templateName, upperName.c_str());
			fp.print("\n");
		}
	}
	return 0;
}	// TemplateData::writeSourceGetData

/**
 * Writes code used by datalint to test template values.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeSourceTestData(File &fp) const
{
	const std::string & templateNameString = getName();
	const char *templateName = templateNameString.c_str();

	fp.print("#ifdef _DEBUG\n");
	fp.print("/**\n");
	fp.print(" * Special function used by datalint. Checks for duplicate values "
		"in base and derived templates.\n");
	fp.print(" */\n");
	fp.print("void %s::testValues(void) const\n", templateName);
	fp.print("{\n");

	ParameterList::const_iterator iter;
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.list_type != LIST_NONE)
		{
			continue;
		}

		std::string upperName(param.name);
		upperName[0] = static_cast<char>(toupper(upperName[0]));

		switch (param.type)
		{
			case TYPE_INTEGER:
			case TYPE_FLOAT:
				fp.print("\tIGNORE_RETURN(get%sMin(true));\n", upperName.c_str());
				fp.print("\tIGNORE_RETURN(get%sMax(true));\n", upperName.c_str());
				break;
			case TYPE_BOOL:
			case TYPE_STRING:
			case TYPE_STRINGID:
			case TYPE_FILENAME:
			case TYPE_TRIGGER_VOLUME:
			case TYPE_ENUM:
				fp.print("\tIGNORE_RETURN(get%s(true));\n", upperName.c_str());
				break;
			case TYPE_STRUCT:
				break;
			default:
				break;
		}
	}

	if (m_fileParent != nullptr)
	{
		// check the base class
		if (!getBaseName().empty() && getBaseName() != ROOT_TEMPLATE_NAME)
		{
			fp.print("\t%s::testValues();\n", getBaseName().c_str());
		}
	}

	fp.print("}\t// %s::testValues\n", templateName);
	fp.print("#endif\n\n");
}	// TemplateData::writeSourceTestData

/**
 * Writes functions to get a parameter that doesn't have a special access mode.
 *
 * @param fp		the file to write to
 * @param param		the parameter to get
 */
void TemplateData::writeSourceGetGeneric(File &fp, const Parameter &param) const
{
	const std::string & templateNameString = getName();
	const char *templateName = templateNameString.c_str();

	// make a version of the parameter name with the first character upper case
	const char *pname = param.name.c_str();
	std::string upperName(param.name);
	upperName[0] = static_cast<char>(toupper(upperName[0]));

	for (int i = 0; i < 3; ++i)
	{
		fp.print("%s %s::get%s%s(", UnpaddedDataMethodNames[param.type],
			templateName, upperName.c_str(), MinMaxNames[i]);

		writeMethodParams(fp, param, false);
		fp.print(") const\n");
		fp.print("{\n");

		if (param.list_type == LIST_NONE)
		{
			// set up var to receive the test base value
			fp.print("#ifdef _DEBUG\n");
			fp.print("%s testDataValue = %s;\n", UnpaddedDataStructNames[param.type],
				DefaultDataReturnValue[param.type]);
			fp.print("#else\n");
			fp.print("UNREF(testData);\n");
			fp.print("#endif\n\n");
		}

		fp.print("\tconst %s * base = nullptr;\n", templateName);
		fp.print("\tif (m_baseData != nullptr)\n");
		fp.print("\t{\n");
		fp.print("\t\tbase = dynamic_cast<const %s *>(m_baseData);\n",
			templateName);
//		fp.print("\t\tDEBUG_FATAL(base == nullptr, (\"base template wrong type\"));\n");

		if (param.list_type == LIST_NONE)
		{
			// get the test base value
			fp.print("#ifdef _DEBUG\n");
			fp.print("\t\tif (testData && base != nullptr)\n");
			fp.print("\t\t\ttestDataValue = base->get%s%s(true);\n", upperName.c_str(),
				MinMaxNames[i]);
			fp.print("#endif\n");
		}

		fp.print("\t}\n\n");

		const char *arrayIndex = "";
		std::string indexName = m_templateParent == nullptr ? "" : "versionOk";
		const char *access = ".";
		if (param.list_type == LIST_NONE)
		{
			writeSourceReturnBaseValue(fp, param, MinMaxNames[i]);
		}
		else
		{
			arrayIndex = "[index]";
			if (!indexName.empty())
				indexName = ", " + indexName;
			indexName = "index" + indexName;
			if (param.list_type == LIST_LIST)
			{
				access = "->";
				writeSourceReturnBaseValue(fp, param, MinMaxNames[i]);
				fp.print("\tDEBUG_FATAL(index < 0 || static_cast<size_t>(index) "
					">= m_%s.size(), (\"template param index out of range\"));\n",
					pname);
			}
			else
			{
				fp.print("\tDEBUG_FATAL(index < 0 || index >= %d, (\""
					"template param index out of range\"));\n", param.list_size);
				writeSourceReturnBaseValue(fp, param, MinMaxNames[i]);
			}
		}
		fp.print("\t%s value = m_%s%s%sget%sValue();\n",
			UnpaddedDataMethodNames[param.type], pname, arrayIndex, access,
			MinMaxNames[i]);
		if ((param.type == TYPE_INTEGER || param.type == TYPE_FLOAT) &&
			param.list_type != LIST_LIST)
		{
			fp.print("\tchar delta = m_%s%s%sgetDeltaType();\n", pname, arrayIndex,
				access);
			fp.print("\tif (delta == '+' || delta == '-' || delta == '_' || delta == '=')\n");
			fp.print("\t{\n");
			fp.print("\t\t%s baseValue = 0;\n", UnpaddedDataMethodNames[param.type]);
			fp.print("\t\tif (m_baseData != nullptr)\n");
			fp.print("\t\t{\n");
			fp.print("\t\t\tif (base != nullptr)\n");
			fp.print("\t\t\t\tbaseValue = base->get%s%s(%s);\n", upperName.c_str(),
				MinMaxNames[i], indexName.c_str());
			fp.print("\t\t\telse if (ms_allowDefaultTemplateParams)\n");
			fp.print("\t\t\t\tDEBUG_WARNING(true, (\"No base template for delta, using 0\"));\n");
			fp.print("\t\t\telse\n");
			fp.print("\t\t\t\tNOT_NULL(base);\n");
			fp.print("\t\t}\n");
			fp.print("\t\tif (delta == '+')\n");
			fp.print("\t\t\tvalue = baseValue + value;\n");
			fp.print("\t\telse if (delta == '-')\n");
			fp.print("\t\t\tvalue = baseValue - value;\n");
			fp.print("\t\telse if (delta == '=')\n");
			fp.print("\t\t\tvalue = baseValue + static_cast<%s>(baseValue * (value "
				"/ 100.0f));\n", UnpaddedDataMethodNames[param.type]);
			fp.print("\t\telse if (delta == '_')\n");
			fp.print("\t\t\tvalue = baseValue - static_cast<%s>(baseValue * (value "
				"/ 100.0f));\n", UnpaddedDataMethodNames[param.type]);
			fp.print("\t}\n");
		}

		if (param.list_type == LIST_NONE)
		{
			// check the return value vs the base value, and warn if the same
			fp.print("#ifdef _DEBUG\n");
			fp.print("\tif (testData && base != nullptr)\n");
			fp.print("\t{\n");
//			fp.print("\t\tif (testDataValue == value)\n");
//			fp.print("\t\t\tDEBUG_WARNING(true, (\"Template %%s, parameter %s is "
//				"returning same value as base template.\", DataResource::getName()));"
//				"\n", pname);
			fp.print("\t}\n");
			fp.print("#endif\n\n");
		}

		fp.print("\treturn value;\n");
		fp.print("}\t// %s::get%s%s\n", templateName, upperName.c_str(), MinMaxNames[i]);
		fp.print("\n");

		if (!HasMinMax[param.type])
			break;
	}
}	// TemplateData::writeSourceGetGeneric

/**
 * Writes functions to get Vector parameters.
 *
 * @param fp		the file to write to
 * @param param		the parameter to get
 */
void TemplateData::writeSourceGetVector(File &fp, const Parameter &param) const
{
	const std::string & templateNameString = getName();
	const char *templateName = templateNameString.c_str();

	// make a version of the parameter name with the first character upper case
	const char *pname = param.name.c_str();
	std::string upperName = param.name;
	upperName[0] = static_cast<char>(toupper(upperName[0]));

	fp.print("void %s::get%s(", templateName,
		upperName.c_str());

	writeMethodParams(fp, param, false);
	fp.print(") const\n");
	fp.print("{\n");

	fp.print("\tconst %s * base = nullptr;\n", templateName);
	fp.print("\tif (m_baseData != nullptr)\n");
	fp.print("\t{\n");
	fp.print("\t\tbase = dynamic_cast<const %s *>(m_baseData);\n",
		templateName);
//	fp.print("\t\tDEBUG_FATAL(base == nullptr, (\"base template wrong type\"));\n");
	fp.print("\t}\n\n");

	const char *arrayIndex = "";
	const char *access = ".";
	if (param.list_type == LIST_NONE)
	{
		writeSourceReturnBaseValue(fp, param, "");
	}
	else
	{
		arrayIndex = "[index]";
		if (param.list_type == LIST_LIST)
		{
			access = "->";
			writeSourceReturnBaseValue(fp, param, "");
			fp.print("\tDEBUG_FATAL(index < 0 || static_cast<size_t>(index) "
				">= m_%s.size(), (\"template param index out of range\"));\n",
				pname);
		}
		else
		{
			fp.print("\tDEBUG_FATAL(index < 0 || index >= %d, (\""
				"template param index out of range\"));\n", param.list_size);
			writeSourceReturnBaseValue(fp, param, "");
		}
	}

	fp.print("\tconst VectorParamData &myPos = m_%s%s%sgetValue();\n", pname,
		arrayIndex, access);
	fp.print("\tmyPos.adjustVector(pos);\n");
	fp.print("}\t// %s::get%s\n", templateName, upperName.c_str());
	fp.print("\n");
}	// TemplateData::writeSourceGetVector

/**
 * Writes functions to get DynamicVariable parameters.
 *
 * @param fp		the file to write to
 * @param param		the parameter to get
 */
void TemplateData::writeSourceGetDynamicVariable(File &fp, const Parameter &param) const
{
	const std::string & templateNameString = getName();
	const char *templateName = templateNameString.c_str();

	// make a version of the parameter name with the first character upper case
	const char *pname = param.name.c_str();
	std::string upperName = param.name;
	upperName[0] = static_cast<char>(toupper(upperName[0]));

	fp.print("void %s::get%s(", templateName, upperName.c_str());

	writeMethodParams(fp, param, false);
	fp.print(") const\n");
	fp.print("{\n");

	fp.print("\tconst %s * base = nullptr;\n", templateName);
	fp.print("\tif (m_baseData != nullptr)\n");
	fp.print("\t{\n");
	fp.print("\t\tbase = dynamic_cast<const %s *>(m_baseData);\n",
		templateName);
//	fp.print("\t\tDEBUG_FATAL(base == nullptr, (\"base template wrong type\"));\n");
	fp.print("\t}\n\n");

	if (param.list_type == LIST_NONE)
	{
		writeSourceReturnBaseValue(fp, param, "");
		fp.print("\tif (m_%s.isExtendingBaseList() && base != nullptr)\n", pname);
		fp.print("\t\tbase->get%s(list);\n", upperName.c_str());
		fp.print("\tm_%s.getDynamicVariableList(list);\n", pname);
	}
	else
	{
		fp.print("\tDEBUG_FATAL(index < 0 || index >= %d, (\""
			"template param index out of range\"));\n", param.list_size);
		writeSourceReturnBaseValue(fp, param, "");
		fp.print("\tif (m_%s[index].isExtendingBaseList() && base != nullptr)\n", pname);
		fp.print("\t\tbase->get%s(list, index);\n", upperName.c_str());
		fp.print("\tm_%s[index].getDynamicVariableList(list);\n", pname);
	}
	fp.print("}\t// %s::get%s\n", templateName, upperName.c_str());
	fp.print("\n");
}	// TemplateData::writeSourceGetDynamicVariable

/**
 * Writes functions to get Template parameters.
 *
 * @param fp		the file to write to
 * @param param		the parameter to get
 */
void TemplateData::writeSourceGetTemplate(File &fp, const Parameter &param) const
{
	const std::string & templateNameString = getName();
	const char *templateName = templateNameString.c_str();

	// make a version of the parameter name with the first character upper case
	const char *pname = param.name.c_str();
	std::string upperName = param.name;
	upperName[0] = static_cast<char>(toupper(upperName[0]));

	fp.print("const %s%s * %s::get%s(", EnumLocationTypes[getTemplateLocation()],
		filenameLowerToUpper(param.extendedName).c_str(), templateName,
		upperName.c_str());

	writeMethodParams(fp, param, false);
	fp.print(") const\n");
	fp.print("{\n");

	fp.print("\tconst %s * base = nullptr;\n", templateName);
	fp.print("\tif (m_baseData != nullptr)\n");
	fp.print("\t{\n");
	fp.print("\t\tbase = dynamic_cast<const %s *>(m_baseData);\n",
		templateName);
//	fp.print("\t\tDEBUG_FATAL(base == nullptr, (\"base template wrong type\"));\n");
	fp.print("\t}\n\n");

	if (param.list_type == LIST_NONE)
	{
		writeSourceReturnBaseValue(fp, param, "");

		fp.print("\tconst %s%s * returnValue = nullptr;\n",
			EnumLocationTypes[getTemplateLocation()],
			filenameLowerToUpper(param.extendedName).c_str());
		fp.print("\tconst std::string & templateName = m_%s.getValue();\n", pname);
		fp.print("\tif (!templateName.empty())\n");
		fp.print("\t{\n");
		fp.print("\t\treturnValue = dynamic_cast<const %s%s *>("
			"ObjectTemplateList::fetch(templateName));\n",
			EnumLocationTypes[getTemplateLocation()],
			filenameLowerToUpper(param.extendedName).c_str());
		fp.print("\t\tif (returnValue == nullptr)\n");
		fp.print("\t\t\tWARNING_STRICT_FATAL(true, (\"Error loading template %%s\","
			"templateName.c_str()));\n");
		fp.print("\t}\n");
		fp.print("\treturn returnValue;\n");
	}
	else
	{
		const char * access = ".";
		if (param.list_type == LIST_LIST)
		{
			access = "->";
			writeSourceReturnBaseValue(fp, param, "");
			fp.print("\tDEBUG_FATAL(index < 0 || static_cast<size_t>(index) "
				">= m_%s.size(), (\"template param index out of range\"));\n",
				pname);
		}
		else
		{
			fp.print("\tDEBUG_FATAL(index < 0 || index >= %d, (\""
				"template param index out of range\"));\n", param.list_size);
			writeSourceReturnBaseValue(fp, param, "");
		}

		fp.print("\tconst %s%s * returnValue = nullptr;\n",
			EnumLocationTypes[getTemplateLocation()],
			filenameLowerToUpper(param.extendedName).c_str());
		fp.print("\tconst std::string & templateName = m_%s[index]%sgetValue();\n",
			pname, access);
		fp.print("\tif (!templateName.empty())\n");
		fp.print("\t{\n");
		fp.print("\t\treturnValue = dynamic_cast<const %s%s *>("
			"ObjectTemplateList::fetch(templateName));\n",
			EnumLocationTypes[getTemplateLocation()],
			filenameLowerToUpper(param.extendedName).c_str());
		fp.print("\t\tif (returnValue == nullptr)\n");
		fp.print("\t\t\tWARNING_STRICT_FATAL(true, (\"Error loading template %%s\","
			"templateName.c_str()));\n");
		fp.print("\t}\n");
		fp.print("\treturn returnValue;\n");
	}
	fp.print("}\t// %s::get%s\n", templateName, upperName.c_str());
	fp.print("\n");
}	// TemplateData::writeSourceGetTemplate

/**
 * Writes functions to get enum parameters.
 *
 * @param fp		the file to write to
 * @param param		the parameter to get
 */
void TemplateData::writeSourceGetEnum(File &fp, const Parameter &param) const
{
	const std::string & classNameString = getName();
	const char *className = classNameString.c_str();
	const char *templateName = getFileParent()->getTemplateName().c_str();

	// make a version of the parameter name with the first character upper case
	const char *pname = param.name.c_str();
	std::string upperName = param.name;
	upperName[0] = static_cast<char>(toupper(upperName[0]));

	fp.print("%s::%s %s::get%s(", templateName,
		param.extendedName.c_str(), className, upperName.c_str());

	writeMethodParams(fp, param, false);
	fp.print(") const\n");
	fp.print("{\n");

	if (param.list_type == LIST_NONE)
	{
		// set up var to receive the test base value
		fp.print("#ifdef _DEBUG\n");
		fp.print("%s::%s testDataValue = static_cast<%s::%s>(0);\n", templateName,
			param.extendedName.c_str(), templateName, param.extendedName.c_str());
		fp.print("#else\n");
		fp.print("UNREF(testData);\n");
		fp.print("#endif\n\n");
	}

	fp.print("\tconst %s * base = nullptr;\n", className);
	fp.print("\tif (m_baseData != nullptr)\n");
	fp.print("\t{\n");
	fp.print("\t\tbase = dynamic_cast<const %s *>(m_baseData);\n", className);
//	fp.print("\t\tDEBUG_FATAL(base == nullptr, (\"base template wrong type\"));\n");

	if (param.list_type == LIST_NONE)
	{
		// get the test base value
		fp.print("#ifdef _DEBUG\n");
		fp.print("\t\tif (testData && base != nullptr)\n");
		fp.print("\t\t\ttestDataValue = base->get%s(true);\n", upperName.c_str());
		fp.print("#endif\n");
	}

	fp.print("\t}\n\n");

	if (param.list_type == LIST_NONE)
	{
		writeSourceReturnBaseValue(fp, param, "");
		fp.print("\t%s value = static_cast<%s>(m_%s.getValue());\n",
			param.extendedName.c_str(), param.extendedName.c_str(), pname);

		// check the return value vs the base value, and warn if the same
		fp.print("#ifdef _DEBUG\n");
		fp.print("\tif (testData && base != nullptr)\n");
		fp.print("\t{\n");
//		fp.print("\t\tif (testDataValue == value)\n");
//		fp.print("\t\t\tDEBUG_WARNING(true, (\"Template %%s, parameter %s is "
//			"returning same value as base template.\", DataResource::getName()));"
//			"\n", pname);
		fp.print("\t}\n");
		fp.print("#endif\n\n");

		fp.print("\treturn value;\n");
	}
	else
	{
		if (param.list_type == LIST_LIST)
		{
			writeSourceReturnBaseValue(fp, param, "");
			fp.print("\tDEBUG_FATAL(index < 0 || static_cast<size_t>"
				"(index) >= m_%s.size(), (\"template param index out of "
				"range\"));\n", pname);
		}
		else
		{
			fp.print("\tDEBUG_FATAL(index < 0 || index >= %d, (\""
				"template param index out of range\"));\n", param.list_size);
			writeSourceReturnBaseValue(fp, param, "");
		}
		fp.print("\treturn static_cast<%s>(m_%s[index]->getValue());\n",
			param.extendedName.c_str(), pname);
	}
	fp.print("}\t// %s::get%s\n", className, upperName.c_str());
	fp.print("\n");
}	// TemplateData::writeSourceGetEnum

/**
 * Writes functions to get structure parameters.
 *
 * @param fp		the file to write to
 * @param param		the parameter to get
 *
 * @return 0 on success, -1 on fail
 */
int TemplateData::writeSourceGetStruct(File &fp, const Parameter &param) const
{
std::string upperName;

	const std::string & templateNameString = getName();
	const char *templateName = templateNameString.c_str();

	// make a version of the parameter name with the first character upper case
	const char *pname = param.name.c_str();
	upperName = param.name;
	upperName[0] = static_cast<char>(toupper(upperName[0]));

	for (int i = 0; i < 3; ++i)
	{
		fp.print("void %s::get%s%s(", templateName, upperName.c_str(),
			MinMaxNames[i]);

		writeMethodParams(fp, param, false);
		fp.print(") const\n");
		fp.print("{\n");

		fp.print("\tconst %s * base = nullptr;\n", templateName);
		fp.print("\tif (m_baseData != nullptr)\n");
		fp.print("\t{\n");
		fp.print("\t\tbase = dynamic_cast<const %s *>(m_baseData);\n", templateName);
//		fp.print("\t\tDEBUG_FATAL(base == nullptr, (\"base template wrong type\"));\n");
		fp.print("\t}\n\n");

		if (param.list_type == LIST_NONE)
		{
			writeSourceReturnBaseValue(fp, param, MinMaxNames[i]);
			fp.print("\tconst ObjectTemplate * structTemplate = m_%s.getValue();\n",
				pname);
		}
		else
		{
			const char * access = "";
			if (param.list_type == LIST_LIST)
			{
				access = "*";

				writeSourceReturnBaseValue(fp, param, MinMaxNames[i]);
				fp.print("\tDEBUG_FATAL(index < 0 || static_cast<size_t>(index) >= m_%s.size(), ("
					"\"template param index out of range\"));\n", pname);
			}
			else
			{
				fp.print("\tDEBUG_FATAL(index < 0 || index >= %d, (\""
					"template param index out of range\"));\n", param.list_size);
				writeSourceReturnBaseValue(fp, param, MinMaxNames[i]);
			}
			fp.print("\tconst ObjectTemplate * structTemplate = (%sm_%s[index])."
				"getValue();\n", access, pname);
		}
		fp.print("\tNOT_NULL(structTemplate);\n");
		fp.print("\tconst _%s *param = dynamic_cast<const _%s *>(structTemplate);\n",
			param.extendedName.c_str(), param.extendedName.c_str());
		fp.print("\tNOT_NULL(param);\n");

		const TemplateData *structData = getStruct(param.extendedName.c_str());
		if (structData == nullptr)
		{
			fprintf(stderr, "unable to find structure %s\n",
				param.extendedName.c_str());
			return -1;
		}

		std::string versionString = "versionOk";
		if (m_templateParent == nullptr)
			versionString = "m_" + versionString;
		structData->writeSourceGetStructAssignments(fp, versionString, MinMaxNames[i]);

		fp.print("}\t// %s::get%s%s\n", templateName, upperName.c_str(),
			MinMaxNames[i]);
		fp.print("\n");
	}

	return 0;
}	// TemplateData::writeSourceGetStruct

/**
 * Writes the assignment statements for retrieving a structure.
 *
 * @param fp				the file to write to
 * @param versionString		string to use for versionOk checking
 * @param minMaxString		string to use for min/max getters
 */
void TemplateData::writeSourceGetStructAssignments(File &fp,
	const std::string & versionString, const std::string & minMaxString) const
{
const char * realMinMaxString;	// the min/max string for each param

	ParameterList::const_iterator iter;
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
			continue;

		if (HasMinMax[param.type])
			realMinMaxString = minMaxString.c_str();
		else
			realMinMaxString = "";

		const char *name = param.name.c_str();
		std::string upperName = param.name;
		upperName[0] = static_cast<char>(toupper(upperName[0]));
		if (param.type == TYPE_DYNAMIC_VAR || param.type == TYPE_STRUCT)
			switch (param.list_type)
		{
			case LIST_NONE:
				fp.print("\tparam->get%s%s(data.%s, %s);\n", upperName.c_str(),
					realMinMaxString, name, versionString.c_str());
				break;
			case LIST_INT_ARRAY:
				fp.print("\t{\n");
				fp.print("\t\tfor (int i = 0; i < %d; ++i)\n",
					param.list_size);
				fp.print("\t\t\tparam->get%s%s(data.%s[i], i, %s);\n",
					upperName.c_str(), realMinMaxString, name, versionString.c_str());
				fp.print("\t}\n");
				break;
			case LIST_ENUM_ARRAY:
				fp.print("\t{\n");
				fp.print("\t\tfor (int i = 0; i < %d; ++i)\n", param.list_size);
				fp.print("\t\t\tparam->get%s%s(data.%s[i], static_cast<%s>(i), "
					"%s);\n", upperName.c_str(), realMinMaxString, name,
					param.enum_list_name.c_str(), versionString.c_str());
				fp.print("\t}\n");
				break;
			case LIST_LIST:
				fp.print("\t{\n");
				fp.print("\t\tfor (size_t i = 0; i < param->get%sCount(); ++i)\n",
					upperName.c_str());
				fp.print("\t\t{\n");
				fp.print("\t\t\tdata.%s.push_back(%s());\n", name,
					param.extendedName.c_str());
				fp.print("\t\t\tparam->get%s%s(data.%s.back(), i, %s);\n",
					upperName.c_str(), realMinMaxString, name, versionString.c_str());
				fp.print("\t\t}\n");
				fp.print("\t}\n");
				break;
			default:
				break;
		}
		else switch (param.list_type)
		{
			case LIST_NONE:
				fp.print("\tdata.%s = param->get%s%s(%s);\n", name, upperName.c_str(),
					realMinMaxString, versionString.c_str());
				break;
			case LIST_LIST:
				fp.print("\t{\n");
				fp.print("\t\tfor (size_t i = 0; i < param->get%sCount(); ++i)\n",
					upperName.c_str());
				fp.print("\t\t\tdata.%s.push_back(param->get%s%s(i, %s));\n",
					name, upperName.c_str(), realMinMaxString, versionString.c_str());
				fp.print("\t}\n");
				break;
			case LIST_INT_ARRAY:
				fp.print("\t{\n");
				fp.print("\t\tfor (int i = 0; i < %d; ++i)\n", param.list_size);
				fp.print("\t\t\tdata.%s[i] = param->get%s%s(i, %s);\n", name,
					upperName.c_str(), realMinMaxString, versionString.c_str());
				fp.print("\t}\n");
				break;
			case LIST_ENUM_ARRAY:
				fp.print("\t{\n");
				fp.print("\t\tfor (int i = 0; i < %d; ++i)\n", param.list_size);
				fp.print("\t\t\tdata.%s[i] = param->get%s%s(static_cast<%s>(i), "
					"%s);\n", name, upperName.c_str(), realMinMaxString,
					param.enum_list_name.c_str(), versionString.c_str());
				fp.print("\t}\n");
				break;
			default:
				break;
		}
	}
}	// TemplateData::writeSourceGetStructAssignments

/**
 * Writes the source code to load the template data.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeSourceReadIff(File &fp) const
{
	const char * const * variableNames = DataVariableNames;
	if (m_writeForCompilerFlag)
		variableNames = CompilerDataVariableNames;

	const std::string & templateNameString = getName();
	const std::string & baseNameString = getBaseName();
	const char *templateName = templateNameString.c_str();
	const char *baseName = baseNameString.c_str();

	fp.print("/**\n");
	fp.print(" * Loads the template data from an iff file. We should already be in the form\n");
	fp.print(" * for this template.\n");
	fp.print(" *\n");
	fp.print(" * @param file\t\tfile to load from\n");
	fp.print(" */\n");
	fp.print("void %s::load(Iff &file)\n", templateName);
	fp.print("{\n");
	fp.print("static const int MAX_NAME_SIZE = 256;\n");
	fp.print("char paramName[MAX_NAME_SIZE];\n");
	fp.print("\n");

	if (m_fileParent != nullptr)
	{
		// check that we are in our form
		fp.print("\tif (file.getCurrentName() != %s_tag)\n",
			m_fileParent->getTemplateName().c_str());
		fp.print("\t{\n");
		if (!getBaseName().empty() &&  getBaseName() != ROOT_TEMPLATE_NAME)
			fp.print("\t\t%s::load(file);\n", baseName);
		fp.print("\t\treturn;\n");
		fp.print("\t}\n");
		fp.print("\n");

		// enter the form and check it's version
		fp.print("\tfile.enterForm();\n");
		fp.print("\tm_templateVersion = file.getCurrentName();\n");
		fp.print("\tif (m_templateVersion == %s)\n", DERIVED_TEMPLATE_TAG_STRING);

		// load the base template
		fp.print("\t{\n");
		fp.print("\t\tfile.enterForm();\n");
		fp.print("\t\tfile.enterChunk();\n");
		fp.print("\t\tstd::string baseFilename;\n");
		fp.print("\t\tfile.read_string(baseFilename);\n");
		fp.print("\t\tfile.exitChunk();\n");
		fp.print("\t\tconst ObjectTemplate *base = ObjectTemplateList::fetch(baseFilename);\n");


		//		fp.print("\t\t%s * mybase = dynamic_cast<%s *>(base);\n",
		//			templateName, templateName);
		//		fp.print("\t\tFATAL(mybase == nullptr, (\"trying to derive a template from an incompatable template type\"));\n");
		fp.print("\t\tDEBUG_WARNING(base == nullptr, (\"was unable to load base template %%s\", baseFilename.c_str()));\n");
		fp.print("\t\tif (m_baseData == base && base != nullptr)\n");
		fp.print("\t\t\tbase->releaseReference();\n");

		fp.print("\t\telse\n");
		fp.print("\t\t{\n");
		fp.print("\t\t\tif (m_baseData != nullptr)\n");
		fp.print("\t\t\t\tm_baseData->releaseReference();\n");

		fp.print("\t\t\tm_baseData = base;\n");

		fp.print("\t\t}\n");
		fp.print("\t\tfile.exitForm();\n");
		fp.print("\t\tm_templateVersion = file.getCurrentName();\n");
		fp.print("\t}\n");
		fp.print("\tif (getHighestTemplateVersion() != %s)\n", m_version.tagString.c_str());
		fp.print("\t{\n");
		fp.print("\t\tif (DataLint::isEnabled())\n");
		fp.print("\t\t\tDEBUG_WARNING(true, (\"template %%s version out of date\", file.getFileName()));\n");
		if (!isWritingForCompiler())
			fp.print("\t\tm_versionOk = false;\n");
		fp.print("\t}\n");
		fp.print("\n");
		fp.print("\tfile.enterForm();\n");
	}
	else
	{
		fp.print("\tfile.enterForm();\n");
	}
	fp.print("\n");

	int paramCount = 0;
	ParameterList::const_iterator iter;
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
			continue;
		++paramCount;
	}

	// read the number of params
	fp.print("\tfile.enterChunk();\n");
	fp.print("\tint paramCount = file.read_int32();\n");
	fp.print("\tfile.exitChunk();\n");

	if (paramCount > 0)
	{
		fp.print("\tfor (int i = 0; i < paramCount; ++i)\n");
		fp.print("\t{\n");
		fp.print("\t\tfile.enterChunk();\n");
		fp.print("\t\tfile.read_string(paramName, MAX_NAME_SIZE);\n");

		bool firstParam = true;
		for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
		{
			const Parameter &param = (*iter);
			if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
				continue;

			fp.print("\t\t");
			if (!firstParam)
				fp.print("else ");
			fp.print("if (strcmp(paramName, \"%s\") == 0)\n", param.name.c_str());
			firstParam = false;
			switch (param.list_type)
			{
				case LIST_NONE:
					fp.print("\t\t\tm_%s.loadFromIff(file);\n", param.name.c_str());
					break;
				case LIST_LIST:
					fp.print("\t\t{\n");
					fp.print("\t\t\tstd::vector<%s *>::iterator iter;\n",
						variableNames[param.type]);
					fp.print("\t\t\tfor (iter = m_%s.begin(); iter != m_%s.end();"
						" ++iter)\n", param.name.c_str(), param.name.c_str());
					fp.print("\t\t\t{\n");
					fp.print("\t\t\t\tdelete *iter;\n");
					fp.print("\t\t\t\t*iter = nullptr;\n");
					fp.print("\t\t\t}\n");
					fp.print("\t\t\tm_%s.clear();\n", param.name.c_str());
					fp.print("\t\t\tm_%sAppend = file.read_bool8();\n", param.name.c_str());
					fp.print("\t\t\tint listCount = file.read_int32();\n");
					fp.print("\t\t\tfor (int j = 0; j < listCount; ++j)\n");
					fp.print("\t\t\t{\n");
					fp.print("\t\t\t\t%s * newData = new %s;\n",
						variableNames[param.type],
						variableNames[param.type]);
					fp.print("\t\t\t\tnewData->loadFromIff(file);\n");
					fp.print("\t\t\t\tm_%s.push_back(newData);\n", param.name.c_str());
					fp.print("\t\t\t}\n");
					fp.print("\t\t\tm_%sLoaded = true;\n", param.name.c_str());
					fp.print("\t\t}\n");
					break;
				case LIST_INT_ARRAY:
				case LIST_ENUM_ARRAY:
					fp.print("\t\t{\n");
					fp.print("\t\t\tint listCount = file.read_int32();\n");
					fp.print("\t\t\tDEBUG_WARNING(listCount != %d, (\"Template %%s: "
						"read array size of %%d for array \\\"%s\\\" of size %d, reading "
						"values anyway\", file.getFileName(), listCount));\n",
						param.list_size, param.name.c_str(), param.list_size);
					fp.print("\t\t\tint j;\n");
					fp.print("\t\t\tfor (j = 0; j < %d && j < listCount; ++j)\n", param.list_size);
					fp.print("\t\t\t\tm_%s[j].loadFromIff(file);\n", param.name.c_str());
					fp.print("\t\t\t// if there are more params for %s read and dump them\n", param.name.c_str());
					fp.print("\t\t\tfor (; j < listCount; ++j)\n");
					fp.print("\t\t\t{\n");
					fp.print("\t\t\t\t%s dummy;\n", variableNames[param.type]);
					fp.print("\t\t\t\tdummy.loadFromIff(file);\n");
					fp.print("\t\t\t}\n");
					fp.print("\t\t}\n");
					break;
				default:
					break;
			}
		}
		fp.print("\t\tfile.exitChunk(true);\n");
		fp.print("\t}\n");
	}
	else
	{
		fp.print("\tUNREF(paramName);\n");
		fp.print("\tUNREF(paramCount);\n");
	}

	// exit the form
	fp.print("\n");
	fp.print("\tfile.exitForm();\n");

	if (m_fileParent != nullptr)
	{
		// enter the next form
		if (!baseNameString.empty() && baseNameString != ROOT_TEMPLATE_NAME)
		{
			fp.print("\t%s::load(file);\n", baseName);
			fp.print("\tfile.exitForm();\n");
		}
		fp.print("\treturn;\n");
	}
	else
		fp.print("\tUNREF(file);\n");

	fp.print("}	// %s::load\n", templateName);
	fp.print("\n");
}	// TemplateData::writeSourceReadIff

/**
 * Writes the source code to save the template data.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeSourceWriteIff(File &fp) const
{
	const std::string & templateNameString = getName();
	const std::string & baseNameString = getBaseName();
	const char *templateName = templateNameString.c_str();
	const char *baseName = baseNameString.c_str();

	fp.print("/**\n");
	fp.print(" * Saves the template data to an iff file.\n");
	fp.print(" *\n");
	fp.print(" * @param file\t\t\tfile to save to\n");
	if (m_writeForCompilerFlag)
		fp.print(" * @param location\t\tfile type (client or server)\n");
	fp.print(" */\n");
	fp.print("void %s::save(Iff &file)\n", templateName);
	fp.print("{\n");

	fp.print("int count;\n\n");

	// write form enter header stuff
	if (m_fileParent != nullptr)
	{
		fp.print("\tfile.insertForm(%s_tag);\n",
			m_fileParent->getTemplateName().c_str());
		fp.print("\tif (m_baseTemplateName.size() != 0)\n");
		fp.print("\t{\n");
		fp.print("\t\tfile.insertForm(%s);\n", DERIVED_TEMPLATE_TAG_STRING);
		fp.print("\t\tfile.insertChunk(TAG(X, X, X, X));\n");
		fp.print("\t\tfile.insertChunkData(m_baseTemplateName.c_str(), "
			"m_baseTemplateName.size() + 1);\n");
		fp.print("\t\tfile.exitChunk();\n");
		fp.print("\t\tfile.exitForm();\n");
		fp.print("\t}\n");
		fp.print("\tfile.insertForm(%s);\n", m_version.tagString.c_str());
		fp.print("\tfile.allowNonlinearFunctions();\n");
	}
	else
	{
		fp.print("\tfile.insertForm(_%s_tag);\n", m_name.c_str());
	}
	fp.print("\n");

	// determine how many parameters there are
	ParameterList::const_iterator iter;
	fp.print("\tint paramCount = 0;\n");
	fp.print("\n");

	bool countReffed = false;		// flag that the 'count' parameter will
									// be referenced
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
			continue;

		const char * extraTab = "";
		if (m_writeForCompilerFlag)
		{
			if (param.list_type == LIST_LIST)
			{
				fp.print("\tif (!m_%sLoaded)\n", param.name.c_str());
				fp.print("\t{\n");
				fp.print("\t\t// mark the list as empty and extending the base list\n");
				fp.print("\t\tm_%sAppend = true;\n", param.name.c_str());
				fp.print("\t}\n");
			}
//			fp.print("\t{\n");
//			extraTab = "\t";
		}
		fp.print("%s\t// save %s\n", extraTab, param.name.c_str());
		fp.print("%s\tfile.insertChunk(TAG(X, X, X, X));\n", extraTab);
		fp.print("%s\tfile.insertChunkString(\"%s\");\n", extraTab,
			param.name.c_str());
		switch (param.list_type)
		{
			case LIST_NONE:
				fp.print("%s\tm_%s.saveToIff(file);\n", extraTab, param.name.c_str());
				break;
			case LIST_LIST:
				fp.print("%s\tfile.insertChunkData(&m_%sAppend, sizeof(bool));\n",
					extraTab, param.name.c_str());
				fp.print("%s\tcount = m_%s.size();\n", extraTab, param.name.c_str());
				fp.print("%s\tfile.insertChunkData(&count, sizeof(count"
					"));\n", extraTab);
				fp.print("%s\t{for (int i = 0; i < count; ++i)\n", extraTab);
				fp.print("%s\t\tm_%s[i]->saveToIff(file);}\n", extraTab,
					param.name.c_str());
				countReffed = true;
				break;
			case LIST_INT_ARRAY:
			case LIST_ENUM_ARRAY:
				fp.print("%s\tcount = %d;\n", extraTab, param.list_size);
				fp.print("%s\tfile.insertChunkData(&count, sizeof(count"
					"));\n", extraTab);
				fp.print("%s\t{for (int i = 0; i < %d; ++i)\n", extraTab,
					param.list_size);
				fp.print("%s\t\tm_%s[i].saveToIff(file);}\n", extraTab,
					param.name.c_str());
				countReffed = true;
				break;
			default:
				break;
		}
		fp.print("%s\tfile.exitChunk();\n", extraTab);
		fp.print("%s\t++paramCount;\n", extraTab);
		if (m_writeForCompilerFlag)
		{
//			fp.print("\t}\n");
		}
	}

	// write exit header stuff
	fp.print("\n");
	fp.print("\t// write number of parameters\n");
	fp.print("\tfile.goToTopOfForm();\n");
	fp.print("\tfile.insertChunk(TAG(P, C, N, T));\n");
//	fp.print("\tfile.enterChunk();\n");
	fp.print("\tfile.insertChunkData(&paramCount, sizeof(paramCount));\n");
	fp.print("\tfile.exitChunk();\n");
	fp.print("\n");
	fp.print("\tfile.exitForm(true);\n");

	if (m_fileParent != nullptr)
	{
		// call base class write iff method
		if (!getBaseName().empty() && getBaseName() != ROOT_TEMPLATE_NAME)
			fp.print("\t%s::save(file);\n", baseName);

		// exit the template form
		fp.print("\tfile.exitForm();\n");
	}

	if (!countReffed)
		fp.print("\tUNREF(count);\n");

	fp.print("}	// %s::save\n", templateName);
	fp.print("\n");
}	// TemplateData::writeSourceWriteIff

/**
 * Writes the source code to clean up class data.
 *
 * @param fp		the file to write to
 */
void TemplateData::writeSourceCleanup(File &fp) const
{
	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_CLEANUP_BEGIN);

	const char * const * variableNames = DataVariableNames;
	if (m_writeForCompilerFlag)
		variableNames = CompilerDataVariableNames;

	ParameterList::const_iterator iter;
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
			continue;

		switch (param.type)
		{
			case TYPE_INTEGER:
			case TYPE_ENUM:
			case TYPE_FLOAT:
			case TYPE_BOOL:
			case TYPE_STRING:
			case TYPE_STRINGID:
			case TYPE_FILENAME:
			case TYPE_VECTOR:
			case TYPE_TEMPLATE:
			case TYPE_DYNAMIC_VAR:
			case TYPE_STRUCT:
			case TYPE_TRIGGER_VOLUME:
				if (param.list_type == LIST_LIST)
				{
					fp.print("\t{\n");
					fp.print("\t\tstd::vector<%s *>::iterator iter;\n",
						variableNames[param.type]);
					fp.print("\t\tfor (iter = m_%s.begin(); iter != m_%s.end();"
						" ++iter)\n", param.name.c_str(), param.name.c_str());
					fp.print("\t\t{\n");
					fp.print("\t\t\tdelete *iter;\n");
					fp.print("\t\t\t*iter = nullptr;\n");
					fp.print("\t\t}\n");
					fp.print("\t\tm_%s.clear();\n", param.name.c_str());
					fp.print("\t}\n");
				}
				break;
			default:
				break;
		}
	}

	if (m_fileParent != nullptr)
		fp.print("%s\n", TDF_CLEANUP_END);
}	// TemplateData::writeSourceCleanup

/**
 * Writes parameter access functions for use in the template compiler.
 *
 * @param fp		the file to write to
 * @param location	the file location (client or server)
 */
void TemplateData::writeCompilerSourceAccessMethods(File &fp) const
{
const char * const FUNC_NAMES[] = {
	"CompilerIntegerParam",
	"FloatParam",
	"BoolParam",
	"StringParam",
	"StringIdParam",
	"VectorParam",
	"DynamicVariableParam",
	"StructParamOT",
	"TriggerVolumeParam"
};
const int FUNC_NAME_SIZE = sizeof(FUNC_NAMES) / sizeof(FUNC_NAMES[0]);
// map data types to FUNC_NAMES above
const std::map<int, int>::value_type PARAM_FUNC_MAP[] = {
	std::map<int, int>::value_type(TYPE_NONE, -1),
	std::map<int, int>::value_type(TYPE_COMMENT, -1),
	std::map<int, int>::value_type(TYPE_INTEGER, 0),
	std::map<int, int>::value_type(TYPE_FLOAT, 1),
	std::map<int, int>::value_type(TYPE_BOOL, 2),
	std::map<int, int>::value_type(TYPE_STRING, 3),
	std::map<int, int>::value_type(TYPE_STRINGID, 4),
	std::map<int, int>::value_type(TYPE_VECTOR, 5),
	std::map<int, int>::value_type(TYPE_DYNAMIC_VAR, 6),
	std::map<int, int>::value_type(TYPE_TEMPLATE, 3),
	std::map<int, int>::value_type(TYPE_ENUM, 0),
	std::map<int, int>::value_type(TYPE_STRUCT, 7),
	std::map<int, int>::value_type(TYPE_TRIGGER_VOLUME, 8),
	std::map<int, int>::value_type(TYPE_FILENAME, 3),
	std::map<int, int>::value_type(-1, -1)
};
const int PARAM_FUNC_MAP_SIZE = sizeof(PARAM_FUNC_MAP) / sizeof(PARAM_FUNC_MAP[0]);
const std::map<int, int> paramFuncMap(&PARAM_FUNC_MAP[0], &PARAM_FUNC_MAP[PARAM_FUNC_MAP_SIZE-1]);
int paramCount;
bool first;
ParameterList::const_iterator iter;

	DEBUG_FATAL(!m_writeForCompilerFlag, ("write for compiler not enabled"));

	const char * const * variableNames = DataVariableNames;
	if (m_writeForCompilerFlag)
		variableNames = CompilerDataVariableNames;

	const std::string & templateNameString = getName();
	const std::string & baseNameString = getBaseName();
	const char *templateName = templateNameString.c_str();
	const char *baseName = baseNameString.c_str();

	// write code for predefined-type access methods
	for (int i = 0; i < FUNC_NAME_SIZE; ++i)
	{
		fp.print("%s * %s::get%s(const char *name, bool deepCheck, int index)\n",
			FUNC_NAMES[i], templateName, FUNC_NAMES[i]);
		fp.print("{\n");

		paramCount = 0;
		first = true;
		for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
		{
			const Parameter &param = (*iter);
			if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
				continue;

			std::map<int, int>::const_iterator mapIter = paramFuncMap.find(param.type);
			if (mapIter != paramFuncMap.end() && (*mapIter).second == i)
			{
				++paramCount;
				const char *pname = param.name.c_str();
				if (first)
				{
					fp.print("\tif");
					first = false;
				}
				else
					fp.print("\telse if");
				fp.print(" (strcmp(name, \"%s\") == 0)\n", pname);
				fp.print("\t{\n");
				switch (param.list_type)
				{
					case LIST_NONE:
						fp.print("\t\tif (index == 0)\n");
						fp.print("\t\t{\n");
						fp.print("\t\t\tif (deepCheck && !isParamLoaded(name, false, 0))\n");
						fp.print("\t\t\t{\n");
						fp.print("\t\t\t\tif (getBaseTemplate() != nullptr)\n");
						fp.print("\t\t\t\t\treturn getBaseTemplate()->get%s(name, deepCheck, index);\n", FUNC_NAMES[i]);
						fp.print("\t\t\t\treturn nullptr;\n");
						fp.print("\t\t\t}\n");
						fp.print("\t\t\treturn &m_%s;\n", pname);
						fp.print("\t\t}\n");
						fp.print("\t\tfprintf(stderr, \"trying to access "
							"single-parameter \\\"%s\\\" as an array\\n\");\n",
							pname);
						break;
					case LIST_LIST:
						fp.print("\t\tif (index >= 0 && index < static_cast<int>(m_%s.size()))\n",
							pname);
						fp.print("\t\t\treturn m_%s[index];\n", pname);
						fp.print("\t\tif (index == static_cast<int>(m_%s.size()))\n",
							pname);
						fp.print("\t\t{\n");
						fp.print("\t\t\t%s *temp = new %s();\n",
							FUNC_NAMES[i], FUNC_NAMES[i]);
						fp.print("\t\t\tm_%s.push_back(temp);\n", pname);
						fp.print("\t\t\treturn temp;\n");
						fp.print("\t\t}\n");
						fp.print("\t\tfprintf(stderr, \"index for parameter "
							"\\\"%s\\\" out of bounds\\n\");\n", pname);
						break;
					case LIST_INT_ARRAY:
					case LIST_ENUM_ARRAY:
						fp.print("\t\tif (index >= 0 && index < %d)\n",
							param.list_size);
						fp.print("\t\t{\n");
						fp.print("\t\t\tif (deepCheck && !isParamLoaded(name, false, index))\n");
						fp.print("\t\t\t{\n");
						fp.print("\t\t\t\tif (getBaseTemplate() != nullptr)\n");
						fp.print("\t\t\t\t\treturn getBaseTemplate()->get%s(name, deepCheck, index);\n", FUNC_NAMES[i]);
						fp.print("\t\t\t\treturn nullptr;\n");
						fp.print("\t\t\t}\n");
						fp.print("\t\t\treturn &m_%s[index];\n", pname);
						fp.print("\t\t}\n");
						fp.print("\t\tfprintf(stderr, \"index for parameter "
							"\\\"%s\\\" out of bounds\\n\");\n", pname);
						break;
					default:
						break;
				}
				fp.print("\t}\n");
			}
		}

		if (getBaseName().size() > 0)
		{
			if (paramCount != 0)
				fp.print("\telse\n\t");
			fp.print("\treturn %s::get%s(name, deepCheck, index);\n", baseName, FUNC_NAMES[i]);
		}
		if (paramCount != 0)
			fp.print("\treturn nullptr;\n");
		fp.print("}\t//%s::get%s\n", templateName, FUNC_NAMES[i]);
		fp.print("\n");
	}

	// write structure initialization function
	paramCount = 0;
	first = true;
	fp.print("void %s::initStructParamOT(StructParamOT &param, const char *name)\n",
		templateName);
	fp.print("{\n");
	fp.print("\tif (param.isInitialized())\n");
	fp.print("\t\treturn;\n");
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type != TYPE_STRUCT)
			continue;

		++paramCount;
		const char *pname = param.name.c_str();
		if (first)
		{
			fp.print("\tif");
			first = false;
		}
		else
			fp.print("\telse if");
		fp.print(" (strcmp(name, \"%s\") == 0)\n", pname);
		fp.print("\t\tparam.setValue(new _%s(\"\"));\n", param.extendedName.c_str());
	}
	if (getBaseName().size() > 0)
	{
		if (paramCount != 0)
			fp.print("\telse\n\t");
		fp.print("\t%s::initStructParamOT(param, name);\n", baseName);
	}
	fp.print("}	// %s::initStructParamOT\n", templateName);

	// write empty list flag function
	paramCount = 0;
	first = true;
	fp.print("\n");
	fp.print("void %s::setAsEmptyList(const char *name)\n", templateName);
	fp.print("{\n");
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.list_type != LIST_LIST)
			continue;

		++paramCount;
		const char *pname = param.name.c_str();
		if (first)
		{
			fp.print("\tif");
			first = false;
		}
		else
			fp.print("\telse if");
		fp.print(" (strcmp(name, \"%s\") == 0)\n", pname);
		fp.print("\t{\n");
		fp.print("\t\tm_%s.clear();\n", pname);
		fp.print("\t\tm_%sLoaded = true;\n", pname);
		fp.print("\t}\n");
	}
	if (getBaseName().size() > 0)
	{
		if (paramCount != 0)
			fp.print("\telse\n\t");
		fp.print("\t%s::setAsEmptyList(name);\n", baseName);
	}
	fp.print("}	// %s::setAsEmptyList\n", templateName);
	fp.print("\n");
/*
	// write isLoaded function
	paramCount = 0;
	first = true;
	bool needFinalReturn = false;
	fp.print("bool %s::isLoaded(const char *name, int index) const\n", templateName);
	fp.print("{\n");
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
			continue;

		++paramCount;
		const char *pname = param.name.c_str();
		if (first)
		{
			fp.print("\tif");
			first = false;
		}
		else
			fp.print("\telse if");
		fp.print(" (strcmp(name, \"%s\") == 0)\n", pname);
		switch (param.list_type)
		{
			case LIST_NONE:
				fp.print("\t{\n");
				fp.print("\t\tif (index == 0)\n");
				fp.print("\t\t\treturn m_%s.isLoaded();\n", pname);
				fp.print("\t\tfprintf(stderr, \"trying to access "
					"single-parameter \\\"%s\\\" as an array\\n\");\n",
					pname);
				fp.print("\t}\n");
				needFinalReturn = true;
				break;
			case LIST_LIST:
				fp.print("\t\treturn m_%sLoaded;\n", pname);
				break;
			case LIST_INT_ARRAY:
			case LIST_ENUM_ARRAY:
				fp.print("\t{\n");
				fp.print("\t\tif (index >= 0 && index < %d)\n",
					param.list_size);
				fp.print("\t\t\treturn m_%s[index].isLoaded();\n", pname);
				fp.print("\t\tfprintf(stderr, \"index for parameter "
					"\\\"%s\\\" out of bounds\\n\");\n", pname);
				fp.print("\t}\n");
				needFinalReturn = true;
				break;
			default:
				break;
		}
	}
	// if we are a derived class, go down the hierarchy
	if (getBaseName().size() > 0)
	{
		if (paramCount != 0)
			fp.print("\telse\n\t");
		fp.print("\treturn %s::isLoaded(name, index);\n", baseName);
	}
	if (needFinalReturn)
		fp.print("\treturn false;\n");
	fp.print("}	// %s::isLoaded\n", templateName);
	fp.print("\n");
*/
	// write setAppend function
	first = true;
	paramCount = 0;
	fp.print("void %s::setAppend(const char *name)\n", templateName);
	fp.print("{\n");
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.list_type != LIST_LIST)
			continue;

		++paramCount;
		const char *pname = param.name.c_str();
		if (first)
		{
			fp.print("\tif");
			first = false;
		}
		else
			fp.print("\telse if");
		fp.print(" (strcmp(name, \"%s\") == 0)\n", pname);
		fp.print("\t\tm_%sAppend = true;\n", pname);
	}
	// if we are a derived class, go down the hierarchy
	if (getBaseName().size() > 0)
	{
		if (paramCount > 0)
			fp.print("\telse\n\t");
		fp.print("\t%s::setAppend(name);\n", baseName);
	}
	else if (paramCount == 0)
		fp.print("\tUNREF(name);\n");
	fp.print("}	// %s::setAppend\n", templateName);
	fp.print("\n");

	// write isAppend function
	first = true;
	paramCount = 0;
	fp.print("bool %s::isAppend(const char *name) const\n", templateName);
	fp.print("{\n");
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.list_type != LIST_LIST)
			continue;

		++paramCount;
		const char *pname = param.name.c_str();
		if (first)
		{
			fp.print("\tif");
			first = false;
		}
		else
			fp.print("\telse if");
		fp.print(" (strcmp(name, \"%s\") == 0)\n", pname);
		fp.print("\t\treturn m_%sAppend;\n", pname);
	}
	// if we are a derived class, go down the hierarchy
	if (getBaseName().size() > 0)
	{
		if (paramCount > 0)
			fp.print("\telse\n\t");
		fp.print("\treturn %s::isAppend(name);\n", baseName);
	}
	else if (paramCount == 0)
		fp.print("\tUNREF(name);\n");
	fp.print("}	// %s::isAppend\n", templateName);
	fp.print("\n");

	// write getListLength function
	paramCount = 0;
	first = true;
	fp.print("\n");
	fp.print("int %s::getListLength(const char *name) const\n", templateName);
	fp.print("{\n");
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = (*iter);
		if (param.list_type == LIST_NONE)
			continue;

		++paramCount;
		const char *pname = param.name.c_str();
		if (first)
		{
			fp.print("\tif");
			first = false;
		}
		else
			fp.print("\telse if");
		fp.print(" (strcmp(name, \"%s\") == 0)\n", pname);
		fp.print("\t{\n");

		if(param.list_type == LIST_LIST)
		{
			fp.print("\t\treturn m_%s.size();\n", pname);
		}
		else // param.list_type == LIST_INT or LIST_ENUM
		{
			fp.print("\t\treturn sizeof(m_%s) / sizeof(%s);\n", pname, variableNames[param.type]);
		}

		fp.print("\t}\n");
	}
	// if we are a derived class, go down the hierarchy
	if (getBaseName().size() > 0)
	{
		if (paramCount != 0)
			fp.print("\telse\n\t");
		fp.print("\treturn %s::getListLength(name);\n", baseName);
	}
	fp.print("}	// %s::getListLength\n", templateName);
	fp.print("\n");
}	// TemplateData::writeCompilerSourceAccessMethods


//========================================================================
// default template writing functions

/**
 * Writes a default template file that can be used to make a template iff file.
 *
 * @param fp		file to write to
 */
void TemplateData::writeDefaultTemplateFile(File &fp) const
{
	writeEnumsToDefaultTemplate(fp);
	writeStructsToDefaultTemplate(fp);

	ParameterList::const_iterator iter;
	for (iter = m_parameters.begin(); iter != m_parameters.end(); ++iter)
	{
		const Parameter &param = *iter;
		if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
			continue;

		writeParamTypeToDefaultTemplate(fp, param);
		writeParameterDefault(fp, param);
	}
}	// TemplateData::writeDefaultTemplateFile

/**
 * Writes the enums defined for this template as comments.
 *
 * @param fp		file to write to
 */
void TemplateData::writeEnumsToDefaultTemplate(File &fp) const
{
	EnumMap::const_iterator mapIter;
	for (mapIter = m_enumMap.begin(); mapIter != m_enumMap.end(); ++mapIter)
	{
		fp.print("// enum type %s:\n", (*mapIter).first.c_str());

		const EnumList &list = (*mapIter).second;
		EnumList::const_iterator listIter;
		for (listIter = list.begin(); listIter != list.end(); ++listIter)
		{
			const EnumData &data = *listIter;
			fp.print("//     %s", data.name.c_str());
			if (data.comment.size() != 0)
				fp.print("\t\t(%s)", data.comment.c_str());
			fp.print("\n");
		}

		fp.print("\n");
	}
}	// TemplateData::writeEnumsToDefaultTemplate

/**
 * Writes the structs defined for this template as comments.
 *
 * @param fp		file to write to
 */
void TemplateData::writeStructsToDefaultTemplate(File &fp) const
{
	StructMap::const_iterator mapIter;
	for (mapIter = m_structMap.begin(); mapIter != m_structMap.end(); ++mapIter)
	{
		fp.print("// struct %s:\n", (*mapIter).first.c_str());

		const TemplateData *data = (*mapIter).second;
		NOT_NULL(data);
		ParameterList::const_iterator listIter;
		for (listIter = data->m_parameters.begin();
			listIter != data->m_parameters.end(); ++listIter)
		{
			const Parameter &param = *listIter;
			if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
				continue;
			fp.print("//     %s\t\t", param.name.c_str());
			writeParamTypeToDefaultTemplate(fp, param);
		}

		fp.print("\n");
	}
}	// TemplateData::writeStructsToDefaultTemplate

/**
 * Writes a parameter type as a comment.
 *
 * @param fp		file to write to
 * @param param		parameter to write about
 */
void TemplateData::writeParamTypeToDefaultTemplate(File &fp,
	const Parameter &param) const
{
	fp.print("// ");
	switch (param.list_type)
	{
		case LIST_LIST:
			fp.print("list, of type ");
			break;
		case LIST_INT_ARRAY:
			fp.print("array, size %d, indexed by an integer, of type ",
				param.list_size);
			break;
		case LIST_ENUM_ARRAY:
			fp.print("array, indexed by enum type %s, of type ",
				param.enum_list_name.c_str());
			break;
		case LIST_NONE:
		default:
			break;
	}

	switch (param.type)
	{
		case TYPE_INTEGER:
			fp.print("integer\n");
			break;
		case TYPE_FLOAT:
			fp.print("floating point\n");
			break;
		case TYPE_BOOL:
			fp.print("boolean\n");
			break;
		case TYPE_STRING:
			fp.print("string\n");
			break;
		case TYPE_STRINGID:
			fp.print("string id\n");
			break;
		case TYPE_VECTOR:
			fp.print("vector\n");
			break;
		case TYPE_DYNAMIC_VAR:
			fp.print("objvar list\n");
			break;
		case TYPE_TEMPLATE:
			fp.print("template type %s\n", param.extendedName.c_str());
			break;
		case TYPE_ENUM:
			fp.print("enum type %s\n", param.extendedName.c_str());
			break;
		case TYPE_STRUCT:
			fp.print("struct %s\n", param.extendedName.c_str());
			break;
		case TYPE_TRIGGER_VOLUME:
			fp.print("trigger volume\n");
			break;
		case TYPE_FILENAME:
			fp.print("filename\n");
			break;
		case TYPE_COMMENT:
		case TYPE_NONE:
		default:
			break;
	}
}	// TemplateData::writeParamTypeToDefaultTemplate

/**
 * Writes a parameter default to a template file.
 *
 * @param fp		file to write to
 * @param param		parameter to write
 * @param index		index of param to write (for fixed-length lists; <0 to write all of list)
 */
void TemplateData::writeParameterDefault(File &fp, const Parameter &param, int index) const
{
	switch (param.list_type)
	{
		case LIST_NONE:
			fp.print("%s = ", param.name.c_str());
			writeDefaultValue(fp, param);
			fp.print("\n");
			break;
		case LIST_LIST:
			fp.print("%s = [ ", param.name.c_str());
//			writeDefaultValue(fp, param);
			fp.print(" ]\n");
			break;
		case LIST_INT_ARRAY:
			{
				if (index >= 0 && index < param.list_size)
				{
					fp.print("%s[%d] = ", param.name.c_str(), index);
					writeDefaultValue(fp, param);
					fp.print("\n");
				}
				else
				{
					for (int i = 0; i < param.list_size; ++i)
					{
						fp.print("%s[%d] = ", param.name.c_str(), i);
						writeDefaultValue(fp, param);
						fp.print("\n");
					}
				}
			}
			break;
		case LIST_ENUM_ARRAY:
			{
				const EnumList * enumList = getEnumList(param.enum_list_name, false);
				DEBUG_FATAL(enumList == nullptr, ("unknown enum name %s",
					param.enum_list_name.c_str()));

				if (index >= 0 && index < param.list_size)
				{
					fp.print("%s[%s] = ", param.name.c_str(), enumList->at(index).name.c_str());
					writeDefaultValue(fp, param);
					fp.print("\n");
				}
				else
				{
					EnumList::const_iterator listIter;
					for (listIter = enumList->begin(); listIter != enumList->end(); ++listIter)
					{
						fp.print("%s[%s] = ", param.name.c_str(), (*listIter).name.c_str());
						writeDefaultValue(fp, param);
						fp.print("\n");
					}
				}
			}
			break;
		default:
			break;
	}
}	// TemplateData::writeParameterDefault

/**
 * Writes a structure parameter default to a template file.
 *
 * @param fp		file to write to
 * @param param		parameter to write
 * @param final		flag that this is the final struct
 */
void TemplateData::writeStructParameterDefault(File &fp, const Parameter &param,
	bool final) const
{
	switch (param.list_type)
	{
		case LIST_NONE:
			fp.print("\t%s = ", param.name.c_str());
			writeDefaultValue(fp, param);
			break;
		case LIST_LIST:
			fp.print("\t%s = [ ", param.name.c_str());
//			writeDefaultValue(fp, param);
			fp.print(" ]", param.name.c_str());
			break;
		case LIST_INT_ARRAY:
			{
				for (int i = 0; i < param.list_size; ++i)
				{
					fp.print("\t%s[%d] = ", param.name.c_str(), i);
					writeDefaultValue(fp, param);
					if (i < param.list_size - 1)
						fp.print(",\n", param.name.c_str());
				}
			}
			break;
		case LIST_ENUM_ARRAY:
			{
				const EnumList * enumList = getEnumList(param.enum_list_name, false);
				DEBUG_FATAL(enumList == nullptr, ("unknown enum name %s",
					param.enum_list_name.c_str()));

				EnumList::const_iterator listIter;
				for (listIter = enumList->begin(); listIter != enumList->end(); ++listIter)
				{
					fp.print("\t%s[%s] = ", param.name.c_str(), (*listIter).name.c_str());
					writeDefaultValue(fp, param);
					if (listIter + 1 != enumList->end())
						fp.print(",\n", param.name.c_str());
				}
			}
			break;
		default:
			break;
	}
	if (!final)
		fp.print(",\n", param.name.c_str());
}	// TemplateData::writeStructParameterDefault

/**
 * Writes the default value for a template parameter to a template file.
 *
 * @param fp		file to write to
 * @param param		parameter to write
 */
void TemplateData::writeDefaultValue(File &fp, const Parameter &param) const
{
	switch (param.type)
	{
		case TYPE_INTEGER:
			if (0 < param.min_int_limit)
				fp.print("%d", param.min_int_limit);
			else if (0 > param.max_int_limit)
				fp.print("%d", param.max_int_limit);
			else
				fp.print("0");
			break;
		case TYPE_FLOAT:
			if (0 < param.min_float_limit)
				fp.print("%f", param.min_float_limit);
			else if (0 > param.max_float_limit)
				fp.print("%f", param.max_float_limit);
			else
			fp.print("0.0");
			break;
		case TYPE_BOOL:
			fp.print("false");
			break;
		case TYPE_STRING:
		case TYPE_FILENAME:
			fp.print("\"\"");
			break;
		case TYPE_STRINGID:
			fp.print("\"string_id_table\" \"\"");
			break;
		case TYPE_VECTOR:
			fp.print("0.0 0.0 0.0 0.0");
			break;
		case TYPE_DYNAMIC_VAR:
			fp.print("[]");
			break;
		case TYPE_TEMPLATE:
//			fp.print("\"%s\"", getName().c_str());
			fp.print("\"\"");
			break;
		case TYPE_ENUM:
			{
				const EnumList * enumList = getEnumList(param.extendedName, false);
				DEBUG_FATAL(enumList == nullptr, ("unknown enum name %s",
					param.extendedName.c_str()));
				fp.print("%s", enumList->at(0).name.c_str());
			}
			break;
		case TYPE_STRUCT:
			{
				fp.print("[\n");
				StructMap::const_iterator mapIter = m_structMap.find(param.extendedName);
				DEBUG_FATAL(mapIter == m_structMap.end(), ("unknown struct name %s",
					param.enum_list_name.c_str()));
				const TemplateData *data = (*mapIter).second;
				NOT_NULL(data);
				ParameterList::const_iterator listIter;
				for (listIter = data->m_parameters.begin();
					listIter != data->m_parameters.end(); ++listIter)
				{
					const Parameter &param = *listIter;
					if (param.type == TYPE_NONE || param.type == TYPE_COMMENT)
						continue;
					writeStructParameterDefault(fp, param,
						listIter + 1 == data->m_parameters.end());
				}
				fp.print("]");
			}
			break;
		case TYPE_TRIGGER_VOLUME:
			fp.print("\"dummyVolume\" 0.1");
			break;
		case TYPE_COMMENT:
		case TYPE_NONE:
		default:
			break;
	}
}	// TemplateData::writeDefaultValue

const TemplateData::Parameter *TemplateData::getParameter(
	const char *name, bool deepCheck) const
{
	NOT_NULL(name);

	TemplateData::Parameter const *result = nullptr;

	// Shallow check, just checks this immediate tdf

	ParameterMap::const_iterator iter = m_parameterMap.find(name);

	if (iter != m_parameterMap.end())
	{
		result = &(*iter).second;
	}
	else if (deepCheck)
	{
		// Deep check recurses all inherited tdf files

		const TemplateDefinitionFile *TemplateDefinitionFile = getTdfForParameter(name);

		if (TemplateDefinitionFile != nullptr)
		{
			result = TemplateDefinitionFile->getTemplateData(TemplateDefinitionFile->getHighestVersion())->getParameter(name);
		}
	}

	return result;
}	// TemplateData::getParameter

//========================================================================
// template creation functions
