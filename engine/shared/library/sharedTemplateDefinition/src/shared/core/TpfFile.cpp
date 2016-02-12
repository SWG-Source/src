//========================================================================
//
// TpfFile.cpp - a template file interpreter.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedTemplateDefinition/FirstSharedTemplateDefinition.h"
#include "sharedTemplateDefinition/TpfFile.h"

#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/CompilerDynamicVariableParamData.h"
#include "sharedTemplateDefinition/CompilerIntegerParam.h"
#include "sharedTemplateDefinition/TemplateGlobals.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "UnicodeUtils.h"

#include <cstdio>

#ifdef WIN32
#include <direct.h>
#endif // WIN32


/**
 * Class constructor.
 */
TpfFile::TpfFile(void) :
	m_template(nullptr),
	m_baseTemplateName(),
	m_currTemplateDef(nullptr),
	m_templateData(nullptr),
	m_highestTemplateData(nullptr),
	m_parameter(nullptr),
	m_path(),
	m_iffPath(),
	m_templateLocation(LOC_NONE)
//	m_checkParameters(true)
{
}	// TpfFile::TpfFile

/**
 * Class destructor.
 */
TpfFile::~TpfFile()
{
	cleanup();
}	// TpfFile::~TpfFile

/**
 * Resets class variables.
 */
void TpfFile::cleanup(void)
{
	m_parameter = nullptr;
	m_fp.close();
	if (m_template != nullptr)
	{
		delete m_template;
		m_template = nullptr;
	}
	m_currTemplateDef = nullptr;
	IGNORE_RETURN(m_baseTemplateName.erase());
}	// TpfFile::cleanup

/**
 * Prints an error when an integer param could go below the minimum limit for
 * the param.
 */
void TpfFile::printMinIntError(void)
{
	char numbuf[16];
	sprintf(numbuf, "%d", m_parameter->min_int_limit);
	std::string errbuf = "number cannot be lower than the min limit for this "
		"parameter (";
	errbuf += numbuf;
	errbuf += ")";
	m_fp.printError(errbuf.c_str());
}	// TpfFile::printMinIntError

/**
 * Prints an error when an integer param could go below the maximum limit for
 * the param.
 */
void TpfFile::printMaxIntError(void)
{
	char numbuf[16];
	sprintf(numbuf, "%d", m_parameter->max_int_limit);
	std::string errbuf = "number cannot be higher than the max limit for this "
		"parameter (";
	errbuf += numbuf;
	errbuf += ")";
	m_fp.printError(errbuf.c_str());
}	// TpfFile::printMaxIntError

/**
 * Prints an error when a float param could go below the minimum limit for
 * the param.
 */
void TpfFile::printMinFloatError(void)
{
	char numbuf[24];
	sprintf(numbuf, "%f", m_parameter->min_float_limit);
	std::string errbuf = "number cannot be lower than the min limit for this "
		"parameter (";
	errbuf += numbuf;
	errbuf += ")";
	m_fp.printError(errbuf.c_str());
}	// TpfFile::printMinFloatError

/**
 * Prints an error when a float param could go below the maximum limit for
 * the param.
 */
void TpfFile::printMaxFloatError(void)
{
	char numbuf[24];
	sprintf(numbuf, "%f", m_parameter->max_float_limit);
	std::string errbuf = "number cannot be higher than the max limit for this "
		"parameter (";
	errbuf += numbuf;
	errbuf += ")";
	m_fp.printError(errbuf.c_str());
}	// TpfFile::printMaxFloatError

/**
 * Reads the next line of the template file for parameters that are split
 * across lines.
 *
 * @return the start of the next line, or CHAR_ERROR on error
 */
const char * TpfFile::goToNextLine(void)
{
	do
	{
		int lineLen = m_fp.readLine(m_buffer, BUFFER_SIZE);
		if (lineLen < 0)
		{
			if (lineLen == -1)
				m_fp.printError("unexpected end-of-file");
			return CHAR_ERROR;
		}
		IGNORE_RETURN(getNextToken(m_buffer, m_token));
	} while (*m_token == '\0');

	return m_buffer;
}	// TpfFile::goToNextLine

/**
 * Loads a template file into memory and parses it.
 *
 * @param filename		template file to read from
 *
 * @return 0 on success, -1 on fail
 */
int TpfFile::loadTemplate(const Filename & filename)
{
	int result = 0;

	cleanup();

	if (!m_fp.open(filename, "rt"))
	{
		fprintf(stderr, "Cannot open template file %s\n",
			m_fp.getFilename().getFullFilename().c_str());
		return -1;
	}

	// read the template file and make a template from the data
	while (result == 0)
	{
		int lineLen = m_fp.readLine(m_buffer, BUFFER_SIZE);
		if (lineLen == -1)
		{
			// if we are a base template, check for missing parameters
			if (m_highestTemplateData != nullptr && m_template != nullptr &&
				!m_highestTemplateData->verifyTemplate(m_template, m_fp))
			{
				result = -1;
			}
			break;
		}
		else if (lineLen == -2)
		{
			result = -1;
			break;
		}

		// parse the 1st token: comment, @flag, or variable name
		const char *line = m_buffer;
		line = getNextToken(line, m_token);
		if (*m_token == '\0' && line == nullptr)
		{
			// empty line or comment
			continue;
		}
		else if (*m_token == '@')
		{
			result = parseTemplateCommand(line);
		}
		else if (isalpha(*m_token))
		{
			if (m_template == nullptr)
			{
				m_fp.printError("unable to parse parameters, no template class defined");
				return -1;
			}
			line = parseAssignment(line);
			if (line == CHAR_ERROR)
				result = -1;
			else if (line != nullptr)
			{
				char buffer[1024];
				if (getNextToken(line, buffer))
				{
					sprintf(buffer, "excess characters ignored : <%s>", line);
					m_fp.printWarning(buffer);
				}
			}
		}
		else
		{
			m_fp.printError("unexpected token at start of line");
			result = -1;
		}
	}

	if (result == 0)
	{
		// determine the iff path based on the template directory
		m_iffPath = filename.getAbsolutePath();
		std::string::size_type dsrcPos = m_iffPath.find("dsrc");
		if (dsrcPos != std::string::npos)
		{
			m_iffPath.replace(dsrcPos, strlen("dsrc"), "data");
		}
		else
		{
			m_iffPath.clear();
			m_fp.printError("can't find \"dsrc\" in template path");
			result = -1;
		}
	}

	m_fp.close();

	return result;
}	// TpfFile::loadTemplate

/**
 * Makes client and server template iff files from a template file.
 *
 * @param filename		template file to read from
 *
 * @return 0 on success, -1 on fail
 */
int TpfFile::makeIffFiles(const Filename & filename)
{
	int result = loadTemplate(filename);
	if (result != 0)
		return result;

	Filename iffname(nullptr, m_iffPath.c_str(), filename.getName().c_str(),
		IFF_EXTENSION);
	Iff iffFile(1024, true, true);
	m_template->save(iffFile);
	return WriteIffFile(iffFile, iffname);
}	// TpfFile::makeIffFiles

/**
 * Writes iff data to a file.
 *
 * @param iffData		the iff data to write
 * @param fileName		the file name to write to
 *
 * @return 0 on success, -1 on fail
 */
int TpfFile::WriteIffFile(Iff & iffData, const Filename & fileName)
{
	int result = 0;

	fileName.verifyAndCreatePath();

#ifdef WIN32

		char currentWorkingDirectory[_MAX_PATH];
		_getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory));

		if (WindowsUnicode)
		{
			// there are problems with long path names in Windows that changing to
			// the directory seems to fix
			char *buffer = nullptr;
			DWORD buflen;
			// get our current path
			buflen = GetFullPathName(".", 0, buffer, nullptr);
			buffer = new char[buflen + 1];
			buflen = GetFullPathName(".", buflen + 1, buffer, nullptr);
			Unicode::String srcPath = Unicode::narrowToWide(buffer);
			delete[] buffer;
			srcPath = L"\\\\?\\" + srcPath;
			// get the destination path
			std::string correctPath(fileName.getDrive() + fileName.getPath());
			buflen = GetFullPathName(correctPath.c_str(), 0, buffer, nullptr);
			buffer = new char[buflen + 1];
			buflen = GetFullPathName(correctPath.c_str(), buflen + 1, buffer, nullptr);
			Unicode::String destPath = Unicode::narrowToWide(buffer);
			delete[] buffer;
			destPath = L"\\\\?\\" + destPath;
			// change to the destination path

			if (SetCurrentDirectoryW(destPath.c_str()) == 0)
			{
				fprintf(stderr, "Unable to find directory %s\n",
					fileName.getPath().c_str());
				return -1;
			}
			if (!iffData.write((fileName.getName() + fileName.getExtension()).c_str(), true))
			{
				result = -1;
			}
		}
		else
		{
			char *buffer = nullptr;
			DWORD buflen;
			// get our current path
			buflen = GetFullPathName(".", 0, buffer, nullptr);
			buffer = new char[buflen + 1];
			buflen = GetFullPathName(".", buflen + 1, buffer, nullptr);
			delete[] buffer;
			// get the destination path
			std::string correctPath(fileName.getDrive() + fileName.getPath());
			buflen = GetFullPathName(correctPath.c_str(), 0, buffer, nullptr);
			buffer = new char[buflen + 1];
			buflen = GetFullPathName(correctPath.c_str(), buflen + 1, buffer, nullptr);
			std::string destPath = buffer;
			delete[] buffer;
			// change to the destination path
			if (destPath.size() >= MAX_PATH)
			{
				fprintf(stderr, "Path to iff file is too long for your version of "
					"Windows to handle, use Windows 2000 or Linux\n");
				return -1;
			}

			if (SetCurrentDirectory(destPath.c_str()) == 0)
			{
				fprintf(stderr, "Unable to find or create directory %s\n", fileName.getPath().c_str());
				return -1;
			}

			if (!iffData.write((fileName.getName() + fileName.getExtension()).c_str(), true))
			{
				result = -1;
			}
		}

		// Go back to the previous current working directory

		_chdir(currentWorkingDirectory);
#else
		if (!iffData.write(fileName.getFullFilename().c_str(), true))
			result = -1;
#endif

	if (result != 0)
	{
		fprintf(stderr, "Unable to write to file %s\n",
			fileName.getFullFilename().c_str());
	}

	return result;
}	// TpfFile::WriteIffFile

/**
 * Adds or removes parameters from a template based on the current template
 * definition.
 *
 * @param filename		template file to read from
 *
 * @return 0 on success, -1 on fail
 *
int TpfFile::updateTemplate(const Filename & filename)
{
int result = 0;

	cleanup();

	File temp_fp;
	if (!temp_fp.open(tmpnam(nullptr), "wt"))
	{
		fprintf(stderr, "error opening temp file for template replacement\n");
		return -1;
	}

	if (!m_fp.open(filename, "rt"))
	{
		fprintf(stderr, "Cannot open template file %s\n",
			m_fp.getFilename().getFullFilename().c_str());
		return -1;
	}

	// read the template file and make a template from the data
	while (result == 0)
	{
		int lineLen = m_fp.readLine(m_buffer, BUFFER_SIZE);
		if (lineLen == -1)
		{
			break;
		}
		else if (lineLen == -2)
		{
			result = -1;
			break;
		}

		// parse the 1st token: comment, @flag, or variable name
		const char *line = m_buffer;
		line = getNextToken(line, m_token);
		if (*m_token == '\0' && line == nullptr)
		{
			// empty line or comment
			if (temp_fp.puts(m_buffer) < 0)
			{
				m_fp.printError("error writing to temp file");
				result = -1;
			}
		}
		else if (*m_token == '@')
		{
			// @base or @class
			const char *templine = getNextToken(line, m_token);
			if (templine == nullptr)
			{
				m_fp.printEolError();
				return -1;
			}
			if (strcmp(m_token, "base") == 0)
			{
				// this is a derived template, ignore
				temp_fp.close();
				remove(temp_fp.getFilename());
				return 0;
			}
			if (temp_fp.puts(m_buffer) < 0)
			{
				m_fp.printError("error writing to temp file");
				result = -1;
			}
			else
			{
				// if we are a base template, check for missing parameters
				if (m_highestTemplateData != nullptr && m_template != nullptr)
				{
					m_highestTemplateData->updateTemplate(m_template, temp_fp);
				}
				result = parseTemplateCommand(line);
			}
		}
		else if (isalpha(*m_token))
		{
			m_parameter = m_templateData->getParameter(m_token);
			if (m_parameter == nullptr)
			{
			}
			else
			{
			}
		}
		else
		{
			m_fp.printError("unexpected token at start of line");
			result = -1;
		}
	}

	if (result != 0)
	{
		temp_fp.close();
		remove(temp_fp.getFilename());
	}
	return result;
}	// TpfFile::updateTemplate
*/
/**
 * Parses an '@' template command.
 *
 * @param line		buffer containing the command
 *
 * @return 0 on success, -1 on error
 */
int TpfFile::parseTemplateCommand(const char *line)
{
	line = getNextToken(line, m_token);
	if (line == nullptr)
	{
		m_fp.printEolError();
		return -1;
	}
	if (strcmp(m_token, "base") == 0)
	{
		if (m_template != nullptr && !m_template->getBaseTemplateName().empty())
		{
			m_fp.printError("base template already defined");
			return -1;
		}
		line = getNextWhitespaceToken(line, m_token);
		if (isalpha(*m_token))
		{
			if (m_template != nullptr)
			{
				if (m_template->setBaseTemplateName(m_token) != 0)
				{
					std::string error("Unable to open base template: ");
					error += m_token;

					m_fp.printError(error.c_str());
					return -1;
				}
			}
			else
				m_baseTemplateName = m_token;
		}
		else
		{
			m_fp.printError("invalid base name");
			return -1;
		}
	}
	else if (strcmp(m_token, "class") == 0)
	{
		line = getNextToken(line, m_token);
		if (isalpha(*m_token))
		{
			// if we are a base template, check for missing parameters
			// @todo: fix so we can verify non-base templates
			if (m_highestTemplateData != nullptr && m_template != nullptr &&
				!m_highestTemplateData->verifyTemplate(m_template, m_fp))
			{
				return -1;
			}

			// load the template definition for the class
			Filename defName(m_fp.getFilename().getDrive().c_str(), m_fp.getFilename().getPath().c_str(), m_token, TEMPLATE_DEFINITION_EXTENSION);
			File fp;
			int i = 0;
			while (!fp.exists(defName) && i < MAX_DIRECTORY_DEPTH)
			{
				defName.appendPath(NEXT_HIGHER_PATH);
				++i;
			}
			if (i == MAX_DIRECTORY_DEPTH)
			{
				std::string errMsg = "unable to open template definition file " +
					defName.getName();
				m_fp.printError(errMsg.c_str());
				return -1;
			}
			if (!fp.open(defName, "rt"))
			{
				return -1;
			}
			int result = 0;

			if(m_currTemplateDef == nullptr)
			{
				result = m_templateDef.parse(fp);
				m_currTemplateDef = &m_templateDef;
				m_highestTemplateData = m_templateDef.getTemplateData(m_templateDef.
					getHighestVersion());
			}
			else
			{
				const TemplateData *templateDataChild = m_currTemplateDef->getTemplateData(m_currTemplateDef->getHighestVersion());

				bool lookingForMatchingData = true;

				while(lookingForMatchingData)
				{
					if(templateDataChild == nullptr) // DHERMAN Check here for template definition names not matching
					{
						char errbuf[256];

						sprintf(errbuf, "can't find template definition %s", defName.getName().c_str());
						m_fp.printError(errbuf);
						result = -1;

						lookingForMatchingData = false;
					}
					else
					{
						m_currTemplateDef = templateDataChild->getTdfParent();

						if(templateDataChild->getName() == defName.getName())
						{
							templateDataChild = m_currTemplateDef->getTemplateData(m_currTemplateDef->getHighestVersion());
						}
						else
						{
							lookingForMatchingData = false;
						}
					}
				}
			}

			fp.close();
			if (result != 0)
				return result;

			line = getNextToken(line, m_token);
			if (!isdigit(*m_token))
			{
				m_fp.printError("no version given for template definition class");
				return -1;
			}
			int version = static_cast<int>(atol(m_token));

			m_templateData = m_currTemplateDef->getTemplateData(version);
			if (m_templateData == nullptr)
			{
				char errbuf[256];
				sprintf(errbuf, "can't find version %d in template definition %s",
					version, defName.getName().c_str());
				m_fp.printError(errbuf);
				return -1;
			}

			// check if the template filename matches the definition filter
			if (!m_currTemplateDef->isValidTemplateName(m_fp.getFilename()))
			{
				fprintf(stderr, "Invalid template name, match filter = %s\n", m_currTemplateDef->getTemplateNameFilter().c_str());
				return -1;
			}

			if (m_template == nullptr)
			{
				// this is the highest class level, make a blank template
				// with it
				const TagInfo & templateId = m_currTemplateDef->getTemplateId();
				m_template = dynamic_cast<TpfTemplate*>(TpfTemplate::createTemplate(
					templateId.tag));
				if (m_template == nullptr)
				{
					m_fp.printError("Unable to create template class. May not be installed.");
					return -1;
				}
				m_template->setParentFile(this);
				if (!m_baseTemplateName.empty())
				{
					if (m_template->setBaseTemplateName(m_baseTemplateName) != 0)
					{
						std::string error("Unable to open base template: ");
						error += m_baseTemplateName;

						m_fp.printError(error.c_str());
						return -1;
					}
					m_baseTemplateName.clear();
				}
				m_path = m_currTemplateDef->getPath();
				m_templateLocation = m_currTemplateDef->getTemplateLocation();
			}
			else
			{
				// we are moving down the hierarchy
				// do we care? (only if we have duplicate parameter names)
			}
		}
		else
		{
			m_fp.printError("invalid class name");
			return -1;
		}
	}
	else if (strcmp(m_token, "enum_header") == 0)
	{
		// line should be a fully pathed name of a c-style header file
		// containing enum definitions
		return parseEnumHeader(line);
	}
	else
	{
		m_fp.printError("unknown template command");
		return -1;
	}
	return 0;
}	// TpfFile::parseTemplateCommand

/**
 * Parses a c-style header containing enum definitions.
 *
 * @param headerName		the file name of the header file
 *
 * @return 0 on success, -1 on fail
 */
int TpfFile::parseEnumHeader(const char * headerName)
{
static int nextEnumNameId = 0;
enum
{
	PARSE_ENUM_TOKEN,
	PARSE_ENUM_NAME,
	PARSE_START_BRACKET,
	PARSE_END_BRACKET,
	PARSE_ENUM_DEF,
	PARSE_ENUM_ASSIGNMENT,
	PARSE_ENUM_VALUE
} parseState = PARSE_ENUM_TOKEN;
TemplateData::EnumList * enumList = nullptr;	// current list being defined
TemplateData::EnumData * enumData = nullptr;	// current item being defined
int currentEnumValue = 0;

	File fp;
	Filename headerFilename(nullptr, nullptr, headerName, nullptr);
	int i = 0;
	while (!fp.exists(headerFilename) && i < MAX_DIRECTORY_DEPTH)
	{
		headerFilename.prependPath(NEXT_HIGHER_PATH);
		++i;
	}
	if (i == MAX_DIRECTORY_DEPTH)
	{
		m_fp.printError("Can't open enum header file");
		return -1;
	}
	if (!fp.open(headerFilename, "rt"))
		return -1;

	for (;;)
	{
		int lineLen = fp.readLine(m_buffer, BUFFER_SIZE);
		if (lineLen == -1)
		{
			if (parseState == PARSE_ENUM_TOKEN)
				return 0;
			fp.printError("unexpected end of file");
			return -1;
		}
		else if (lineLen == -2)
		{
			return -1;
		}

		const char * line = m_buffer;
		while (line != nullptr)
		{
			const char * templine = getNextToken(line, m_token);
			switch (parseState)
			{
				case PARSE_ENUM_TOKEN :
					if (strcmp(m_token, "enum") != 0)
						break;
					parseState = PARSE_ENUM_NAME;
					break;
				case PARSE_ENUM_NAME :
					{
						std::string enumName;
						if (isalpha(*m_token))
						{
							enumName = m_token;
						}
						else
						{
							char tempBuf[64];
							sprintf(tempBuf, "enumName%d", nextEnumNameId++);
							enumName = tempBuf;
							templine = line;
						}
						std::pair<TemplateData::EnumMap::iterator, bool> result =
							m_enumHeaderMap.insert(std::make_pair(enumName,
							TemplateData::EnumList()));
						if (!result.second)
						{
							fp.printError("can't create enum list");
							return -1;
						}
						enumList = &(*result.first).second;
						currentEnumValue = 0;
						parseState = PARSE_START_BRACKET;
					}
					break;
				case PARSE_START_BRACKET :
					if (*m_token == '{')
						parseState = PARSE_END_BRACKET;
					break;
				case PARSE_END_BRACKET :
					if (*m_token == '}')
					{
						enumList = nullptr;
						parseState = PARSE_ENUM_TOKEN;
					}
					else
					{
						parseState = PARSE_ENUM_DEF;
						templine = line;
					}
					break;
				case PARSE_ENUM_DEF:
					if (isalpha(*m_token) && enumList != nullptr)
					{
						enumList->push_back(TemplateData::EnumData());
						enumData = &enumList->back();
						enumData->name = m_token;
						parseState = PARSE_ENUM_ASSIGNMENT;
					}
					else
						parseState = PARSE_END_BRACKET;
					break;
				case PARSE_ENUM_ASSIGNMENT:
					if (*m_token == '=')
						parseState = PARSE_ENUM_VALUE;
					else
					{
						enumData->value = currentEnumValue++;
						templine = line;
						enumData = nullptr;
						parseState = PARSE_END_BRACKET;
					}
					break;
				case PARSE_ENUM_VALUE:
					if (isalpha(*m_token))
					{
						enumData->valueName = m_token;
						TemplateData::EnumList::const_iterator iter;
						for (iter = enumList->begin(); iter != enumList->end(); ++iter)
						{
							if ((*iter).name == std::string(enumData->valueName))
							{
								break;
							}
						}
						if (iter == enumList->end())
						{
							fp.printError("can't find matching enum value");
							return -1;
						}
						currentEnumValue = (*iter).value;
						enumData->value = currentEnumValue++;
						enumData = nullptr;
						parseState = PARSE_END_BRACKET;
					}
					else
					{
						fp.printError("expected enum value");
						return -1;
					}
					break;
				default:
					break;
			}
			line = templine;
		}
	}
}	// TpfFile::parseEnumHeader

/**
 * Finds the integral value associated with an enum value (from a C header).
 *
 * @param enumValue		the enumeration value
 *
 * @return the value, or INVALID_ENUM_RESULT if not found
 */
int TpfFile::getEnumHeaderValue(const char * enumValue) const
{
	NOT_NULL(enumValue);

	TemplateData::EnumMap::const_iterator mapIter;
	for (mapIter = m_enumHeaderMap.begin(); mapIter != m_enumHeaderMap.end(); ++mapIter)
	{
		const TemplateData::EnumList &elist = (*mapIter).second;
		TemplateData::EnumList::const_iterator listIter;
		for (listIter = elist.begin(); listIter != elist.end(); ++listIter)
		{
			if ((*listIter).name == enumValue)
				return (*listIter).value;
		}
	}

	return INVALID_ENUM_RESULT;
}	// TpfFile::getEnumHeaderValue(const char *)

/**
 * Parses a parameter assignment. m_token should contain the parameter name on
 * entry
 *
 * @param line		buffer containing the assignment statement
 *
 * @return character where parsing stopped on success, -1 on error
 */
const char * TpfFile::parseAssignment(const char *line)
{
	NOT_NULL(m_template);

	// get the parameter type info
	m_parameter = m_templateData->getParameter(m_token);
	if (m_parameter == nullptr)
	{
		std::string errmsg = "cannot find parameter ";
		errmsg += m_token;
		m_fp.printError(errmsg.c_str());
		return CHAR_ERROR;
	}

	// get the array index (if any)
	int arrayIndex = -1;
	line = getNextToken(line, m_token);

	if (*m_token == '[')
	{
		if (m_parameter->list_type != TemplateData::LIST_INT_ARRAY &&
			m_parameter->list_type != TemplateData::LIST_ENUM_ARRAY)
		{
			m_fp.printError("non-array parameter being assigned as array");
			return CHAR_ERROR;
		}
		if (line == nullptr)
		{
			// we need to read the next line to continue parsing
			line = goToNextLine();
			if (line == CHAR_ERROR)
				return line;
		}
		// get the index, either a number or enum name
		line = getNextToken(line, m_token);
		if (isdigit(*m_token))
			arrayIndex = atoi(m_token);
		else
		{
			arrayIndex = m_highestTemplateData->getEnumValue(
				m_parameter->enum_list_name, m_token);
			if (arrayIndex == INVALID_ENUM_RESULT)
			{
				m_fp.printError("enum value not found - invalid for index type");
				return CHAR_ERROR;
			}
		}
		if (arrayIndex < 0 || arrayIndex >= m_parameter->list_size)
		{
			m_fp.printError("array index out of range");
			return CHAR_ERROR;
		}
		// check for the ending ]
		line = getNextToken(line, m_token);
		if (line == nullptr)
		{
			m_fp.printEolError();
			return CHAR_ERROR;
		}
		if (*m_token != ']')
		{
			m_fp.printError("expected ] at end of parameter array index");
			return CHAR_ERROR;
		}
		line = getNextToken(line, m_token);
		if (line == nullptr)
		{
			m_fp.printEolError();
			return CHAR_ERROR;
		}
	}
	else if (line == nullptr)
	{
		m_fp.printEolError();
		return CHAR_ERROR;
	}

	// m_token should now be =
	if (*m_token != '=')
	{
		m_fp.printError("expected = after parameter name");
		return CHAR_ERROR;
	}

	switch (m_parameter->type)
	{
		case TemplateData::TYPE_INTEGER:
			line = parseValue<CompilerIntegerParam>(getIntegerParam2,
				parseIntegerParameter2, *this, line, arrayIndex);
			break;
		case TemplateData::TYPE_FLOAT:
			line = parseValue<FloatParam>(getFloatParam2,
				parseFloatParameter2, *this, line, arrayIndex);
			break;
		case TemplateData::TYPE_BOOL:
			line = parseValue<BoolParam>(getBoolParam2,
				parseBoolParameter2, *this, line, arrayIndex);
			break;
		case TemplateData::TYPE_STRING:
			line = parseValue<StringParam>(getStringParam2,
				parseStringParameter2, *this, line, arrayIndex);
			break;
		case TemplateData::TYPE_STRINGID:
			line = parseValue<StringIdParam>(getStringIdParam2,
				parseStringIdParameter2, *this, line, arrayIndex);
			break;
		case TemplateData::TYPE_FILENAME:
			line = parseValue<StringParam>(getStringParam2,
				parseFilenameParameter2, *this, line, arrayIndex);
			break;
		case TemplateData::TYPE_VECTOR:
			line = parseValue<VectorParam>(getVectorParam2,
				parseVectorParameter2, *this, line, arrayIndex);
			break;
		case TemplateData::TYPE_TEMPLATE:
			line = parseValue<StringParam>(getStringParam2,
				parseTemplateParameter2, *this, line, arrayIndex);
			break;
		case TemplateData::TYPE_ENUM:
			line = parseValue<CompilerIntegerParam>(getIntegerParam2,
				parseEnumParameter2, *this, line, arrayIndex);
			break;
		case TemplateData::TYPE_DYNAMIC_VAR:
			line = parseValue<DynamicVariableParam>(getDynamicVariableParam2,
				parseDynamicVariableParameter2, *this, line, arrayIndex);
			break;
		case TemplateData::TYPE_STRUCT:
			line = parseValue<StructParamOT>(getStructParam2,
				parseStructParameter2, *this, line, arrayIndex);
			break;
		case TemplateData::TYPE_TRIGGER_VOLUME:
			line = parseValue<TriggerVolumeParam>(getTriggerVolumeParam2,
				parseTriggerVolumeParameter2, *this, line, arrayIndex);
			break;
		case TemplateData::TYPE_COMMENT:
		case TemplateData::TYPE_NONE:
		default:
			break;
	}

	return line;
}	// TpfFile::parseAssignment

/**
 * Parses an integer parameter assignment.
 *
 * @param param		template parameter
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseIntegerParameter(CompilerIntegerParam & param, const char *line)
{
	NOT_NULL(line);

	const char * tempLine = getNextToken(line, m_token);
	if (*m_token == '\0')
	{
		m_fp.printEolError();
		return CHAR_ERROR;
	}
	else if (*m_token == '{')
	{
		// weighted list
		line = parseWeightedList<CompilerIntegerParam, 
			CompilerIntegerParam::WeightedList, CompilerIntegerParam::WeightedValue>(
			parseIntegerParameter2, *this, param, tempLine);
	}
	else if (isinteger(m_token) || isalpha(*m_token))
	{
		int value = m_highestTemplateData->parseIntValue(m_fp, line, &line, m_token);
		if (line == CHAR_ERROR)
			return CHAR_ERROR;

		if (line != nullptr && *line == 'd')
		{
			// rolling die
			int base = 0;
			int num_dice = value;
			if (num_dice < 1)
			{
				m_fp.printError("number of dice < 1");
				return CHAR_ERROR;
			}
			++line;
			line = getNextToken(line, m_token);
			if (*m_token == '\0')
			{
				m_fp.printEolError();
				return CHAR_ERROR;
			}
			if (!isinteger(m_token))
			{
				m_fp.printError("can't find die sides value");
				return CHAR_ERROR;
			}
			int num_sides = atoi(m_token);
			if (num_sides < 2)
			{
				m_fp.printError("number of die sides < 2");
				return CHAR_ERROR;
			}
			line = getNextToken(line, m_token);
			if (*m_token != '\0')
			{
				if (!isinteger(m_token))
				{
					m_fp.printError("can't find base value");
					return CHAR_ERROR;
				}
				base = atoi(m_token);
			}
			// check lower and upper limits
			if (num_dice + base < m_parameter->min_int_limit)
			{
				printMinIntError();
				return CHAR_ERROR;
			}
			if (num_dice * num_sides + base > m_parameter->max_int_limit)
			{
				printMaxIntError();
				return CHAR_ERROR;
			}
			param.setValue(num_dice, num_sides, base);
		}
		else if (line != nullptr && *line == '.' && *(line+1) == '.')
		{
			// range
			int min_value = value;
			int max_value = INT_MAX;
			line += 2;
			tempLine = getNextToken(line, m_token);
			if (*m_token != '\0' && (isinteger(m_token) || isalpha(*m_token)))
			{
				max_value = m_highestTemplateData->parseIntValue(m_fp, line, &line, m_token);
				if (line == CHAR_ERROR)
					return CHAR_ERROR;
			}
			else if (*m_token == '\0')
				line = tempLine;
			if (min_value < m_parameter->min_int_limit)
			{
				printMinIntError();
				return CHAR_ERROR;
			}
			if (max_value > m_parameter->max_int_limit)
			{
				printMaxIntError();
				return CHAR_ERROR;
			}
			if (min_value > max_value)
			{
				m_fp.printError("min value greater than max value");
				return CHAR_ERROR;
			}
			param.setValue(min_value, max_value);
		}
		else 
		{
			if (value < m_parameter->min_int_limit)
			{
				printMinIntError();
				return CHAR_ERROR;
			}
			if (value > m_parameter->max_int_limit)
			{
				printMaxIntError();
				return CHAR_ERROR;
			}
			param.setValue(value);

			// if this integer was defined by enum value(s), we need to save off
			// a vector of the enums used to make the integer
			if (isalpha(*m_token))
			{
				tempLine = m_token;
				char tempBuffer[64];
				std::vector<std::string> enumList;
				while (tempLine != nullptr)
				{
					tempLine = getNextToken(tempLine, tempBuffer);
					if (isalpha(*tempBuffer))
						enumList.push_back(tempBuffer);
				}
				param.addSourceEnumList(enumList);
			}
		}
	}
	else if (*m_token == '.' && tempLine != nullptr && *tempLine == '.')
	{
		// range with lower bound of INT_MIN
		line = tempLine + 1;
		int max_value = INT_MAX;
		tempLine = getNextToken(line, m_token);
		if (*m_token != '\0' && (isinteger(m_token) || isalpha(*m_token)))
		{
			max_value = m_highestTemplateData->parseIntValue(m_fp, line, &line, m_token);
			if (line == CHAR_ERROR)
				return CHAR_ERROR;
		}
		else if (*m_token == '\0')
			line = tempLine;
		if (INT_MIN < m_parameter->min_int_limit)
		{
			printMinIntError();
			return CHAR_ERROR;
		}
		if (max_value > m_parameter->max_int_limit)
		{
			printMaxIntError();
			return CHAR_ERROR;
		}
		param.setValue(INT_MIN, max_value);
	}
	else if (*m_token == '(')
	{
		// offset from base template value
		line = getNextToken(tempLine, m_token);
		if (*m_token == '\0')
		{
			m_fp.printEolError();
			return CHAR_ERROR;
		}
		if ((*m_token != '+' && *m_token != '-') || *(m_token + 1) != '\0')
		{
			m_fp.printError("invalid delta flag (expected '+' or '-')");
			return CHAR_ERROR;
		}
		char deltaType = *m_token;
		param.setDeltaType(deltaType);
		line = getNextToken(line, m_token);
		if (*m_token == '\0')
		{
			m_fp.printEolError();
			return CHAR_ERROR;
		}
		if (*m_token != ')')
		{
			m_fp.printError("expected ')' after delta flag");
			return CHAR_ERROR;
		}
		line = parseIntegerParameter(param, line);
		if (line != nullptr && *line == '%')
		{
			// % offset from base value
			line = getNextToken(line, m_token);
			if (deltaType == '+')
				deltaType = '=';
			else
				deltaType = '_';
			param.setDeltaType(deltaType);
		}
	}
	else
	{
		m_fp.printError("invalid value for integer assignment");
		return CHAR_ERROR;
	}
	return line;
}	// parseIntegerParameter

/**
 * Parses a float parameter assignment.
 *
 * @param param		template parameter
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseFloatParameter(FloatParam & param, const char *line)
{
	NOT_NULL(line);

	line = getNextToken(line, m_token);
	if (*m_token == '\0')
	{
		m_fp.printEolError();
		return CHAR_ERROR;
	}
	else if (*m_token == '{')
	{
		// weighted list
		line = parseWeightedList<FloatParam, FloatParam::WeightedList, FloatParam::WeightedValue>(
			parseFloatParameter2, *this, param, line);
	}
	else if (isfloat(m_token))
	{
		if (line != nullptr && *line == '.' && *(line+1) == '.')
		{
			// range
			float min_value = static_cast<float>(atof(m_token));
			float max_value = FLT_MAX;
			line += 2;
			const char * tempLine = getNextToken(line, m_token);
			if (*m_token != '\0' && isfloat(m_token))
			{
				max_value = static_cast<float>(atof(m_token));
				line = tempLine;
			}
			else if (*m_token == '\0')
				line = tempLine;
			if (min_value < m_parameter->min_float_limit)
			{
				printMinFloatError();
				return CHAR_ERROR;
			}
			if (max_value > m_parameter->max_float_limit)
			{
				printMaxFloatError();
				return CHAR_ERROR;
			}
			if (min_value > max_value)
			{
				m_fp.printError("min value greater than max value");
				return CHAR_ERROR;
			}
			param.setValue(min_value, max_value);
		}
		else
		{
			float value = static_cast<float>(atof(m_token));
			if (value < m_parameter->min_float_limit)
			{
				printMinFloatError();
				return CHAR_ERROR;
			}
			if (value > m_parameter->max_float_limit)
			{
				printMaxFloatError();
				return CHAR_ERROR;
			}
			param.setValue(value);
		}
	}
	else if (*m_token == '.' && line != nullptr && *line == '.')
	{
		// range with lower bound of -FLT_MAX
		++line;
		float max_value = FLT_MAX;
		const char * tempLine = getNextToken(line, m_token);
		if (*m_token != '\0' && isfloat(m_token))
		{
			max_value = static_cast<float>(atof(m_token));
			line = tempLine;
		}
		else if (*m_token == '\0')
			line = tempLine;
		if (-FLT_MAX < m_parameter->min_float_limit)
		{
			printMinFloatError();
			return CHAR_ERROR;
		}
		if (max_value > m_parameter->max_float_limit)
		{
			printMaxFloatError();
			return CHAR_ERROR;
		}
		param.setValue(-FLT_MAX, max_value);
	}
	else if (*m_token == '(')
	{
		// offset from base template value
		line = getNextToken(line, m_token);
		if (*m_token == '\0')
		{
			m_fp.printEolError();
			return CHAR_ERROR;
		}
		if ((*m_token != '+' && *m_token != '-') || *(m_token + 1) != '\0')
		{
			m_fp.printError("invalid delta flag (expected '+' or '-')");
			return CHAR_ERROR;
		}
		char deltaType = *m_token;
		param.setDeltaType(deltaType);
		line = getNextToken(line, m_token);
		if (*m_token == '\0')
		{
			m_fp.printEolError();
			return CHAR_ERROR;
		}
		if (*m_token != ')')
		{
			m_fp.printError("expected ')' after delta flag");
			return CHAR_ERROR;
		}
		line = parseFloatParameter(param, line);
		if (line != nullptr && *line == '%')
		{
			// % offset from base value
			line = getNextToken(line, m_token);
			if (deltaType == '+')
				deltaType = '=';
			else
				deltaType = '_';
			param.setDeltaType(deltaType);
		}
	}
	else if (isalpha(*m_token))
	{
		// check to see if this is a valid enum
		int value = m_highestTemplateData->getEnumValue(m_token);
		if (value == INVALID_ENUM_RESULT)
		{
			m_fp.printError("enum name not found");
			return CHAR_ERROR;
		}
		param.setValue(static_cast<float>(value));
	}
	else
	{
		m_fp.printError("invalid value for float assignment");
		return CHAR_ERROR;
	}
	return line;
}	// TpfFile::parseFloatParameter

/**
 * Parses a bool parameter assignment.
 *
 * @param param		template parameter
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseBoolParameter(BoolParam & param, const char *line)
{
	NOT_NULL(line);

	line = getNextToken(line, m_token);
	if (*m_token == '\0')
	{
		m_fp.printEolError();
		return CHAR_ERROR;
	}
	else if (*m_token == '{')
	{
		// weighted list
		line = parseWeightedList<BoolParam, BoolParam::WeightedList,
			BoolParam::WeightedValue>(parseBoolParameter2, *this, param, line);
	}
	else if (strcmp(m_token, "true") == 0)
	{
		param.setValue(true);
	}
	else if (strcmp(m_token, "false") == 0)
	{
		param.setValue(false);
	}
	else
	{
		m_fp.printError("invalid value for boolean assignment");
		return CHAR_ERROR;
	}
	return line;
}	// TpfFile::parseBoolParameter

/**
 * Parses a string parameter assignment.
 *
 * @param param		template parameter
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseStringParameter(StringParam & param, const char *line)
{
	NOT_NULL(line);

	line = getNextToken(line, m_token);
	int tokenLen = strlen(m_token);
	if (*m_token == '\0')
	{
		m_fp.printEolError();
		return CHAR_ERROR;
	}
	else if (*m_token == '{')
	{
		// weighted list
		line = parseWeightedList<StringParam, StringParam::WeightedList,
			StringParam::WeightedValue>(parseStringParameter2, *this, param, line);
	}
	else if (*m_token == '\"' && *(m_token + tokenLen - 1) == '\"')
	{
		*(m_token + tokenLen - 1) = '\0';
		param.setValue(m_token + 1);
	}
	else
	{
		m_fp.printError("invalid value for string assignment");
		return CHAR_ERROR;
	}
	return line;
}	// TpfFile::parseStringParameter

/**
 * Parses a string id parameter assignment.
 *
 * @param param		template parameter
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseStringIdParameter(StringIdParam & param, const char *line)
{
	NOT_NULL(line);

	const char *templine = getNextToken(line, m_token);
	if (*m_token == '{')
	{
		// weighted list
		line = parseWeightedList<StringIdParam, StringIdParam::WeightedList,
			StringIdParam::WeightedValue>(parseStringIdParameter2, *this, param,
			templine);
	}
	else
	{
		StringIdParamData data;
		// get the string id table name
		line = parseFilenameParameter(data.table, line);
		if (line == CHAR_ERROR)
			return line;

		// get the string id table index
		line = parseStringParameter(data.index, line);
		param.setValue(data);
	}
	return line;
}	// TpfFile::parseStringIdParameter

/**
 * Parses a filename parameter assignment.
 *
 * @param param		template parameter
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseFilenameParameter(StringParam & param, const char *line)
{
	NOT_NULL(line);

	line = getNextToken(line, m_token);
	int tokenLen = strlen(m_token);
	if (*m_token == '\0')
	{
		m_fp.printEolError();
		return CHAR_ERROR;
	}
	else if (*m_token == '{')
	{
		// weighted list
		line = parseWeightedList<StringParam, StringParam::WeightedList,
			StringParam::WeightedValue>(parseFilenameParameter2, *this, param,
			line);
	}
	else if (*m_token == '\"' && *(m_token + tokenLen - 1) == '\"')
	{
		*(m_token + tokenLen - 1) = '\0';
		// @todo: test filename for invalid chars and tweak slashes
		Filename localname;
		localname.setName(m_token + 1);
		std::string lowername = localname.getName();
		for (std::string::size_type i = 0; i < lowername.size(); ++i)
		{
			if (!islower(lowername[i]) &&
				!isdigit(lowername[i]) &&
				lowername[i] != '-' &&
				lowername[i] != '_')
			{
				m_fp.printError("Illegal character in filename, only lowercase, "
					"numbers, \"-\" and \"_\" allowed");
				return CHAR_ERROR;
			}
		}
		param.setValue(localname.getFullFilename().c_str());
	}
	else
	{
		m_fp.printError("invalid value for filename assignment");
		return CHAR_ERROR;
	}
	return line;
}	// TpfFile::parseFilenameParameter

/**
 * Parses a vector parameter assignment.
 *
 * @param param		template parameter
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseVectorParameter(VectorParam & param, const char *line)
{
	NOT_NULL(line);

	const char *templine = getNextToken(line, m_token);
	if (*m_token == '\0')
	{
		m_fp.printEolError();
		return CHAR_ERROR;
	}
	else if (*m_token == '{')
	{
		// weighted list
		line = parseWeightedList<VectorParam, VectorParam::WeightedList, VectorParam::WeightedValue>(
			parseVectorParameter2, *this, param, templine);
	}
	else if (isdigit(*m_token) || (*m_token == '.' && *line == '.'))
	{
		VectorParamData data;

		// assume x, z, radius
		data.ignoreY = true;
		line = parseFloatParameter(data.x, line);
		if (line == CHAR_ERROR)
			return line;
		line = parseFloatParameter(data.z, line);
		if (line == CHAR_ERROR)
			return line;
		line = parseFloatParameter(data.radius, line);
		if (line == CHAR_ERROR)
			return line;
		// see if there is a fourth value
		IGNORE_RETURN(getNextToken(line, m_token));
		if (isdigit(*m_token) || (*m_token == '.' && *line == '.'))
		{
			// shift values and get radius
			data.ignoreY = false;
			data.y = data.z;
			data.z = data.radius;
			line = parseFloatParameter(data.radius, line);
			if (line == CHAR_ERROR)
				return line;
		}
		param.setValue(data);
	}
	else
	{
		m_fp.printError("invalid value for vector assignment");
		return CHAR_ERROR;
	}
	return line;
}	// TpfFile::parseVectorParameter

/**
 * Parses a template parameter assignment.
 *
 * @param param		template parameter
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseTemplateParameter(StringParam & param, const char *line)
{
	return parseFilenameParameter(param, line);
}	// TpfFile::parseTemplateParameter

/**
 * Parses an enum parameter assignment.
 *
 * @param param		template parameter
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseEnumParameter(CompilerIntegerParam & param, const char *line)
{
	NOT_NULL(line);

	line = getNextToken(line, m_token);
	if (*m_token == '\0')
	{
		m_fp.printEolError();
		return CHAR_ERROR;
	}
	else if (*m_token == '{')
	{
		// weighted list
		line = parseWeightedList<CompilerIntegerParam, 
			CompilerIntegerParam::WeightedList, CompilerIntegerParam::WeightedValue>(
			parseEnumParameter2, *this, param, line);
	}
	else if (isalpha(*m_token))
	{
		int value = m_highestTemplateData->getEnumValue(
			m_parameter->extendedName, m_token);
		if (value == INVALID_ENUM_RESULT)
		{
			m_fp.printError("enum name not found - invalid for parameter type");
			return CHAR_ERROR;
		}
		param.setValue(value);
	}
	else
	{
		m_fp.printError("invalid value for enum assignment");
		return CHAR_ERROR;
	}
	return line;
}	// TpfFile::parseEnumParameter

/**
 * Parses a DynamicVariable parameter assignment.
 *
 * @param param		template parameter
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseDynamicVariableParameter(DynamicVariableParam & param,
	const char *line)
{
	NOT_NULL(line);

	line = getNextToken(line, m_token);
	if (*m_token == '\0')
	{
		m_fp.printEolError();
		return CHAR_ERROR;
	}
	if (*m_token == '{')
	{
		// weighted list
		line = parseWeightedList<DynamicVariableParam, DynamicVariableParam::WeightedList, DynamicVariableParam::WeightedValue>(
			parseDynamicVariableParameter2, *this, param, line);
		return line;
	}
	if (*m_token == '+')
	{
		// extending an objevar list
		if (m_template != nullptr && m_template->getBaseTemplateName().empty())
		{
			m_fp.printError("trying to extend an objvar list from a base template");
			return CHAR_ERROR;
		}
		param.setExtendingBaseList(true);
		line = getNextToken(line, m_token);
	}
	if (*m_token != '[')
	{
		m_fp.printError("expected [ at start of objvar assignment");
		return CHAR_ERROR;
	}
	else
	{
		if (line == nullptr)
		{
			// we need to read the next line to continue parsing
			line = goToNextLine();
			if (line == CHAR_ERROR)
				return line;
		}
		// check for empty list
		const char *templine = getNextToken(line, m_token);
		if (*m_token == ']')
		{
			param.setIsLoaded();
			return templine;
		}

		// set up the incoming objvar as an objvar list
		DynamicVariableParamData &data = const_cast<DynamicVariableParamData &>
			(param.getValue());
		if (data.m_type != DynamicVariableParamData::UNKNOWN)
		{
			m_fp.printError("objvar parser given non-empty param");
			return CHAR_ERROR;
		}
		data.m_name.clear();
		data.m_type = DynamicVariableParamData::LIST;
		data.m_data.lparam = new std::vector<DynamicVariableParamData *>;

		line = parseDynamicVariableParameterList(data, line);
		if (line != CHAR_ERROR)
			param.setIsLoaded();
	}
	return line;
}	// TpfFile::parseDynamicVariableParameter

/**
 * Parses a DynamicVariable list parameter assignment.
 *
 * @param param		a DynamicVariableParamData containing an empty list
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseDynamicVariableParameterList(
	DynamicVariableParamData & data, const char *line)
{
std::string name;

	NOT_NULL(line);

	if (data.m_type != DynamicVariableParamData::LIST ||
		data.m_data.lparam == nullptr)
	{
		m_fp.printError("parse objvar list not given a list");
		return CHAR_ERROR;
	}
	if (data.m_data.lparam->size() != 0)
	{
		m_fp.printError("parse objvar list not given an empty list");
		return CHAR_ERROR;
	}

	// test for empty list
	IGNORE_RETURN(getNextToken(line, m_token));
	if (*m_token == ']')
	{
		line = getNextToken(line, m_token);
	}

	while (*m_token != ']')
	{
		// test to see if we need to go to the next line
		IGNORE_RETURN(getNextToken(line, m_token));
		if (*m_token == '\0')
		{
			// we need to read the next line to continue parsing
			line = goToNextLine();
			if (line == CHAR_ERROR)
				return line;
		}
		// get the objvar name
		line = getNextToken(line, m_token);
		int tokenLen = strlen(m_token);
		if (tokenLen < 3 || *m_token != '\"' || *(m_token + tokenLen - 1) != '\"')
		{
			m_fp.printError("cannot find objvar name");
			return CHAR_ERROR;
		}
		*(m_token + tokenLen - 1) = '\0';
		name = m_token + 1;
		// next token should be '='
		line = getNextToken(line, m_token);
		if (*m_token != '=')
		{
			m_fp.printError("expected = after objvar name");
			return CHAR_ERROR;
		}
		// we now need to look ahead to see what the objvar type is
		bool expectingNumber = false;
		const char *paramStart = line;
		DynamicVariableParamData::DataType type = DynamicVariableParamData::UNKNOWN;
		for (;;)
		{
			line = getNextToken(line, m_token);
			if (*m_token == '\0')
			{
				m_fp.printEolError();
				return CHAR_ERROR;
			}
			else if (isdigit(*m_token) || expectingNumber)
			{
				// may be integer or float
				if (isinteger(m_token))
				{
					type = DynamicVariableParamData::INTEGER;
					break;
				}
				else if (isfloat(m_token))
				{
					type = DynamicVariableParamData::FLOAT;
					break;
				}
			}
			else if (*m_token == '.' && *line == '.' && isdigit(*(line + 1)))
			{
				// may be integer or float, force into number test on next pass
				++line;
				expectingNumber = true;
				continue;
			}
			else if (*m_token == '{')
			{
				// looks like a weighted list, keep going
				continue;
			}
			else if (*m_token == '[')
			{
				type = DynamicVariableParamData::LIST;
				break;
			}
			else if (*m_token == '"')
			{
				type = DynamicVariableParamData::STRING;
				break;
			}
			else if (isalpha(*m_token))
			{
				// check to see if this is a valid enum
				int value = m_highestTemplateData->getEnumValue(m_token);
				if (value == INVALID_ENUM_RESULT)
				{
					// try the C header enums
					value = getEnumHeaderValue(m_token);
				}
				if (value != INVALID_ENUM_RESULT)
				{
					type = DynamicVariableParamData::INTEGER;
//					param.setValue(value);
					break;
				}
			}
			// the token was nothing we recognize
			std::string errbuf = "unexpected token ";
			errbuf += m_token;
			errbuf += ", cannot determine objvar type";
			m_fp.printError(errbuf.c_str());
			return CHAR_ERROR;
		}
		// now parse the objvar data
		line = paramStart;
		CompilerDynamicVariableParamData *newData = new CompilerDynamicVariableParamData(name, type);
		switch (type)
		{
			case DynamicVariableParamData::INTEGER:
				line = parseIntegerParameter(*dynamic_cast<CompilerIntegerParam *>(
					newData->m_data.iparam), line);
				break;
			case DynamicVariableParamData::FLOAT:
				line = parseFloatParameter(*newData->m_data.fparam, line);
				break;
			case DynamicVariableParamData::STRING:
				line = parseStringParameter(*newData->m_data.sparam, line);
				break;
			case DynamicVariableParamData::LIST:
				line = getNextToken(line, m_token);
				if (*m_token != '[')
				{
					m_fp.printError("expected [ at start of objvar sub-list");
					line = CHAR_ERROR;
				}
				else
				{
					if (line == nullptr)
					{
						// we need to read the next line to continue parsing
						line = goToNextLine();
						if (line == CHAR_ERROR)
							return line;
					}
					line = parseDynamicVariableParameterList(*newData, line);
				}
				break;
			case DynamicVariableParamData::UNKNOWN:
			default:
				m_fp.printError("it should be impossible to get to this line");
				line = CHAR_ERROR;
				break;
		}
		if (line == CHAR_ERROR)
		{
			delete newData;
			return line;
		}
		data.m_data.lparam->push_back(newData);

		// we now expect a ',' or ']'
		line = getNextToken(line, m_token);
		if (*m_token == '\0')
		{
			m_fp.printEolError();
			return CHAR_ERROR;
		}
		if (*m_token != ',' && *m_token != ']')
		{
			m_fp.printError("expected , or ] at end or objvar assignment");
			return CHAR_ERROR;
		}
	}

	return line;
}	// TpfFile::parseDynamicVariableParameterList

/**
 * Parses a struct parameter assignment.
 *
 * @param param		template parameter
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseStructParameter(StructParamOT & param, const char *line)
{
	line = getNextToken(line, m_token);
	if (*m_token == '\0')
	{
		m_fp.printEolError();
		return CHAR_ERROR;
	}
	else if (*m_token == '{')
	{
		// weighted list
		line = parseWeightedList< StructParamOT, StructParamOT::WeightedList, StructParamOT::WeightedValue >(
			parseStructParameter2, *this, param, line);
	}
	else if (*m_token != '[')
	{
		m_fp.printError("expected [ at start of struct data");
		return CHAR_ERROR;
	}
	else
	{
		// we need to swap out the current template and template data for the
		// structure's template and template data, and save the parameter data
		const TemplateData::Parameter *oldParameter = m_parameter;
		const TemplateData *oldTemplateData = m_templateData;
		m_templateData = m_templateData->getStruct(m_parameter->extendedName.c_str());
		NOT_NULL(m_templateData);

		TpfTemplate *oldTemplate = m_template;
		if (!param.isInitialized())
		{
			m_template->initStructParamOT(param, m_parameter->name.c_str());
			if (!param.isInitialized())
			{
				std::string errbuf = "cannot initialize struct " +
					m_parameter->name;
				m_fp.printError(errbuf.c_str());
				m_templateData = oldTemplateData;
				return CHAR_ERROR;
			}
		}
		m_template = dynamic_cast<TpfTemplate *>(const_cast<class ObjectTemplate *>(param.getValue()));
		NOT_NULL(m_template);

		for (;;)
		{
			// test to see if we need to go to the next line
			IGNORE_RETURN(getNextToken(line, m_token));
			if (*m_token == '\0')
			{
				// we need to read the next line to continue parsing
				line = goToNextLine();
				if (line == CHAR_ERROR)
					return line;
			}
			line = getNextToken(line, m_token);
			line = parseAssignment(line);
			if (line == CHAR_ERROR)
				break;
			// check for ',' or ']'
			line = getNextToken(line, m_token);
			if (*m_token == ']')
			{
				// end of list
				break;
			}
			else if (*m_token != ',')
			{
				m_fp.printError("expected , or ]");
				line = CHAR_ERROR;
				break;
			}
		}

		if (!m_templateData->verifyTemplate(m_template, m_fp))
			line = CHAR_ERROR;

		m_templateData = oldTemplateData;
		m_template = oldTemplate;
		m_parameter = oldParameter;
	}
	return line;
}	// TpfFile::parseStructParameter

/**
 * Parses a trigger volume parameter assignment.
 *
 * @param param		template parameter
 * @param line		buffer containing the assignment statement
 *
 * @return character in line where parsing stopped, or nullptr if at eol, or -1 if
 *		error
 */
const char * TpfFile::parseTriggerVolumeParameter(TriggerVolumeParam & param,
	const char *line)
{
	NOT_NULL(line);

	const char *templine = getNextToken(line, m_token);
	if (*m_token == '{')
	{
		// weighted list
		line = parseWeightedList<TriggerVolumeParam, TriggerVolumeParam::WeightedList, TriggerVolumeParam::WeightedValue>(
			parseTriggerVolumeParameter2, *this, param, templine);
	}
	else
	{
		TriggerVolumeParamData data;
		// get the trigger volume name
		line = parseStringParameter(data.name, line);
		if (line == CHAR_ERROR)
			return line;

		// get the trigger volume radius
		line = parseFloatParameter(data.radius, line);
		param.setValue(data);
	}
	return line;
}	// TpfFile::parseTriggerVolumeParameter


//========================================================================
// static callback functions

CompilerIntegerParam *TpfFile::getIntegerParam2(TpfTemplate &templt,
	const std::string &paramName, int arrayIndex)
{
	return templt.getCompilerIntegerParam(paramName.c_str(), false, arrayIndex);
}	// TpfFile::getIntegerParam2

FloatParam *TpfFile::getFloatParam2(TpfTemplate &templt,
	const std::string &paramName, int arrayIndex)
{
	return templt.getFloatParam(paramName.c_str(), false, arrayIndex);
}	// TpfFile::getFloatParam2

BoolParam *TpfFile::getBoolParam2(TpfTemplate &templt,
	const std::string &paramName, int arrayIndex)
{
	return templt.getBoolParam(paramName.c_str(), false, arrayIndex);
}	// TpfFile::getBoolParam2

StringParam *TpfFile::getStringParam2(TpfTemplate &templt,
	const std::string &paramName, int arrayIndex)
{
	return templt.getStringParam(paramName.c_str(), false, arrayIndex);
}	// TpfFile::getStringParam2

StringIdParam *TpfFile::getStringIdParam2(TpfTemplate &templt,
	const std::string &paramName, int arrayIndex)
{
	return templt.getStringIdParam(paramName.c_str(), false, arrayIndex);
}	// TpfFile::getStringIdParam2

VectorParam *TpfFile::getVectorParam2(TpfTemplate &templt,
	const std::string &paramName, int arrayIndex)
{
	return templt.getVectorParam(paramName.c_str(), false, arrayIndex);
}	// TpfFile::getVectorParam2

DynamicVariableParam *TpfFile::getDynamicVariableParam2(TpfTemplate &templt,
	const std::string &paramName, int arrayIndex)
{
	return templt.getDynamicVariableParam(paramName.c_str(), false, arrayIndex);
}	// TpfFile::getDynamicVariableParam2

StructParamOT *TpfFile::getStructParam2(TpfTemplate &templt,
	const std::string &paramName, int arrayIndex)
{
	return templt.getStructParamOT(paramName.c_str(), false, arrayIndex);
}	// TpfFile::getStructParam2

TriggerVolumeParam *TpfFile::getTriggerVolumeParam2(TpfTemplate &templt,
	const std::string &paramName, int arrayIndex)
{
	return templt.getTriggerVolumeParam(paramName.c_str(), false, arrayIndex);
}	// TpfFile::getTriggerVolumeParam2

const char * TpfFile::parseIntegerParameter2(TpfFile &file,
	CompilerIntegerParam & param, const char *line)
{
	return file.parseIntegerParameter(param, line);
}	// TpfFile::parseIntegerParameter2

const char * TpfFile::parseFloatParameter2(TpfFile &file,
	FloatParam & param, const char *line)
{
	return file.parseFloatParameter(param, line);
}	// TpfFile::parseFloatParameter2

const char * TpfFile::parseBoolParameter2(TpfFile &file,
	BoolParam & param, const char *line)
{
	return file.parseBoolParameter(param, line);
}	// TpfFile::parseBoolParameter2

const char * TpfFile::parseStringParameter2(TpfFile &file,
	StringParam & param, const char *line)
{
	return file.parseStringParameter(param, line);
}	// TpfFile::parseStringParameter2

const char * TpfFile::parseStringIdParameter2(TpfFile &file,
	StringIdParam & param, const char *line)
{
	return file.parseStringIdParameter(param, line);
}	// TpfFile::parseStringIdParameter2

const char * TpfFile::parseFilenameParameter2(TpfFile &file,
	StringParam & param, const char *line)
{
	return file.parseFilenameParameter(param, line);
}	// TpfFile::parseFilenameParameter2

const char * TpfFile::parseVectorParameter2(TpfFile &file,
	VectorParam & param, const char *line)
{
	return file.parseVectorParameter(param, line);
}	// TpfFile::parseVectorParameter2

const char * TpfFile::parseTemplateParameter2(TpfFile &file,
	StringParam & param, const char *line)
{
	return file.parseTemplateParameter(param, line);
}	// TpfFile::parseTemplateParameter2

const char * TpfFile::parseEnumParameter2(TpfFile &file,
	CompilerIntegerParam & param, const char *line)
{
	return file.parseEnumParameter(param, line);
}	// TpfFile::parseEnumParameter2

const char * TpfFile::parseDynamicVariableParameter2(TpfFile &file,
	DynamicVariableParam & param, const char *line)
{
	return file.parseDynamicVariableParameter(param, line);
}	// TpfFile::parseDynamicVariableParameter2

const char * TpfFile::parseStructParameter2(TpfFile &file,
	StructParamOT & param, const char *line)
{
	return file.parseStructParameter(param, line);
}	// TpfFile::parseStructParameter2

const char * TpfFile::parseTriggerVolumeParameter2(TpfFile &file,
	TriggerVolumeParam & param, const char *line)
{
	return file.parseTriggerVolumeParameter(param, line);
}	// TpfFile::parseTriggerVolumeParameter2

//========================================================================
// friend functions

/**
 * Parses the value being assigned to a parameter.
 *
 * @param getParamFunc		function used to get the parameter
 * @param parseFunc			function to parse the parameter value
 * @param file				template file being read from
 * @param line				current line being read from
 * @param arrayIndex		parameter array index (if the parameter is an array)
 *
 * @return point where parsing stopped, or -1 on error
 */
template <class Q>
const char * parseValue(
	Q * (*getParamFunc)(TpfTemplate &, const std::string &, int),
	const char * (*parseFunc)(TpfFile &file, Q & param, const char *line),
	TpfFile &file, const char *line, int arrayIndex)
{
Q * param;

	NOT_NULL(file.m_parameter);
	NOT_NULL(file.m_template);

	// check for derived flag
	const char *tempLine = getNextToken(line, file.m_token);
	if (file.m_token[0] == '@')
	{
		tempLine = getNextToken(tempLine, file.m_token);
		if (strcmp(file.m_token, "derived") == 0)
		{
			if (file.m_parameter->list_type == TemplateData::LIST_INT_ARRAY ||
				file.m_parameter->list_type == TemplateData::LIST_ENUM_ARRAY)
			{
				file.m_template->addPureVirtualParam(file.m_parameter->name, arrayIndex);
			}
			else
				file.m_template->addPureVirtualParam(file.m_parameter->name);
			return tempLine;
		}
	}

	if (file.m_parameter->list_type == TemplateData::LIST_LIST ||
		((file.m_parameter->list_type == TemplateData::LIST_INT_ARRAY ||
		file.m_parameter->list_type == TemplateData::LIST_ENUM_ARRAY) &&
		arrayIndex == -1))
	{
		if (arrayIndex != -1)
		{
			file.m_fp.printError("array being assigned as list");
			return CHAR_ERROR;
		}
		line = getNextToken(line, file.m_token);
		if (*file.m_token == '+')
		{
			if (file.m_parameter->list_type == TemplateData::LIST_INT_ARRAY ||
				file.m_parameter->list_type == TemplateData::LIST_ENUM_ARRAY)
			{
				file.m_fp.printError("cannot extend fixed-size list");
				return CHAR_ERROR;
			}
			// extending a list
			if (file.getBaseTemplateName().empty())
			{
				file.m_fp.printError("trying to extend a list from a base template");
				return CHAR_ERROR;
			}
			file.m_template->setAppend(file.m_parameter->name.c_str());
			line = getNextToken(line, file.m_token);
		}
		if (*file.m_token != '[')
		{
			file.m_fp.printError("expected [ at start of list");
			return CHAR_ERROR;
		}
		if (line == nullptr)
		{
			// we need to read the next line to continue parsing
			line = file.goToNextLine();
			if (line == CHAR_ERROR)
				return line;
		}
		if (!file.m_template->isParamLoaded(file.m_parameter->name, false))
		{
			// mark the parameter as being loaded, and clear the list
			file.m_template->addLoadedParam(file.m_parameter->name);
			file.m_template->setAsEmptyList(file.m_parameter->name.c_str());
		}
		// test for empty list
		const char *templine = getNextToken(line, file.m_token);
		if (*file.m_token == ']')
		{
			return templine;
		}
		int index;
		// parse the list elements
		for (index = 0; *file.m_token != ']'; ++index)
		{
			// get the next parameters in the list
			param = (*getParamFunc)(*file.m_template,
				file.m_parameter->name, index);
			if (param == nullptr)
			{
				std::string errmsg = "cannot find parameter " + file.m_parameter->name;
				file.m_fp.printError(errmsg.c_str());
				return CHAR_ERROR;
			}
			// test to see if we need to go to the next line
			IGNORE_RETURN(getNextToken(line, file.m_token));
			if (*file.m_token == '\0')
			{
				// we need to read the next line to continue parsing
				line = file.goToNextLine();
				if (line == CHAR_ERROR)
					return line;
			}
			line = (*parseFunc)(file, *param, line);
			if (line == CHAR_ERROR)
				return line;
			// see if we have more values or if we are finished
			line = getNextToken(line, file.m_token);
			if (*file.m_token != ',' && *file.m_token != ']')
			{
				file.m_fp.printError("expected , or ]");
				return CHAR_ERROR;
			}
		}
		if (file.m_parameter->list_type == TemplateData::LIST_INT_ARRAY ||
			file.m_parameter->list_type == TemplateData::LIST_ENUM_ARRAY)
		{
			// verify that the number of items is the same as the list size
			if (file.m_parameter->list_size != index)
			{
				char errBuf[256];
				sprintf(errBuf, "Invalid number of items in list, expected %d, "
					"got %d", file.m_parameter->list_size, index);
				file.m_fp.printError(errBuf);
				return CHAR_ERROR;
			}
		}
	}
	else
	{
		if (arrayIndex == -1)
			arrayIndex = 0;
		param = (*getParamFunc)(*file.m_template,
			file.m_parameter->name, arrayIndex);
		if (param == nullptr)
		{
			// if the tpf version is less than the current version, print a 
			// warning and ignore
			if (static_cast<int>(file.m_templateData->getVersion().tag) < 
				file.m_currTemplateDef->getHighestVersion())
			{
				std::string errmsg = "ignoring parameter " + file.m_parameter->name + 
					" due to being removed from later version (need to update the "
					"template to the latest version)";
				file.m_fp.printWarning(errmsg.c_str());
				return nullptr;
			}
			else
			{
				std::string errmsg = "cannot find parameter " + file.m_parameter->name + 
					" in definition file";
				file.m_fp.printError(errmsg.c_str());
				return CHAR_ERROR;
			}
		}
		else
			file.m_template->addLoadedParam(file.m_parameter->name, arrayIndex);
		line = (*parseFunc)(file, *param, line);
	}
	return line;
}	// parseValue

/**
 * Parses a weighted list of values.
 *
 * @param parseFunc		function to parse a parameter value
 * @param file			template file being read from
 * @param param			template parameter to fill with data
 * @param line			current line being read from
 *
 * @return point where parsing stopped, or -1 on error
 */
template <class Q, class LIST, class VALUE>
const char *parseWeightedList(
	const char * (*parseFunc)(TpfFile &file, Q & param,	const char *line),
	TpfFile &file, Q & param, const char *line)
{
	LIST *list = new LIST;
	param.setValue(list);
	int totalWeight = 0;
	for (;;)
	{
		VALUE value;
		Q *valueParam = nullptr;
		value.value = valueParam = new Q;
		list->push_back(value);
		VALUE *newValue = &list->back();
		// test to see if we need to go to the next line
		IGNORE_RETURN(getNextToken(line, file.m_token));
		if (*file.m_token == '\0')
		{
			// we need to read the next line to continue parsing
			line = file.goToNextLine();
			if (line == CHAR_ERROR)
				return line;
		}
		// get value
		line = (*parseFunc)(file, *valueParam, line);
		if (line == CHAR_ERROR)
			return line;
		line = getNextToken(line, file.m_token);
		if (*file.m_token == '\0')
		{
			file.m_fp.printEolError();
			return CHAR_ERROR;
		}
		if (*file.m_token != ':')
		{
			file.m_fp.printError("expected ':' in weighted list");
			return CHAR_ERROR;
		}
		// get weight
		line = getNextToken(line, file.m_token);
		if (*file.m_token == '\0')
		{
			file.m_fp.printEolError();
			return CHAR_ERROR;
		}
		if (!isinteger(file.m_token))
		{
			file.m_fp.printError("expected integer weight value");
			return CHAR_ERROR;
		}
		int weight = atoi(file.m_token);
		totalWeight += weight;
		newValue->weight = weight;
		// see if we have more values or if we are finished
		line = getNextToken(line, file.m_token);
		if (*file.m_token == '}')
		{
			// end of list
			break;
		}
		else if (*file.m_token != ',')
		{
			file.m_fp.printError("expected , or }");
			return CHAR_ERROR;
		}
	}
	if (totalWeight != 100)
	{
		file.m_fp.printError("weights do not add up to 100");
		return CHAR_ERROR;
	}
	return line;
}	// parseWeightedList

std::string TpfFile::getFileName() const
{
	return m_fp.getFilename().getName() + m_fp.getFilename().getExtension();
}

const std::string & TpfFile::getBaseTemplateName() const
{
	if (m_template != nullptr)
		return m_template->getBaseTemplateName();
	return m_baseTemplateName;
}
