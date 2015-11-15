//========================================================================
//
// TpfFile.h - a template file interpreter.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_TpfFile_H
#define _INCLUDED_TpfFile_H

#include "File.h"
#include "ObjectTemplate.h"
#include "sharedUtility/TemplateParameter.h"
#include "TemplateDefinitionFile.h"
#include "TemplateData.h"


class CompilerIntegerParam;
class TpfTemplate;
class TpfFile;

typedef StructParam<ObjectTemplate> StructParamOT;


//========================================================================
// friend functions

// can we do this some how?
//template <class T>
//typedef const char * (*ParseParamFuncPtr)(T & param, const char *line);
template <class Q, class LIST, class VALUE>
const char *parseWeightedList(
	const char * (*parseFunc)(TpfFile &, Q &, const char *), 
	TpfFile &file, Q & param, const char *line);

template <class Q>
const char * parseValue(
	Q * (*getParamFunc)(TpfTemplate &, const std::string &, int), 
	const char * (*parseFunc)(TpfFile &, Q &, const char *), 
	TpfFile &file, const char *line, int arrayIndex);


//========================================================================
//

class TpfFile
{
	template <class Q, class LIST, class VALUE>
	friend const char *parseWeightedList(
		const char * (*parseFunc)(TpfFile &, Q &, const char *), 
		TpfFile &file, Q & param, const char *line);

	template <class Q>
	friend const char * parseValue(
		Q * (*getParamFunc)(TpfTemplate &, const std::string &, int), 
		const char * (*parseFunc)(TpfFile &, Q &, const char *), 
		TpfFile &file, const char *line, int arrayIndex);

public:
	TpfFile(void);
	~TpfFile();

	int loadTemplate(const Filename & filename);
	int makeIffFiles(const Filename & filename);
//	int updateTemplate(const Filename & filename);

	TemplateLocation    getTemplateLocation(void) const;
	
	const std::string & getTpfPath(void) const;
	const std::string & getPath(void) const;
	const std::string & getIffPath(void) const;
	std::string         getFileName() const;
	const std::string & getBaseTemplateName() const;

	const TemplateDefinitionFile *     getTemplateDefinitionFile() const;

	TpfTemplate * getTemplate() const;

public:
	enum 
	{
		// we can't define const int values, but we can define enum values
		BUFFER_SIZE = 16384
	};

	File                  m_fp;                     // tpf being read
	char                  m_buffer[BUFFER_SIZE];    // storage for reading tpf
	char                  m_token[BUFFER_SIZE];     // storage for next token
	TpfTemplate *         m_template;				// template we are creating
	std::string           m_baseTemplateName;       // temp storage of template we want to derive our template from
	TemplateDefinitionFile               m_templateDef;            // definition file for the template
	const TemplateDefinitionFile         *m_currTemplateDef;       // which definition (in the TDF hierarchy) we're on
	const TemplateData *  m_templateData;           // template data info for the whole TpfFile
	const TemplateData *  m_highestTemplateData;    // template data info for the highest tdf version
	const TemplateData::Parameter* m_parameter;     // info on current parameter
	std::string           m_path;                   // where to put the C++ files
	std::string           m_iffPath;                // where to put the iff files
	TemplateLocation      m_templateLocation;		// what kind of template this is
	TemplateData::EnumMap m_enumHeaderMap;			// template-wide enum map defined by external c headers

	void cleanup(void);
	const char * goToNextLine(void);
	void printMinIntError(void);
	void printMaxIntError(void);
	void printMinFloatError(void);
	void printMaxFloatError(void);
	int WriteIffFile(Iff & iffData, const Filename & fileName);
	int parseTemplateCommand(const char *line);
	int parseEnumHeader(const char * headerName);
	int getEnumHeaderValue(const char * enumValue) const;
	const char * parseAssignment(const char *line);
	const char * parseIntegerParameter(CompilerIntegerParam & param, const char *line);
	const char * parseFloatParameter(FloatParam & param, const char *line);
	const char * parseBoolParameter(BoolParam & param, const char *line);
	const char * parseStringParameter(StringParam & param, const char *line);
	const char * parseStringIdParameter(StringIdParam & param, const char *line);
	const char * parseFilenameParameter(StringParam & param, const char *line);
	const char * parseVectorParameter(VectorParam & param, const char *line);
	const char * parseTemplateParameter(StringParam & param, const char *line);
	const char * parseEnumParameter(CompilerIntegerParam & param, const char *line);
	const char * parseDynamicVariableParameter(DynamicVariableParam & param, const char *line);
	const char * parseDynamicVariableParameterList(DynamicVariableParamData & data, const char *line);
	const char * parseStructParameter(StructParamOT & param, const char *line);
	const char * parseTriggerVolumeParameter(TriggerVolumeParam & param, const char *line);

private:
	// callback functions
	static CompilerIntegerParam *getIntegerParam2(TpfTemplate &templt,
		const std::string &paramName, int arrayIndex);
	static FloatParam *getFloatParam2(TpfTemplate &templt,
		const std::string &paramName, int arrayIndex);
	static BoolParam *getBoolParam2(TpfTemplate &templt,
		const std::string &paramName, int arrayIndex);
	static StringParam *getStringParam2(TpfTemplate &templt,
		const std::string &paramName, int arrayIndex);
	static StringIdParam *getStringIdParam2(TpfTemplate &templt,
		const std::string &paramName, int arrayIndex);
	static VectorParam *getVectorParam2(TpfTemplate &templt,
		const std::string &paramName, int arrayIndex);
	static DynamicVariableParam *getDynamicVariableParam2(TpfTemplate &templt,
		const std::string &paramName, int arrayIndex);
	static StructParamOT *getStructParam2(TpfTemplate &templt,
		const std::string &paramName, int arrayIndex);
	static TriggerVolumeParam *getTriggerVolumeParam2(TpfTemplate &templt,
		const std::string &paramName, int arrayIndex);
	static const char * parseIntegerParameter2(TpfFile &file, 
		CompilerIntegerParam & param, const char *line);
	static const char * parseFloatParameter2(TpfFile &file, 
		FloatParam & param, const char *line);
	static const char * parseBoolParameter2(TpfFile &file, 
		BoolParam & param, const char *line);
	static const char * parseStringParameter2(TpfFile &file, 
		StringParam & param, const char *line);
	static const char * parseStringIdParameter2(TpfFile &file, 
		StringIdParam & param, const char *line);
	static const char * parseFilenameParameter2(TpfFile &file, 
		StringParam & param, const char *line);
	static const char * parseVectorParameter2(TpfFile &file, 
		VectorParam & param, const char *line);
	static const char * parseTemplateParameter2(TpfFile &file, 
		StringParam & param, const char *line);
	static const char * parseEnumParameter2(TpfFile &file, 
		CompilerIntegerParam & param, const char *line);
	static const char * parseDynamicVariableParameter2(TpfFile &file, 
		DynamicVariableParam & param, const char *line);
	static const char * parseStructParameter2(TpfFile &file, 
		StructParamOT & param, const char *line);
	static const char * parseTriggerVolumeParameter2(TpfFile &file, 
		TriggerVolumeParam & param, const char *line);

private:
	// no copying
	TpfFile(const TpfFile &);
	TpfFile &operator =(const TpfFile &);
};


inline TemplateLocation TpfFile::getTemplateLocation(void) const
{
	return m_templateLocation;
}

inline const std::string & TpfFile::getTpfPath(void) const
{
	return m_fp.getFilename().getPath();
}

inline const std::string & TpfFile::getPath(void) const
{
	return m_path;
}

inline const std::string & TpfFile::getIffPath(void) const
{
	return m_iffPath;
}

inline const TemplateDefinitionFile * TpfFile::getTemplateDefinitionFile() const
{
	return &m_templateDef;
}

inline TpfTemplate * TpfFile::getTemplate() const
{
	return m_template;
}

//========================================================================
// friend functions

template <class Q, class LIST, class VALUE>
const char *parseWeightedList(
	const char * (*parseFunc)(TpfFile &, Q &, const char *), 
	TpfFile &file, Q & param, const char *line);

template <class Q>
const char * parseValue(
	Q * (*getParamFunc)(TpfTemplate &, const std::string &, int), 
	const char * (*parseFunc)(TpfFile &, Q &, const char *), 
	TpfFile &file, const char *line, int arrayIndex);


#endif	// _INCLUDED_TpfFile_H
