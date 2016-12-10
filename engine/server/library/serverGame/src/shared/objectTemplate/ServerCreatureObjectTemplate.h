//========================================================================
//
// ServerCreatureObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerCreatureObjectTemplate_H
#define _INCLUDED_ServerCreatureObjectTemplate_H

#include "ServerTangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"

class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
class ServerWeaponObjectTemplate;
//@END TFD TEMPLATE REFS


class ServerCreatureObjectTemplate : public ServerTangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerCreatureObjectTemplate_tag = TAG(C,R,E,O)
	};
//@END TFD ID
public:
	         ServerCreatureObjectTemplate(const std::string & filename);
	virtual ~ServerCreatureObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);
	void postLoad();

//@BEGIN TFD
public:
	enum PathNodeType
	{
		PN_Open,		// //grasslands
		PN_SparseCover,		// //light forests, vaporator farms
		PN_DenseCover,		// //dense forests etc.
		PN_NaturalInterior,		// //caves
		PN_ArtificialInterior,		// //buildings
		PN_NaturalPath,		// //paths and trails
		PN_ArtificialPath,		// //roads
		PN_PassableWater,		// //rivers, ponds, shorelines
		PN_ImpassableWater,		// //big lakes and oceans
		PathNodeType_Last = PN_ImpassableWater,
	};

public:
	const ServerWeaponObjectTemplate * getDefaultWeapon() const;
	int                    getAttributes(Attributes index) const;
	int                    getAttributesMin(Attributes index) const;
	int                    getAttributesMax(Attributes index) const;
	int                    getMinAttributes(Attributes index) const;
	int                    getMinAttributesMin(Attributes index) const;
	int                    getMinAttributesMax(Attributes index) const;
	int                    getMaxAttributes(Attributes index) const;
	int                    getMaxAttributesMin(Attributes index) const;
	int                    getMaxAttributesMax(Attributes index) const;
	float                  getMinDrainModifier(bool testData = false) const;
	float                  getMinDrainModifierMin(bool testData = false) const;
	float                  getMinDrainModifierMax(bool testData = false) const;
	float                  getMaxDrainModifier(bool testData = false) const;
	float                  getMaxDrainModifierMin(bool testData = false) const;
	float                  getMaxDrainModifierMax(bool testData = false) const;
	float                  getMinFaucetModifier(bool testData = false) const;
	float                  getMinFaucetModifierMin(bool testData = false) const;
	float                  getMinFaucetModifierMax(bool testData = false) const;
	float                  getMaxFaucetModifier(bool testData = false) const;
	float                  getMaxFaucetModifierMin(bool testData = false) const;
	float                  getMaxFaucetModifierMax(bool testData = false) const;
	void              getAttribMods(AttribMod &data, int index) const;
	void              getAttribModsMin(AttribMod &data, int index) const;
	void              getAttribModsMax(AttribMod &data, int index) const;
	size_t            getAttribModsCount(void) const;
	int                    getShockWounds(bool testData = false) const;
	int                    getShockWoundsMin(bool testData = false) const;
	int                    getShockWoundsMax(bool testData = false) const;
	bool                   getCanCreateAvatar(bool testData = false) const;
	const std::string &    getNameGeneratorType(bool testData = false) const;
	float                  getApproachTriggerRange(bool testData = false) const;
	float                  getApproachTriggerRangeMin(bool testData = false) const;
	float                  getApproachTriggerRangeMax(bool testData = false) const;
	float                  getMaxMentalStates(MentalStates index) const;
	float                  getMaxMentalStatesMin(MentalStates index) const;
	float                  getMaxMentalStatesMax(MentalStates index) const;
	float                  getMentalStatesDecay(MentalStates index) const;
	float                  getMentalStatesDecayMin(MentalStates index) const;
	float                  getMentalStatesDecayMax(MentalStates index) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	//Creature Attributes
	StringParam m_defaultWeapon;		// //weapon to use if none is equipped
	IntegerParam m_attributes[6];		// //initial value for attributes
	IntegerParam m_minAttributes[6];		// //minimum value for attributes
	IntegerParam m_maxAttributes[6];		// //maximum value for attributes
	FloatParam m_minDrainModifier;		// //min drain rate in units/sec
	FloatParam m_maxDrainModifier;		// //max drain rate in units/sec
	FloatParam m_minFaucetModifier;		// //min regeneration rate in units/sec
	FloatParam m_maxFaucetModifier;		// //max regeneration rate in units/sec
	std::vector<StructParamOT *> m_attribMods;		// //(de)buffs the creature is created with
	bool m_attribModsLoaded;
	bool m_attribModsAppend;
	IntegerParam m_shockWounds;		// //current shock wounds
	BoolParam m_canCreateAvatar;		// //can a player create an avatar with this template
	StringParam m_nameGeneratorType;		// //identifies which name generator to use
	// AI Behavioral Variables
	FloatParam m_approachTriggerRange;
	FloatParam m_maxMentalStates[4];		// //maximum value for the mental state
	FloatParam m_mentalStatesDecay[4];		// //time for the state to decay from 100 to 0
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
	ServerCreatureObjectTemplate(const ServerCreatureObjectTemplate &);
	ServerCreatureObjectTemplate & operator =(const ServerCreatureObjectTemplate &);
};


inline void ServerCreatureObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	ServerCreatureObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerCreatureObjectTemplate_H
