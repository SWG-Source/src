//========================================================================
//
// SharedBattlefieldMarkerObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_SharedBattlefieldMarkerObjectTemplate_H
#define _INCLUDED_SharedBattlefieldMarkerObjectTemplate_H

#include "SharedTangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class SharedBattlefieldMarkerObjectTemplate : public SharedTangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		SharedBattlefieldMarkerObjectTemplate_tag = TAG(S,B,M,K)
	};
//@END TFD ID
public:
	         SharedBattlefieldMarkerObjectTemplate(const std::string & filename);
	virtual ~SharedBattlefieldMarkerObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);

//@BEGIN TFD
public:
	int                    getNumberOfPoles() const;
	int                    getNumberOfPolesMin() const;
	int                    getNumberOfPolesMax() const;
	float                  getRadius() const;
	float                  getRadiusMin() const;
	float                  getRadiusMax() const;


protected:
	virtual void load(Iff &file);

private:
	IntegerParam m_numberOfPoles;		// // number of child object poles
	FloatParam m_radius;		// // radius in meters
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	SharedBattlefieldMarkerObjectTemplate(const SharedBattlefieldMarkerObjectTemplate &);
	SharedBattlefieldMarkerObjectTemplate & operator =(const SharedBattlefieldMarkerObjectTemplate &);
};


inline void SharedBattlefieldMarkerObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	SharedBattlefieldMarkerObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_SharedBattlefieldMarkerObjectTemplate_H
