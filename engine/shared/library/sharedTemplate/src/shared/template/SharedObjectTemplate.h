//========================================================================
//
// SharedObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_SharedObjectTemplate_H
#define _INCLUDED_SharedObjectTemplate_H

#include "sharedTemplateDefinition/TpfTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedTemplateDefinition/TpfTemplate.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;


class SharedObjectTemplate : public TpfTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		SharedObjectTemplate_tag = TAG(S,H,O,T)
	};
//@END TFD ID
public:
	         SharedObjectTemplate(const std::string & filename);
	virtual ~SharedObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(void);

//@BEGIN TFD
public:
	enum ContainerType
	{
		CT_none,
		CT_slotted,
		CT_volume,		// //This kind of container can hold tangible objects
		CT_volumeIntangible,		// //This kind of container can hold intangible objects
		CT_volumeGeneric,		// //This kind of container can hold any object tangible or not.
		CT_ridable,		// //This should be used for vehicles and mounts only: provides a slotted container with contents visible to the world
		ContainerType_Last = CT_ridable,
	};

	enum GameObjectType
	{
		GOT_none = 0x00000000,
		GOT_corpse,
		GOT_group,
		GOT_guild,
		GOT_lair,
		GOT_static,
		GOT_camp,
		GOT_vendor,
		GOT_loadbeacon,
		GOT_armor = 0x00000100,		// //if you add armor types, please add them to the GOT_powerups as well
		GOT_armor_body,
		GOT_armor_head,
		GOT_armor_misc,
		GOT_armor_leg,
		GOT_armor_arm,
		GOT_armor_hand,
		GOT_armor_foot,
		GOT_armor_shield,
		GOT_armor_layer,
		GOT_armor_segment,
		GOT_armor_core,
		GOT_armor_psg,
		GOT_building = 0x00000200,
		GOT_building_municipal,
		GOT_building_player,
		GOT_building_factional,
		GOT_creature = 0x00000400,
		GOT_creature_character,
		GOT_creature_droid,
		GOT_creature_droid_probe,
		GOT_creature_monster,
		GOT_data = 0x00000800,
		GOT_data_draft_schematic,
		GOT_data_manufacturing_schematic,
		GOT_data_mission_object,
		GOT_data_token,
		GOT_data_waypoint,
		GOT_data_fictional,
		GOT_data_pet_control_device,
		GOT_data_vehicle_control_device,
		GOT_data_draft_schematic_read_only,
		GOT_data_ship_control_device,
		GOT_data_droid_control_device,
		GOT_data_house_control_device,
		GOT_data_vendor_control_device,
		GOT_data_player_quest_object,
		GOT_installation = 0x00001000,
		GOT_installation_factory,
		GOT_installation_generator,
		GOT_installation_harvester,
		GOT_installation_turret,
		GOT_installation_minefield,
		GOT_misc = 0x00002000,
		GOT_misc_ammunition,
		GOT_misc_chemical,
		GOT_misc_clothing_DUMMY,		// // when you remove this, please recompile all the shared object templates
		GOT_misc_component_DUMMY,		// // when you remove this, please recompile all the shared object templates
		GOT_misc_container,
		GOT_misc_crafting_station,
		GOT_misc_deed_DUMMY,
		GOT_misc_electronics,
		GOT_misc_flora,
		GOT_misc_food,
		GOT_misc_furniture,
		GOT_misc_instrument,
		GOT_misc_pharmaceutical,
		GOT_misc_resource_container_DUMMY,		// // when you remove this, please recompile all the shared object templates
		GOT_misc_sign,
		GOT_misc_counter,
		GOT_misc_factory_crate,
		GOT_misc_ticket_travel,
		GOT_misc_item,		// // generic 'usable' item
		GOT_misc_trap,
		GOT_misc_container_wearable,
		GOT_misc_fishing_pole,
		GOT_misc_fishing_bait,
		GOT_misc_drink,
		GOT_misc_firework,
		GOT_misc_item_usable,
		GOT_misc_petmed,
		GOT_misc_firework_show,
		GOT_misc_clothing_attachment,
		GOT_misc_live_sample,
		GOT_misc_armor_attachment,
		GOT_misc_community_crafting_project,
		GOT_misc_force_crystal,
		GOT_misc_droid_programming_chip,
		GOT_misc_asteroid,
		GOT_misc_pob_ship_pilot_chair,
		GOT_misc_operations_chair,
		GOT_misc_turret_access_ladder,
		GOT_misc_container_ship_loot,
		GOT_misc_armor_noequip,
		GOT_misc_enzyme,
		GOT_misc_food_pet,
		GOT_misc_collection,
		GOT_misc_container_public,
		GOT_misc_ground_target,
		GOT_misc_blueprint,
		GOT_misc_enzyme_isomerase,
		GOT_misc_enzyme_lyase,
		GOT_misc_enzyme_hydrolase,
		GOT_misc_tcg_card,
		GOT_misc_appearance_only,
		GOT_misc_appearance_only_invisible,
		GOT_terminal = 0x00004000,
		GOT_terminal_bank,
		GOT_terminal_bazaar,
		GOT_terminal_cloning,
		GOT_terminal_insurance,
		GOT_terminal_manage,
		GOT_terminal_mission,
		GOT_terminal_permissions,
		GOT_terminal_player_structure,
		GOT_terminal_shipping,
		GOT_terminal_travel,
		GOT_terminal_space,
		GOT_terminal_misc,
		GOT_terminal_space_npe,
		GOT_tool = 0x00008000,
		GOT_tool_crafting,
		GOT_tool_survey,
		GOT_tool_repair,
		GOT_tool_camp_kit,
		GOT_tool_ship_component_repair,
		GOT_vehicle = 0x00010000,
		GOT_vehicle_hover,
		GOT_vehicle_hover_ai,
		GOT_weapon = 0x00020000,		// //if you add weapon types, please add them to the GOT_powerups as well
		GOT_weapon_melee_misc,
		GOT_weapon_ranged_misc,
		GOT_weapon_ranged_thrown,
		GOT_weapon_heavy_misc,
		GOT_weapon_heavy_mine,
		GOT_weapon_heavy_special,
		GOT_weapon_melee_1h,
		GOT_weapon_melee_2h,
		GOT_weapon_melee_polearm,
		GOT_weapon_ranged_pistol,
		GOT_weapon_ranged_carbine,
		GOT_weapon_ranged_rifle,
		GOT_component = 0x00040000,
		GOT_component_armor,
		GOT_component_chemistry,
		GOT_component_clothing,
		GOT_component_droid,
		GOT_component_electronics,
		GOT_component_munition,
		GOT_component_structure,
		GOT_component_weapon_melee,
		GOT_component_weapon_ranged,
		GOT_component_tissue,
		GOT_component_genetic,
		GOT_component_saber_crystal,
		GOT_component_community_crafting,
		GOT_component_new_armor,
		GOT_powerup_weapon = 0x00080000,
		GOT_powerup_weapon_melee,
		GOT_powerup_weapon_ranged,
		GOT_powerup_weapon_thrown,
		GOT_powerup_weapon_heavy,
		GOT_powerup_weapon_mine,
		GOT_powerup_weapon_heavy_special,
		GOT_powerup_armor = 0x00100000,
		GOT_powerup_armor_body,
		GOT_powerup_armor_head,
		GOT_powerup_armor_misc,
		GOT_powerup_armor_leg,
		GOT_powerup_armor_arm,
		GOT_powerup_armor_hand,
		GOT_powerup_armor_foot,
		GOT_powerup_armor_layer,
		GOT_powerup_armor_segment,
		GOT_powerup_armor_core,
		GOT_jewelry = 0x00200000,
		GOT_jewelry_ring,
		GOT_jewelry_bracelet,
		GOT_jewelry_necklace,
		GOT_jewelry_earring,
		GOT_resource_container = 0x00400000,
		GOT_resource_container_energy_gas,
		GOT_resource_container_energy_liquid,
		GOT_resource_container_energy_radioactive,
		GOT_resource_container_energy_solid,
		GOT_resource_container_inorganic_chemicals,
		GOT_resource_container_inorganic_gas,
		GOT_resource_container_inorganic_minerals,
		GOT_resource_container_inorganic_water,
		GOT_resource_container_organic_food,
		GOT_resource_container_organic_hide,
		GOT_resource_container_organic_structure,
		GOT_resource_container_pseudo,
		GOT_resource_container_space,
		GOT_deed = 0x00800000,
		GOT_deed_building,
		GOT_deed_installation,
		GOT_deed_pet,
		GOT_deed_droid,
		GOT_deed_vehicle,
		GOT_clothing = 0x01000000,
		GOT_clothing_bandolier,
		GOT_clothing_belt,
		GOT_clothing_bodysuit,
		GOT_clothing_cape,
		GOT_clothing_cloak,
		GOT_clothing_foot,
		GOT_clothing_dress,
		GOT_clothing_hand,
		GOT_clothing_eye,
		GOT_clothing_head,
		GOT_clothing_jacket,
		GOT_clothing_pants,
		GOT_clothing_robe,
		GOT_clothing_shirt,
		GOT_clothing_vest,
		GOT_clothing_wookiee,
		GOT_clothing_misc,
		GOT_clothing_skirt,
		GOT_ship_component = 0x40000000,		// //add space-specific GOTS at the "end" to make merging easier
		GOT_ship_component_reactor,
		GOT_ship_component_engine,
		GOT_ship_component_shield,
		GOT_ship_component_armor,
		GOT_ship_component_weapon,
		GOT_ship_component_capacitor,
		GOT_ship_component_booster,
		GOT_ship_component_droid_interface,
		GOT_ship_component_hangar,
		GOT_ship_component_targeting_station,
		GOT_ship_component_bridge,
		GOT_ship_component_chassis,
		GOT_ship_component_missilepack,
		GOT_ship_component_countermeasurepack,
		GOT_ship_component_missilelauncher,
		GOT_ship_component_countermeasurelauncher,
		GOT_ship_component_cargo_hold,
		GOT_ship_component_modification,
		GOT_ship = 0x20000000,
		GOT_ship_fighter,
		GOT_ship_capital,
		GOT_ship_station,
		GOT_ship_transport,
		GOT_ship_mining_asteroid_static,
		GOT_ship_mining_asteroid_dynamic,
		GOT_cybernetic = 0x20000100,
		GOT_cybernetic_arm,
		GOT_cybernetic_legs,
		GOT_cybernetic_torso,
		GOT_cybernetic_forearm,
		GOT_cybernetic_hand,
		GOT_cybernetic_component,
		GOT_chronicles = 0x00001100,
		GOT_chronicles_relic,
		GOT_chronicles_chronicle,
		GOT_chronicles_quest_holocron,
		GOT_chronicles_quest_holocron_recipe,
		GOT_chronicles_relic_fragment,
		GameObjectType_Last = GOT_chronicles_relic_fragment,
	};

	enum SurfaceType
	{
		ST_other,
		ST_metal,
		ST_stone,
		ST_wood,
		ST_acid,
		ST_ice,
		ST_molten,
		ST_obsidian,
		SurfaceType_Last = ST_obsidian,
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
	StringIdParam m_objectName;		// //the default name of this object
	StringIdParam m_detailedDescription;		// //long description of the object
	StringIdParam m_lookAtText;		// //? does this differ from detailedDescription ?
	BoolParam m_snapToTerrain;		// //flag that the object is snapped to terrain on addition to the world
	CompilerIntegerParam m_containerType;		// //if this is a container, what kind
	CompilerIntegerParam m_containerVolumeLimit;		// //if this object has a volume container, this say how much it can hold.
	StringParam m_tintPalette;		// //what tints are available to the object
	StringParam m_slotDescriptorFilename;		// //what slots are available for this object
	StringParam m_arrangementDescriptorFilename;		// //what slots this object can be put into
	StringParam m_appearanceFilename;		// //?
	StringParam m_portalLayoutFilename;		// //?
	StringParam m_clientDataFile;		// //client-specific data
	FloatParam m_scale;		// //modification to object's size
	CompilerIntegerParam m_gameObjectType;		// // game object type
	BoolParam m_sendToClient;		// //specifies whether or not the object using this template should is sent to the client
	FloatParam m_scaleThresholdBeforeExtentTest;		// //specifies the scale at which the object will use box extents for collision
	FloatParam m_clearFloraRadius;		// // distance to clear collidable flora around this object
	CompilerIntegerParam m_surfaceType;
	FloatParam m_noBuildRadius;		// // distance to not allow structure placement around this object
	BoolParam m_onlyVisibleInTools;
	FloatParam m_locationReservationRadius;
	BoolParam m_forceNoCollision;
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	SharedObjectTemplate(const SharedObjectTemplate &);
	SharedObjectTemplate & operator =(const SharedObjectTemplate &);
};


inline void SharedObjectTemplate::install(void)
{
//@BEGIN TFD INSTALL
	SharedObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_SharedObjectTemplate_H
