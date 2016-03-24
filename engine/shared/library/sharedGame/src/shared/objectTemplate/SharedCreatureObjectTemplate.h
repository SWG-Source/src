//========================================================================
//
// SharedCreatureObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_SharedCreatureObjectTemplate_H
#define _INCLUDED_SharedCreatureObjectTemplate_H

#include "SharedTangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
class MovementTable;

typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class SharedCreatureObjectTemplate : public SharedTangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		SharedCreatureObjectTemplate_tag = TAG(S,C,O,T)
	};
//@END TFD ID
public:
	         SharedCreatureObjectTemplate(const std::string & filename);
	virtual ~SharedCreatureObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);

	void postLoad();
	virtual void createCustomizationDataPropertyAsNeeded(Object &object, bool forceCreation = false) const;

	const MovementTable * getMovementTable() const;

//@BEGIN TFD
public:
	enum Gender
	{
		GE_male,
		GE_female,
		GE_other,
		Gender_Last = GE_other,
	};

	enum MovementTypes
	{
		MT_run,
		MT_walk,
		MovementTypes_Last = MT_walk,
	};

	enum Niche
	{
		NI_none = 0,
		NI_pc = 1,
		NI_ai = 2,
		NI_droid = 3,
		NI_vehicle = 4,
		NI_npc = 5,
		NI_monster = 6,
		NI_herbivore = 7,
		NI_carnivore = 8,
		NI_predator = 9,
		NI_android = 10,
		Niche_Last = NI_android,
	};

	enum Postures
	{
		Upright = 0,
		Crouched = 1,
		Prone = 2,
		Sneaking = 3,
		Blocking = 4,
		Climbing = 5,
		Flying = 6,
		LyingDown = 7,
		Sitting = 8,
		SkillAnimating = 9,
		DrivingVehicle = 10,
		RidingCreature = 11,
		KnockedDown = 12,
		Incapacitated = 13,
		Dead = 14,
		Postures_Last = Dead,
	};

	enum Race
	{
		RA_none,
		RA_aqualish_quara = 1,
		RA_aqualish_aquala = 2,
		RA_ewok_ewok = 1,		//       // plae, green, black, white (subject to change, pending information request to LucasArts)
		RA_ewok_dulok = 2,		//      // blue skin; "Orn Free Taa", "Ann Gella" & "Tann", Sebulba's personal entourage.
		RA_ewok_jinda = 3,		//      // red skin; "Pampy" & "Supi", consorts of Orn Free Taa.
		RA_gungan_ankura = 1,		//   // Boss Nass
		RA_gungan_otolla = 2,		//   // Jar Jar
		RA_nikto_red = 1,		//       // Kajain'sa'Nikto
		RA_nikto_green = 2,		//     // Kadas'sa'Nikto
		RA_nikto_mountain = 3,		//  // Esral'sa'Nikto
		RA_nikto_pale = 4,		//      // Gluss'sa'Nikto
		RA_nikto_southern = 5,		//  // M'shento'su'Nikto
		RA_krayt_canyon = 1,
		RA_krayt_greater = 2,
		RA_R2 = 0,
		RA_R3 = 1,
		RA_R4 = 2,
		RA_R5 = 3,
		Race_Last = RA_R5,
	};

	enum Species
	{
		SP_human = 0,
		SP_rodian = 1,
		SP_trandoshan = 2,
		SP_monCalamari = 3,
		SP_wookiee = 4,
		SP_bothan = 5,
		SP_twilek = 6,
		SP_zabrak = 7,
		SP_abyssin = 8,
		SP_aqualish = 9,
		SP_arcona = 10,
		SP_askajian = 11,
		SP_bith = 12,
		SP_bomarrMonk = 13,
		SP_chadraFan = 14,
		SP_chevin = 15,
		SP_dantari = 16,
		SP_devaronian = 17,
		SP_drall = 18,
		SP_dug = 19,
		SP_duros = 20,
		SP_elomin = 21,
		SP_ewok = 22,
		SP_feeorin = 23,
		SP_frogDog = 24,
		SP_gamorrean = 25,
		SP_gorax = 26,
		SP_gotal = 27,
		SP_gran = 28,
		SP_gungan = 29,
		SP_gupin = 30,
		SP_hutt = 31,
		SP_ishiTib = 32,
		SP_ithorian = 33,
		SP_jawa = 34,
		SP_kiffu = 35,
		SP_kitonak = 36,
		SP_klatooinian = 37,
		SP_kowakianMonkeyLizard = 38,
		SP_kubaz = 39,
		SP_marauder = 40,
		SP_massassiWarrior = 41,
		SP_nikto = 42,
		SP_ortolan = 43,
		SP_palowick = 44,
		SP_phlog = 45,
		SP_quarren = 46,
		SP_selonian = 47,
		SP_shistavanen = 48,
		SP_sullustan = 49,
		SP_talz = 50,
		SP_teek = 51,
		SP_tulgah = 52,
		SP_toydarian = 53,
		SP_tuskenRaider = 54,
		SP_weequay = 55,
		SP_whiffid = 56,
		SP_wistie = 57,
		SP_yuzzum = 58,
		SP_angler = 60,
		SP_bageraset = 61,
		SP_bantha = 62,
		SP_barkMite = 63,
		SP_bazNitch = 64,
		SP_beardedJax = 65,
		SP_blackfish = 66,
		SP_blistmok = 67,
		SP_bluefish = 68,
		SP_blurrg = 69,
		SP_boarWolf = 70,
		SP_bocatt = 71,
		SP_bol = 72,
		SP_bolleBol = 73,
		SP_bolma = 74,
		SP_bordok = 75,
		SP_borgle = 76,
		SP_brackaset = 77,
		SP_capperSpineflap = 78,
		SP_carrionSpat = 79,
		SP_choku = 80,
		SP_chuba = 81,
		SP_coloClawFish = 82,
		SP_condorDragon = 83,
		SP_corellianSandPanther = 84,
		SP_corellianSliceHound = 85,
		SP_crownedRasp = 86,
		SP_crystalSnake = 87,
		SP_cuPa = 88,
		SP_dalyrake = 89,
		SP_dewback = 90,
		SP_duneLizard = 91,
		SP_durni = 92,
		SP_dwarfNuna = 93,
		SP_eopie = 94,
		SP_faa = 95,
		SP_falumpaset = 96,
		SP_fambaa = 97,
		SP_fannedRawl = 98,
		SP_flewt = 99,
		SP_flit = 100,
		SP_fliteRasp = 101,
		SP_fynock = 102,
		SP_gackleBat = 103,
		SP_gapingSpider = 104,
		SP_gekk = 105,
		SP_gnort = 106,
		SP_graul = 107,
		SP_greatGrassPlainsTuskCat = 108,
		SP_gronda = 109,
		SP_gualama = 110,
		SP_gubbur = 111,
		SP_gufDrolg = 112,
		SP_gulginaw = 113,
		SP_gurk = 114,
		SP_gurnaset = 115,
		SP_gurrek = 116,
		SP_hanadak = 117,
		SP_hermitSpider = 118,
		SP_hornedKrevol = 119,
		SP_hornedRasp = 120,
		SP_hufDun = 121,
		SP_huurton = 122,
		SP_ikopi = 123,
		SP_jellyfish = 124,
		SP_kaadu = 125,
		SP_kaitok = 126,
		SP_kima = 127,
		SP_kimogila = 128,
		SP_kittle = 129,
		SP_kliknik = 130,
		SP_krahbu = 131,
		SP_kraytDragon = 132,
		SP_kupernug = 133,
		SP_kusak = 134,
		SP_kwi = 135,
		SP_laa = 136,
		SP_langlatch = 137,
		SP_lanternBird = 138,
		SP_malkloc = 139,
		SP_mamien = 140,
		SP_mawgax = 141,
		SP_merek = 142,
		SP_mott = 143,
		SP_murra = 144,
		SP_mynock = 145,
		SP_narglatch = 146,
		SP_nerf = 147,
		SP_nuna = 148,
		SP_opeeSeaKiller = 149,
		SP_predatorialButterfly = 150,
		SP_pekoPeko = 151,
		SP_perlek = 152,
		SP_pharple = 153,
		SP_piket = 154,
		SP_plumedRasp = 155,
		SP_pufferfish = 156,
		SP_pugoriss = 157,
		SP_purbole = 158,
		SP_quenker = 159,
		SP_qurvel = 160,
		SP_rancor = 161,
		SP_ray = 162,
		SP_remmer = 163,
		SP_reptilianFlyer = 164,
		SP_roba = 165,
		SP_rockMite = 166,
		SP_ronto = 167,
		SP_saltMynock = 168,
		SP_sarlacc = 169,
		SP_scurrier = 170,
		SP_sharnaff = 171,
		SP_shaupaut = 172,
		SP_shearMite = 173,
		SP_skreeg = 174,
		SP_snorbal = 175,
		SP_spinedPuc = 176,
		SP_spinedSnake = 177,
		SP_squall = 178,
		SP_squill = 179,
		SP_stintaril = 180,
		SP_stripedFish = 181,
		SP_swirlProng = 182,
		SP_tancMite = 183,
		SP_taunTaun = 184,
		SP_tesselatedArborealBinjinphant = 185,
		SP_thune = 186,
		SP_torton = 187,
		SP_tybis = 188,
		SP_veermok = 189,
		SP_verne = 190,
		SP_vesp = 191,
		SP_virVur = 192,
		SP_vlutore = 193,
		SP_vogEel = 194,
		SP_voritorLizard = 195,
		SP_vynock = 196,
		SP_whisperBird = 197,
		SP_wingedOrnith = 198,
		SP_wompRat = 199,
		SP_woolamander = 200,
		SP_worrt = 201,
		SP_zuccaBoar = 202,
		SP_assassinDroid = 203,
		SP_astromech = 204,
		SP_bartenderDroid = 205,
		SP_bugDroid = 206,
		SP_darktrooper = 207,
		SP_demolitionmech = 208,
		SP_doorDroid = 209,
		SP_droideka = 210,
		SP_interrogator = 211,
		SP_jediTrainer = 212,
		SP_loadLifter = 213,
		SP_mouseDroid = 214,
		SP_powerDroid = 215,
		SP_probot = 216,
		SP_protocolDroid = 217,
		SP_repairDroid = 218,
		SP_spiderDroid = 219,
		SP_surgicalDroid = 220,
		SP_tattletaleDroid = 221,
		SP_trackerDroid = 222,
		SP_treadwell = 223,
		SP_ev9d9 = 224,
		SP_maulProbeDroid = 225,
		SP_atst = 226,
		SP_atat = 227,
		SP_geonosian = 228,
		SP_veractyle = 229,
		Species_Last = SP_veractyle,
	};

public:
	Gender     getGender() const;
	Niche     getNiche() const;
	Species     getSpecies() const;
	Race     getRace() const;
	float                  getAcceleration(MovementTypes index) const;
	float                  getAccelerationMin(MovementTypes index) const;
	float                  getAccelerationMax(MovementTypes index) const;
	float                  getSpeed(MovementTypes index) const;
	float                  getSpeedMin(MovementTypes index) const;
	float                  getSpeedMax(MovementTypes index) const;
	float                  getTurnRate(MovementTypes index) const;
	float                  getTurnRateMin(MovementTypes index) const;
	float                  getTurnRateMax(MovementTypes index) const;
	const std::string &    getAnimationMapFilename() const;
	float                  getSlopeModAngle() const;
	float                  getSlopeModAngleMin() const;
	float                  getSlopeModAngleMax() const;
	float                  getSlopeModPercent() const;
	float                  getSlopeModPercentMin() const;
	float                  getSlopeModPercentMax() const;
	float                  getWaterModPercent() const;
	float                  getWaterModPercentMin() const;
	float                  getWaterModPercentMax() const;
	float                  getStepHeight() const;
	float                  getStepHeightMin() const;
	float                  getStepHeightMax() const;
	float                  getCollisionHeight() const;
	float                  getCollisionHeightMin() const;
	float                  getCollisionHeightMax() const;
	float                  getCollisionRadius() const;
	float                  getCollisionRadiusMin() const;
	float                  getCollisionRadiusMax() const;
	const std::string &    getMovementDatatable() const;
	bool                   getPostureAlignToTerrain(Postures index) const;
	float                  getSwimHeight() const;
	float                  getSwimHeightMin() const;
	float                  getSwimHeightMax() const;
	float                  getWarpTolerance() const;
	float                  getWarpToleranceMin() const;
	float                  getWarpToleranceMax() const;
	float                  getCollisionOffsetX() const;
	float                  getCollisionOffsetXMin() const;
	float                  getCollisionOffsetXMax() const;
	float                  getCollisionOffsetZ() const;
	float                  getCollisionOffsetZMin() const;
	float                  getCollisionOffsetZMax() const;
	float                  getCollisionLength() const;
	float                  getCollisionLengthMin() const;
	float                  getCollisionLengthMax() const;
	float                  getCameraHeight() const;
	float                  getCameraHeightMin() const;
	float                  getCameraHeightMax() const;


protected:
	virtual void load(Iff &file);

private:
	IntegerParam m_gender;		// //creature gender
	IntegerParam m_niche;		// //creature niche
	IntegerParam m_species;		// //creature species
	IntegerParam m_race;		// //creature subspecies
	FloatParam m_acceleration[2];		// //how fast the creature accelerates in m/s/s
	FloatParam m_speed[2];		// //how fast the creature moves in m/s
	FloatParam m_turnRate[2];		// //how fast the creature turns in deg/s
	StringParam m_animationMapFilename;		// //info on how to animate the creature
	FloatParam m_slopeModAngle;		// //the maximum slope (in degrees) the creature will scale
	FloatParam m_slopeModPercent;		// //the relative speed the creature travels up a vertical slope
	FloatParam m_waterModPercent;		// //the relative speed the creature travels in water
	FloatParam m_stepHeight;		// //The maximum vertical step distance for the creature
	FloatParam m_collisionHeight;		// //The height of the creature's collision cylinder
	FloatParam m_collisionRadius;		// //The radius of the creature's collision cylinder
	StringParam m_movementDatatable;		// //The datatable to use for creature movement
	BoolParam m_postureAlignToTerrain[15];		// //align to terrain flags per posture
	FloatParam m_swimHeight;		// //Height below the water where the creature decides to swim
	FloatParam m_warpTolerance;
	FloatParam m_collisionOffsetX;		// // X offset of the collision sphere
	FloatParam m_collisionOffsetZ;		// // Z offset of the collision sphere
	FloatParam m_collisionLength;		// // Length of the creature, in meters
	FloatParam m_cameraHeight;
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling
	MovementTable * m_movementTable;

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	SharedCreatureObjectTemplate(const SharedCreatureObjectTemplate &);
	SharedCreatureObjectTemplate & operator =(const SharedCreatureObjectTemplate &);
};


inline void SharedCreatureObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	SharedCreatureObjectTemplate::registerMe();
//@END TFD INSTALL
}

inline const MovementTable * SharedCreatureObjectTemplate::getMovementTable() const
{
	return m_movementTable;
}

#endif	// _INCLUDED_SharedCreatureObjectTemplate_H
