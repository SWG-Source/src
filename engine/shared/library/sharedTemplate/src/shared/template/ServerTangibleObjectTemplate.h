//========================================================================
//
// ServerTangibleObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerTangibleObjectTemplate_H
#define _INCLUDED_ServerTangibleObjectTemplate_H

#include "ServerObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedTemplateDefinition/TpfTemplate.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;


class ServerTangibleObjectTemplate : public ServerObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerTangibleObjectTemplate_tag = TAG(T,A,N,O)
	};
//@END TFD ID
public:
	         ServerTangibleObjectTemplate(const std::string & filename);
	virtual ~ServerTangibleObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(void);

//@BEGIN TFD
public:
	enum CombatSkeleton
	{
		CS_none,		// // all "body" or not attackable
		CS_humanoid,		// // head, body, 2 arms, 2 legs, standard human proportions/layout
		CombatSkeleton_Last = CS_humanoid,
	};

	enum Conditions
	{
		C_onOff = 0x00000001,
		C_vendor = 0x00000002,
		C_insured = 0x00000004,
		C_conversable = 0x00000008,
		C_hibernating = 0x00000010,
		C_magicItem = 0x00000020,
		C_aggressive = 0x00000040,
		C_wantSawAttackTrigger = 0x00000080,
		C_invulnerable = 0x00000100,
		C_disabled = 0x00000200,
		C_uninsurable = 0x00000400,
		C_interesting = 0x00000800,
		C_mount = 0x00001000,		//   Set programmatically by mount system.  Do not set this in the template.
		C_crafted = 0x00002000,		//   Set programmatically by crafting system.  Do not set this in the template.
		C_wingsOpened = 0x00004000,		//   Set programmatically by wing system.  Do not set this in the template.
		C_spaceInteresting = 0x00008000,
		C_docking = 0x00010000,		//   Set programmatically by docking system.  Do not set this in the template.
		C_destroying = 0x00020000,		//   Set programmatically by destruction system.  Do not set this in the template.
		C_commable = 0x00040000,
		C_dockable = 0x00080000,
		C_eject = 0x00100000,
		C_inspectable = 0x00200000,
		C_transferable = 0x00400000,
		C_inflightTutorial = 0x00800000,
		C_spaceCombatMusic = 0x01000000,		//   Set programmatically by the AI system.  Do not set this in the template.
		C_encounterLocked = 0x02000000,
		C_spawnedCreature = 0x04000000,
		C_holidayInteresting = 0x08000000,
		C_locked = 0x10000000,
		Conditions_Last = C_locked,
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
	// these MUST be reflected in:
	// //depot/swg/current/dsrc/sku.0/sys.server/compiled/game/object/tangible_object_template.tdf
	// //depot/swg/current/dsrc/sku.0/sys.server/compiled/game/script/base_class.java
	// //depot/swg/current/src/engine/client/library/clientGame/src/shared/object/TangibleObject.h
	// //depot/swg/current/src/engine/server/library/serverGame/src/shared/object/TangibleObject.h
	std::vector<TriggerVolumeParam *> m_triggerVolumes;		// trigger volume(s) attached to the object
	bool m_triggerVolumesLoaded;
	bool m_triggerVolumesAppend;
	CompilerIntegerParam m_combatSkeleton;		// this should be fixed (not random) for any template type
	CompilerIntegerParam m_maxHitPoints;		// hp for non-creature objects
	StringParam m_armor;		// what kind of armor this object has (if any)
	CompilerIntegerParam m_interestRadius;		// area of interest of the object
	CompilerIntegerParam m_count;		// generic counter
	CompilerIntegerParam m_condition;		// object condition
	BoolParam m_wantSawAttackTriggers;		// whether we're interested in OnSawAttack triggers
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerTangibleObjectTemplate(const ServerTangibleObjectTemplate &);
	ServerTangibleObjectTemplate & operator =(const ServerTangibleObjectTemplate &);
};


inline void ServerTangibleObjectTemplate::install(void)
{
//@BEGIN TFD INSTALL
	ServerTangibleObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerTangibleObjectTemplate_H
