// ======================================================================
//
// Schema.h
// copyright (c) 2001 Sony Online Entertainment
//
// Edit the file Schema_h.template.  Do not edit Schema.h.
//
// ======================================================================

#ifndef INCLUDED_Schema_H
#define INCLUDED_Schema_H

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/BufferString.h"
#include "sharedDatabaseInterface/DbRow.h"

namespace DBSchema
{

struct PlayerObjectRow : public DB::Row
{
	enum {QUEST_DATA_SIZE=4000};

	DB::BindableNetworkId object_id;
	DB::BindableLong station_id;
	DB::BindableNetworkId house_id;
	DB::BindableLong account_num_lots;
	DB::BindableBool account_is_outcast;
	DB::BindableDouble account_cheater_level;
	DB::BindableLong account_max_lots_adjustment;
	DB::BindableString<200> personal_profile_id;
	DB::BindableString<200> character_profile_id;
	DB::BindableString<200> skill_title;
	DB::BindableLong born_date;
	DB::BindableLong played_time;
	DB::BindableDouble force_regen_rate;	
	DB::BindableLong force_power;
	DB::BindableLong max_force_power;
	DB::BindableBitArray<2047> active_quests;
	DB::BindableBitArray<2047> completed_quests;
	DB::BindableLong current_quest;
	DB::BindableString<QUEST_DATA_SIZE> quests;
	DB::BindableLong role_icon_choice;
	DB::BindableString<QUEST_DATA_SIZE> quests2;
	DB::BindableString<QUEST_DATA_SIZE> quests3;
	DB::BindableString<QUEST_DATA_SIZE> quests4;
	DB::BindableString<200> skill_template;
	DB::BindableString<200> working_skill;
	DB::BindableLong current_gcw_points;
	DB::BindableLong current_gcw_rating;
	DB::BindableLong current_pvp_kills;
	DB::BindableInt64 lifetime_gcw_points;
	DB::BindableLong max_gcw_imperial_rating;
	DB::BindableLong max_gcw_rebel_rating;
	DB::BindableLong lifetime_pvp_kills;
	DB::BindableLong next_gcw_rating_calc_time;
	DB::BindableBitArray<16000> collections;
	DB::BindableBool show_backpack;
	DB::BindableBool show_helmet;
	DB::BindableBitArray<16000> collections2;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const PlayerObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}

};

struct PlayerObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong station_id;
	DB::BindableNetworkId house_id;
	DB::BindableLong account_num_lots;
	DB::BindableBool account_is_outcast;
	DB::BindableDouble account_cheater_level;
	DB::BindableLong account_max_lots_adjustment;
	DB::BufferString personal_profile_id;
	DB::BufferString character_profile_id;
	DB::BufferString skill_title;
	DB::BindableLong born_date;
	DB::BindableLong played_time;
	DB::BindableDouble force_regen_rate;	
	DB::BindableLong force_power;	
	DB::BindableLong max_force_power;
	DB::BindableBitArray<2047> active_quests;
	DB::BindableBitArray<2047> completed_quests;
	DB::BindableLong current_quest;
	DB::BufferString quests;
	DB::BindableLong role_icon_choice;
	DB::BufferString quests2;
	DB::BufferString quests3;
	DB::BufferString quests4;
	DB::BufferString skill_template;
	DB::BufferString working_skill;
	DB::BindableLong current_gcw_points;
	DB::BindableLong current_gcw_rating;
	DB::BindableLong current_pvp_kills;
	DB::BindableInt64 lifetime_gcw_points;
	DB::BindableLong max_gcw_imperial_rating;
	DB::BindableLong max_gcw_rebel_rating;
	DB::BindableLong lifetime_pvp_kills;
	DB::BindableLong next_gcw_rating_calc_time;
	DB::BindableBitArray<16000> collections;
	DB::BindableBool show_backpack;
	DB::BindableBool show_helmet;
	DB::BindableBitArray<16000> collections2;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const PlayerObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}

	PlayerObjectBufferRow (const PlayerObjectRow & rhs) :
			object_id (rhs.object_id),
			station_id (rhs.station_id),
			house_id (rhs.house_id),
			account_num_lots (rhs.account_num_lots),
			account_is_outcast (rhs.account_is_outcast),
			account_cheater_level (rhs.account_cheater_level),
			account_max_lots_adjustment (rhs.account_max_lots_adjustment),
			personal_profile_id (rhs.personal_profile_id),
			character_profile_id (rhs.character_profile_id),
			skill_title (rhs.skill_title),
			born_date (rhs.born_date),
			played_time (rhs.played_time),
			force_regen_rate (rhs.force_regen_rate),
			force_power (rhs.force_power),
			max_force_power (rhs.max_force_power),
			active_quests (rhs.active_quests),
			completed_quests (rhs.completed_quests),
			current_quest (rhs.current_quest),
			quests (rhs.quests),
			role_icon_choice (rhs.role_icon_choice),
			quests2 (rhs.quests2),
			quests3 (rhs.quests3),
			quests4 (rhs.quests4),
			skill_template (rhs.skill_template),
			working_skill (rhs.working_skill),
			current_gcw_points (rhs.current_gcw_points),
			current_gcw_rating (rhs.current_gcw_rating),
			current_pvp_kills (rhs.current_pvp_kills),
			lifetime_gcw_points (rhs.lifetime_gcw_points),
			max_gcw_imperial_rating (rhs.max_gcw_imperial_rating),
			max_gcw_rebel_rating (rhs.max_gcw_rebel_rating),
			lifetime_pvp_kills (rhs.lifetime_pvp_kills),
			next_gcw_rating_calc_time (rhs.next_gcw_rating_calc_time),
			collections (rhs.collections),
			show_backpack(rhs.show_backpack),
			show_helmet(rhs.show_helmet),
			collections2 (rhs.collections2)
	{
	}

	PlayerObjectBufferRow() :
			object_id(),
			station_id(),
			house_id(),
			account_num_lots(),
			account_is_outcast(),
			account_cheater_level(),
			account_max_lots_adjustment(),
			personal_profile_id(200),
			character_profile_id(200),
			skill_title(200),
			born_date(),
			played_time(),
			force_regen_rate(),	
			force_power(),	
			max_force_power(),
			active_quests(),
			completed_quests(),
			current_quest(),
			quests(PlayerObjectRow::QUEST_DATA_SIZE),
			role_icon_choice(),
			quests2(PlayerObjectRow::QUEST_DATA_SIZE),
			quests3(PlayerObjectRow::QUEST_DATA_SIZE),
			quests4(PlayerObjectRow::QUEST_DATA_SIZE),
			skill_template(200),
			working_skill(200),
			current_gcw_points(),
			current_gcw_rating(),
			current_pvp_kills(),
			lifetime_gcw_points(),
			max_gcw_imperial_rating(),
			max_gcw_rebel_rating(),
			lifetime_pvp_kills(),
			next_gcw_rating_calc_time(),
			collections(),
			show_backpack(),
			show_helmet(),
			collections2()
	{
	}
	
};

struct ObjectVariableRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong name_id;
	DB::BindableLong type;
	DB::BindableString<1000> value;
	DB::BindableLong detached;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ObjectVariableRow&>(rhs);
	}
};

struct WaypointRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableNetworkId waypoint_id;
	DB::BindableLong appearance_name_crc;
	DB::BindableDouble location_x;
	DB::BindableDouble location_y;
	DB::BindableDouble location_z;
	DB::BindableNetworkId location_cell;
	DB::BindableLong location_scene;
	DB::BindableUnicode<512> name;
	DB::BindableLong color;
	DB::BindableBool active;
	DB::BindableBool detached;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const WaypointRow&>(rhs);
	}
};

//!!!BEGIN GENERATED DECLARATIONS
struct ArmorRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong layer;
	DB::BindableLong effectiveness;
	DB::BindableLong integrity;
	DB::BindableString<1000> special_protections;
	DB::BindableLong encumberance_0;
	DB::BindableLong encumberance_1;
	DB::BindableLong encumberance_2;
	DB::BindableLong encumberance_3;
	DB::BindableLong encumberance_4;
	DB::BindableLong encumberance_5;
	DB::BindableLong encumberance_6;
	DB::BindableLong encumberance_7;
	DB::BindableLong encumberance_8;
	DB::BindableString<255> object_template;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ArmorRow&>(rhs);
	}
};

struct ArmorBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong layer;
	DB::BindableLong effectiveness;
	DB::BindableLong integrity;
	DB::BufferString special_protections;
	DB::BindableLong encumberance_0;
	DB::BindableLong encumberance_1;
	DB::BindableLong encumberance_2;
	DB::BindableLong encumberance_3;
	DB::BindableLong encumberance_4;
	DB::BindableLong encumberance_5;
	DB::BindableLong encumberance_6;
	DB::BindableLong encumberance_7;
	DB::BindableLong encumberance_8;
	DB::BufferString object_template;

	ArmorBufferRow() :
		object_id(),
		layer(),
		effectiveness(),
		integrity(),
		special_protections(1000),
		encumberance_0(),
		encumberance_1(),
		encumberance_2(),
		encumberance_3(),
		encumberance_4(),
		encumberance_5(),
		encumberance_6(),
		encumberance_7(),
		encumberance_8(),
		object_template(255)
	{
	}

	ArmorBufferRow(const ArmorRow & rhs) :
		object_id(rhs.object_id),
		layer(rhs.layer),
		effectiveness(rhs.effectiveness),
		integrity(rhs.integrity),
		special_protections(rhs.special_protections),
		encumberance_0(rhs.encumberance_0),
		encumberance_1(rhs.encumberance_1),
		encumberance_2(rhs.encumberance_2),
		encumberance_3(rhs.encumberance_3),
		encumberance_4(rhs.encumberance_4),
		encumberance_5(rhs.encumberance_5),
		encumberance_6(rhs.encumberance_6),
		encumberance_7(rhs.encumberance_7),
		encumberance_8(rhs.encumberance_8),
		object_template(rhs.object_template)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ArmorBufferRow&>(rhs);
	}
};

struct BattlefieldMarkerObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableString<500> region_name;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const BattlefieldMarkerObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct BattlefieldMarkerObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BufferString region_name;

	BattlefieldMarkerObjectBufferRow() :
		object_id(),
		region_name(500)
	{
	}

	BattlefieldMarkerObjectBufferRow(const BattlefieldMarkerObjectRow & rhs) :
		object_id(rhs.object_id),
		region_name(rhs.region_name)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const BattlefieldMarkerObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct BattlefieldParticipantRow : public DB::Row
{
	DB::BindableNetworkId region_object_id;
	DB::BindableNetworkId character_object_id;
	DB::BindableLong faction_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const BattlefieldParticipantRow&>(rhs);
	}
};

struct BattlefieldParticipantBufferRow : public DB::Row
{
	DB::BindableNetworkId region_object_id;
	DB::BindableNetworkId character_object_id;
	DB::BindableLong faction_id;

	BattlefieldParticipantBufferRow() :
		region_object_id(),
		character_object_id(),
		faction_id()
	{
	}

	BattlefieldParticipantBufferRow(const BattlefieldParticipantRow & rhs) :
		region_object_id(rhs.region_object_id),
		character_object_id(rhs.character_object_id),
		faction_id(rhs.faction_id)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const BattlefieldParticipantBufferRow&>(rhs);
	}
};

struct BountyHunterTargetRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableNetworkId target_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const BountyHunterTargetRow&>(rhs);
	}
};

struct BountyHunterTargetBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableNetworkId target_id;

	BountyHunterTargetBufferRow() :
		object_id(),
		target_id()
	{
	}

	BountyHunterTargetBufferRow(const BountyHunterTargetRow & rhs) :
		object_id(rhs.object_id),
		target_id(rhs.target_id)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const BountyHunterTargetBufferRow&>(rhs);
	}
};

struct BuildingObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong maintenance_cost;
	DB::BindableDouble time_last_checked;
	DB::BindableBool is_public;
	DB::BindableLong city_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const BuildingObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct BuildingObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong maintenance_cost;
	DB::BindableDouble time_last_checked;
	DB::BindableBool is_public;
	DB::BindableLong city_id;

	BuildingObjectBufferRow() :
		object_id(),
		maintenance_cost(),
		time_last_checked(),
		is_public(),
		city_id()
	{
	}

	BuildingObjectBufferRow(const BuildingObjectRow & rhs) :
		object_id(rhs.object_id),
		maintenance_cost(rhs.maintenance_cost),
		time_last_checked(rhs.time_last_checked),
		is_public(rhs.is_public),
		city_id(rhs.city_id)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const BuildingObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct CellObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong cell_number;
	DB::BindableBool is_public;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const CellObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct CellObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong cell_number;
	DB::BindableBool is_public;

	CellObjectBufferRow() :
		object_id(),
		cell_number(),
		is_public()
	{
	}

	CellObjectBufferRow(const CellObjectRow & rhs) :
		object_id(rhs.object_id),
		cell_number(rhs.cell_number),
		is_public(rhs.is_public)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const CellObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct CityObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const CityObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct CityObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;

	CityObjectBufferRow() :
		object_id()
	{
	}

	CityObjectBufferRow(const CityObjectRow & rhs) :
		object_id(rhs.object_id)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const CityObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct ClockRow : public DB::Row
{
	DB::BindableLong last_save_time;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ClockRow&>(rhs);
	}
};

struct ClockBufferRow : public DB::Row
{
	DB::BindableLong last_save_time;

	ClockBufferRow() :
		last_save_time()
	{
	}

	ClockBufferRow(const ClockRow & rhs) :
		last_save_time(rhs.last_save_time)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ClockBufferRow&>(rhs);
	}
};

struct CreatureObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableDouble scale_factor;
	DB::BindableInt64 states;
	DB::BindableLong posture;
	DB::BindableLong shock_wounds;
	DB::BindableNetworkId master_id;
	DB::BindableLong rank;
	DB::BindableDouble base_walk_speed;
	DB::BindableDouble base_run_speed;
	DB::BindableLong attribute_0;
	DB::BindableLong attribute_1;
	DB::BindableLong attribute_2;
	DB::BindableLong attribute_3;
	DB::BindableLong attribute_4;
	DB::BindableLong attribute_5;
	DB::BindableLong attribute_6;
	DB::BindableLong attribute_7;
	DB::BindableLong attribute_8;
	DB::BindableLong attribute_9;
	DB::BindableLong attribute_10;
	DB::BindableLong attribute_11;
	DB::BindableLong attribute_12;
	DB::BindableLong attribute_13;
	DB::BindableLong attribute_14;
	DB::BindableLong attribute_15;
	DB::BindableLong attribute_16;
	DB::BindableLong attribute_17;
	DB::BindableLong attribute_18;
	DB::BindableLong attribute_19;
	DB::BindableLong attribute_20;
	DB::BindableLong attribute_21;
	DB::BindableLong attribute_22;
	DB::BindableLong attribute_23;
	DB::BindableLong attribute_24;
	DB::BindableLong attribute_25;
	DB::BindableLong attribute_26;
	DB::BindableString<1000> persisted_buffs;
	DB::BindableDouble ws_x;
	DB::BindableDouble ws_y;
	DB::BindableDouble ws_z;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const CreatureObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct CreatureObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableDouble scale_factor;
	DB::BindableInt64 states;
	DB::BindableLong posture;
	DB::BindableLong shock_wounds;
	DB::BindableNetworkId master_id;
	DB::BindableLong rank;
	DB::BindableDouble base_walk_speed;
	DB::BindableDouble base_run_speed;
	DB::BindableLong attribute_0;
	DB::BindableLong attribute_1;
	DB::BindableLong attribute_2;
	DB::BindableLong attribute_3;
	DB::BindableLong attribute_4;
	DB::BindableLong attribute_5;
	DB::BindableLong attribute_6;
	DB::BindableLong attribute_7;
	DB::BindableLong attribute_8;
	DB::BindableLong attribute_9;
	DB::BindableLong attribute_10;
	DB::BindableLong attribute_11;
	DB::BindableLong attribute_12;
	DB::BindableLong attribute_13;
	DB::BindableLong attribute_14;
	DB::BindableLong attribute_15;
	DB::BindableLong attribute_16;
	DB::BindableLong attribute_17;
	DB::BindableLong attribute_18;
	DB::BindableLong attribute_19;
	DB::BindableLong attribute_20;
	DB::BindableLong attribute_21;
	DB::BindableLong attribute_22;
	DB::BindableLong attribute_23;
	DB::BindableLong attribute_24;
	DB::BindableLong attribute_25;
	DB::BindableLong attribute_26;
	DB::BufferString persisted_buffs;
	DB::BindableDouble ws_x;
	DB::BindableDouble ws_y;
	DB::BindableDouble ws_z;

	CreatureObjectBufferRow() :
		object_id(),
		scale_factor(),
		states(),
		posture(),
		shock_wounds(),
		master_id(),
		rank(),
		base_walk_speed(),
		base_run_speed(),
		attribute_0(),
		attribute_1(),
		attribute_2(),
		attribute_3(),
		attribute_4(),
		attribute_5(),
		attribute_6(),
		attribute_7(),
		attribute_8(),
		attribute_9(),
		attribute_10(),
		attribute_11(),
		attribute_12(),
		attribute_13(),
		attribute_14(),
		attribute_15(),
		attribute_16(),
		attribute_17(),
		attribute_18(),
		attribute_19(),
		attribute_20(),
		attribute_21(),
		attribute_22(),
		attribute_23(),
		attribute_24(),
		attribute_25(),
		attribute_26(),
		persisted_buffs(1000),
		ws_x(),
		ws_y(),
		ws_z()
	{
	}

	CreatureObjectBufferRow(const CreatureObjectRow & rhs) :
		object_id(rhs.object_id),
		scale_factor(rhs.scale_factor),
		states(rhs.states),
		posture(rhs.posture),
		shock_wounds(rhs.shock_wounds),
		master_id(rhs.master_id),
		rank(rhs.rank),
		base_walk_speed(rhs.base_walk_speed),
		base_run_speed(rhs.base_run_speed),
		attribute_0(rhs.attribute_0),
		attribute_1(rhs.attribute_1),
		attribute_2(rhs.attribute_2),
		attribute_3(rhs.attribute_3),
		attribute_4(rhs.attribute_4),
		attribute_5(rhs.attribute_5),
		attribute_6(rhs.attribute_6),
		attribute_7(rhs.attribute_7),
		attribute_8(rhs.attribute_8),
		attribute_9(rhs.attribute_9),
		attribute_10(rhs.attribute_10),
		attribute_11(rhs.attribute_11),
		attribute_12(rhs.attribute_12),
		attribute_13(rhs.attribute_13),
		attribute_14(rhs.attribute_14),
		attribute_15(rhs.attribute_15),
		attribute_16(rhs.attribute_16),
		attribute_17(rhs.attribute_17),
		attribute_18(rhs.attribute_18),
		attribute_19(rhs.attribute_19),
		attribute_20(rhs.attribute_20),
		attribute_21(rhs.attribute_21),
		attribute_22(rhs.attribute_22),
		attribute_23(rhs.attribute_23),
		attribute_24(rhs.attribute_24),
		attribute_25(rhs.attribute_25),
		attribute_26(rhs.attribute_26),
		persisted_buffs(rhs.persisted_buffs),
		ws_x(rhs.ws_x),
		ws_y(rhs.ws_y),
		ws_z(rhs.ws_z)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const CreatureObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct ExperiencePointRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableString<500> experience_type;
	DB::BindableLong points;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ExperiencePointRow&>(rhs);
	}
};

struct ExperiencePointBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BufferString experience_type;
	DB::BindableLong points;

	ExperiencePointBufferRow() :
		object_id(),
		experience_type(500),
		points()
	{
	}

	ExperiencePointBufferRow(const ExperiencePointRow & rhs) :
		object_id(rhs.object_id),
		experience_type(rhs.experience_type),
		points(rhs.points)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ExperiencePointBufferRow&>(rhs);
	}
};

struct FactoryObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const FactoryObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct FactoryObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;

	FactoryObjectBufferRow() :
		object_id()
	{
	}

	FactoryObjectBufferRow(const FactoryObjectRow & rhs) :
		object_id(rhs.object_id)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const FactoryObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct FreeObjectIdRow : public DB::Row
{
	DB::BindableLong start_id;
	DB::BindableLong end_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const FreeObjectIdRow&>(rhs);
	}
};

struct FreeObjectIdBufferRow : public DB::Row
{
	DB::BindableLong start_id;
	DB::BindableLong end_id;

	FreeObjectIdBufferRow() :
		start_id(),
		end_id()
	{
	}

	FreeObjectIdBufferRow(const FreeObjectIdRow & rhs) :
		start_id(rhs.start_id),
		end_id(rhs.end_id)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const FreeObjectIdBufferRow&>(rhs);
	}
};

struct GuildObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const GuildObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct GuildObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;

	GuildObjectBufferRow() :
		object_id()
	{
	}

	GuildObjectBufferRow(const GuildObjectRow & rhs) :
		object_id(rhs.object_id)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const GuildObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct HarvesterInstallationObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableDouble installed_efficiency;
	DB::BindableLong max_extraction_rate;
	DB::BindableDouble current_extraction_rate;
	DB::BindableLong max_hopper_amount;
	DB::BindableNetworkId hopper_resource;
	DB::BindableDouble hopper_amount;
	DB::BindableNetworkId resource_type;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const HarvesterInstallationObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct HarvesterInstallationObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableDouble installed_efficiency;
	DB::BindableLong max_extraction_rate;
	DB::BindableDouble current_extraction_rate;
	DB::BindableLong max_hopper_amount;
	DB::BindableNetworkId hopper_resource;
	DB::BindableDouble hopper_amount;
	DB::BindableNetworkId resource_type;

	HarvesterInstallationObjectBufferRow() :
		object_id(),
		installed_efficiency(),
		max_extraction_rate(),
		current_extraction_rate(),
		max_hopper_amount(),
		hopper_resource(),
		hopper_amount(),
		resource_type()
	{
	}

	HarvesterInstallationObjectBufferRow(const HarvesterInstallationObjectRow & rhs) :
		object_id(rhs.object_id),
		installed_efficiency(rhs.installed_efficiency),
		max_extraction_rate(rhs.max_extraction_rate),
		current_extraction_rate(rhs.current_extraction_rate),
		max_hopper_amount(rhs.max_hopper_amount),
		hopper_resource(rhs.hopper_resource),
		hopper_amount(rhs.hopper_amount),
		resource_type(rhs.resource_type)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const HarvesterInstallationObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct InstallationObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong installation_type;
	DB::BindableBool activated;
	DB::BindableDouble tick_count;
	DB::BindableDouble activate_start_time;
	DB::BindableDouble power;
	DB::BindableDouble power_rate;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const InstallationObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct InstallationObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong installation_type;
	DB::BindableBool activated;
	DB::BindableDouble tick_count;
	DB::BindableDouble activate_start_time;
	DB::BindableDouble power;
	DB::BindableDouble power_rate;

	InstallationObjectBufferRow() :
		object_id(),
		installation_type(),
		activated(),
		tick_count(),
		activate_start_time(),
		power(),
		power_rate()
	{
	}

	InstallationObjectBufferRow(const InstallationObjectRow & rhs) :
		object_id(rhs.object_id),
		installation_type(rhs.installation_type),
		activated(rhs.activated),
		tick_count(rhs.tick_count),
		activate_start_time(rhs.activate_start_time),
		power(rhs.power),
		power_rate(rhs.power_rate)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const InstallationObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct IntangibleObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong count;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const IntangibleObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct IntangibleObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong count;

	IntangibleObjectBufferRow() :
		object_id(),
		count()
	{
	}

	IntangibleObjectBufferRow(const IntangibleObjectRow & rhs) :
		object_id(rhs.object_id),
		count(rhs.count)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const IntangibleObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct LoadbeaconServerMapRow : public DB::Row
{
	DB::BindableString<255> object_template;
	DB::BindableLong server_id;
	DB::BindableLong secondary_server_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const LoadbeaconServerMapRow&>(rhs);
	}
};

struct LoadbeaconServerMapBufferRow : public DB::Row
{
	DB::BufferString object_template;
	DB::BindableLong server_id;
	DB::BindableLong secondary_server_id;

	LoadbeaconServerMapBufferRow() :
		object_template(255),
		server_id(),
		secondary_server_id()
	{
	}

	LoadbeaconServerMapBufferRow(const LoadbeaconServerMapRow & rhs) :
		object_template(rhs.object_template),
		server_id(rhs.server_id),
		secondary_server_id(rhs.secondary_server_id)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const LoadbeaconServerMapBufferRow&>(rhs);
	}
};

struct ManufactureInstallationObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ManufactureInstallationObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct ManufactureInstallationObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;

	ManufactureInstallationObjectBufferRow() :
		object_id()
	{
	}

	ManufactureInstallationObjectBufferRow(const ManufactureInstallationObjectRow & rhs) :
		object_id(rhs.object_id)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ManufactureInstallationObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct ManufactureSchematicObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableNetworkId creator_id;
	DB::BindableString<127> creator_name;
	DB::BindableLong items_per_container;
	DB::BindableDouble manufacture_time;
	DB::BindableLong draft_schematic;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ManufactureSchematicObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct ManufactureSchematicObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableNetworkId creator_id;
	DB::BufferString creator_name;
	DB::BindableLong items_per_container;
	DB::BindableDouble manufacture_time;
	DB::BindableLong draft_schematic;

	ManufactureSchematicObjectBufferRow() :
		object_id(),
		creator_id(),
		creator_name(127),
		items_per_container(),
		manufacture_time(),
		draft_schematic()
	{
	}

	ManufactureSchematicObjectBufferRow(const ManufactureSchematicObjectRow & rhs) :
		object_id(rhs.object_id),
		creator_id(rhs.creator_id),
		creator_name(rhs.creator_name),
		items_per_container(rhs.items_per_container),
		manufacture_time(rhs.manufacture_time),
		draft_schematic(rhs.draft_schematic)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ManufactureSchematicObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct MessageRow : public DB::Row
{
	DB::BindableNetworkId message_id;
	DB::BindableNetworkId target;
	DB::BindableString<50> method;
	DB::BindableString<4000> data;
	DB::BindableLong call_time;
	DB::BindableBool guaranteed;
	DB::BindableLong delivery_type;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const MessageRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		message_id.setValue(keyValue);
	}
};

struct MessageBufferRow : public DB::Row
{
	DB::BindableNetworkId message_id;
	DB::BindableNetworkId target;
	DB::BufferString method;
	DB::BufferString data;
	DB::BindableLong call_time;
	DB::BindableBool guaranteed;
	DB::BindableLong delivery_type;

	MessageBufferRow() :
		message_id(),
		target(),
		method(50),
		data(4000),
		call_time(),
		guaranteed(),
		delivery_type()
	{
	}

	MessageBufferRow(const MessageRow & rhs) :
		message_id(rhs.message_id),
		target(rhs.target),
		method(rhs.method),
		data(rhs.data),
		call_time(rhs.call_time),
		guaranteed(rhs.guaranteed),
		delivery_type(rhs.delivery_type)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const MessageBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		message_id.setValue(keyValue);
	}
};

struct MissionObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong difficulty;
	DB::BindableDouble end_x;
	DB::BindableDouble end_y;
	DB::BindableDouble end_z;
	DB::BindableNetworkId end_cell;
	DB::BindableLong end_scene;
	DB::BindableString<127> mission_creator;
	DB::BindableLong reward;
	DB::BindableString<100> root_script_name;
	DB::BindableDouble start_x;
	DB::BindableDouble start_y;
	DB::BindableDouble start_z;
	DB::BindableNetworkId start_cell;
	DB::BindableLong start_scene;
	DB::BindableString<100> description_table;
	DB::BindableString<100> description_text;
	DB::BindableString<100> title_table;
	DB::BindableString<100> title_text;
	DB::BindableNetworkId mission_holder_id;
	DB::BindableLong status;
	DB::BindableLong mission_type;
	DB::BindableLong target_appearance;
	DB::BindableString<127> target_name;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const MissionObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct MissionObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong difficulty;
	DB::BindableDouble end_x;
	DB::BindableDouble end_y;
	DB::BindableDouble end_z;
	DB::BindableNetworkId end_cell;
	DB::BindableLong end_scene;
	DB::BufferString mission_creator;
	DB::BindableLong reward;
	DB::BufferString root_script_name;
	DB::BindableDouble start_x;
	DB::BindableDouble start_y;
	DB::BindableDouble start_z;
	DB::BindableNetworkId start_cell;
	DB::BindableLong start_scene;
	DB::BufferString description_table;
	DB::BufferString description_text;
	DB::BufferString title_table;
	DB::BufferString title_text;
	DB::BindableNetworkId mission_holder_id;
	DB::BindableLong status;
	DB::BindableLong mission_type;
	DB::BindableLong target_appearance;
	DB::BufferString target_name;

	MissionObjectBufferRow() :
		object_id(),
		difficulty(),
		end_x(),
		end_y(),
		end_z(),
		end_cell(),
		end_scene(),
		mission_creator(127),
		reward(),
		root_script_name(100),
		start_x(),
		start_y(),
		start_z(),
		start_cell(),
		start_scene(),
		description_table(100),
		description_text(100),
		title_table(100),
		title_text(100),
		mission_holder_id(),
		status(),
		mission_type(),
		target_appearance(),
		target_name(127)
	{
	}

	MissionObjectBufferRow(const MissionObjectRow & rhs) :
		object_id(rhs.object_id),
		difficulty(rhs.difficulty),
		end_x(rhs.end_x),
		end_y(rhs.end_y),
		end_z(rhs.end_z),
		end_cell(rhs.end_cell),
		end_scene(rhs.end_scene),
		mission_creator(rhs.mission_creator),
		reward(rhs.reward),
		root_script_name(rhs.root_script_name),
		start_x(rhs.start_x),
		start_y(rhs.start_y),
		start_z(rhs.start_z),
		start_cell(rhs.start_cell),
		start_scene(rhs.start_scene),
		description_table(rhs.description_table),
		description_text(rhs.description_text),
		title_table(rhs.title_table),
		title_text(rhs.title_text),
		mission_holder_id(rhs.mission_holder_id),
		status(rhs.status),
		mission_type(rhs.mission_type),
		target_appearance(rhs.target_appearance),
		target_name(rhs.target_name)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const MissionObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct ObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableDouble x;
	DB::BindableDouble y;
	DB::BindableDouble z;
	DB::BindableDouble quaternion_w;
	DB::BindableDouble quaternion_x;
	DB::BindableDouble quaternion_y;
	DB::BindableDouble quaternion_z;
	DB::BindableLong node_x;
	DB::BindableLong node_y;
	DB::BindableLong node_z;
	DB::BindableLong type_id;
	DB::BindableString<50> scene_id;
	DB::BindableLong controller_type;
	DB::BindableLong deleted;
	DB::BindableString<127> object_name;
	DB::BindableLong volume;
	DB::BindableNetworkId contained_by;
	DB::BindableLong slot_arrangement;
	DB::BindableBool player_controlled;
	DB::BindableLong cache_version;
	DB::BindableBool load_contents;
	DB::BindableLong cash_balance;
	DB::BindableLong bank_balance;
	DB::BindableDouble complexity;
	DB::BindableString<500> name_string_table;
	DB::BindableString<500> name_string_text;
	DB::BindableLong object_template_id;
	DB::BindableString<128> static_item_name;
	DB::BindableLong static_item_version;
	DB::BindableLong conversion_id;
	DB::BindableNetworkId load_with;
	DB::BindableString<50> objvar_0_name;
	DB::BindableLong objvar_0_type;
	DB::BindableString<50> objvar_0_value;
	DB::BindableString<50> objvar_1_name;
	DB::BindableLong objvar_1_type;
	DB::BindableString<50> objvar_1_value;
	DB::BindableString<50> objvar_2_name;
	DB::BindableLong objvar_2_type;
	DB::BindableString<50> objvar_2_value;
	DB::BindableString<50> objvar_3_name;
	DB::BindableLong objvar_3_type;
	DB::BindableString<50> objvar_3_value;
	DB::BindableString<50> objvar_4_name;
	DB::BindableLong objvar_4_type;
	DB::BindableString<50> objvar_4_value;
	DB::BindableString<50> objvar_5_name;
	DB::BindableLong objvar_5_type;
	DB::BindableString<50> objvar_5_value;
	DB::BindableString<50> objvar_6_name;
	DB::BindableLong objvar_6_type;
	DB::BindableString<50> objvar_6_value;
	DB::BindableString<50> objvar_7_name;
	DB::BindableLong objvar_7_type;
	DB::BindableString<50> objvar_7_value;
	DB::BindableString<50> objvar_8_name;
	DB::BindableLong objvar_8_type;
	DB::BindableString<50> objvar_8_value;
	DB::BindableString<50> objvar_9_name;
	DB::BindableLong objvar_9_type;
	DB::BindableString<50> objvar_9_value;
	DB::BindableString<50> objvar_10_name;
	DB::BindableLong objvar_10_type;
	DB::BindableString<50> objvar_10_value;
	DB::BindableString<50> objvar_11_name;
	DB::BindableLong objvar_11_type;
	DB::BindableString<50> objvar_11_value;
	DB::BindableString<50> objvar_12_name;
	DB::BindableLong objvar_12_type;
	DB::BindableString<50> objvar_12_value;
	DB::BindableString<50> objvar_13_name;
	DB::BindableLong objvar_13_type;
	DB::BindableString<50> objvar_13_value;
	DB::BindableString<50> objvar_14_name;
	DB::BindableLong objvar_14_type;
	DB::BindableString<50> objvar_14_value;
	DB::BindableString<50> objvar_15_name;
	DB::BindableLong objvar_15_type;
	DB::BindableString<50> objvar_15_value;
	DB::BindableString<50> objvar_16_name;
	DB::BindableLong objvar_16_type;
	DB::BindableString<50> objvar_16_value;
	DB::BindableString<50> objvar_17_name;
	DB::BindableLong objvar_17_type;
	DB::BindableString<50> objvar_17_value;
	DB::BindableString<50> objvar_18_name;
	DB::BindableLong objvar_18_type;
	DB::BindableString<50> objvar_18_value;
	DB::BindableString<50> objvar_19_name;
	DB::BindableLong objvar_19_type;
	DB::BindableString<50> objvar_19_value;
	DB::BindableString<2000> script_list;

	DB::BindableLong container_level;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct ObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableDouble x;
	DB::BindableDouble y;
	DB::BindableDouble z;
	DB::BindableDouble quaternion_w;
	DB::BindableDouble quaternion_x;
	DB::BindableDouble quaternion_y;
	DB::BindableDouble quaternion_z;
	DB::BindableLong node_x;
	DB::BindableLong node_y;
	DB::BindableLong node_z;
	DB::BindableLong type_id;
	DB::BufferString scene_id;
	DB::BindableLong controller_type;
	DB::BindableLong deleted;
	DB::BufferString object_name;
	DB::BindableLong volume;
	DB::BindableNetworkId contained_by;
	DB::BindableLong slot_arrangement;
	DB::BindableBool player_controlled;
	DB::BindableLong cache_version;
	DB::BindableBool load_contents;
	DB::BindableLong cash_balance;
	DB::BindableLong bank_balance;
	DB::BindableDouble complexity;
	DB::BufferString name_string_table;
	DB::BufferString name_string_text;
	DB::BindableLong object_template_id;
	DB::BufferString static_item_name;
	DB::BindableLong static_item_version;
	DB::BindableLong conversion_id;
	DB::BindableNetworkId load_with;
	DB::BufferString objvar_0_name;
	DB::BindableLong objvar_0_type;
	DB::BufferString objvar_0_value;
	DB::BufferString objvar_1_name;
	DB::BindableLong objvar_1_type;
	DB::BufferString objvar_1_value;
	DB::BufferString objvar_2_name;
	DB::BindableLong objvar_2_type;
	DB::BufferString objvar_2_value;
	DB::BufferString objvar_3_name;
	DB::BindableLong objvar_3_type;
	DB::BufferString objvar_3_value;
	DB::BufferString objvar_4_name;
	DB::BindableLong objvar_4_type;
	DB::BufferString objvar_4_value;
	DB::BufferString objvar_5_name;
	DB::BindableLong objvar_5_type;
	DB::BufferString objvar_5_value;
	DB::BufferString objvar_6_name;
	DB::BindableLong objvar_6_type;
	DB::BufferString objvar_6_value;
	DB::BufferString objvar_7_name;
	DB::BindableLong objvar_7_type;
	DB::BufferString objvar_7_value;
	DB::BufferString objvar_8_name;
	DB::BindableLong objvar_8_type;
	DB::BufferString objvar_8_value;
	DB::BufferString objvar_9_name;
	DB::BindableLong objvar_9_type;
	DB::BufferString objvar_9_value;
	DB::BufferString objvar_10_name;
	DB::BindableLong objvar_10_type;
	DB::BufferString objvar_10_value;
	DB::BufferString objvar_11_name;
	DB::BindableLong objvar_11_type;
	DB::BufferString objvar_11_value;
	DB::BufferString objvar_12_name;
	DB::BindableLong objvar_12_type;
	DB::BufferString objvar_12_value;
	DB::BufferString objvar_13_name;
	DB::BindableLong objvar_13_type;
	DB::BufferString objvar_13_value;
	DB::BufferString objvar_14_name;
	DB::BindableLong objvar_14_type;
	DB::BufferString objvar_14_value;
	DB::BufferString objvar_15_name;
	DB::BindableLong objvar_15_type;
	DB::BufferString objvar_15_value;
	DB::BufferString objvar_16_name;
	DB::BindableLong objvar_16_type;
	DB::BufferString objvar_16_value;
	DB::BufferString objvar_17_name;
	DB::BindableLong objvar_17_type;
	DB::BufferString objvar_17_value;
	DB::BufferString objvar_18_name;
	DB::BindableLong objvar_18_type;
	DB::BufferString objvar_18_value;
	DB::BufferString objvar_19_name;
	DB::BindableLong objvar_19_type;
	DB::BufferString objvar_19_value;
	DB::BufferString script_list;

	ObjectBufferRow() :
		object_id(),
		x(),
		y(),
		z(),
		quaternion_w(),
		quaternion_x(),
		quaternion_y(),
		quaternion_z(),
		node_x(),
		node_y(),
		node_z(),
		type_id(),
		scene_id(50),
		controller_type(),
		deleted(),
		object_name(127),
		volume(),
		contained_by(),
		slot_arrangement(),
		player_controlled(),
		cache_version(),
		load_contents(),
		cash_balance(),
		bank_balance(),
		complexity(),
		name_string_table(500),
		name_string_text(500),
		object_template_id(),
		static_item_name(128),
		static_item_version(),
		conversion_id(),
		load_with(),
		objvar_0_name(50),
		objvar_0_type(),
		objvar_0_value(50),
		objvar_1_name(50),
		objvar_1_type(),
		objvar_1_value(50),
		objvar_2_name(50),
		objvar_2_type(),
		objvar_2_value(50),
		objvar_3_name(50),
		objvar_3_type(),
		objvar_3_value(50),
		objvar_4_name(50),
		objvar_4_type(),
		objvar_4_value(50),
		objvar_5_name(50),
		objvar_5_type(),
		objvar_5_value(50),
		objvar_6_name(50),
		objvar_6_type(),
		objvar_6_value(50),
		objvar_7_name(50),
		objvar_7_type(),
		objvar_7_value(50),
		objvar_8_name(50),
		objvar_8_type(),
		objvar_8_value(50),
		objvar_9_name(50),
		objvar_9_type(),
		objvar_9_value(50),
		objvar_10_name(50),
		objvar_10_type(),
		objvar_10_value(50),
		objvar_11_name(50),
		objvar_11_type(),
		objvar_11_value(50),
		objvar_12_name(50),
		objvar_12_type(),
		objvar_12_value(50),
		objvar_13_name(50),
		objvar_13_type(),
		objvar_13_value(50),
		objvar_14_name(50),
		objvar_14_type(),
		objvar_14_value(50),
		objvar_15_name(50),
		objvar_15_type(),
		objvar_15_value(50),
		objvar_16_name(50),
		objvar_16_type(),
		objvar_16_value(50),
		objvar_17_name(50),
		objvar_17_type(),
		objvar_17_value(50),
		objvar_18_name(50),
		objvar_18_type(),
		objvar_18_value(50),
		objvar_19_name(50),
		objvar_19_type(),
		objvar_19_value(50),
		script_list(2000)
	{
	}

	ObjectBufferRow(const ObjectRow & rhs) :
		object_id(rhs.object_id),
		x(rhs.x),
		y(rhs.y),
		z(rhs.z),
		quaternion_w(rhs.quaternion_w),
		quaternion_x(rhs.quaternion_x),
		quaternion_y(rhs.quaternion_y),
		quaternion_z(rhs.quaternion_z),
		node_x(rhs.node_x),
		node_y(rhs.node_y),
		node_z(rhs.node_z),
		type_id(rhs.type_id),
		scene_id(rhs.scene_id),
		controller_type(rhs.controller_type),
		deleted(rhs.deleted),
		object_name(rhs.object_name),
		volume(rhs.volume),
		contained_by(rhs.contained_by),
		slot_arrangement(rhs.slot_arrangement),
		player_controlled(rhs.player_controlled),
		cache_version(rhs.cache_version),
		load_contents(rhs.load_contents),
		cash_balance(rhs.cash_balance),
		bank_balance(rhs.bank_balance),
		complexity(rhs.complexity),
		name_string_table(rhs.name_string_table),
		name_string_text(rhs.name_string_text),
		object_template_id(rhs.object_template_id),
		static_item_name(rhs.static_item_name),
		static_item_version(rhs.static_item_version),
		conversion_id(rhs.conversion_id),
		load_with(rhs.load_with),
		objvar_0_name(rhs.objvar_0_name),
		objvar_0_type(rhs.objvar_0_type),
		objvar_0_value(rhs.objvar_0_value),
		objvar_1_name(rhs.objvar_1_name),
		objvar_1_type(rhs.objvar_1_type),
		objvar_1_value(rhs.objvar_1_value),
		objvar_2_name(rhs.objvar_2_name),
		objvar_2_type(rhs.objvar_2_type),
		objvar_2_value(rhs.objvar_2_value),
		objvar_3_name(rhs.objvar_3_name),
		objvar_3_type(rhs.objvar_3_type),
		objvar_3_value(rhs.objvar_3_value),
		objvar_4_name(rhs.objvar_4_name),
		objvar_4_type(rhs.objvar_4_type),
		objvar_4_value(rhs.objvar_4_value),
		objvar_5_name(rhs.objvar_5_name),
		objvar_5_type(rhs.objvar_5_type),
		objvar_5_value(rhs.objvar_5_value),
		objvar_6_name(rhs.objvar_6_name),
		objvar_6_type(rhs.objvar_6_type),
		objvar_6_value(rhs.objvar_6_value),
		objvar_7_name(rhs.objvar_7_name),
		objvar_7_type(rhs.objvar_7_type),
		objvar_7_value(rhs.objvar_7_value),
		objvar_8_name(rhs.objvar_8_name),
		objvar_8_type(rhs.objvar_8_type),
		objvar_8_value(rhs.objvar_8_value),
		objvar_9_name(rhs.objvar_9_name),
		objvar_9_type(rhs.objvar_9_type),
		objvar_9_value(rhs.objvar_9_value),
		objvar_10_name(rhs.objvar_10_name),
		objvar_10_type(rhs.objvar_10_type),
		objvar_10_value(rhs.objvar_10_value),
		objvar_11_name(rhs.objvar_11_name),
		objvar_11_type(rhs.objvar_11_type),
		objvar_11_value(rhs.objvar_11_value),
		objvar_12_name(rhs.objvar_12_name),
		objvar_12_type(rhs.objvar_12_type),
		objvar_12_value(rhs.objvar_12_value),
		objvar_13_name(rhs.objvar_13_name),
		objvar_13_type(rhs.objvar_13_type),
		objvar_13_value(rhs.objvar_13_value),
		objvar_14_name(rhs.objvar_14_name),
		objvar_14_type(rhs.objvar_14_type),
		objvar_14_value(rhs.objvar_14_value),
		objvar_15_name(rhs.objvar_15_name),
		objvar_15_type(rhs.objvar_15_type),
		objvar_15_value(rhs.objvar_15_value),
		objvar_16_name(rhs.objvar_16_name),
		objvar_16_type(rhs.objvar_16_type),
		objvar_16_value(rhs.objvar_16_value),
		objvar_17_name(rhs.objvar_17_name),
		objvar_17_type(rhs.objvar_17_type),
		objvar_17_value(rhs.objvar_17_value),
		objvar_18_name(rhs.objvar_18_name),
		objvar_18_type(rhs.objvar_18_type),
		objvar_18_value(rhs.objvar_18_value),
		objvar_19_name(rhs.objvar_19_name),
		objvar_19_type(rhs.objvar_19_type),
		objvar_19_value(rhs.objvar_19_value),
		script_list(rhs.script_list)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct PlanetObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableString<100> planet_name;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const PlanetObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct PlanetObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BufferString planet_name;

	PlanetObjectBufferRow() :
		object_id(),
		planet_name(100)
	{
	}

	PlanetObjectBufferRow(const PlanetObjectRow & rhs) :
		object_id(rhs.object_id),
		planet_name(rhs.planet_name)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const PlanetObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct PlayerQuestObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableString<256> title;
	DB::BindableString<4000> description;
	DB::BindableNetworkId creator;
	DB::BindableLong total_tasks;
	DB::BindableLong difficulty;
	DB::BindableString<128> task_title1;
	DB::BindableString<4000> task_description1;
	DB::BindableString<128> task_title2;
	DB::BindableString<4000> task_description2;
	DB::BindableString<128> task_title3;
	DB::BindableString<4000> task_description3;
	DB::BindableString<128> task_title4;
	DB::BindableString<4000> task_description4;
	DB::BindableString<128> task_title5;
	DB::BindableString<4000> task_description5;
	DB::BindableString<128> task_title6;
	DB::BindableString<4000> task_description6;
	DB::BindableString<128> task_title7;
	DB::BindableString<4000> task_description7;
	DB::BindableString<128> task_title8;
	DB::BindableString<4000> task_description8;
	DB::BindableString<128> task_title9;
	DB::BindableString<4000> task_description9;
	DB::BindableString<128> task_title10;
	DB::BindableString<4000> task_description10;
	DB::BindableString<128> task_title11;
	DB::BindableString<4000> task_description11;
	DB::BindableString<128> task_title12;
	DB::BindableString<4000> task_description12;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const PlayerQuestObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct PlayerQuestObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BufferString title;
	DB::BufferString description;
	DB::BindableNetworkId creator;
	DB::BindableLong total_tasks;
	DB::BindableLong difficulty;
	DB::BufferString task_title1;
	DB::BufferString task_description1;
	DB::BufferString task_title2;
	DB::BufferString task_description2;
	DB::BufferString task_title3;
	DB::BufferString task_description3;
	DB::BufferString task_title4;
	DB::BufferString task_description4;
	DB::BufferString task_title5;
	DB::BufferString task_description5;
	DB::BufferString task_title6;
	DB::BufferString task_description6;
	DB::BufferString task_title7;
	DB::BufferString task_description7;
	DB::BufferString task_title8;
	DB::BufferString task_description8;
	DB::BufferString task_title9;
	DB::BufferString task_description9;
	DB::BufferString task_title10;
	DB::BufferString task_description10;
	DB::BufferString task_title11;
	DB::BufferString task_description11;
	DB::BufferString task_title12;
	DB::BufferString task_description12;

	PlayerQuestObjectBufferRow() :
		object_id(),
		title(256),
		description(4000),
		creator(),
		total_tasks(),
		difficulty(),
		task_title1(128),
		task_description1(4000),
		task_title2(128),
		task_description2(4000),
		task_title3(128),
		task_description3(4000),
		task_title4(128),
		task_description4(4000),
		task_title5(128),
		task_description5(4000),
		task_title6(128),
		task_description6(4000),
		task_title7(128),
		task_description7(4000),
		task_title8(128),
		task_description8(4000),
		task_title9(128),
		task_description9(4000),
		task_title10(128),
		task_description10(4000),
		task_title11(128),
		task_description11(4000),
		task_title12(128),
		task_description12(4000)
	{
	}

	PlayerQuestObjectBufferRow(const PlayerQuestObjectRow & rhs) :
		object_id(rhs.object_id),
		title(rhs.title),
		description(rhs.description),
		creator(rhs.creator),
		total_tasks(rhs.total_tasks),
		difficulty(rhs.difficulty),
		task_title1(rhs.task_title1),
		task_description1(rhs.task_description1),
		task_title2(rhs.task_title2),
		task_description2(rhs.task_description2),
		task_title3(rhs.task_title3),
		task_description3(rhs.task_description3),
		task_title4(rhs.task_title4),
		task_description4(rhs.task_description4),
		task_title5(rhs.task_title5),
		task_description5(rhs.task_description5),
		task_title6(rhs.task_title6),
		task_description6(rhs.task_description6),
		task_title7(rhs.task_title7),
		task_description7(rhs.task_description7),
		task_title8(rhs.task_title8),
		task_description8(rhs.task_description8),
		task_title9(rhs.task_title9),
		task_description9(rhs.task_description9),
		task_title10(rhs.task_title10),
		task_description10(rhs.task_description10),
		task_title11(rhs.task_title11),
		task_description11(rhs.task_description11),
		task_title12(rhs.task_title12),
		task_description12(rhs.task_description12)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const PlayerQuestObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct PropertyListRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong list_id;
	DB::BindableString<500> value;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const PropertyListRow&>(rhs);
	}
};

struct PropertyListBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong list_id;
	DB::BufferString value;

	PropertyListBufferRow() :
		object_id(),
		list_id(),
		value(500)
	{
	}

	PropertyListBufferRow(const PropertyListRow & rhs) :
		object_id(rhs.object_id),
		list_id(rhs.list_id),
		value(rhs.value)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const PropertyListBufferRow&>(rhs);
	}
};

struct ResourceContainerObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableNetworkId resource_type;
	DB::BindableLong quantity;
	DB::BindableNetworkId source;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ResourceContainerObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct ResourceContainerObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableNetworkId resource_type;
	DB::BindableLong quantity;
	DB::BindableNetworkId source;

	ResourceContainerObjectBufferRow() :
		object_id(),
		resource_type(),
		quantity(),
		source()
	{
	}

	ResourceContainerObjectBufferRow(const ResourceContainerObjectRow & rhs) :
		object_id(rhs.object_id),
		resource_type(rhs.resource_type),
		quantity(rhs.quantity),
		source(rhs.source)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ResourceContainerObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct ResourceTypeRow : public DB::Row
{
	DB::BindableNetworkId resource_id;
	DB::BindableString<100> resource_name;
	DB::BindableString<100> resource_class;
	DB::BindableString<1024> attributes;
	DB::BindableString<1024> fractal_seeds;
	DB::BindableLong depleted_timestamp;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ResourceTypeRow&>(rhs);
	}
};

struct ResourceTypeBufferRow : public DB::Row
{
	DB::BindableNetworkId resource_id;
	DB::BufferString resource_name;
	DB::BufferString resource_class;
	DB::BufferString attributes;
	DB::BufferString fractal_seeds;
	DB::BindableLong depleted_timestamp;

	ResourceTypeBufferRow() :
		resource_id(),
		resource_name(100),
		resource_class(100),
		attributes(1024),
		fractal_seeds(1024),
		depleted_timestamp()
	{
	}

	ResourceTypeBufferRow(const ResourceTypeRow & rhs) :
		resource_id(rhs.resource_id),
		resource_name(rhs.resource_name),
		resource_class(rhs.resource_class),
		attributes(rhs.attributes),
		fractal_seeds(rhs.fractal_seeds),
		depleted_timestamp(rhs.depleted_timestamp)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ResourceTypeBufferRow&>(rhs);
	}
};

struct ScriptRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong sequence_no;
	DB::BindableString<100> script;
	DB::BindableLong detached;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ScriptRow&>(rhs);
	}
};

struct ScriptBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong sequence_no;
	DB::BufferString script;
	DB::BindableLong detached;

	ScriptBufferRow() :
		object_id(),
		sequence_no(),
		script(100),
		detached()
	{
	}

	ScriptBufferRow(const ScriptRow & rhs) :
		object_id(rhs.object_id),
		sequence_no(rhs.sequence_no),
		script(rhs.script),
		detached(rhs.detached)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ScriptBufferRow&>(rhs);
	}
};

struct ShipObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableDouble slide_dampener;
	DB::BindableDouble current_chassis_hit_points;
	DB::BindableDouble maximum_chassis_hit_points;
	DB::BindableLong chassis_type;
	DB::BindableString<400> cmp_armor_hp_maximum;
	DB::BindableString<400> cmp_armor_hp_current;
	DB::BindableString<400> cmp_efficiency_general;
	DB::BindableString<400> cmp_efficiency_eng;
	DB::BindableString<400> cmp_eng_maintenance;
	DB::BindableString<400> cmp_mass;
	DB::BindableString<500> cmp_crc;
	DB::BindableString<400> cmp_hp_current;
	DB::BindableString<400> cmp_hp_maximum;
	DB::BindableString<400> cmp_flags;
	DB::BindableString<4000> cmp_names;
	DB::BindableString<400> weapon_damage_maximum;
	DB::BindableString<400> weapon_damage_minimum;
	DB::BindableString<400> weapon_effectiveness_shields;
	DB::BindableString<400> weapon_effectiveness_armor;
	DB::BindableString<400> weapon_eng_per_shot;
	DB::BindableString<400> weapon_refire_rate;
	DB::BindableString<400> weapon_ammo_current;
	DB::BindableString<400> weapon_ammo_maximum;
	DB::BindableString<400> weapon_ammo_type;
	DB::BindableDouble shield_hp_front_maximum;
	DB::BindableDouble shield_hp_back_maximum;
	DB::BindableDouble shield_recharge_rate;
	DB::BindableDouble capacitor_eng_maximum;
	DB::BindableDouble capacitor_eng_recharge_rate;
	DB::BindableDouble engine_acc_rate;
	DB::BindableDouble engine_deceleration_rate;
	DB::BindableDouble engine_pitch_acc_rate;
	DB::BindableDouble engine_yaw_acc_rate;
	DB::BindableDouble engine_roll_acc_rate;
	DB::BindableDouble engine_pitch_rate_maximum;
	DB::BindableDouble engine_yaw_rate_maximum;
	DB::BindableDouble engine_roll_rate_maximum;
	DB::BindableDouble engine_speed_maximum;
	DB::BindableDouble reactor_eng_generation_rate;
	DB::BindableDouble booster_eng_maximum;
	DB::BindableDouble booster_eng_recharge_rate;
	DB::BindableDouble booster_eng_consumption_rate;
	DB::BindableDouble booster_acc;
	DB::BindableDouble booster_speed_maximum;
	DB::BindableDouble droid_if_cmd_speed;
	DB::BindableNetworkId installed_dcd;
	DB::BindableDouble chassis_cmp_mass_maximum;
	DB::BindableString<400> cmp_creators;
	DB::BindableLong cargo_hold_contents_maximum;
	DB::BindableLong cargo_hold_contents_current;
	DB::BindableString<500> cargo_hold_contents;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ShipObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct ShipObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableDouble slide_dampener;
	DB::BindableDouble current_chassis_hit_points;
	DB::BindableDouble maximum_chassis_hit_points;
	DB::BindableLong chassis_type;
	DB::BufferString cmp_armor_hp_maximum;
	DB::BufferString cmp_armor_hp_current;
	DB::BufferString cmp_efficiency_general;
	DB::BufferString cmp_efficiency_eng;
	DB::BufferString cmp_eng_maintenance;
	DB::BufferString cmp_mass;
	DB::BufferString cmp_crc;
	DB::BufferString cmp_hp_current;
	DB::BufferString cmp_hp_maximum;
	DB::BufferString cmp_flags;
	DB::BufferString cmp_names;
	DB::BufferString weapon_damage_maximum;
	DB::BufferString weapon_damage_minimum;
	DB::BufferString weapon_effectiveness_shields;
	DB::BufferString weapon_effectiveness_armor;
	DB::BufferString weapon_eng_per_shot;
	DB::BufferString weapon_refire_rate;
	DB::BufferString weapon_ammo_current;
	DB::BufferString weapon_ammo_maximum;
	DB::BufferString weapon_ammo_type;
	DB::BindableDouble shield_hp_front_maximum;
	DB::BindableDouble shield_hp_back_maximum;
	DB::BindableDouble shield_recharge_rate;
	DB::BindableDouble capacitor_eng_maximum;
	DB::BindableDouble capacitor_eng_recharge_rate;
	DB::BindableDouble engine_acc_rate;
	DB::BindableDouble engine_deceleration_rate;
	DB::BindableDouble engine_pitch_acc_rate;
	DB::BindableDouble engine_yaw_acc_rate;
	DB::BindableDouble engine_roll_acc_rate;
	DB::BindableDouble engine_pitch_rate_maximum;
	DB::BindableDouble engine_yaw_rate_maximum;
	DB::BindableDouble engine_roll_rate_maximum;
	DB::BindableDouble engine_speed_maximum;
	DB::BindableDouble reactor_eng_generation_rate;
	DB::BindableDouble booster_eng_maximum;
	DB::BindableDouble booster_eng_recharge_rate;
	DB::BindableDouble booster_eng_consumption_rate;
	DB::BindableDouble booster_acc;
	DB::BindableDouble booster_speed_maximum;
	DB::BindableDouble droid_if_cmd_speed;
	DB::BindableNetworkId installed_dcd;
	DB::BindableDouble chassis_cmp_mass_maximum;
	DB::BufferString cmp_creators;
	DB::BindableLong cargo_hold_contents_maximum;
	DB::BindableLong cargo_hold_contents_current;
	DB::BufferString cargo_hold_contents;

	ShipObjectBufferRow() :
		object_id(),
		slide_dampener(),
		current_chassis_hit_points(),
		maximum_chassis_hit_points(),
		chassis_type(),
		cmp_armor_hp_maximum(400),
		cmp_armor_hp_current(400),
		cmp_efficiency_general(400),
		cmp_efficiency_eng(400),
		cmp_eng_maintenance(400),
		cmp_mass(400),
		cmp_crc(500),
		cmp_hp_current(400),
		cmp_hp_maximum(400),
		cmp_flags(400),
		cmp_names(4000),
		weapon_damage_maximum(400),
		weapon_damage_minimum(400),
		weapon_effectiveness_shields(400),
		weapon_effectiveness_armor(400),
		weapon_eng_per_shot(400),
		weapon_refire_rate(400),
		weapon_ammo_current(400),
		weapon_ammo_maximum(400),
		weapon_ammo_type(400),
		shield_hp_front_maximum(),
		shield_hp_back_maximum(),
		shield_recharge_rate(),
		capacitor_eng_maximum(),
		capacitor_eng_recharge_rate(),
		engine_acc_rate(),
		engine_deceleration_rate(),
		engine_pitch_acc_rate(),
		engine_yaw_acc_rate(),
		engine_roll_acc_rate(),
		engine_pitch_rate_maximum(),
		engine_yaw_rate_maximum(),
		engine_roll_rate_maximum(),
		engine_speed_maximum(),
		reactor_eng_generation_rate(),
		booster_eng_maximum(),
		booster_eng_recharge_rate(),
		booster_eng_consumption_rate(),
		booster_acc(),
		booster_speed_maximum(),
		droid_if_cmd_speed(),
		installed_dcd(),
		chassis_cmp_mass_maximum(),
		cmp_creators(400),
		cargo_hold_contents_maximum(),
		cargo_hold_contents_current(),
		cargo_hold_contents(500)
	{
	}

	ShipObjectBufferRow(const ShipObjectRow & rhs) :
		object_id(rhs.object_id),
		slide_dampener(rhs.slide_dampener),
		current_chassis_hit_points(rhs.current_chassis_hit_points),
		maximum_chassis_hit_points(rhs.maximum_chassis_hit_points),
		chassis_type(rhs.chassis_type),
		cmp_armor_hp_maximum(rhs.cmp_armor_hp_maximum),
		cmp_armor_hp_current(rhs.cmp_armor_hp_current),
		cmp_efficiency_general(rhs.cmp_efficiency_general),
		cmp_efficiency_eng(rhs.cmp_efficiency_eng),
		cmp_eng_maintenance(rhs.cmp_eng_maintenance),
		cmp_mass(rhs.cmp_mass),
		cmp_crc(rhs.cmp_crc),
		cmp_hp_current(rhs.cmp_hp_current),
		cmp_hp_maximum(rhs.cmp_hp_maximum),
		cmp_flags(rhs.cmp_flags),
		cmp_names(rhs.cmp_names),
		weapon_damage_maximum(rhs.weapon_damage_maximum),
		weapon_damage_minimum(rhs.weapon_damage_minimum),
		weapon_effectiveness_shields(rhs.weapon_effectiveness_shields),
		weapon_effectiveness_armor(rhs.weapon_effectiveness_armor),
		weapon_eng_per_shot(rhs.weapon_eng_per_shot),
		weapon_refire_rate(rhs.weapon_refire_rate),
		weapon_ammo_current(rhs.weapon_ammo_current),
		weapon_ammo_maximum(rhs.weapon_ammo_maximum),
		weapon_ammo_type(rhs.weapon_ammo_type),
		shield_hp_front_maximum(rhs.shield_hp_front_maximum),
		shield_hp_back_maximum(rhs.shield_hp_back_maximum),
		shield_recharge_rate(rhs.shield_recharge_rate),
		capacitor_eng_maximum(rhs.capacitor_eng_maximum),
		capacitor_eng_recharge_rate(rhs.capacitor_eng_recharge_rate),
		engine_acc_rate(rhs.engine_acc_rate),
		engine_deceleration_rate(rhs.engine_deceleration_rate),
		engine_pitch_acc_rate(rhs.engine_pitch_acc_rate),
		engine_yaw_acc_rate(rhs.engine_yaw_acc_rate),
		engine_roll_acc_rate(rhs.engine_roll_acc_rate),
		engine_pitch_rate_maximum(rhs.engine_pitch_rate_maximum),
		engine_yaw_rate_maximum(rhs.engine_yaw_rate_maximum),
		engine_roll_rate_maximum(rhs.engine_roll_rate_maximum),
		engine_speed_maximum(rhs.engine_speed_maximum),
		reactor_eng_generation_rate(rhs.reactor_eng_generation_rate),
		booster_eng_maximum(rhs.booster_eng_maximum),
		booster_eng_recharge_rate(rhs.booster_eng_recharge_rate),
		booster_eng_consumption_rate(rhs.booster_eng_consumption_rate),
		booster_acc(rhs.booster_acc),
		booster_speed_maximum(rhs.booster_speed_maximum),
		droid_if_cmd_speed(rhs.droid_if_cmd_speed),
		installed_dcd(rhs.installed_dcd),
		chassis_cmp_mass_maximum(rhs.chassis_cmp_mass_maximum),
		cmp_creators(rhs.cmp_creators),
		cargo_hold_contents_maximum(rhs.cargo_hold_contents_maximum),
		cargo_hold_contents_current(rhs.cargo_hold_contents_current),
		cargo_hold_contents(rhs.cargo_hold_contents)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const ShipObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct StaticObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const StaticObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct StaticObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;

	StaticObjectBufferRow() :
		object_id()
	{
	}

	StaticObjectBufferRow(const StaticObjectRow & rhs) :
		object_id(rhs.object_id)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const StaticObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct TangibleObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong max_hit_points;
	DB::BindableNetworkId owner_id;
	DB::BindableBool visible;
	DB::BindableString<2000> appearance_data;
	DB::BindableLong interest_radius;
	DB::BindableLong pvp_type;
	DB::BindableLong pvp_faction;
	DB::BindableLong damage_taken;
	DB::BindableString<2000> custom_appearance;
	DB::BindableLong count;
	DB::BindableLong condition;
	DB::BindableNetworkId creator_id;
	DB::BindableLong source_draft_schematic;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const TangibleObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct TangibleObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong max_hit_points;
	DB::BindableNetworkId owner_id;
	DB::BindableBool visible;
	DB::BufferString appearance_data;
	DB::BindableLong interest_radius;
	DB::BindableLong pvp_type;
	DB::BindableLong pvp_faction;
	DB::BindableLong damage_taken;
	DB::BufferString custom_appearance;
	DB::BindableLong count;
	DB::BindableLong condition;
	DB::BindableNetworkId creator_id;
	DB::BindableLong source_draft_schematic;

	TangibleObjectBufferRow() :
		object_id(),
		max_hit_points(),
		owner_id(),
		visible(),
		appearance_data(2000),
		interest_radius(),
		pvp_type(),
		pvp_faction(),
		damage_taken(),
		custom_appearance(2000),
		count(),
		condition(),
		creator_id(),
		source_draft_schematic()
	{
	}

	TangibleObjectBufferRow(const TangibleObjectRow & rhs) :
		object_id(rhs.object_id),
		max_hit_points(rhs.max_hit_points),
		owner_id(rhs.owner_id),
		visible(rhs.visible),
		appearance_data(rhs.appearance_data),
		interest_radius(rhs.interest_radius),
		pvp_type(rhs.pvp_type),
		pvp_faction(rhs.pvp_faction),
		damage_taken(rhs.damage_taken),
		custom_appearance(rhs.custom_appearance),
		count(rhs.count),
		condition(rhs.condition),
		creator_id(rhs.creator_id),
		source_draft_schematic(rhs.source_draft_schematic)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const TangibleObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct TokenObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableNetworkId reference;
	DB::BindableString<500> target_server_template_name;
	DB::BindableString<500> target_shared_template_name;
	DB::BindableNetworkId waypoint;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const TokenObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct TokenObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableNetworkId reference;
	DB::BufferString target_server_template_name;
	DB::BufferString target_shared_template_name;
	DB::BindableNetworkId waypoint;

	TokenObjectBufferRow() :
		object_id(),
		reference(),
		target_server_template_name(500),
		target_shared_template_name(500),
		waypoint()
	{
	}

	TokenObjectBufferRow(const TokenObjectRow & rhs) :
		object_id(rhs.object_id),
		reference(rhs.reference),
		target_server_template_name(rhs.target_server_template_name),
		target_shared_template_name(rhs.target_shared_template_name),
		waypoint(rhs.waypoint)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const TokenObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct UniverseObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const UniverseObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct UniverseObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;

	UniverseObjectBufferRow() :
		object_id()
	{
	}

	UniverseObjectBufferRow(const UniverseObjectRow & rhs) :
		object_id(rhs.object_id)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const UniverseObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct VehicleObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong bogus;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const VehicleObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct VehicleObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong bogus;

	VehicleObjectBufferRow() :
		object_id(),
		bogus()
	{
	}

	VehicleObjectBufferRow(const VehicleObjectRow & rhs) :
		object_id(rhs.object_id),
		bogus(rhs.bogus)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const VehicleObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct WeaponObjectRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong min_damage;
	DB::BindableLong max_damage;
	DB::BindableLong damage_type;
	DB::BindableLong elemental_type;
	DB::BindableLong elemental_value;
	DB::BindableDouble attack_speed;
	DB::BindableDouble wound_chance;
	DB::BindableLong accuracy;
	DB::BindableLong attack_cost;
	DB::BindableDouble damage_radius;
	DB::BindableDouble min_range;
	DB::BindableDouble max_range;

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const WeaponObjectRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

struct WeaponObjectBufferRow : public DB::Row
{
	DB::BindableNetworkId object_id;
	DB::BindableLong min_damage;
	DB::BindableLong max_damage;
	DB::BindableLong damage_type;
	DB::BindableLong elemental_type;
	DB::BindableLong elemental_value;
	DB::BindableDouble attack_speed;
	DB::BindableDouble wound_chance;
	DB::BindableLong accuracy;
	DB::BindableLong attack_cost;
	DB::BindableDouble damage_radius;
	DB::BindableDouble min_range;
	DB::BindableDouble max_range;

	WeaponObjectBufferRow() :
		object_id(),
		min_damage(),
		max_damage(),
		damage_type(),
		elemental_type(),
		elemental_value(),
		attack_speed(),
		wound_chance(),
		accuracy(),
		attack_cost(),
		damage_radius(),
		min_range(),
		max_range()
	{
	}

	WeaponObjectBufferRow(const WeaponObjectRow & rhs) :
		object_id(rhs.object_id),
		min_damage(rhs.min_damage),
		max_damage(rhs.max_damage),
		damage_type(rhs.damage_type),
		elemental_type(rhs.elemental_type),
		elemental_value(rhs.elemental_value),
		attack_speed(rhs.attack_speed),
		wound_chance(rhs.wound_chance),
		accuracy(rhs.accuracy),
		attack_cost(rhs.attack_cost),
		damage_radius(rhs.damage_radius),
		min_range(rhs.min_range),
		max_range(rhs.max_range)
	{
	}

	virtual void copy(const DB::Row &rhs)
	{
		*this = dynamic_cast<const WeaponObjectBufferRow&>(rhs);
	}

	void setPrimaryKey(const NetworkId &keyValue)
	{
		object_id.setValue(keyValue);
	}

	NetworkId getPrimaryKey() const
	{
		return object_id.getValue();
	}
};

//!!!END GENERATED DECLARATIONS

}
#endif
