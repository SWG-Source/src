//========================================================================
//
// ServerCellObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerCellObjectTemplate_H
#define _INCLUDED_ServerCellObjectTemplate_H

#include "ServerObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class ServerCellObjectTemplate : public ServerObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerCellObjectTemplate_tag = TAG(S,C,L,T)
	};
//@END TFD ID
public:
	         ServerCellObjectTemplate(const std::string & filename);
	virtual ~ServerCellObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);

//@BEGIN TFD
public:

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
//@END TFD

public:
	// user functions
	virtual Object * createObject(void) const;

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerCellObjectTemplate(const ServerCellObjectTemplate &);
	ServerCellObjectTemplate & operator =(const ServerCellObjectTemplate &);
};


inline void ServerCellObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	ServerCellObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerCellObjectTemplate_H
