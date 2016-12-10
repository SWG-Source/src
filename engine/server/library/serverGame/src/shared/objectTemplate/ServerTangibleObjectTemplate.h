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
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
class ServerArmorTemplate;
//@END TFD TEMPLATE REFS


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
	static void install(bool allowDefaultTemplateParams = true);

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
	const TriggerVolumeData getTriggerVolumes(int index) const;
	size_t            getTriggerVolumesCount(void) const;
	CombatSkeleton     getCombatSkeleton(bool testData = false) const;
	int                    getMaxHitPoints(bool testData = false) const;
	int                    getMaxHitPointsMin(bool testData = false) const;
	int                    getMaxHitPointsMax(bool testData = false) const;
	const ServerArmorTemplate * getArmor() const;
	int                    getInterestRadius(bool testData = false) const;
	int                    getInterestRadiusMin(bool testData = false) const;
	int                    getInterestRadiusMax(bool testData = false) const;
	int                    getCount(bool testData = false) const;
	int                    getCountMin(bool testData = false) const;
	int                    getCountMax(bool testData = false) const;
	int                    getCondition(bool testData = false) const;
	int                    getConditionMin(bool testData = false) const;
	int                    getConditionMax(bool testData = false) const;
	bool                   getWantSawAttackTriggers(bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	// these MUST be reflected in:
	// //depot/swg/current/dsrc/sku.0/sys.server/compiled/game/object/tangible_object_template.tdf
	// //depot/swg/current/dsrc/sku.0/sys.server/compiled/game/script/base_class.java
	// //depot/swg/current/src/engine/client/library/clientGame/src/shared/object/TangibleObject.h
	// //depot/swg/current/src/engine/server/library/serverGame/src/shared/object/TangibleObject.h
	std::vector<TriggerVolumeParam *> m_triggerVolumes;		// trigger volume(s) attached to the object
	bool m_triggerVolumesLoaded;
	bool m_triggerVolumesAppend;
	IntegerParam m_combatSkeleton;		// this should be fixed (not random) for any template type
	IntegerParam m_maxHitPoints;		// hp for non-creature objects
	StringParam m_armor;		// what kind of armor this object has (if any)
	IntegerParam m_interestRadius;		// area of interest of the object
	IntegerParam m_count;		// generic counter
	IntegerParam m_condition;		// object condition
	BoolParam m_wantSawAttackTriggers;		// whether we're interested in OnSawAttack triggers
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
	ServerTangibleObjectTemplate(const ServerTangibleObjectTemplate &);
	ServerTangibleObjectTemplate & operator =(const ServerTangibleObjectTemplate &);
};


inline void ServerTangibleObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	ServerTangibleObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerTangibleObjectTemplate_H
