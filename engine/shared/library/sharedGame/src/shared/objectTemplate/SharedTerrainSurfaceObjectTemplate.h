//========================================================================
//
// SharedTerrainSurfaceObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_SharedTerrainSurfaceObjectTemplate_H
#define _INCLUDED_SharedTerrainSurfaceObjectTemplate_H

#include "sharedObject/ObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class SharedTerrainSurfaceObjectTemplate : public ObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		SharedTerrainSurfaceObjectTemplate_tag = TAG(S,T,E,R)
	};
//@END TFD ID
public:
	         SharedTerrainSurfaceObjectTemplate(const std::string & filename);
	virtual ~SharedTerrainSurfaceObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);

//@BEGIN TFD
public:
	float                  getCover(bool testData = false) const;
	float                  getCoverMin(bool testData = false) const;
	float                  getCoverMax(bool testData = false) const;
	const std::string &    getSurfaceType(bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	FloatParam m_cover;
	StringParam m_surfaceType;
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	SharedTerrainSurfaceObjectTemplate(const SharedTerrainSurfaceObjectTemplate &);
	SharedTerrainSurfaceObjectTemplate & operator =(const SharedTerrainSurfaceObjectTemplate &);
};


inline void SharedTerrainSurfaceObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	SharedTerrainSurfaceObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_SharedTerrainSurfaceObjectTemplate_H
