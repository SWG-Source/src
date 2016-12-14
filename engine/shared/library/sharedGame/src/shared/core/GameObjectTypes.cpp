//======================================================================
//
// GameObjectTypes.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/GameObjectTypes.h"

#include "sharedGame/SharedObjectTemplate.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "UnicodeUtils.h"
#include <map>
#include <algorithm>
#include <vector>

//======================================================================

namespace
{
	typedef GameObjectTypes::TypeStringIdMap      TypeStringIdMap;
	typedef GameObjectTypes::TypeStringMap        TypeStringMap;
	typedef GameObjectTypes::StringTypeMap        StringTypeMap;
	typedef GameObjectTypes::TypeUnicodeStringMap TypeUnicodeStringMap;

	TypeStringIdMap       typeStringIds;
	TypeStringMap         typeStrings;
	StringTypeMap         stringTypes;
	TypeUnicodeStringMap  typeUnicodeStrings;
	std::map<int, std::string> typeExcludedFromCommodities;

	bool installed = false;

	static const std::string stringTableName = "got_n";

	const std::string     emptyString   ("unknown");
	const StringId        emptyStringId (stringTableName, "unknown");
	const Unicode::String emptyUnicodeString (Unicode::narrowToWide ("Serious Error"));

	void install ()
	{
		DEBUG_FATAL (installed, ("already installed"));

		installed = true;

#define MAKE_NAME(a) typeStrings [SharedObjectTemplate::GOT_##a] = #a ;

		MAKE_NAME(none);
		MAKE_NAME(corpse);
		MAKE_NAME(group);
		MAKE_NAME(guild);
		MAKE_NAME(lair);
		MAKE_NAME(static);
		MAKE_NAME(camp);
		MAKE_NAME(vendor);
		MAKE_NAME(loadbeacon);
		MAKE_NAME(armor);
		MAKE_NAME(armor_body);
		MAKE_NAME(armor_head);
		MAKE_NAME(armor_misc);
		MAKE_NAME(armor_leg);
		MAKE_NAME(armor_arm);
		MAKE_NAME(armor_hand);
		MAKE_NAME(armor_foot);
		MAKE_NAME(armor_shield);
		MAKE_NAME(armor_layer);
		MAKE_NAME(armor_segment);
		MAKE_NAME(armor_core);
		MAKE_NAME(armor_psg);
		MAKE_NAME(building);
		MAKE_NAME(building_municipal);
		MAKE_NAME(building_player);
		MAKE_NAME(building_factional);
		MAKE_NAME(creature);
		MAKE_NAME(creature_character);
		MAKE_NAME(creature_droid);
		MAKE_NAME(creature_droid_probe);
		MAKE_NAME(creature_monster);
		MAKE_NAME(data);
		MAKE_NAME(data_draft_schematic);
		MAKE_NAME(data_manufacturing_schematic);
		MAKE_NAME(data_mission_object);
		MAKE_NAME(data_token);
		MAKE_NAME(data_waypoint);
		MAKE_NAME(data_fictional);
		MAKE_NAME(data_pet_control_device);
		MAKE_NAME(data_vehicle_control_device);
		MAKE_NAME(data_draft_schematic_read_only);
		MAKE_NAME(data_ship_control_device);
		MAKE_NAME(data_droid_control_device);
		MAKE_NAME(data_house_control_device);
		MAKE_NAME(data_vendor_control_device);
		MAKE_NAME(data_player_quest_object);
		MAKE_NAME(installation);
		MAKE_NAME(installation_factory);
		MAKE_NAME(installation_generator);
		MAKE_NAME(installation_harvester);
		MAKE_NAME(installation_turret);
		MAKE_NAME(installation_minefield);
		MAKE_NAME(misc);
		MAKE_NAME(misc_ammunition);
		MAKE_NAME(misc_chemical);
		MAKE_NAME(misc_clothing_DUMMY);
		MAKE_NAME(misc_component_DUMMY);
		MAKE_NAME(misc_container);
		MAKE_NAME(misc_crafting_station);
		MAKE_NAME(misc_deed_DUMMY);
		MAKE_NAME(misc_electronics);
		MAKE_NAME(misc_flora);
		MAKE_NAME(misc_food);
		MAKE_NAME(misc_furniture);
		MAKE_NAME(misc_instrument);
		MAKE_NAME(misc_pharmaceutical);
		MAKE_NAME(misc_resource_container_DUMMY);
		MAKE_NAME(misc_sign);
		MAKE_NAME(misc_counter);
		MAKE_NAME(misc_factory_crate);
		MAKE_NAME(misc_ticket_travel);
		MAKE_NAME(misc_item);
		MAKE_NAME(misc_trap);
		MAKE_NAME(misc_container_wearable);
		MAKE_NAME(misc_fishing_pole);
		MAKE_NAME(misc_fishing_bait);
		MAKE_NAME(misc_drink);
		MAKE_NAME(misc_firework);
		MAKE_NAME(misc_item_usable);
		MAKE_NAME(misc_petmed);
		MAKE_NAME(misc_firework_show);
		MAKE_NAME(misc_clothing_attachment);
		MAKE_NAME(misc_live_sample);
		MAKE_NAME(misc_armor_attachment);
		MAKE_NAME(misc_community_crafting_project);
		MAKE_NAME(misc_force_crystal);
		MAKE_NAME(misc_droid_programming_chip);
		MAKE_NAME(misc_asteroid);
		MAKE_NAME(misc_pob_ship_pilot_chair);
		MAKE_NAME(misc_operations_chair);
		MAKE_NAME(misc_turret_access_ladder);
		MAKE_NAME(misc_container_ship_loot);
		MAKE_NAME(misc_armor_noequip);
		MAKE_NAME(misc_enzyme);
		MAKE_NAME(misc_food_pet);
		MAKE_NAME(misc_collection);
		MAKE_NAME(misc_container_public);
		MAKE_NAME(misc_ground_target);
		MAKE_NAME(misc_blueprint);
		MAKE_NAME(misc_enzyme_isomerase);
		MAKE_NAME(misc_enzyme_lyase);
		MAKE_NAME(misc_enzyme_hydrolase);
		MAKE_NAME(misc_tcg_card);
		MAKE_NAME(misc_appearance_only);
		MAKE_NAME(misc_appearance_only_invisible);
		MAKE_NAME(terminal);
		MAKE_NAME(terminal_bank);
		MAKE_NAME(terminal_bazaar);
		MAKE_NAME(terminal_cloning);
		MAKE_NAME(terminal_insurance);
		MAKE_NAME(terminal_manage);
		MAKE_NAME(terminal_mission);
		MAKE_NAME(terminal_permissions);
		MAKE_NAME(terminal_player_structure);
		MAKE_NAME(terminal_shipping);
		MAKE_NAME(terminal_travel);
		MAKE_NAME(terminal_space);
		MAKE_NAME(terminal_misc);
		MAKE_NAME(terminal_space_npe);
		MAKE_NAME(tool);
		MAKE_NAME(tool_crafting);
		MAKE_NAME(tool_survey);
		MAKE_NAME(tool_repair);
		MAKE_NAME(tool_camp_kit);
		MAKE_NAME(tool_ship_component_repair);
		MAKE_NAME(vehicle);
		MAKE_NAME(vehicle_hover);
		MAKE_NAME(vehicle_hover_ai);
		MAKE_NAME(weapon);
		MAKE_NAME(weapon_melee_misc);
		MAKE_NAME(weapon_ranged_misc);
		MAKE_NAME(weapon_ranged_thrown);
		MAKE_NAME(weapon_heavy_misc);
		MAKE_NAME(weapon_heavy_mine);
		MAKE_NAME(weapon_heavy_special);
		MAKE_NAME(weapon_melee_1h);
		MAKE_NAME(weapon_melee_2h);
		MAKE_NAME(weapon_melee_polearm);
		MAKE_NAME(weapon_ranged_pistol);
		MAKE_NAME(weapon_ranged_carbine);
		MAKE_NAME(weapon_ranged_rifle);
		MAKE_NAME(component);
		MAKE_NAME(component_armor);
		MAKE_NAME(component_chemistry);
		MAKE_NAME(component_clothing);
		MAKE_NAME(component_droid);
		MAKE_NAME(component_electronics);
		MAKE_NAME(component_munition);
		MAKE_NAME(component_structure);
		MAKE_NAME(component_weapon_melee);
		MAKE_NAME(component_weapon_ranged);
		MAKE_NAME(component_tissue);
		MAKE_NAME(component_genetic);
		MAKE_NAME(component_saber_crystal);
		MAKE_NAME(component_community_crafting);
		MAKE_NAME(component_new_armor);
		MAKE_NAME(powerup_weapon);
		MAKE_NAME(powerup_weapon_melee);
		MAKE_NAME(powerup_weapon_ranged);
		MAKE_NAME(powerup_weapon_thrown);
		MAKE_NAME(powerup_weapon_heavy);
		MAKE_NAME(powerup_weapon_mine);
		MAKE_NAME(powerup_weapon_heavy_special);
		MAKE_NAME(powerup_armor);
		MAKE_NAME(powerup_armor_body);
		MAKE_NAME(powerup_armor_head);
		MAKE_NAME(powerup_armor_misc);
		MAKE_NAME(powerup_armor_leg);
		MAKE_NAME(powerup_armor_arm);
		MAKE_NAME(powerup_armor_hand);
		MAKE_NAME(powerup_armor_foot);
		MAKE_NAME(powerup_armor_layer);
		MAKE_NAME(powerup_armor_segment);
		MAKE_NAME(powerup_armor_core);
		MAKE_NAME(jewelry);
		MAKE_NAME(jewelry_ring);
		MAKE_NAME(jewelry_bracelet);
		MAKE_NAME(jewelry_necklace);
		MAKE_NAME(jewelry_earring);
		MAKE_NAME(resource_container);
		MAKE_NAME(resource_container_energy_gas);
		MAKE_NAME(resource_container_energy_liquid);
		MAKE_NAME(resource_container_energy_radioactive);
		MAKE_NAME(resource_container_energy_solid);
		MAKE_NAME(resource_container_inorganic_chemicals);
		MAKE_NAME(resource_container_inorganic_gas);
		MAKE_NAME(resource_container_inorganic_minerals);
		MAKE_NAME(resource_container_inorganic_water);
		MAKE_NAME(resource_container_organic_food);
		MAKE_NAME(resource_container_organic_hide);
		MAKE_NAME(resource_container_organic_structure);
		MAKE_NAME(resource_container_pseudo);
		MAKE_NAME(resource_container_space);
		MAKE_NAME(deed);
		MAKE_NAME(deed_building);
		MAKE_NAME(deed_installation);
		MAKE_NAME(deed_pet);
		MAKE_NAME(deed_droid);
		MAKE_NAME(deed_vehicle);
		MAKE_NAME(clothing);
		MAKE_NAME(clothing_bandolier);
		MAKE_NAME(clothing_belt);
		MAKE_NAME(clothing_bodysuit);
		MAKE_NAME(clothing_cape);
		MAKE_NAME(clothing_cloak);
		MAKE_NAME(clothing_foot);
		MAKE_NAME(clothing_dress);
		MAKE_NAME(clothing_hand);
		MAKE_NAME(clothing_eye);
		MAKE_NAME(clothing_head);
		MAKE_NAME(clothing_jacket);
		MAKE_NAME(clothing_pants);
		MAKE_NAME(clothing_robe);
		MAKE_NAME(clothing_shirt);
		MAKE_NAME(clothing_vest);
		MAKE_NAME(clothing_wookiee);
		MAKE_NAME(clothing_misc);
		MAKE_NAME(clothing_skirt);
		MAKE_NAME(ship_component);
		MAKE_NAME(ship_component_reactor);
		MAKE_NAME(ship_component_engine);
		MAKE_NAME(ship_component_shield);
		MAKE_NAME(ship_component_armor);
		MAKE_NAME(ship_component_weapon);
		MAKE_NAME(ship_component_capacitor);
		MAKE_NAME(ship_component_booster);
		MAKE_NAME(ship_component_droid_interface);
		MAKE_NAME(ship_component_hangar);
		MAKE_NAME(ship_component_targeting_station);
		MAKE_NAME(ship_component_bridge);
		MAKE_NAME(ship_component_chassis);
		MAKE_NAME(ship_component_missilepack);
		MAKE_NAME(ship_component_countermeasurepack);
		MAKE_NAME(ship_component_missilelauncher);
		MAKE_NAME(ship_component_countermeasurelauncher);
		MAKE_NAME(ship_component_cargo_hold);
		MAKE_NAME(ship_component_modification);
		MAKE_NAME(ship);
		MAKE_NAME(ship_fighter);
		MAKE_NAME(ship_capital);
		MAKE_NAME(ship_station);
		MAKE_NAME(ship_transport);
		MAKE_NAME(ship_mining_asteroid_static);
		MAKE_NAME(ship_mining_asteroid_dynamic);
		MAKE_NAME(cybernetic);
		MAKE_NAME(cybernetic_arm);
		MAKE_NAME(cybernetic_legs);
		MAKE_NAME(cybernetic_torso);
		MAKE_NAME(cybernetic_forearm);
		MAKE_NAME(cybernetic_hand);
		MAKE_NAME(cybernetic_component);
		MAKE_NAME(chronicles);
		MAKE_NAME(chronicles_relic);
		MAKE_NAME(chronicles_chronicle);
		MAKE_NAME(chronicles_quest_holocron);
		MAKE_NAME(chronicles_quest_holocron_recipe);
		MAKE_NAME(chronicles_relic_fragment);

#undef MAKE_NAME

		for (TypeStringMap::const_iterator it = typeStrings.begin (); it != typeStrings.end (); ++it)
		{
			const int got               = (*it).first;
			const std::string & gotName = (*it).second;
			typeStringIds.insert (std::make_pair (got, StringId (stringTableName, gotName)));
			stringTypes.insert (std::make_pair (Unicode::toLower(gotName), got));
		}

		DataTable const * const commoditiesExclusionData = DataTableManager::getTable("datatables/commodity/got_exclusion.iff", true);
		if (!commoditiesExclusionData)
		{
			WARNING(true, ("Could not find commodities GOT type exclusion data table datatables/commodity/got_exclusion.iff"));
			return;
		}

		int const gotColumn = commoditiesExclusionData->findColumnNumber("got");
		if (gotColumn < 0)
		{
			WARNING(true, ("Could not find column \"got\" in commodities GOT type exclusion data table datatables/commodity/got_exclusion.iff"));
			return;
		}

		unsigned int const numRows = static_cast<unsigned int>(commoditiesExclusionData->getNumRows());
		if (numRows == 0)
		{
			WARNING(true, ("Commodities GOT type exclusion data table datatables/commodity/got_exclusion.iff is empty"));
			return;
		}

		for (unsigned int i = 0; i < numRows; ++i)
		{
			std::string const excludeGot = commoditiesExclusionData->getStringValue(gotColumn, i);

			StringTypeMap::const_iterator const it = stringTypes.find(Unicode::toLower(excludeGot));
			FATAL((it == stringTypes.end()), ("Commodities GOT type exclusion data table datatables/commodity/got_exclusion.iff contains a non-existing GOT type %s", excludeGot.c_str()));

			typeExcludedFromCommodities[it->second] = excludeGot;
		}
	}
}

//----------------------------------------------------------------------

const int GameObjectTypes::ms_type_mask = 0xffffff00;

//----------------------------------------------------------------------

bool GameObjectTypes::getTypeByName    (const std::string & name, int & type)
{
	if (!installed)
		install ();

	StringTypeMap::const_iterator const it = stringTypes.find (Unicode::toLower (name.c_str ()));
	if (it != stringTypes.end ())
	{
		type = (*it).second;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

const std::string & GameObjectTypes::getCanonicalName (int type)
{
	if (!installed)
		install ();

	TypeStringMap::const_iterator it = typeStrings.find (type);
	if (it != typeStrings.end ())
		return (*it).second;

	return emptyString;
}

//----------------------------------------------------------------------

int GameObjectTypes::getGameObjectType (std::string const & canonicalName)
{
	if (!installed)
		install ();

	StringTypeMap::const_iterator const it = stringTypes.find (Unicode::toLower (canonicalName.c_str ()));
	if (it != stringTypes.end ())
	{
		return it->second;
	}

	return 0;
}

//----------------------------------------------------------------------

const StringId &   GameObjectTypes::getStringId      (int type)
{
	if (!installed)
		install ();

	const SharedObjectTemplate::GameObjectType got = static_cast<SharedObjectTemplate::GameObjectType>(type);

	TypeStringIdMap::const_iterator it = typeStringIds.find (got);
	if (it != typeStringIds.end ())
		return (*it).second;

	return emptyStringId;
}

//----------------------------------------------------------------------

const Unicode::String & GameObjectTypes::getLocalizedName (int type)
{
	if (!installed)
		install ();

	const SharedObjectTemplate::GameObjectType got = static_cast<SharedObjectTemplate::GameObjectType>(type);

	TypeUnicodeStringMap::const_iterator it = typeUnicodeStrings.find (got);
	if (it != typeUnicodeStrings.end ())
		return (*it).second;

	const StringId & stringId = getStringId (type);

	if (!stringId.isInvalid ())
	{
		return (typeUnicodeStrings [got] = stringId.localize ());
	}

	return emptyUnicodeString;
}

//----------------------------------------------------------------------

bool GameObjectTypes::isTypeOf         (int type, int typeToTestAgainst)
{
	if (type == typeToTestAgainst)
		return true;

	if (getMaskedType (type) == typeToTestAgainst)
		return true;

	return false;
}

//----------------------------------------------------------------------

const GameObjectTypes::TypeStringMap & GameObjectTypes::getTypeStringMap ()
{
	if (!installed)
		install ();

	return typeStrings;
}

//----------------------------------------------------------------------

bool GameObjectTypes::isSubType        (int type)
{
	return (type & ~ms_type_mask) != 0;
}

//----------------------------------------------------------------------

bool GameObjectTypes::doesPowerupApply (int powerupType, int targetType)
{
	//@todo: this should be data driven
	typedef std::vector<int> IntVector;
	typedef std::map<int, IntVector> IntVectorMap;
	static IntVectorMap s_powerupGotMap;
	static bool s_powerupGotMapInstalled = false;
	if (!s_powerupGotMapInstalled)
	{
		s_powerupGotMapInstalled = true;

		IntVector iv;
		iv.clear ();
		iv.push_back (SharedObjectTemplate::GOT_weapon_melee_misc);
		iv.push_back (SharedObjectTemplate::GOT_weapon_melee_1h);
		iv.push_back (SharedObjectTemplate::GOT_weapon_melee_2h);
		iv.push_back (SharedObjectTemplate::GOT_weapon_melee_polearm);
		std::sort (iv.begin (), iv.end ());
		s_powerupGotMap.insert (std::make_pair (SharedObjectTemplate::GOT_powerup_weapon_melee, iv));

		iv.clear ();
		iv.push_back (SharedObjectTemplate::GOT_weapon_ranged_misc);
		iv.push_back (SharedObjectTemplate::GOT_weapon_ranged_pistol);
		iv.push_back (SharedObjectTemplate::GOT_weapon_ranged_carbine);
		iv.push_back (SharedObjectTemplate::GOT_weapon_ranged_rifle);
		std::sort (iv.begin (), iv.end ());
		s_powerupGotMap.insert (std::make_pair (SharedObjectTemplate::GOT_powerup_weapon_ranged, iv));

		iv.clear ();
		iv.push_back (SharedObjectTemplate::GOT_weapon_ranged_thrown);
		std::sort (iv.begin (), iv.end ());
		s_powerupGotMap.insert (std::make_pair (SharedObjectTemplate::GOT_powerup_weapon_thrown, iv));

		iv.clear ();
		iv.push_back (SharedObjectTemplate::GOT_weapon_heavy_misc);
		std::sort (iv.begin (), iv.end ());
		s_powerupGotMap.insert (std::make_pair (SharedObjectTemplate::GOT_powerup_weapon_heavy, iv));

		iv.clear ();
		iv.push_back (SharedObjectTemplate::GOT_weapon_heavy_special);
		std::sort (iv.begin (), iv.end ());
		s_powerupGotMap.insert (std::make_pair (SharedObjectTemplate::GOT_powerup_weapon_heavy_special, iv));
		
		iv.clear ();
		iv.push_back (SharedObjectTemplate::GOT_weapon_heavy_mine);
		std::sort (iv.begin (), iv.end ());
		s_powerupGotMap.insert (std::make_pair (SharedObjectTemplate::GOT_powerup_weapon_mine, iv));
	}
	
	if ((GameObjectTypes::isTypeOf (powerupType, SharedObjectTemplate::GOT_powerup_armor)  && GameObjectTypes::isTypeOf (targetType, SharedObjectTemplate::GOT_armor)) ||
		(GameObjectTypes::isTypeOf (powerupType, SharedObjectTemplate::GOT_powerup_weapon) && GameObjectTypes::isTypeOf (targetType, SharedObjectTemplate::GOT_weapon)))
	{
		const IntVectorMap::const_iterator it = s_powerupGotMap.find (powerupType);
		
		if (it != s_powerupGotMap.end ())
		{
			const IntVector & iv = (*it).second;
			
			//-- powerup not applicable to target
			if (!std::binary_search (iv.begin (), iv.end (), targetType))
				return false;
		}
		
		return true;
	}
	
	return false;
}

//----------------------------------------------------------------------

bool GameObjectTypes::isExcludedFromCommodities(int type)
{
	if (!installed)
		install();

	return (typeExcludedFromCommodities.count(type) > 0);
}

//----------------------------------------------------------------------

std::map<int, std::string> const & GameObjectTypes::getTypesExcludedFromCommodities()
{
	if (!installed)
		install();

	return typeExcludedFromCommodities;
}

//======================================================================
