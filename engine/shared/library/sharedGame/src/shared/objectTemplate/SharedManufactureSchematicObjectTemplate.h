//========================================================================
//
// SharedManufactureSchematicObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_SharedManufactureSchematicObjectTemplate_H
#define _INCLUDED_SharedManufactureSchematicObjectTemplate_H

#include "SharedIntangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class SharedManufactureSchematicObjectTemplate : public SharedIntangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		SharedManufactureSchematicObjectTemplate_tag = TAG(S,M,S,C)
	};
//@END TFD ID
public:
	         SharedManufactureSchematicObjectTemplate(const std::string & filename);
	virtual ~SharedManufactureSchematicObjectTemplate();

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
	SharedManufactureSchematicObjectTemplate(const SharedManufactureSchematicObjectTemplate &);
	SharedManufactureSchematicObjectTemplate & operator =(const SharedManufactureSchematicObjectTemplate &);
};


inline void SharedManufactureSchematicObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	SharedManufactureSchematicObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_SharedManufactureSchematicObjectTemplate_H
