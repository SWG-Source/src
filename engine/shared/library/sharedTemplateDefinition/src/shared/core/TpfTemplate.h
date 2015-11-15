//========================================================================
//
// TpfTemplate.h - adds extra functionality to Template for the compiler
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_TpfTemplate_H
#define _INCLUDED_TpfTemplate_H

#include "sharedTemplateDefinition/CompilerIntegerParam.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedUtility/TemplateParameter.h"

#include <map>
#include <set>
#include <string>

class CompilerIntegerParam;
class TpfFile;
typedef StructParam<ObjectTemplate> StructParamOT;


//========================================================================

class TpfTemplate : public ObjectTemplate
{
public:
	explicit TpfTemplate(const std::string & filename);
	virtual ~TpfTemplate();

	TpfFile *           getParentFile(void) const;
	void                setParentFile(TpfFile * file);

	const std::string & getBaseTemplateName(void) const;
	int                 setBaseTemplateName(const std::string & name);

	TpfTemplate *       getBaseTemplate(void) const;

	virtual void save(Iff &file) = 0;

	static ObjectTemplate * createTemplate(Tag id);

	virtual CompilerIntegerParam *getCompilerIntegerParam(const char *name, bool deepCheck = true, int index = 0);
	virtual FloatParam *getFloatParam(const char *name, bool deepCheck = true, int index = 0);
	virtual BoolParam *getBoolParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringParam *getStringParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringIdParam *getStringIdParam(const char *name, bool deepCheck = true, int index = 0);
	virtual VectorParam *getVectorParam(const char *name, bool deepCheck = true, int index = 0);
	virtual DynamicVariableParam *getDynamicVariableParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StructParamOT *getStructParamOT(const char *name, bool deepCheck = true, int index = 0);
	virtual TriggerVolumeParam *getTriggerVolumeParam(const char *name, bool deepCheck = true, int index = 0);
	virtual void initStructParamOT(StructParamOT &param, const char *name);
	virtual void setAsEmptyList(const char *name);
	virtual void setAppend(const char *name);
	virtual bool isAppend(const char *name) const;
	virtual int getListLength(const char *name) const;

	void addLoadedParam(const std::string & name, int index = 0);
	bool isParamLoaded(const std::string & name, bool deepCheck = true, int index = 0) const;
	void addPureVirtualParam(const std::string & name, int index = 0);
	bool isParamPureVirtual(const std::string &name, bool deepCheck = true, int index = 0) const;

protected:
	TpfFile *     m_parentFile;
	std::string   m_baseTemplateName;
	TpfFile *     m_baseTemplateFile;

private:
	std::set<std::string>                            m_loadedParams;
	std::set<std::string>                            m_pureVirtualParams;
	std::map<std::string, std::vector<std::string> > m_intParamEnumMap;

	void nameAndIndexToName(const std::string & name, int index, std::string & newName) const;
	bool isParamLoadedLocal(const std::string &name, bool deepCheck = true) const;
	bool isParamPureVirtualLocal(const std::string &name, bool deepCheck = true) const;

private:
	// no copying templates
	TpfTemplate(const TpfTemplate &source);
	TpfTemplate & operator =(const TpfTemplate &source);
};


inline TpfFile * TpfTemplate::getParentFile(void) const
{
	return m_parentFile;
}

inline void TpfTemplate::setParentFile(TpfFile * file)
{
	m_parentFile = file;
}

inline const std::string & TpfTemplate::getBaseTemplateName(void) const
{
	return m_baseTemplateName;
}	// TpfTemplate::getBaseTemplateName

inline ObjectTemplate * TpfTemplate::createTemplate(Tag id)
{
	return ObjectTemplateList::fetch(id);
}


#endif	// _INCLUDED_TpfTemplate_H
