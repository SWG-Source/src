//========================================================================
//
// ServerResourceContainerObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerResourceContainerObjectTemplate_H
#define _INCLUDED_ServerResourceContainerObjectTemplate_H

#include "ServerTangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class ServerResourceContainerObjectTemplate : public ServerTangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerResourceContainerObjectTemplate_tag = TAG(R,C,N,O)
	};
//@END TFD ID
public:
	         ServerResourceContainerObjectTemplate(const std::string & filename);
	virtual ~ServerResourceContainerObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);

//@BEGIN TFD
public:
	int                    getMaxResources(bool testData = false) const;
	int                    getMaxResourcesMin(bool testData = false) const;
	int                    getMaxResourcesMax(bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	IntegerParam m_maxResources;		// Maximum number of units of resource the container can hold.
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
	ServerResourceContainerObjectTemplate(const ServerResourceContainerObjectTemplate &);
	ServerResourceContainerObjectTemplate & operator =(const ServerResourceContainerObjectTemplate &);
};


inline void ServerResourceContainerObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	ServerResourceContainerObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerResourceContainerObjectTemplate_H
