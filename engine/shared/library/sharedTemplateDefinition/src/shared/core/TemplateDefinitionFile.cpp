//========================================================================
//
// TemplateDefinitionFile.cpp - a template definition file interpreter.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedTemplateDefinition/FirstSharedTemplateDefinition.h"
#include "File.h"
#include "TemplateGlobals.h"
#include "TemplateDefinitionFile.h"
#include "TemplateData.h"
#include "sharedRegex/RegexServices.h"
#include "sharedFoundation/SetupSharedFoundation.h"

//==============================================================================

/**
 * Class constructor.
 */
TemplateDefinitionFile::TemplateDefinitionFile(void) :
	m_baseDefinitionFile(nullptr),
	m_writeForCompilerFlag(false),
	m_filterCompiledRegex(nullptr)
{
	cleanup();
}	// TemplateDefinitionFile::TemplateDefinitionFile

/**
 * Class destructor.
 */
TemplateDefinitionFile::~TemplateDefinitionFile()
{
	cleanup();
}	// TemplateDefinitionFile::~TemplateDefinitionFile

/**
 * Resets all the class data.
 */
void TemplateDefinitionFile::cleanup(void)
{
	m_highestVersion = 0;
	m_templateId.tag = NO_TAG;
	m_templateId.tagString = NO_TAG_STRING;
	m_templateLocation = LOC_NONE;
	IGNORE_RETURN(m_templateName.erase());
	IGNORE_RETURN(m_baseName.erase());
	IGNORE_RETURN(m_templateNameFilter.erase());
	m_path.clear();
	m_compilerPath.clear();
	m_fileComments.clear();

	if (m_baseDefinitionFile != nullptr)
	{
		delete m_baseDefinitionFile;
		m_baseDefinitionFile = nullptr;
	}

	std::map<int, TemplateData *>::iterator iter;
	for (iter = m_templateMap.begin(); iter != m_templateMap.end(); ++iter)
	{
		delete (*iter).second;
		(*iter).second = nullptr;
	}
	m_templateMap.clear();

	if (m_filterCompiledRegex != nullptr)
	{
		RegexServices::freeMemory(m_filterCompiledRegex);
		m_filterCompiledRegex = nullptr;
	}
}	// TemplateDefinitionFile::cleanup

/**
 * Sets the write for compile flags on the tamplate data instances.
 *
 * @param flag		flag that we are going to write code for the template compiler
 */
void TemplateDefinitionFile::setWriteForCompiler(bool flag)
{
	m_writeForCompilerFlag = flag;
	if (flag && m_baseName.size() == 0)
	{
		m_baseName = COMPILER_ROOT_TEMPLATE_NAME;
	}
	else if (!flag && m_baseName == COMPILER_ROOT_TEMPLATE_NAME)
	{
		m_baseName.clear();
	}

	std::map<int, TemplateData *>::iterator iter;
	for (iter = m_templateMap.begin(); iter != m_templateMap.end(); ++iter)
		(*iter).second->setWriteForCompiler(flag);
}	// TemplateDefinitionFile::setWriteForCompiler

/**
 * Returns the template file name filter for this definition.
 */
const std::string & TemplateDefinitionFile::getTemplateNameFilter(void) const
{
	static const std::string wildcard = "*";

	if (!m_templateNameFilter.empty())
		return m_templateNameFilter;

	if (m_baseDefinitionFile != nullptr)
		return m_baseDefinitionFile->getTemplateNameFilter();

	return wildcard;
}	// TemplateDefinitionFile::getTemplateNameFileter

/**
 * Tests if a template file name is valid for this definition.
 *
 * @param name		the template file name
 *
 * @return true if the name is valid, false if not
 */
bool TemplateDefinitionFile::isValidTemplateName(const Filename & name) const
{
	if (m_templateNameFilter.empty())
	{
		if (m_baseDefinitionFile != nullptr)
			return m_baseDefinitionFile->isValidTemplateName(name);
		else
			return true;
	}

	int const maxCaptureCount = 10;
	int const matchDataElementCount = maxCaptureCount * 3;
	int       matchData[matchDataElementCount];

	int const matchCode = pcre_exec(m_filterCompiledRegex, nullptr, name.getName().c_str(), name.getName().length(), 0, 0, matchData, matchDataElementCount);
	bool const result = (matchCode >= 0);

	if (matchCode < -1)
	{
		fprintf(stderr, "ERROR: pcre_exec() failed, error code [%d].\n", matchCode);
		return false;
	}

	return result;
}	// TemplateDefinitionFile::isValidTemplateName

/**
 * Writes file-level comments.
 *
 * @param fp		the file to write to
 */
void TemplateDefinitionFile::writeFileComments(File &fp) const
{
	std::vector<std::string>::const_iterator iter;
	for (iter = getFileComments().begin(); iter != getFileComments().end(); ++iter)
	{
		fp.print("%s\n", (*iter).c_str());
	}
}	// TemplateDefinitionFile::writeFileComments

/**
 * Writes the template id tag definition.
 *
 * @param fp		the file to write to
 */
void TemplateDefinitionFile::writeTemplateId(File &fp) const
{
	fp.print("%s\n", TDF_ID_BEGIN);
	fp.print("\tenum\n");
	fp.print("\t{\n");
	fp.print("\t\t%s_tag = %s\n", getTemplateName().c_str(), getTemplateId().tagString.c_str());
	fp.print("\t};\n");
	fp.print("%s\n", TDF_ID_END);
}	// TemplateDefinitionFile::writeTemplateId

/**
 * Writes the beginning of a class header file.
 *
 * @param fp		the file to write to
 */
void TemplateDefinitionFile::writeClassHeaderBegin(File &fp) const
{
	const char * name = getTemplateName().c_str();
	std::string bn = getBaseName();
	const char * baseName = bn.c_str();
	const char * baseNamePath = "";
	if (baseName[0] == '\0' && m_templateLocation != LOC_NONE)
	{
		baseNamePath = "sharedObject/";
		baseName = ROOT_TEMPLATE_NAME;
	}
	else if (_stricmp(baseName, "tpfTemplate") == 0)
	{
		baseNamePath = "sharedTemplateDefinition/";
	}

	fp.print("//========================================================================\n");
	fp.print("//\n");
	fp.print("// %s.h\n", name);
	fp.print("//\n");
	fp.print("//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be\n");
	fp.print("//overwritten the next time the template definition is compiled. Do not\n");
	fp.print("//make changes to code inside these blocks.\n");
	fp.print("//\n");
	if (getFileComments().size() > 0)
	{
		writeFileComments(fp);
		fp.print("//\n");
	}
	fp.print("// copyright 2001 Sony Online Entertainment\n");
	fp.print("//\n");
	fp.print("//========================================================================\n");
	fp.print("\n");
	fp.print("#ifndef _INCLUDED_%s_H\n", name);
	fp.print("#define _INCLUDED_%s_H\n", name);
	fp.print("\n");
	fp.print("#include \"%s%s.h\"\n", baseNamePath, baseName);
	fp.print("#include \"sharedFoundation/DynamicVariable.h\"\n");
	if (m_writeForCompilerFlag)
		fp.print("#include \"sharedTemplateDefinition/TpfTemplate.h\"\n");
	else
		fp.print("#include \"sharedUtility/TemplateParameter.h\"\n");
	fp.print("\n");
	fp.print("\n");
	fp.print("class Vector;\n");
	fp.print("typedef StructParam<ObjectTemplate> StructParamOT;\n");
	if (!m_writeForCompilerFlag)
	{
		std::map<int, TemplateData *>::const_iterator iter;
		iter = m_templateMap.find(m_highestVersion);
		if (iter != m_templateMap.end())
			(*iter).second->writeHeaderFwdDecls(fp);
	}
	fp.print("\n");
	fp.print("\n");
	fp.print("class %s : public %s\n", name, baseName);
	fp.print("{\n");
	fp.print("public:\n");
	writeTemplateId(fp);
	fp.print("public:\n");
	fp.print("\t         %s(const std::string & filename);\n", name);
	fp.print("\tvirtual ~%s();\n", name);
	fp.print("\n");
	fp.print("\tvirtual Tag getId(void) const;\n");
	fp.print("\tvirtual Tag getTemplateVersion(void) const;\n");
	fp.print("\tvirtual Tag getHighestTemplateVersion(void) const;\n");
	if (m_writeForCompilerFlag)
		fp.print("\tstatic void install(void);\n");
	else
		fp.print("\tstatic void install(bool allowDefaultTemplateParams = true);\n");
	fp.print("\n");
}	// writeClassHeaderBegin

/**
 * Writes the ending of a class header file.
 *
 * @param fp				the file to write to
 * @param sourceTemplate	the template we are writing code for
 */
void TemplateDefinitionFile::writeClassHeaderEnd(File &fp, const TemplateData & sourceTemplate) const
{
	UNREF(sourceTemplate);

	const char *name = getTemplateName().c_str();

	fp.print("\n");
	fp.print("private:\n");
	fp.print("\tTag  m_templateVersion;\t// the template version\n");
	fp.print("\tbool m_versionOk;\t// flag that the template version loaded is "
		"the one we expect\n");
	if (!m_writeForCompilerFlag)
		fp.print("\tstatic bool ms_allowDefaultTemplateParams;\t// flag to allow defaut params instead of fataling\n");
	fp.print("\n");
	fp.print("\tstatic void registerMe(void);\n");
	fp.print("\tstatic ObjectTemplate * create(const std::string & filename);\n");
	fp.print("\n");
	fp.print("\t// no copying\n");
	fp.print("\t%s(const %s &);\n", name, name);
	fp.print("\t%s & operator =(const %s &);\n", name, name);
	fp.print("};\n");
	fp.print("\n");
	fp.print("\n");
	if (m_writeForCompilerFlag)
	{
		fp.print("inline void %s::install(void)\n", name);
		fp.print("{\n");
	}
	else
	{
		fp.print("inline void %s::install(bool allowDefaultTemplateParams)\n", name);
		fp.print("{\n");
		fp.print("\tms_allowDefaultTemplateParams = allowDefaultTemplateParams;\n");
	}
	sourceTemplate.writeRegisterTemplate(fp, "\t");
	fp.print("}\n");
	fp.print("\n");
	fp.print("\n");
	fp.print("#endif\t// _INCLUDED_%s_H\n", name);
}	// TemplateDefinitionFile::writeClassHeaderEnd

/**
 * Writes the beginning of a class source file.
 *
 * @param fp				the file to write to
 * @param sourceTemplate	the template we are writing code for
 */
void TemplateDefinitionFile::writeClassSourceBegin(File &fp, const TemplateData & sourceTemplate) const
{
	const char *name = getTemplateName().c_str();

	fp.print("//========================================================================\n");
	fp.print("//\n");
	fp.print("// %s.cpp\n", name);
	fp.print("//\n");
	fp.print("//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be\n");
	fp.print("//overwritten the next time the template definition is compiled. Do not\n");
	fp.print("//make changes to code inside these blocks.\n");
	fp.print("//\n");
	if (getFileComments().size() > 0)
	{
		writeFileComments(fp);
		fp.print("//\n");
	}
	fp.print("// copyright 2001 Sony Online Entertainment\n");
	fp.print("//\n");
	fp.print("//========================================================================\n");
	fp.print("\n");
	if (m_writeForCompilerFlag)
	{
		fp.print("#include \"sharedTemplate/FirstSharedTemplate.h\"\n");
		fp.print("#include \"sharedTemplateDefinition/TemplateData.h\"\n");
		fp.print("#include \"sharedTemplateDefinition/TemplateGlobals.h\"\n", name);
	}
	else if (m_templateLocation == LOC_CLIENT)
		fp.print("#include \"%s\"\n", CLIENT_TEMPLATE_PRECOMPILED_HEADER_NAME);
	else if (m_templateLocation == LOC_SERVER)
		fp.print("#include \"%s\"\n", SERVER_TEMPLATE_PRECOMPILED_HEADER_NAME);
	else
		fp.print("#include \"%s\"\n", SHARED_TEMPLATE_PRECOMPILED_HEADER_NAME);
	fp.print("#include \"%s.h\"\n", name);
	fp.print("#include \"sharedDebug/DataLint.h\"\n");
	fp.print("#include \"sharedFile/Iff.h\"\n");
	if (m_writeForCompilerFlag)
		fp.print("#include \"sharedTemplateDefinition/ObjectTemplate.h\"\n");
	else
	{
		fp.print("#include \"sharedObject/ObjectTemplate.h\"\n");
		fp.print("#include \"sharedObject/ObjectTemplateList.h\"\n");
		sourceTemplate.writeSourceTemplateIncludes(fp);
	}
	fp.print("#include <stdio.h>\n");
	fp.print("\n");

	if (!m_writeForCompilerFlag)
	{
		fp.print("const std::string DefaultString(\"\");\n");
		fp.print("const StringId DefaultStringId(\"\", 0);\n");
		fp.print("const Vector DefaultVector(0,0,0);\n");
		fp.print("const TriggerVolumeData DefaultTriggerVolumeData;\n");
		fp.print("\n");
		fp.print("bool %s::ms_allowDefaultTemplateParams = true;\n", name);
	}

	fp.print("\n");
	fp.print("\n");
	fp.print("/**\n");
	fp.print(" * Class constructor.\n");
	fp.print(" */\n");
	fp.print("%s::%s(const std::string & filename)\n", name, name);
	//	if (sourceTemplate.hasList())
	sourceTemplate.writeSourceLoadedFlagInit(fp);
	fp.print("{\n");
	//	fp.print("\tsetId(%s);\n", getTemplateId().tagString.c_str());
	fp.print("}	// %s::%s\n", name, name);
	fp.print("\n");
	fp.print("/**\n");
	fp.print(" * Class destructor.\n");
	fp.print(" */\n");
	fp.print("%s::~%s()\n", name, name);
	fp.print("{\n");
	sourceTemplate.writeSourceCleanup(fp);
	fp.print("}	// %s::~%s\n", name, name);
	fp.print("\n");
	fp.print("/**\n");
	fp.print(" * Static function used to register this template.\n");
	fp.print(" */\n");
	fp.print("void %s::registerMe(void)\n", name);
	fp.print("{\n");
	fp.print("\tObjectTemplateList::registerTemplate(%s_tag, create);\n", name);
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
	fp.print("}	// %s::create\n", name);
	fp.print("\n");
	fp.print("/**\n");
	fp.print(" * Returns the template id.\n");
	fp.print(" *\n");
	fp.print(" * @return the template id\n");
	fp.print(" */\n");
	fp.print("Tag %s::getId(void) const\n", name);
	fp.print("{\n");
	fp.print("\treturn %s_tag;\n", name);
	fp.print("}	// %s::getId\n", name);
	fp.print("\n");
	fp.print("/**\n");
	fp.print(" * Returns this template's version.\n");
	fp.print(" *\n");
	fp.print(" * @return the version\n");
	fp.print(" */\n");
	fp.print("Tag %s::getTemplateVersion(void) const\n", name);
	fp.print("{\n");
	fp.print("\treturn m_templateVersion;\n");
	fp.print("} // %s::getTemplateVersion\n", name);
	fp.print("\n");
	fp.print("/**\n");
	fp.print(" * Returns the highest version of this template or it's base templates.\n");
	fp.print(" *\n");
	fp.print(" * @return the highest version\n");
	fp.print(" */\n");
	fp.print("Tag %s::getHighestTemplateVersion(void) const\n", name);
	fp.print("{\n");
	fp.print("\tif (m_baseData == nullptr)\n");
	fp.print("\t\treturn m_templateVersion;\n");
	fp.print("\tconst %s * base = dynamic_cast<const %s *>(m_baseData);\n", name, name);
	fp.print("\tif (base == nullptr)\n");
	fp.print("\t\treturn m_templateVersion;\n");
	fp.print("\treturn std::max(m_templateVersion, base->getHighestTemplateVersion());\n");
	fp.print("} // %s::getHighestTemplateVersion\n", name);
	fp.print("\n");
}	// TemplateDefinitionFile::writeClassSourceBegin

/**
 * Main parser for a tdf file. Parses initialization data, creates the header and
 * source files, and passes parameter definitions to the appropriate function.
 *
 * @param fp				file to parse
 *
 * @return 0 on success, -1 on fail
 */
int TemplateDefinitionFile::parse(File &fp)
{
	static const int BUFFER_SIZE = 1024;
	int lineLen;
	char buffer[BUFFER_SIZE];
	char token[BUFFER_SIZE];
	TemplateData *currentTemplate = nullptr;

	cleanup();

	setTemplateFilename(fp.getFilename().getName());

	for (;;)
	{
		lineLen = fp.readLine(buffer, BUFFER_SIZE);
		if (lineLen == -1)
			break;
		else if (lineLen == -2)
			return -1;

		const char *line = buffer;
		line = getNextWhitespaceToken(line, token);
		if (*token == '\0')
			break;

		if (strcmp(token, "version") == 0)
		{
			if (m_templateName.size() == 0)
			{
				fp.printError("no template name defined");
				return -1;
			}
			else if (m_templateId.tag == NO_TAG)
			{
				fp.printError("no template id defined");
				return -1;
			}
			else if (m_path.getPath().size() == 0)
			{
				fp.printError("no path defined");
				return -1;
			}
			else if (m_compilerPath.getPath().size() == 0)
			{
				fp.printError("no compiler path defined");
				return -1;
			}
			//			if (m_baseName.size() == 0 && m_templateName != ROOT_TEMPLATE_NAME)
			//				m_baseName = ROOT_TEMPLATE_NAME;
			line = getNextWhitespaceToken(line, token);
			int version = atoi(token);
			if (version < 0 || version > 9999)
			{
				fp.printError("version out of range");
				return -1;
			}
			if (m_templateMap.find(version) != m_templateMap.end())
			{
				fp.printError("version already defined");
				return -1;
			}
			if (version > m_highestVersion)
				m_highestVersion = version;

			currentTemplate = new TemplateData(version, *this);
			m_templateMap[version] = currentTemplate;
		}
		else if (currentTemplate != nullptr)
		{
			line = currentTemplate->parseLine(fp, buffer, token);
			if (line == CHAR_ERROR)
				return -1;
		}
		else if (*token == '/' && *(token + 1) == '/')
		{
			if (m_baseName.size() == 0 && m_templateId.tag == NO_TAG)
				m_fileComments.push_back(buffer);
		}
		else if (strcmp(token, "base") == 0)
		{
			if (m_baseName.size() != 0)
			{
				fp.printError("base name already defined");
				return -1;
			}
			line = getNextWhitespaceToken(line, token);
			setBaseFilename(token);

			// load and parse the base template
			Filename baseFileName = fp.getFilename();
			baseFileName.setName(token);
			File baseFp(baseFileName, "rt");
			if (!baseFp.isOpened())
			{
				fp.printError("unable to open base template definition");
				return -1;
			}
			if (m_baseDefinitionFile == nullptr)
				m_baseDefinitionFile = new TemplateDefinitionFile;
			else
				m_baseDefinitionFile->cleanup();
			int result = m_baseDefinitionFile->parse(baseFp);
			if (result != 0)
				return result;
		}
		else if (strcmp(token, "id") == 0)
		{
			if (m_templateId.tag != NO_TAG)
			{
				fp.printError("template id already defined");
				return -1;
			}
			line = getNextWhitespaceToken(line, token);
			if (strlen(token) != 4)
			{
				fp.printError("id not 4 characters");
				return -1;
			}
			m_templateId.tag = ConvertStringToTag(token);
			m_templateId.tagString = ConvertStringToTagString(token);
		}
		else if (strcmp(token, "templatename") == 0)
		{
			line = getNextWhitespaceToken(line, token);
			m_templateNameFilter = token;

			//-- Attempt to compile the regex.
			if (m_filterCompiledRegex != nullptr)
			{
				// First free the existing compiled regex.
				RegexServices::freeMemory(m_filterCompiledRegex);
				m_filterCompiledRegex = nullptr;
			}

			//-- Compile the new regex.
			char const *errorString = nullptr;
			int         errorOffset = 0;

			m_filterCompiledRegex = pcre_compile(m_templateNameFilter.c_str(), 0, &errorString, &errorOffset, nullptr);
			WARNING(m_filterCompiledRegex == nullptr, ("TemplateDefinitionFile::parse(): pcre_compile() failed, error=[%s], errorOffset=[%d], regex text=[%s].", errorString, errorOffset, m_templateNameFilter.c_str()));
		}
		else if (strcmp(token, "clientpath") == 0 ||
			strcmp(token, "serverpath") == 0 ||
			strcmp(token, "sharedpath") == 0)
		{
			if (m_path.getPath().size() != 0)
			{
				fp.printError("path already defined");
				return -1;
			}
			if (strcmp(token, "clientpath") == 0)
				m_templateLocation = LOC_CLIENT;
			else if (strcmp(token, "serverpath") == 0)
				m_templateLocation = LOC_SERVER;
			else
				m_templateLocation = LOC_SHARED;
			line = getNextWhitespaceToken(line, token);
			m_path.setPath(token);
			m_path.prependPath(fp.getFilename());
			// reset the template name to add the corrent prefix to the template
			// name
			setTemplateFilename(m_templateFilename);
			if (!m_baseFilename.empty())
				setBaseFilename(m_baseFilename);
		}
		else if (strcmp(token, "compilerpath") == 0)
		{
			if (m_compilerPath.getPath().size() != 0)
			{
				fp.printError("compiler path already defined");
				return -1;
			}
			line = getNextWhitespaceToken(line, token);
			m_compilerPath.setPath(token);
			m_compilerPath.prependPath(fp.getFilename());
		}
		else
		{
			char errbuf[2048];
			snprintf(errbuf, 2048, "I don't know how to handle this line!: <%s>. "
				"Barfed on token <%s>.", buffer, token);
			fp.printError(errbuf);
			return -1;
		}
	}
	return 0;
}	// TemplateDefinitionFile::parse