//========================================================================
//
// ServerBuildingObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerBuildingObjectTemplate_H
#define _INCLUDED_ServerBuildingObjectTemplate_H

#include "ServerTangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class ServerBuildingObjectTemplate : public ServerTangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerBuildingObjectTemplate_tag = TAG(B,U,I,O)
	};
//@END TFD ID
public:
	         ServerBuildingObjectTemplate(const std::string & filename);
	virtual ~ServerBuildingObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);

//@BEGIN TFD
public:
	int                    getMaintenanceCost(bool testData = false) const;
	int                    getMaintenanceCostMin(bool testData = false) const;
	int                    getMaintenanceCostMax(bool testData = false) const;
	bool                   getIsPublic(bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	IntegerParam m_maintenanceCost;		// The weekly cost (in credits) of maintaining this Building.
	BoolParam m_isPublic;		// Whether by default the building is flagged public.
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
	ServerBuildingObjectTemplate(const ServerBuildingObjectTemplate &);
	ServerBuildingObjectTemplate & operator =(const ServerBuildingObjectTemplate &);
};


inline void ServerBuildingObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	ServerBuildingObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerBuildingObjectTemplate_H
