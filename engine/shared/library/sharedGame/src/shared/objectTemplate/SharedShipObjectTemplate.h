//========================================================================
//
// SharedShipObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_SharedShipObjectTemplate_H
#define _INCLUDED_SharedShipObjectTemplate_H

#include "SharedTangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class SharedShipObjectTemplate : public SharedTangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		SharedShipObjectTemplate_tag = TAG(S,S,H,P)
	};
//@END TFD ID
public:
	         SharedShipObjectTemplate(const std::string & filename);
	virtual ~SharedShipObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);
	virtual void createCustomizationDataPropertyAsNeeded(Object &object, bool forceCreation) const;

//@BEGIN TFD
public:
	const std::string &    getCockpitFilename(bool testData = false) const;
	bool                   getHasWings(bool testData = false) const;
	bool                   getPlayerControlled(bool testData = false) const;
	const std::string &    getInteriorLayoutFileName(bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	StringParam m_cockpitFilename;
	BoolParam m_hasWings;
	BoolParam m_playerControlled;
	StringParam m_interiorLayoutFileName;
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	SharedShipObjectTemplate(const SharedShipObjectTemplate &);
	SharedShipObjectTemplate & operator =(const SharedShipObjectTemplate &);
};


inline void SharedShipObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	SharedShipObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_SharedShipObjectTemplate_H
