//========================================================================
//
// ServerMissionListEntryObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerMissionListEntryObjectTemplate_H
#define _INCLUDED_ServerMissionListEntryObjectTemplate_H

#include "ServerIntangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class ServerMissionListEntryObjectTemplate : public ServerIntangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerMissionListEntryObjectTemplate_tag = TAG(M,L,E,O)
	};
//@END TFD ID
public:
	         ServerMissionListEntryObjectTemplate(const std::string & filename);
	virtual ~ServerMissionListEntryObjectTemplate();

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

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerMissionListEntryObjectTemplate(const ServerMissionListEntryObjectTemplate &);
	ServerMissionListEntryObjectTemplate & operator =(const ServerMissionListEntryObjectTemplate &);
};


inline void ServerMissionListEntryObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	ServerMissionListEntryObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerMissionListEntryObjectTemplate_H
