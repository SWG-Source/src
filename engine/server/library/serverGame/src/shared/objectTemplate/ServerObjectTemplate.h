//========================================================================
//
// ServerObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerObjectTemplate_H
#define _INCLUDED_ServerObjectTemplate_H

#include "sharedObject/ObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
class ServerObjectTemplate;
//@END TFD TEMPLATE REFS


class ServerObjectTemplate : public ObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerObjectTemplate_tag = TAG(S,W,O,O)
	};
//@END TFD ID

public:
	         ServerObjectTemplate(const std::string & filename);
	virtual ~ServerObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);
	static void exit();

//@BEGIN TFD
public:
	enum ArmorCategory
	{
		AC_reconnaissance,
		AC_battle,
		AC_assault,
		AC_psg,		// // personal shield generator
		ArmorCategory_Last = AC_psg,
	};

	enum ArmorLevel
	{
		AL_basic,
		AL_standard,
		AL_advanced,
		ArmorLevel_Last = AL_advanced,
	};

	enum ArmorRating
	{
		AR_armorNone,
		AR_armorLight,
		AR_armorMedium,
		AR_armorHeavy,
		ArmorRating_Last = AR_armorHeavy,
	};

	enum Attributes
	{
		AT_health,
		AT_constitution,
		AT_action,
		AT_stamina,
		AT_mind,
		AT_willpower,
		Attributes_Last = AT_willpower,
	};

	enum CraftingType
	{
		CT_weapon = 0x00000001,
		CT_armor = 0x00000002,
		CT_food = 0x00000004,
		CT_clothing = 0x00000008,
		CT_vehicle = 0x00000010,
		CT_droid = 0x00000020,
		CT_chemical = 0x00000040,
		CT_plantBreeding = 0x00000080,
		CT_animalBreeding = 0x00000100,
		CT_furniture = 0x00000200,
		CT_installation = 0x00000400,
		CT_lightsaber = 0x00000800,
		CT_genericItem = 0x00001000,
		CT_genetics = 0x00002000,
		CT_mandalorianTailor = 0x00004000,
		CT_mandalorianArmorsmith = 0x00008000,
		CT_mandalorianDroidEngineer = 0x00010000,
		CT_space = 0x00020000,
		CT_reverseEngineering = 0x00040000,
		CT_misc = 0x00080000,
		CT_spaceComponent = 0x00100000,
		CT_mission = 0x80000000,
		CT_weaponMission = 0x80000001,
		CT_armorMission = 0x80000002,
		CT_foodMission = 0x80000004,
		CT_clothingMission = 0x80000008,
		CT_vehicleMission = 0x80000010,
		CT_droidMission = 0x80000020,
		CT_chemicalMission = 0x80000040,
		CT_plantBreedingMission = 0x80000080,
		CT_animalBreedingMission = 0x80000100,
		CT_furnitureMission = 0x80000200,
		CT_installationMission = 0x80000400,
		CT_lightsaberMission = 0x80000800,
		CT_genericItemMission = 0x80001000,
		CT_geneticsMission = 0x80002000,
		CT_mandalorianTailorMission = 0x80004000,
		CT_mandalorianArmorsmithMission = 0x80008000,
		CT_mandalorianDroidEngineerMission = 0x80010000,
		CT_spaceMission = 0x80020000,
		CT_reverseEngineeringMission = 0x80040000,
		CT_miscMission = 0x80080000,
		CT_spaceComponentMission = 0x80100000,
		CraftingType_Last = CT_spaceComponentMission,
	};

	enum DamageType
	{
		DT_none = 0x00000000,
		DT_kinetic = 0x00000001,
		DT_energy = 0x00000002,
		DT_blast = 0x00000004,
		DT_stun = 0x00000008,
		DT_restraint = 0x00000010,
		DT_elemental_heat = 0x00000020,
		DT_elemental_cold = 0x00000040,
		DT_elemental_acid = 0x00000080,
		DT_elemental_electrical = 0x00000100,
		DT_environmental_heat = 0x00000200,
		DT_environmental_cold = 0x00000400,
		DT_environmental_acid = 0x00000800,
		DT_environmental_electrical = 0x00001000,
		DamageType_Last = DT_environmental_electrical,
	};

	enum DeleteFlags
	{
		DF_gm,
		DF_player,
		DeleteFlags_Last = DF_player,
	};

	enum MentalStates
	{
		MS_fear,
		MS_anger,
		MS_interest,
		MS_distress,
		MentalStates_Last = MS_distress,
	};

	enum MoveFlags
	{
		MF_gm,
		MF_player,
		MoveFlags_Last = MF_player,
	};

	enum UpdateRanges
	{
		UR_near,
		UR_normal,
		UR_far,
		UpdateRanges_Last = UR_far,
	};

	enum VisibleFlags
	{
		VF_gm,
		VF_player,
		VisibleFlags_Last = VF_player,
	};

	enum XpTypes
	{
		XP_rangedCombat,
		XP_meleeCombat,
		XP_tamingNoTime,
		XP_tamingShortTime,
		XP_tamingMediumTime,
		XP_tamingLargeTime,
		XP_architect,
		XP_botany,
		XP_bountyHunter,
		XP_chemist,
		XP_dancing,
		XP_facilityManagementSavings,
		XP_facilityManagementWearAndTear,
		XP_squadLeaderNoTime,
		XP_squadLeaderSmallTime,
		XP_squadLeaderMediumTime,
		XP_squadLeaderLargeTime,
		XP_hairdressing,
		XP_holonetSlicingPopularity,
		XP_holonetSlicingQuantity,
		XP_investigation,
		XP_medic,
		XP_military,
		XP_mining,
		XP_miningSavings,
		XP_musician,
		XP_musicanPopularity,
		XP_politics,
		XP_slicing,
		XP_smuggling,
		XP_surveyingNoMinimum,
		XP_surveyingLowMinimum,
		XP_surveyingMediumMinimum,
		XP_surveyingHighMinimum,
		XP_survivalHealing,
		XP_survivalTracking,
		XP_vehicularControl,
		XP_vehicularWearAndTear,
		XP_resourceHarvestingOrganic,
		XP_resourceHarvestingInorganic,
		XP_crafting,
		XP_craftingClothing,
		XP_craftingClothingArmor,
		XP_craftingDroid,
		XP_craftingFood,
		XP_craftingMedicine,
		XP_craftingStructure,
		XP_craftingWeapons,
		XP_craftingWeaponsMelee,
		XP_craftingWeaponsRanged,
		XP_craftingWeaponsMunition,
		XP_industrialist,
		XP_craftingScout,
		XP_craftingCreature,
		XP_craftingTissue,
		XP_craftingCamp,
		XP_craftingTrapping,
		XP_craftingSpice,
		XP_jediGeneral,
		XP_shipwright,
		XP_reverseEngineering,
		XpTypes_Last = XP_reverseEngineering,
	};

public:
	struct AttribMod
	{
		enum Attributes target;
		int                    value;
		float                  time;
		float                  timeAtValue;
		float                  decay;
	};

	struct MentalStateMod
	{
		enum MentalStates target;
		float                  value;
		float                  time;
		float                  timeAtValue;
		float                  decay;
	};

	struct Contents
	{
		std::string            slotName;
		bool                   equipObject;
		const ObjectTemplate * content;

		 Contents(void);
		 Contents(const Contents & source);
		~Contents();
#ifdef WIN32
		Contents & operator =(const Contents &);
#endif
#ifdef LINUX

	private:
		Contents & operator =(const Contents &);
#endif 
	};

	struct Xp
	{
		enum XpTypes type;
		int                    level;
		int                    value;
	};

protected:
	class _AttribMod : public ObjectTemplate
	{
		friend class ServerObjectTemplate;
	public:
		enum
		{
			_AttribMod_tag = TAG(A,T,T,S)
		};

	public:
		         _AttribMod(const std::string & filename);
		virtual ~_AttribMod();

	virtual Tag getId(void) const;

public:
	Attributes     getTarget(bool versionOk, bool testData = false) const;
	int                    getValue(bool versionOk, bool testData = false) const;
	int                    getValueMin(bool versionOk, bool testData = false) const;
	int                    getValueMax(bool versionOk, bool testData = false) const;
	float                  getTime(bool versionOk, bool testData = false) const;
	float                  getTimeMin(bool versionOk, bool testData = false) const;
	float                  getTimeMax(bool versionOk, bool testData = false) const;
	float                  getTimeAtValue(bool versionOk, bool testData = false) const;
	float                  getTimeAtValueMin(bool versionOk, bool testData = false) const;
	float                  getTimeAtValueMax(bool versionOk, bool testData = false) const;
	float                  getDecay(bool versionOk, bool testData = false) const;
	float                  getDecayMin(bool versionOk, bool testData = false) const;
	float                  getDecayMax(bool versionOk, bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	IntegerParam m_target;		// server        //which attribute is affected
	IntegerParam m_value;		// server        //max strength of the modifier
	FloatParam m_time;		// server        //how long it takes for the modifier to reach full strength (in secs)
	FloatParam m_timeAtValue;		// server        //how long the modifier remains at full strength (in secs)
	FloatParam m_decay;		// server        //how long it takes for the modifier to go from full strength to 0 (in secs)
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_AttribMod(const _AttribMod &);
		_AttribMod & operator =(const _AttribMod &);
	};
	friend class ServerObjectTemplate::_AttribMod;

	class _MentalStateMod : public ObjectTemplate
	{
		friend class ServerObjectTemplate;
	public:
		enum
		{
			_MentalStateMod_tag = TAG(M,E,S,M)
		};

	public:
		         _MentalStateMod(const std::string & filename);
		virtual ~_MentalStateMod();

	virtual Tag getId(void) const;

public:
	MentalStates     getTarget(bool versionOk, bool testData = false) const;
	float                  getValue(bool versionOk, bool testData = false) const;
	float                  getValueMin(bool versionOk, bool testData = false) const;
	float                  getValueMax(bool versionOk, bool testData = false) const;
	float                  getTime(bool versionOk, bool testData = false) const;
	float                  getTimeMin(bool versionOk, bool testData = false) const;
	float                  getTimeMax(bool versionOk, bool testData = false) const;
	float                  getTimeAtValue(bool versionOk, bool testData = false) const;
	float                  getTimeAtValueMin(bool versionOk, bool testData = false) const;
	float                  getTimeAtValueMax(bool versionOk, bool testData = false) const;
	float                  getDecay(bool versionOk, bool testData = false) const;
	float                  getDecayMin(bool versionOk, bool testData = false) const;
	float                  getDecayMax(bool versionOk, bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	IntegerParam m_target;		// server       //which mental state is affected
	FloatParam m_value;		// server       //max strength of the modifier
	FloatParam m_time;		// server       //how long it takes for the modifier to reach full strength (in secs)
	FloatParam m_timeAtValue;		// server       //how long the modifier remains at full strength (in secs)
	FloatParam m_decay;		// server       //how long it takes for the modifier to go from full strength to 0 (in secs)
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_MentalStateMod(const _MentalStateMod &);
		_MentalStateMod & operator =(const _MentalStateMod &);
	};
	friend class ServerObjectTemplate::_MentalStateMod;

	class _Contents : public ObjectTemplate
	{
		friend class ServerObjectTemplate;
	public:
		enum
		{
			_Contents_tag = TAG(C,O,N,T)
		};

	public:
		         _Contents(const std::string & filename);
		virtual ~_Contents();

	virtual Tag getId(void) const;

public:
	const std::string &    getSlotName(bool versionOk, bool testData = false) const;
	bool                   getEquipObject(bool versionOk, bool testData = false) const;
	const ServerObjectTemplate * getContent(bool versionOk) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	StringParam m_slotName;		// //slot name, as defined in the object's slotDescriptorFilename
	BoolParam m_equipObject;		// //flag to put the contents in the slot, as opposed to adding it to the container in the slot
	StringParam m_content;		// //the object to add
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_Contents(const _Contents &);
		_Contents & operator =(const _Contents &);
	};
	friend class ServerObjectTemplate::_Contents;

	class _Xp : public ObjectTemplate
	{
		friend class ServerObjectTemplate;
	public:
		enum
		{
			_Xp_tag = TAG(E,X,P,S)
		};

	public:
		         _Xp(const std::string & filename);
		virtual ~_Xp();

	virtual Tag getId(void) const;

public:
	XpTypes     getType(bool versionOk, bool testData = false) const;
	int                    getLevel(bool versionOk, bool testData = false) const;
	int                    getLevelMin(bool versionOk, bool testData = false) const;
	int                    getLevelMax(bool versionOk, bool testData = false) const;
	int                    getValue(bool versionOk, bool testData = false) const;
	int                    getValueMin(bool versionOk, bool testData = false) const;
	int                    getValueMax(bool versionOk, bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	IntegerParam m_type;		// //type of experience earned
	IntegerParam m_level;		// //difficulty level, 0 for leveless xp types
	IntegerParam m_value;		// //amount of experience earned
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_Xp(const _Xp &);
		_Xp & operator =(const _Xp &);
	};
	friend class ServerObjectTemplate::_Xp;

public:
	const std::string &    getSharedTemplate(bool testData = false) const;
	const std::string &    getScripts(int index) const;
	size_t            getScriptsCount(void) const;
	void                   getObjvars(DynamicVariableList &list) const;
	int                    getVolume(bool testData = false) const;
	int                    getVolumeMin(bool testData = false) const;
	int                    getVolumeMax(bool testData = false) const;
	VisibleFlags     getVisibleFlags(int index) const;
	size_t            getVisibleFlagsCount(void) const;
	DeleteFlags     getDeleteFlags(int index) const;
	size_t            getDeleteFlagsCount(void) const;
	MoveFlags     getMoveFlags(int index) const;
	size_t            getMoveFlagsCount(void) const;
	bool                   getInvulnerable(bool testData = false) const;
	float                  getComplexity(bool testData = false) const;
	float                  getComplexityMin(bool testData = false) const;
	float                  getComplexityMax(bool testData = false) const;
	int                    getTintIndex(bool testData = false) const;
	int                    getTintIndexMin(bool testData = false) const;
	int                    getTintIndexMax(bool testData = false) const;
	float                  getUpdateRanges(UpdateRanges index) const;
	float                  getUpdateRangesMin(UpdateRanges index) const;
	float                  getUpdateRangesMax(UpdateRanges index) const;
	void              getContents(Contents &data, int index) const;
	void              getContentsMin(Contents &data, int index) const;
	void              getContentsMax(Contents &data, int index) const;
	size_t            getContentsCount(void) const;
	void              getXpPoints(Xp &data, int index) const;
	void              getXpPointsMin(Xp &data, int index) const;
	void              getXpPointsMax(Xp &data, int index) const;
	size_t            getXpPointsCount(void) const;
	bool                   getPersistByDefault(bool testData = false) const;
	bool                   getPersistContents(bool testData = false) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:
	virtual void load(Iff &file);

private:
	// crafting types also defined in base_class.java
	// NOTE: when you update this list, make sure to update the registerMe() function in ServerObjectTemplate.cpp
	// this enum is also defined in the shared draft_schematic_object_template.tdf file
	// this enum is also defined in the shared draft_schematic_object_template.tdf file
	StringParam m_sharedTemplate;		// //template the client should associate with this template
	std::vector<StringParam *> m_scripts;		// //scripts attached to the object
	bool m_scriptsLoaded;
	bool m_scriptsAppend;
	DynamicVariableParam m_objvars;		// //default objvars added to the object
	IntegerParam m_volume;		// //how much room does the object take up
	std::vector<IntegerParam *> m_visibleFlags;		// //who can see this object
	bool m_visibleFlagsLoaded;
	bool m_visibleFlagsAppend;
	std::vector<IntegerParam *> m_deleteFlags;		// //who can delete this object
	bool m_deleteFlagsLoaded;
	bool m_deleteFlagsAppend;
	std::vector<IntegerParam *> m_moveFlags;		// //who can move this object
	bool m_moveFlagsLoaded;
	bool m_moveFlagsAppend;
	BoolParam m_invulnerable;		// //can this object not be destroyed/killed
	FloatParam m_complexity;		// //how hard this item is to craft (?)
	IntegerParam m_tintIndex;		// //index into client tint palette
	FloatParam m_updateRanges[3];		// //distances for network updates
	std::vector<StructParamOT *> m_contents;		// //what's in this object
	bool m_contentsLoaded;
	bool m_contentsAppend;
	std::vector<StructParamOT *> m_xpPoints;		// //experience points for interacting with this object
	bool m_xpPointsLoaded;
	bool m_xpPointsAppend;
	BoolParam m_persistByDefault;		// //flag to automatically persist this object on creation
	BoolParam m_persistContents;		// //flag that this container persists its contents
//@END TFD

public:
	
	// user functions
	virtual ServerObjectTemplate *       asServerObjectTemplate();
	virtual ServerObjectTemplate const * asServerObjectTemplate() const;

	static const std::string & getXpString          (XpTypes type);
	static const std::string & getDamageTypeString  (DamageType type);
	static const std::string & getArmorRatingString (ArmorRating type);

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerObjectTemplate(const ServerObjectTemplate &);
	ServerObjectTemplate & operator =(const ServerObjectTemplate &);
};


inline void ServerObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	ServerObjectTemplate::registerMe();
	ServerObjectTemplate::_AttribMod::registerMe();
	ServerObjectTemplate::_Contents::registerMe();
	ServerObjectTemplate::_MentalStateMod::registerMe();
	ServerObjectTemplate::_Xp::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerObjectTemplate_H
