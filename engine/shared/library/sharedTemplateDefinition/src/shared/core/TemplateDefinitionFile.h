//========================================================================
//
// TemplateDefinitionFile.h - a template definition file interpreter.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_TemplateDefinitionFile_H
#define _INCLUDED_TemplateDefinitionFile_H

#include "pcre.h"
#include "sharedFoundation/Tag.h"
#include "TemplateData.h"

class File;


class TemplateDefinitionFile
{
public:
	TemplateDefinitionFile(void);
	~TemplateDefinitionFile();

	void setTemplateFilename(const std::string &name);
	void setBaseFilename(const std::string &name);

	bool isWriteForCompiler(void) const;
	void setWriteForCompiler(bool flag);

	const std::string & getTemplateNameFilter(void) const;
	bool isValidTemplateName(const Filename & name) const;

	void writeFileComments(File &fp) const;
	void writeTemplateId(File &fp) const;
	void writeClassHeaderBegin(File &fp) const;
	void writeClassHeaderEnd(File &fp, const TemplateData & sourceTemplate) const;
	void writeClassSourceBegin(File &fp, const TemplateData & sourceTemplate) const;

	int parse(File &fp);

	const Filename &                    getPath(void) const;
	const Filename &                    getCompilerPath(void) const;
	TemplateLocation                    getTemplateLocation(void) const;
	TemplateData *                      getTemplateData(int version) const;
	const std::vector<std::string> &    getFileComments(void) const;
	const std::string &                 getTemplateName(void) const;
	const std::string &                 getTemplateFilename(void) const;
	const std::string &                 getBaseName(void) const;
	const std::string &                 getBaseFilename(void) const;
	const TemplateDefinitionFile *                     getBaseDefinitionFile(void) const;
	const TagInfo &                     getTemplateId(void) const;
	int                                 getHighestVersion(void) const;

private:
	void cleanup(void);

	Filename                        m_path;
	Filename                        m_compilerPath;
	TemplateLocation                m_templateLocation;
	std::map<int, TemplateData *>   m_templateMap;
	std::vector<std::string>        m_fileComments;
	std::string                     m_templateName;
	std::string                     m_templateFilename;
	std::string                     m_baseName;
	std::string                     m_baseFilename;
	TemplateDefinitionFile *                       m_baseDefinitionFile;
	TagInfo                         m_templateId;
	bool                            m_writeForCompilerFlag;		// flag that the C++ we are writing is for the template compiler
	int                             m_highestVersion;
	std::string                     m_templateNameFilter;
	pcre                           *m_filterCompiledRegex;
};


inline void TemplateDefinitionFile::setTemplateFilename(const std::string &name)
{
	m_templateFilename = name;
	m_templateName = EnumLocationTypes[m_templateLocation] +
		filenameLowerToUpper(name);
}

inline void TemplateDefinitionFile::setBaseFilename(const std::string &name)
{
	m_baseFilename = name;
	m_baseName = EnumLocationTypes[m_templateLocation] +
		filenameLowerToUpper(name);
}

inline bool TemplateDefinitionFile::isWriteForCompiler(void) const
{
	return m_writeForCompilerFlag;
}

inline const Filename & TemplateDefinitionFile::getPath(void) const
{
	return m_path;
}

inline const Filename & TemplateDefinitionFile::getCompilerPath(void) const
{
	return m_compilerPath;
}

inline TemplateLocation TemplateDefinitionFile::getTemplateLocation(void) const
{
	return m_templateLocation;
}

inline TemplateData *TemplateDefinitionFile::getTemplateData(int version) const
{
	std::map<int, TemplateData *>::const_iterator iter = m_templateMap.find(version);
	if (iter == m_templateMap.end())
		return nullptr;
	return (*iter).second;
}

inline const std::vector<std::string> &TemplateDefinitionFile::getFileComments(void) const
{
	return m_fileComments;
}

inline const std::string &TemplateDefinitionFile::getTemplateName(void) const
{
	return m_templateName;
}

inline const std::string &TemplateDefinitionFile::getTemplateFilename(void) const
{
	return m_templateFilename;
}

inline const std::string &TemplateDefinitionFile::getBaseName(void) const
{
	return m_baseName;
}

inline const std::string &TemplateDefinitionFile::getBaseFilename(void) const
{
	return m_baseFilename;
}

inline const TemplateDefinitionFile * TemplateDefinitionFile::getBaseDefinitionFile(void) const
{
	return m_baseDefinitionFile;
}

inline const TagInfo & TemplateDefinitionFile::getTemplateId(void) const
{
	return m_templateId;
}

inline int TemplateDefinitionFile::getHighestVersion(void) const
{
	return m_highestVersion;
}


#endif	// _INCLUDED_TemplateDefinitionFile_H
