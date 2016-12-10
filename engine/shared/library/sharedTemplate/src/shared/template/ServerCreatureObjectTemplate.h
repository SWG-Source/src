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
#include "sharedTemplateDefinition/TpfTemplate.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;


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
	static void install(void);

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
	virtual CompilerIntegerParam *getCompilerIntegerParam(const char *name, bool deepCheck = true, int index = 0);
	virtual FloatParam *getFloatParam(const char *name, bool deepCheck = true, int index = 0);
	virtual BoolParam *getBoolParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringParam *getStringParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringIdParam *getStringIdParam(const char *name, bool deepCheck = true, int index = 0);
	virtual VectorParam *getVectorParam(const char *name, bool deepCheck = true, int index = 0);
	virtual DynamicVariableParam *getDynamicVariableParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StructParamOT *getStructParamOT(const char *name, bool deepCheck = true, int index = 0);
	virtual TriggerVolumeParam *getTriggerVolumeParam(const char *name, bool deepCheck = true, int index = 0);
	virtual void initStructParamOT(StructParamOT &param, const char *name);
	virtual void setAsEmptyList(const char *name);
	virtual void setAppend(const char *name);
	virtual bool isAppend(const char *name) const;
	virtual int getListLength(const char *name) const;

protected:
	virtual void load(Iff &file);
	virtual void save(Iff &file);

private:
	//Creature Attributes
	StringParam m_defaultWeapon;		// //weapon to use if none is equipped
	CompilerIntegerParam m_attributes[6];		// //initial value for attributes
	CompilerIntegerParam m_minAttributes[6];		// //minimum value for attributes
	CompilerIntegerParam m_maxAttributes[6];		// //maximum value for attributes
	FloatParam m_minDrainModifier;		// //min drain rate in units/sec
	FloatParam m_maxDrainModifier;		// //max drain rate in units/sec
	FloatParam m_minFaucetModifier;		// //min regeneration rate in units/sec
	FloatParam m_maxFaucetModifier;		// //max regeneration rate in units/sec
	std::vector<StructParamOT *> m_attribMods;		// //(de)buffs the creature is created with
	bool m_attribModsLoaded;
	bool m_attribModsAppend;
	CompilerIntegerParam m_shockWounds;		// //current shock wounds
	BoolParam m_canCreateAvatar;		// //can a player create an avatar with this template
	StringParam m_nameGeneratorType;		// //identifies which name generator to use
	// AI Behavioral Variables
	FloatParam m_approachTriggerRange;
	FloatParam m_maxMentalStates[4];		// //maximum value for the mental state
	FloatParam m_mentalStatesDecay[4];		// //time for the state to decay from 100 to 0
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerCreatureObjectTemplate(const ServerCreatureObjectTemplate &);
	ServerCreatureObjectTemplate & operator =(const ServerCreatureObjectTemplate &);
};


inline void ServerCreatureObjectTemplate::install(void)
{
//@BEGIN TFD INSTALL
	ServerCreatureObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerCreatureObjectTemplate_H
