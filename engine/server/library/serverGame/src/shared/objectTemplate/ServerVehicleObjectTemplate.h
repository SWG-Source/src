//========================================================================
//
// ServerVehicleObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerVehicleObjectTemplate_H
#define _INCLUDED_ServerVehicleObjectTemplate_H

#include "ServerTangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class ServerVehicleObjectTemplate : public ServerTangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerVehicleObjectTemplate_tag = TAG(V,E,H,O)
	};
//@END TFD ID
public:
	         ServerVehicleObjectTemplate(const std::string & filename);
	virtual ~ServerVehicleObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);

//@BEGIN TFD
public:
	const std::string &    getFuelType(bool testData = false) const;
	float                  getCurrentFuel(bool testData = false) const;
	float                  getCurrentFuelMin(bool testData = false) const;
	float                  getCurrentFuelMax(bool testData = false) const;
	float                  getMaxFuel(bool testData = false) const;
	float                  getMaxFuelMin(bool testData = false) const;
	float                  getMaxFuelMax(bool testData = false) const;
	float                  getConsumpsion(bool testData = false) const;
	float                  getConsumpsionMin(bool testData = false) const;
	float                  getConsumpsionMax(bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	StringParam m_fuelType;		// type of fuel used
	FloatParam m_currentFuel;		// current amount of fuel the vehicle has
	FloatParam m_maxFuel;		// max amount of fuel the vehicle can hold
	FloatParam m_consumpsion;		// units/sec/speed(?) fuel used
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
	ServerVehicleObjectTemplate(const ServerVehicleObjectTemplate &);
	ServerVehicleObjectTemplate & operator =(const ServerVehicleObjectTemplate &);
};


inline void ServerVehicleObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	ServerVehicleObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerVehicleObjectTemplate_H
