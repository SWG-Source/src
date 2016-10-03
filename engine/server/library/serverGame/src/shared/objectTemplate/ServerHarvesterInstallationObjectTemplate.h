//========================================================================
//
// ServerHarvesterInstallationObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerHarvesterInstallationObjectTemplate_H
#define _INCLUDED_ServerHarvesterInstallationObjectTemplate_H

#include "ServerInstallationObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class ServerHarvesterInstallationObjectTemplate : public ServerInstallationObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerHarvesterInstallationObjectTemplate_tag = TAG(H,I,N,O)
	};
//@END TFD ID
public:
	         ServerHarvesterInstallationObjectTemplate(const std::string & filename);
	virtual ~ServerHarvesterInstallationObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);

//@BEGIN TFD
public:
	int                    getMaxExtractionRate(bool testData = false) const;
	int                    getMaxExtractionRateMin(bool testData = false) const;
	int                    getMaxExtractionRateMax(bool testData = false) const;
	int                    getCurrentExtractionRate(bool testData = false) const;
	int                    getCurrentExtractionRateMin(bool testData = false) const;
	int                    getCurrentExtractionRateMax(bool testData = false) const;
	int                    getMaxHopperSize(bool testData = false) const;
	int                    getMaxHopperSizeMin(bool testData = false) const;
	int                    getMaxHopperSizeMax(bool testData = false) const;
	const std::string &    getMasterClassName(bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	IntegerParam m_maxExtractionRate;		// max extraction rate
	IntegerParam m_currentExtractionRate;		// rate at which the machine could currently operate
	IntegerParam m_maxHopperSize;		// max extracted resource storage capacity of the installation
	StringParam m_masterClassName;		// highest level resource class that the harverster can extract
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
	ServerHarvesterInstallationObjectTemplate(const ServerHarvesterInstallationObjectTemplate &);
	ServerHarvesterInstallationObjectTemplate & operator =(const ServerHarvesterInstallationObjectTemplate &);
};


inline void ServerHarvesterInstallationObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	ServerHarvesterInstallationObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerHarvesterInstallationObjectTemplate_H
