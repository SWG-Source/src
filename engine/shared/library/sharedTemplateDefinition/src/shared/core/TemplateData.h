//========================================================================
//
// TemplateData.h - data for a given version of a template
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_TemplateData_H
#define _INCLUDED_TemplateData_H

#include "sharedFoundation/Tag.h"
#include "TemplateGlobals.h"

#include <climits>

class File;
class TemplateDefinitionFile;
class TpfTemplate;
class TemplateDataIterator;

static const int INVALID_ENUM_RESULT = INT_MIN;

struct TagInfo
{
	Tag tag;
	std::string tagString;
};


class TemplateData
{
	friend class TemplateDataIterator;

public:

	// public enums
	enum ParamType
	{
		TYPE_NONE,
		TYPE_COMMENT,
		TYPE_INTEGER,
		TYPE_FLOAT,
		TYPE_BOOL,
		TYPE_STRING,
		TYPE_STRINGID,
		TYPE_VECTOR,
		TYPE_DYNAMIC_VAR,
		TYPE_TEMPLATE,
		TYPE_ENUM,
		TYPE_STRUCT,
		TYPE_TRIGGER_VOLUME,
		TYPE_FILENAME,
		NUM_PARAM_TYPES
	};
	enum ListType
	{
		LIST_NONE,
		LIST_LIST,
		LIST_INT_ARRAY,
		LIST_ENUM_ARRAY
	};

	// info about a template parameter
	struct Parameter
	{
		ParamType           type;
		std::string         name;
		std::string         description;		// description of the parameter or a line comment
		std::string         extendedName;		// name for template, enum, or struct types
		int                 min_int_limit;		// for type range
		int                 max_int_limit;		// for type range
		float               min_float_limit;	// for type range
		float               max_float_limit;	// for type range
		ListType            list_type;
		int                 list_size;
		std::string         enum_list_name;		// enum used for enum indexed list
	};

	struct EnumData
	{
		std::string name;
		std::string valueName;
		int value;
		std::string comment;
	};

	typedef std::vector<EnumData> EnumList;
	typedef std::map<std::string, EnumList> EnumMap;

public:

	explicit TemplateData(int version, const TemplateDefinitionFile &parent);
	explicit TemplateData(const TemplateData *parent, const std::string &name);
	~TemplateData();

	const std::string getName(void) const;
	const std::string getBaseName(void) const;
	TemplateLocation getTemplateLocation(void) const;
	const TagInfo & getVersion(void) const;

	bool hasList(void) const;

	void setWriteForCompiler(bool flag);
	bool isWritingForCompiler(void) const;

	// template definition parsing functions
	const char * parseLine(const File &fp, const char *buffer, char *tokenbuf);
	const Parameter *getParameter(const char *name, bool deepCheck = false) const;
	int parseIntValue(const File &fp, const char * line, const char ** endLine, char * intbuf) const;
	int getEnumValue(const char * enumValue) const;
	int getEnumValue(const std::string & enumType, const char * enumValue) const;
	const EnumList * getEnumList(const std::string & name, bool define) const;
	const TemplateData * getStruct(const char *name) const;
	const TemplateDefinitionFile * getTdf() const;
	const TemplateDefinitionFile * getTdfParent() const;
	const TemplateDefinitionFile * getTdfForParameter(const char *parameterName) const;

	// template verification functions
	bool verifyTemplate(const TpfTemplate *tpfTemplate, const File &file) const;
	void updateTemplate(const TpfTemplate *tpfTemplate, File & file) const;

	// template C++ writing functions
	void writeRegisterTemplate(File &fp, const char * leadInChars) const;
	void writeHeaderFwdDecls(File &fp) const;
	void writeHeaderParams(File &fp) const;
	void writeSourceTemplateIncludes(File &fp) const;
	void writeSourceLoadedFlagInit(File &fp) const;
	int  writeSourceMethods(File &fp) const;
	void writeSourceCleanup(File &fp) const;

	// template writing functions
	void writeDefaultTemplateFile(File &fp) const;

private:

	enum ParseState
	{
		STATE_PARAM,
		STATE_ENUM,
		STATE_STRUCT
	};

	typedef std::vector<Parameter> ParameterList;
	typedef std::map<std::string, Parameter> ParameterMap;
	typedef std::vector<TemplateData *> StructList;
	typedef std::map<std::string, TemplateData *> StructMap;

	const TemplateDefinitionFile *         m_fileParent;				// if the data is for the root template
	const TemplateData *    m_templateParent;			// if the data is for a structure in a template
	TagInfo                 m_structId;					// id of the structure
	bool					m_hasTemplateParam;			// flag that one of the parameters is a template (for structures)
	bool                    m_hasDynamicVarParam;		// flag that one of the parameters is a dynamic variable (for structures)
	bool                    m_hasList;					// flag that one of the parameters is a list
	std::string             m_name;						// template name
	std::string             m_baseName;					// name of base class (if any)
	bool                    m_writeForCompilerFlag;		// flag that the C++ we are writing is for the template compiler
	TagInfo                 m_version;					// version of this data
	int                     m_bracketCount;				// number of '[' we haved parsed 
	ParseState				m_parseState;				// whether we are parsing parameters, enum def, of struct def
	ParameterList           m_parameters;				// parameters for the template
	ParameterMap            m_parameterMap;				// param name->parameter lookup
	EnumList *				m_currentEnumList;			// current enum being parsed from tdf file
	EnumMap                 m_enumMap;					// enums defined for the template
	TemplateData *			m_currentStruct;			// current struct being parsed from tdf file
	StructMap               m_structMap;				// structs defined for the template
	StructList              m_structList;				// structs defined for the template, in order that they were in the definition file

private:

	// no default constructor
	TemplateData(void);
	TemplateData(const TemplateData &parent);
	TemplateData & operator =(const TemplateData &);

	// access functions
	void getTemplateNames(std::set<std::string> &names) const;
	const TemplateDefinitionFile * getFileParent(void) const;

	// template definition parsing functions
	const char * parseEnum(const File &fp, const char *buffer, char *tokenbuf);
	const char * parseStruct(const File &fp, const char *buffer, char *tokenbuf);

	// template C++ writing functions
	void writeMethodParams(File &fp, const Parameter &param, bool headerParams) const;
	void writeHeaderEnums(File &fp) const;
	void writeHeaderStructs(File &fp) const;
	void writeHeaderMethods(File &fp) const;
	void writeHeaderVariables(File &fp) const;
	void writeHeaderVariables(File &fp, const ParameterList &list, 
		const char *namePrefix) const;
	void writeStructDefinition(File &fp) const;
	void writeSourceStructStart(File &fp) const;
	void writeSourceStructStart(File &fp, const std::string &name) const;
	void writeSourceReturnBaseValue(File &fp, const Parameter &param, const std::string & minMaxString) const;
	int  writeSourceGetData(File &fp) const;
	void writeSourceTestData(File &fp) const;
	void writeSourceGetGeneric(File &fp, const Parameter &param) const;
	void writeSourceGetVector(File &fp, const Parameter &param) const;
	void writeSourceGetDynamicVariable(File &fp, const Parameter &param) const;
	void writeSourceGetTemplate(File &fp, const Parameter &param) const;
	void writeSourceGetEnum(File &fp, const Parameter &param) const;
	int  writeSourceGetStruct(File &fp, const Parameter &param) const;
	void writeSourceGetStructAssignments(File &fp, const std::string & versionString, const std::string & minMaxString) const;
	void writeSourceReadIff(File &fp) const;
	void writeSourceWriteIff(File &fp) const;

	// template C++ template compiler functions
	void writeCompilerHeaderParams(File &fp) const;
	void writeCompilerSourceMethods(File &fp) const;
	void writeCompilerHeaderMethods(File &fp) const;
	void writeCompilerSourceAccessMethods(File &fp) const;

	// default template writing functions
	void writeEnumsToDefaultTemplate(File &fp) const;
	void writeStructsToDefaultTemplate(File &fp) const;
	void writeParamTypeToDefaultTemplate(File &fp, const Parameter &param) const;
	void writeParameterDefault(File &fp, const Parameter &param, int index = -1) const;
	void writeStructParameterDefault(File &fp, const Parameter &param, bool final) const;
	void writeDefaultValue(File &fp, const Parameter &param) const;
};


inline const TagInfo & TemplateData::getVersion(void) const
{
	return m_version;
}

inline bool TemplateData::hasList(void) const
{
	return m_hasList;
}

inline bool TemplateData::isWritingForCompiler(void) const
{
	return m_writeForCompilerFlag;
}

inline const TemplateDefinitionFile * TemplateData::getFileParent(void) const
{
	if (m_fileParent != nullptr)
		return m_fileParent;
	if (m_templateParent != nullptr)
		return m_templateParent->getFileParent();
	return nullptr;
}	// TemplateData::getFileParent


#endif	// _INCLUDED_TemplateData_H
