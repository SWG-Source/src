//======================================================================
//
// SharedObjectAttributes.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SharedObjectAttributes_H
#define INCLUDED_SharedObjectAttributes_H

//======================================================================

#include <string>

//----------------------------------------------------------------------

namespace SharedObjectAttributes
{
	namespace StringTables
	{
		const std::string Name = "obj_attr_n";
		const std::string Desc = "obj_attr_d";
	}

#define MAKE_ATTRIB(a) const std::string a = #a

	MAKE_ATTRIB (craft_tool_status);
	MAKE_ATTRIB (craft_tool_time);
	MAKE_ATTRIB (serial_number);
	MAKE_ATTRIB (crafter);
	MAKE_ATTRIB (craft_tool_effectiveness);

	MAKE_ATTRIB (armorRating);
	MAKE_ATTRIB (baseEffectiveness);
	MAKE_ATTRIB (baseIntegrity);
	
	MAKE_ATTRIB (volume);
	MAKE_ATTRIB (data_volume);
	MAKE_ATTRIB (condition);
	MAKE_ATTRIB (contents);
	MAKE_ATTRIB (data_contents);
	MAKE_ATTRIB (resource_contents);
	MAKE_ATTRIB (resource_name);
	MAKE_ATTRIB (resource_class);
	MAKE_ATTRIB (waypoint_planet);
	MAKE_ATTRIB (waypoint_region);
	MAKE_ATTRIB (waypoint_position);
	MAKE_ATTRIB (waypoint_active);

	MAKE_ATTRIB (counter_uses_remaining);
	MAKE_ATTRIB (counter_charges_remaining);

	MAKE_ATTRIB (cat_wpn_attack_cost);
	MAKE_ATTRIB (cat_pup_attack_cost);

	MAKE_ATTRIB (attr_health);
	MAKE_ATTRIB (attr_strength);
	MAKE_ATTRIB (attr_constitution);

	MAKE_ATTRIB (attr_action);
	MAKE_ATTRIB (attr_quickness);
	MAKE_ATTRIB (attr_stamina);

	MAKE_ATTRIB (attr_mind);
	MAKE_ATTRIB (attr_focus);
	MAKE_ATTRIB (attr_willpower);

	MAKE_ATTRIB (wpn_accuracy);
	MAKE_ATTRIB (wpn_attack_speed);
	MAKE_ATTRIB (wpn_damage);
	MAKE_ATTRIB (wpn_damage_max);
	MAKE_ATTRIB (wpn_damage_min);
	MAKE_ATTRIB (wpn_damage_radius);
	MAKE_ATTRIB (wpn_damage_type);
	MAKE_ATTRIB (wpn_elemental_type);
	MAKE_ATTRIB (wpn_elemental_value);
	MAKE_ATTRIB (wpn_range_max);
	MAKE_ATTRIB (wpn_range_min);
	MAKE_ATTRIB (wpn_wound_chance);
	MAKE_ATTRIB (wpn_armor_pierce_rating);

	MAKE_ATTRIB (armor_pierce_);

	MAKE_ATTRIB (complexity);

	MAKE_ATTRIB (manufacture_schematic_volume);
	MAKE_ATTRIB (sockets);

	MAKE_ATTRIB (travel_arrival_planet);
	MAKE_ATTRIB (travel_arrival_point);
	MAKE_ATTRIB (travel_departure_planet);
	MAKE_ATTRIB (travel_departure_point);

	// link to player
	MAKE_ATTRIB (bio_link);
	MAKE_ATTRIB (no_trade);
	MAKE_ATTRIB (no_trade_shared);
	MAKE_ATTRIB (no_trade_removable);

	MAKE_ATTRIB (unique);

	//--
	//-- categories
	//--
	MAKE_ATTRIB (cat_attrib_mod_bonus);
	MAKE_ATTRIB (cat_manf_schem_ing_resource);
	MAKE_ATTRIB (cat_manf_schem_ing_component);
	MAKE_ATTRIB (manf_limit);
	MAKE_ATTRIB (manf_attribs);

	MAKE_ATTRIB (cat_armor_encumbrance);
	MAKE_ATTRIB (cat_armor_effectiveness);
	MAKE_ATTRIB (armor_eff_);
	MAKE_ATTRIB (armor_eff_base);
	MAKE_ATTRIB (cat_armor_special_protection);
	MAKE_ATTRIB (cat_armor_vulnerability);

	MAKE_ATTRIB (cat_wpn_damage);

	MAKE_ATTRIB (cat_skill_mod_bonus);
	MAKE_ATTRIB (weapon_cert_status);

	MAKE_ATTRIB (original_name);
	MAKE_ATTRIB (original_name_creature);

	MAKE_ATTRIB (factory_count);
	MAKE_ATTRIB (factory_attribs);

	MAKE_ATTRIB (manufacture_object);
	MAKE_ATTRIB (manufacture_time);
	MAKE_ATTRIB (manufacture_count);

	MAKE_ATTRIB (object_type);

	MAKE_ATTRIB (consider);

	MAKE_ATTRIB (ship_component_armor);
	MAKE_ATTRIB (ship_component_hitpoints);
	MAKE_ATTRIB (ship_component_efficiency_general);
	MAKE_ATTRIB (ship_component_efficiency_energy);
	MAKE_ATTRIB (ship_component_energy_required);
	MAKE_ATTRIB (ship_component_mass);
	MAKE_ATTRIB (ship_component_flags);

	MAKE_ATTRIB (ship_component_booster_energy);
	MAKE_ATTRIB (ship_component_booster_energy_recharge_rate);
	MAKE_ATTRIB (ship_component_booster_energy_consumption_rate);
	MAKE_ATTRIB (ship_component_booster_acceleration);
	MAKE_ATTRIB (ship_component_booster_speed_maximum);

	MAKE_ATTRIB (ship_component_capacitor_energy);
	MAKE_ATTRIB (ship_component_capacitor_energy_recharge_rate);

	MAKE_ATTRIB (ship_component_droidinterface_speed);

	MAKE_ATTRIB (ship_component_engine_acceleration_rate);
	MAKE_ATTRIB (ship_component_engine_deceleration_rate);
	MAKE_ATTRIB (ship_component_engine_pitch_acceleration_rate);
	MAKE_ATTRIB (ship_component_engine_yaw_acceleration_rate);
	MAKE_ATTRIB (ship_component_engine_roll_acceleration_rate);
	MAKE_ATTRIB (ship_component_engine_pitch_rate_maximum);
	MAKE_ATTRIB (ship_component_engine_yaw_rate_maximum);
	MAKE_ATTRIB (ship_component_engine_roll_rate_maximum);
	MAKE_ATTRIB (ship_component_engine_speed_maximum);

	MAKE_ATTRIB (ship_component_reactor_generation_rate);

	MAKE_ATTRIB (ship_component_shield_hitpoints_front);
	MAKE_ATTRIB (ship_component_shield_hitpoints_back);
	MAKE_ATTRIB (ship_component_shield_recharge_rate);

	MAKE_ATTRIB (ship_component_weapon_damage);
	MAKE_ATTRIB (ship_component_weapon_effectiveness_shields);
	MAKE_ATTRIB (ship_component_weapon_effectiveness_armor);
	MAKE_ATTRIB (ship_component_weapon_energy_per_shot);
	MAKE_ATTRIB (ship_component_weapon_refire_rate);
	MAKE_ATTRIB (ship_component_weapon_ammo);

	MAKE_ATTRIB (ship_component_cargo_hold_contents_max);

	MAKE_ATTRIB (tooltip);

	MAKE_ATTRIB (tier);

#undef MAKE_ATTRIB

};

//======================================================================

#endif
