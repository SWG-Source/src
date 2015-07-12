// ======================================================================
//
// ObjectQueries.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectQueries_H
#define INCLUDED_ObjectQueries_H

// ======================================================================

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/DatabaseProcessQuery.h"
#include "sharedDatabaseInterface/DbBindableVarray.h"
#include "sharedDatabaseInterface/DbModeQuery.h"
#include "sharedFoundation/NetworkId.h"

#include <vector>

// ======================================================================

namespace DBQuery
{
	
//!!!BEGIN GENERATED QUERYDECLARATIONS
class BattlefieldMarkerObjectQuery : public DatabaseProcessQuery
{
	BattlefieldMarkerObjectQuery(const BattlefieldMarkerObjectQuery&);
	BattlefieldMarkerObjectQuery& operator= (const BattlefieldMarkerObjectQuery&);
public:
	BattlefieldMarkerObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayString	 m_region_names;
	DB::BindableLong			 m_numItems;
};

class BattlefieldMarkerObjectQuerySelect : public DB::Query
{
public:
	BattlefieldMarkerObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::BattlefieldMarkerObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::BattlefieldMarkerObjectRow> m_data;
	const std::string m_schema;

	BattlefieldMarkerObjectQuerySelect            (const BattlefieldMarkerObjectQuerySelect&);
	BattlefieldMarkerObjectQuerySelect& operator= (const BattlefieldMarkerObjectQuerySelect&);
};

class BuildingObjectQuery : public DatabaseProcessQuery
{
	BuildingObjectQuery(const BuildingObjectQuery&);
	BuildingObjectQuery& operator= (const BuildingObjectQuery&);
public:
	BuildingObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayNumber	 m_maintenance_costs;
	DB::BindableVarrayNumber	 m_time_last_checkeds;
	DB::BindableVarrayString	 m_is_publics;
	DB::BindableVarrayNumber	 m_city_ids;
	DB::BindableLong			 m_numItems;
};

class BuildingObjectQuerySelect : public DB::Query
{
public:
	BuildingObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::BuildingObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::BuildingObjectRow> m_data;
	const std::string m_schema;

	BuildingObjectQuerySelect            (const BuildingObjectQuerySelect&);
	BuildingObjectQuerySelect& operator= (const BuildingObjectQuerySelect&);
};

class CellObjectQuery : public DatabaseProcessQuery
{
	CellObjectQuery(const CellObjectQuery&);
	CellObjectQuery& operator= (const CellObjectQuery&);
public:
	CellObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayNumber	 m_cell_numbers;
	DB::BindableVarrayString	 m_is_publics;
	DB::BindableLong			 m_numItems;
};

class CellObjectQuerySelect : public DB::Query
{
public:
	CellObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::CellObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::CellObjectRow> m_data;
	const std::string m_schema;

	CellObjectQuerySelect            (const CellObjectQuerySelect&);
	CellObjectQuerySelect& operator= (const CellObjectQuerySelect&);
};

class CityObjectQuery : public DatabaseProcessQuery
{
	CityObjectQuery(const CityObjectQuery&);
	CityObjectQuery& operator= (const CityObjectQuery&);
public:
	CityObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableLong			 m_numItems;
};

class CityObjectQuerySelect : public DB::Query
{
public:
	CityObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::CityObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::CityObjectRow> m_data;
	const std::string m_schema;

	CityObjectQuerySelect            (const CityObjectQuerySelect&);
	CityObjectQuerySelect& operator= (const CityObjectQuerySelect&);
};

class CreatureObjectQuery : public DatabaseProcessQuery
{
	CreatureObjectQuery(const CreatureObjectQuery&);
	CreatureObjectQuery& operator= (const CreatureObjectQuery&);
public:
	CreatureObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayNumber	 m_scale_factors;
	DB::BindableVarrayNumber	 m_statess;
	DB::BindableVarrayNumber	 m_postures;
	DB::BindableVarrayNumber	 m_shock_woundss;
	DB::BindableVarrayString	 m_master_ids;
	DB::BindableVarrayNumber	 m_ranks;
	DB::BindableVarrayNumber	 m_base_walk_speeds;
	DB::BindableVarrayNumber	 m_base_run_speeds;
	DB::BindableVarrayNumber	 m_attribute_0s;
	DB::BindableVarrayNumber	 m_attribute_1s;
	DB::BindableVarrayNumber	 m_attribute_2s;
	DB::BindableVarrayNumber	 m_attribute_3s;
	DB::BindableVarrayNumber	 m_attribute_4s;
	DB::BindableVarrayNumber	 m_attribute_5s;
	DB::BindableVarrayNumber	 m_attribute_6s;
	DB::BindableVarrayNumber	 m_attribute_7s;
	DB::BindableVarrayNumber	 m_attribute_8s;
	DB::BindableVarrayNumber	 m_attribute_9s;
	DB::BindableVarrayNumber	 m_attribute_10s;
	DB::BindableVarrayNumber	 m_attribute_11s;
	DB::BindableVarrayNumber	 m_attribute_12s;
	DB::BindableVarrayNumber	 m_attribute_13s;
	DB::BindableVarrayNumber	 m_attribute_14s;
	DB::BindableVarrayNumber	 m_attribute_15s;
	DB::BindableVarrayNumber	 m_attribute_16s;
	DB::BindableVarrayNumber	 m_attribute_17s;
	DB::BindableVarrayNumber	 m_attribute_18s;
	DB::BindableVarrayNumber	 m_attribute_19s;
	DB::BindableVarrayNumber	 m_attribute_20s;
	DB::BindableVarrayNumber	 m_attribute_21s;
	DB::BindableVarrayNumber	 m_attribute_22s;
	DB::BindableVarrayNumber	 m_attribute_23s;
	DB::BindableVarrayNumber	 m_attribute_24s;
	DB::BindableVarrayNumber	 m_attribute_25s;
	DB::BindableVarrayNumber	 m_attribute_26s;
	DB::BindableVarrayString	 m_persisted_buffss;
	DB::BindableVarrayNumber	 m_ws_xs;
	DB::BindableVarrayNumber	 m_ws_ys;
	DB::BindableVarrayNumber	 m_ws_zs;
	DB::BindableLong			 m_numItems;
};

class CreatureObjectQuerySelect : public DB::Query
{
public:
	CreatureObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::CreatureObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::CreatureObjectRow> m_data;
	const std::string m_schema;

	CreatureObjectQuerySelect            (const CreatureObjectQuerySelect&);
	CreatureObjectQuerySelect& operator= (const CreatureObjectQuerySelect&);
};

class FactoryObjectQuery : public DatabaseProcessQuery
{
	FactoryObjectQuery(const FactoryObjectQuery&);
	FactoryObjectQuery& operator= (const FactoryObjectQuery&);
public:
	FactoryObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableLong			 m_numItems;
};

class FactoryObjectQuerySelect : public DB::Query
{
public:
	FactoryObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::FactoryObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::FactoryObjectRow> m_data;
	const std::string m_schema;

	FactoryObjectQuerySelect            (const FactoryObjectQuerySelect&);
	FactoryObjectQuerySelect& operator= (const FactoryObjectQuerySelect&);
};

class GuildObjectQuery : public DatabaseProcessQuery
{
	GuildObjectQuery(const GuildObjectQuery&);
	GuildObjectQuery& operator= (const GuildObjectQuery&);
public:
	GuildObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableLong			 m_numItems;
};

class GuildObjectQuerySelect : public DB::Query
{
public:
	GuildObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::GuildObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::GuildObjectRow> m_data;
	const std::string m_schema;

	GuildObjectQuerySelect            (const GuildObjectQuerySelect&);
	GuildObjectQuerySelect& operator= (const GuildObjectQuerySelect&);
};

class HarvesterInstallationObjectQuery : public DatabaseProcessQuery
{
	HarvesterInstallationObjectQuery(const HarvesterInstallationObjectQuery&);
	HarvesterInstallationObjectQuery& operator= (const HarvesterInstallationObjectQuery&);
public:
	HarvesterInstallationObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayNumber	 m_installed_efficiencys;
	DB::BindableVarrayNumber	 m_max_extraction_rates;
	DB::BindableVarrayNumber	 m_current_extraction_rates;
	DB::BindableVarrayNumber	 m_max_hopper_amounts;
	DB::BindableVarrayString	 m_hopper_resources;
	DB::BindableVarrayNumber	 m_hopper_amounts;
	DB::BindableVarrayString	 m_resource_types;
	DB::BindableLong			 m_numItems;
};

class HarvesterInstallationObjectQuerySelect : public DB::Query
{
public:
	HarvesterInstallationObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::HarvesterInstallationObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::HarvesterInstallationObjectRow> m_data;
	const std::string m_schema;

	HarvesterInstallationObjectQuerySelect            (const HarvesterInstallationObjectQuerySelect&);
	HarvesterInstallationObjectQuerySelect& operator= (const HarvesterInstallationObjectQuerySelect&);
};

class InstallationObjectQuery : public DatabaseProcessQuery
{
	InstallationObjectQuery(const InstallationObjectQuery&);
	InstallationObjectQuery& operator= (const InstallationObjectQuery&);
public:
	InstallationObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayNumber	 m_installation_types;
	DB::BindableVarrayString	 m_activateds;
	DB::BindableVarrayNumber	 m_tick_counts;
	DB::BindableVarrayNumber	 m_activate_start_times;
	DB::BindableVarrayNumber	 m_powers;
	DB::BindableVarrayNumber	 m_power_rates;
	DB::BindableLong			 m_numItems;
};

class InstallationObjectQuerySelect : public DB::Query
{
public:
	InstallationObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::InstallationObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::InstallationObjectRow> m_data;
	const std::string m_schema;

	InstallationObjectQuerySelect            (const InstallationObjectQuerySelect&);
	InstallationObjectQuerySelect& operator= (const InstallationObjectQuerySelect&);
};

class IntangibleObjectQuery : public DatabaseProcessQuery
{
	IntangibleObjectQuery(const IntangibleObjectQuery&);
	IntangibleObjectQuery& operator= (const IntangibleObjectQuery&);
public:
	IntangibleObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayNumber	 m_counts;
	DB::BindableLong			 m_numItems;
};

class IntangibleObjectQuerySelect : public DB::Query
{
public:
	IntangibleObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::IntangibleObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::IntangibleObjectRow> m_data;
	const std::string m_schema;

	IntangibleObjectQuerySelect            (const IntangibleObjectQuerySelect&);
	IntangibleObjectQuerySelect& operator= (const IntangibleObjectQuerySelect&);
};

class ManufactureSchematicObjectQuery : public DatabaseProcessQuery
{
	ManufactureSchematicObjectQuery(const ManufactureSchematicObjectQuery&);
	ManufactureSchematicObjectQuery& operator= (const ManufactureSchematicObjectQuery&);
public:
	ManufactureSchematicObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayString	 m_creator_ids;
	DB::BindableVarrayString	 m_creator_names;
	DB::BindableVarrayNumber	 m_items_per_containers;
	DB::BindableVarrayNumber	 m_manufacture_times;
	DB::BindableVarrayNumber	 m_draft_schematics;
	DB::BindableLong			 m_numItems;
};

class ManufactureSchematicObjectQuerySelect : public DB::Query
{
public:
	ManufactureSchematicObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::ManufactureSchematicObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::ManufactureSchematicObjectRow> m_data;
	const std::string m_schema;

	ManufactureSchematicObjectQuerySelect            (const ManufactureSchematicObjectQuerySelect&);
	ManufactureSchematicObjectQuerySelect& operator= (const ManufactureSchematicObjectQuerySelect&);
};

class ManufactureInstallationObjectQuery : public DatabaseProcessQuery
{
	ManufactureInstallationObjectQuery(const ManufactureInstallationObjectQuery&);
	ManufactureInstallationObjectQuery& operator= (const ManufactureInstallationObjectQuery&);
public:
	ManufactureInstallationObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableLong			 m_numItems;
};

class ManufactureInstallationObjectQuerySelect : public DB::Query
{
public:
	ManufactureInstallationObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::ManufactureInstallationObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::ManufactureInstallationObjectRow> m_data;
	const std::string m_schema;

	ManufactureInstallationObjectQuerySelect            (const ManufactureInstallationObjectQuerySelect&);
	ManufactureInstallationObjectQuerySelect& operator= (const ManufactureInstallationObjectQuerySelect&);
};

class MissionObjectQuery : public DatabaseProcessQuery
{
	MissionObjectQuery(const MissionObjectQuery&);
	MissionObjectQuery& operator= (const MissionObjectQuery&);
public:
	MissionObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayNumber	 m_difficultys;
	DB::BindableVarrayNumber	 m_end_xs;
	DB::BindableVarrayNumber	 m_end_ys;
	DB::BindableVarrayNumber	 m_end_zs;
	DB::BindableVarrayString	 m_end_cells;
	DB::BindableVarrayNumber	 m_end_scenes;
	DB::BindableVarrayString	 m_mission_creators;
	DB::BindableVarrayNumber	 m_rewards;
	DB::BindableVarrayString	 m_root_script_names;
	DB::BindableVarrayNumber	 m_start_xs;
	DB::BindableVarrayNumber	 m_start_ys;
	DB::BindableVarrayNumber	 m_start_zs;
	DB::BindableVarrayString	 m_start_cells;
	DB::BindableVarrayNumber	 m_start_scenes;
	DB::BindableVarrayString	 m_description_tables;
	DB::BindableVarrayString	 m_description_texts;
	DB::BindableVarrayString	 m_title_tables;
	DB::BindableVarrayString	 m_title_texts;
	DB::BindableVarrayString	 m_mission_holder_ids;
	DB::BindableVarrayNumber	 m_statuss;
	DB::BindableVarrayNumber	 m_mission_types;
	DB::BindableVarrayNumber	 m_target_appearances;
	DB::BindableVarrayString	 m_target_names;
	DB::BindableLong			 m_numItems;
};

class MissionObjectQuerySelect : public DB::Query
{
public:
	MissionObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::MissionObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::MissionObjectRow> m_data;
	const std::string m_schema;

	MissionObjectQuerySelect            (const MissionObjectQuerySelect&);
	MissionObjectQuerySelect& operator= (const MissionObjectQuerySelect&);
};

class PlanetObjectQuery : public DatabaseProcessQuery
{
	PlanetObjectQuery(const PlanetObjectQuery&);
	PlanetObjectQuery& operator= (const PlanetObjectQuery&);
public:
	PlanetObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayString	 m_planet_names;
	DB::BindableLong			 m_numItems;
};

class PlanetObjectQuerySelect : public DB::Query
{
public:
	PlanetObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::PlanetObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::PlanetObjectRow> m_data;
	const std::string m_schema;

	PlanetObjectQuerySelect            (const PlanetObjectQuerySelect&);
	PlanetObjectQuerySelect& operator= (const PlanetObjectQuerySelect&);
};

class PlayerObjectQuery : public DatabaseProcessQuery
{
	PlayerObjectQuery(const PlayerObjectQuery&);
	PlayerObjectQuery& operator= (const PlayerObjectQuery&);
public:
	PlayerObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayNumber	 m_station_ids;
	DB::BindableVarrayString	 m_house_ids;
	DB::BindableVarrayNumber	 m_account_num_lotss;
	DB::BindableVarrayString	 m_account_is_outcasts;
	DB::BindableVarrayNumber	 m_account_cheater_levels;
	DB::BindableVarrayNumber	 m_account_max_lots_adjustments;
	DB::BindableVarrayString	 m_personal_profile_ids;
	DB::BindableVarrayString	 m_character_profile_ids;
	DB::BindableVarrayString	 m_skill_titles;
	DB::BindableVarrayNumber	 m_born_dates;
	DB::BindableVarrayNumber	 m_played_times;
	DB::BindableVarrayNumber	 m_force_regen_rates;
	DB::BindableVarrayNumber	 m_force_powers;
	DB::BindableVarrayNumber	 m_max_force_powers;
	DB::BindableVarrayString	 m_active_questss;
	DB::BindableVarrayString	 m_completed_questss;
	DB::BindableVarrayNumber	 m_current_quests;
	DB::BindableVarrayString	 m_questss;
	DB::BindableVarrayNumber	 m_role_icon_choices;
	DB::BindableVarrayString	 m_quests2s;
	DB::BindableVarrayString	 m_quests3s;
	DB::BindableVarrayString	 m_quests4s;
	DB::BindableVarrayString	 m_skill_templates;
	DB::BindableVarrayString	 m_working_skills;
	DB::BindableVarrayNumber	 m_current_gcw_pointss;
	DB::BindableVarrayNumber	 m_current_gcw_ratings;
	DB::BindableVarrayNumber	 m_current_pvp_killss;
	DB::BindableVarrayNumber	 m_lifetime_gcw_pointss;
	DB::BindableVarrayNumber	 m_max_gcw_imperial_ratings;
	DB::BindableVarrayNumber	 m_max_gcw_rebel_ratings;
	DB::BindableVarrayNumber	 m_lifetime_pvp_killss;
	DB::BindableVarrayNumber	 m_next_gcw_rating_calc_times;
	DB::BindableVarrayString	 m_collectionss;
	DB::BindableVarrayString	 m_show_backpacks;
	DB::BindableVarrayString	 m_show_helmets;
	DB::BindableVarrayString	 m_collections2s;
	DB::BindableLong			 m_numItems;
};

class PlayerObjectQuerySelect : public DB::Query
{
public:
	PlayerObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::PlayerObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::PlayerObjectRow> m_data;
	const std::string m_schema;

	PlayerObjectQuerySelect            (const PlayerObjectQuerySelect&);
	PlayerObjectQuerySelect& operator= (const PlayerObjectQuerySelect&);
};

class PlayerQuestObjectQuery : public DatabaseProcessQuery
{
	PlayerQuestObjectQuery(const PlayerQuestObjectQuery&);
	PlayerQuestObjectQuery& operator= (const PlayerQuestObjectQuery&);
public:
	PlayerQuestObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayString	 m_titles;
	DB::BindableVarrayString	 m_descriptions;
	DB::BindableVarrayString	 m_creators;
	DB::BindableVarrayNumber	 m_total_taskss;
	DB::BindableVarrayNumber	 m_difficultys;
	DB::BindableVarrayString	 m_task_title1s;
	DB::BindableVarrayString	 m_task_description1s;
	DB::BindableVarrayString	 m_task_title2s;
	DB::BindableVarrayString	 m_task_description2s;
	DB::BindableVarrayString	 m_task_title3s;
	DB::BindableVarrayString	 m_task_description3s;
	DB::BindableVarrayString	 m_task_title4s;
	DB::BindableVarrayString	 m_task_description4s;
	DB::BindableVarrayString	 m_task_title5s;
	DB::BindableVarrayString	 m_task_description5s;
	DB::BindableVarrayString	 m_task_title6s;
	DB::BindableVarrayString	 m_task_description6s;
	DB::BindableVarrayString	 m_task_title7s;
	DB::BindableVarrayString	 m_task_description7s;
	DB::BindableVarrayString	 m_task_title8s;
	DB::BindableVarrayString	 m_task_description8s;
	DB::BindableVarrayString	 m_task_title9s;
	DB::BindableVarrayString	 m_task_description9s;
	DB::BindableVarrayString	 m_task_title10s;
	DB::BindableVarrayString	 m_task_description10s;
	DB::BindableVarrayString	 m_task_title11s;
	DB::BindableVarrayString	 m_task_description11s;
	DB::BindableVarrayString	 m_task_title12s;
	DB::BindableVarrayString	 m_task_description12s;
	DB::BindableLong			 m_numItems;
};

class PlayerQuestObjectQuerySelect : public DB::Query
{
public:
	PlayerQuestObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::PlayerQuestObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::PlayerQuestObjectRow> m_data;
	const std::string m_schema;

	PlayerQuestObjectQuerySelect            (const PlayerQuestObjectQuerySelect&);
	PlayerQuestObjectQuerySelect& operator= (const PlayerQuestObjectQuerySelect&);
};

class ResourceContainerObjectQuery : public DatabaseProcessQuery
{
	ResourceContainerObjectQuery(const ResourceContainerObjectQuery&);
	ResourceContainerObjectQuery& operator= (const ResourceContainerObjectQuery&);
public:
	ResourceContainerObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayString	 m_resource_types;
	DB::BindableVarrayNumber	 m_quantitys;
	DB::BindableVarrayString	 m_sources;
	DB::BindableLong			 m_numItems;
};

class ResourceContainerObjectQuerySelect : public DB::Query
{
public:
	ResourceContainerObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::ResourceContainerObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::ResourceContainerObjectRow> m_data;
	const std::string m_schema;

	ResourceContainerObjectQuerySelect            (const ResourceContainerObjectQuerySelect&);
	ResourceContainerObjectQuerySelect& operator= (const ResourceContainerObjectQuerySelect&);
};

class ShipObjectQuery : public DatabaseProcessQuery
{
	ShipObjectQuery(const ShipObjectQuery&);
	ShipObjectQuery& operator= (const ShipObjectQuery&);
public:
	ShipObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayNumber	 m_slide_dampeners;
	DB::BindableVarrayNumber	 m_current_chassis_hit_pointss;
	DB::BindableVarrayNumber	 m_maximum_chassis_hit_pointss;
	DB::BindableVarrayNumber	 m_chassis_types;
	DB::BindableVarrayString	 m_cmp_armor_hp_maximums;
	DB::BindableVarrayString	 m_cmp_armor_hp_currents;
	DB::BindableVarrayString	 m_cmp_efficiency_generals;
	DB::BindableVarrayString	 m_cmp_efficiency_engs;
	DB::BindableVarrayString	 m_cmp_eng_maintenances;
	DB::BindableVarrayString	 m_cmp_masss;
	DB::BindableVarrayString	 m_cmp_crcs;
	DB::BindableVarrayString	 m_cmp_hp_currents;
	DB::BindableVarrayString	 m_cmp_hp_maximums;
	DB::BindableVarrayString	 m_cmp_flagss;
	DB::BindableVarrayString	 m_cmp_namess;
	DB::BindableVarrayString	 m_weapon_damage_maximums;
	DB::BindableVarrayString	 m_weapon_damage_minimums;
	DB::BindableVarrayString	 m_weapon_effectiveness_shieldss;
	DB::BindableVarrayString	 m_weapon_effectiveness_armors;
	DB::BindableVarrayString	 m_weapon_eng_per_shots;
	DB::BindableVarrayString	 m_weapon_refire_rates;
	DB::BindableVarrayString	 m_weapon_ammo_currents;
	DB::BindableVarrayString	 m_weapon_ammo_maximums;
	DB::BindableVarrayString	 m_weapon_ammo_types;
	DB::BindableVarrayNumber	 m_shield_hp_front_maximums;
	DB::BindableVarrayNumber	 m_shield_hp_back_maximums;
	DB::BindableVarrayNumber	 m_shield_recharge_rates;
	DB::BindableVarrayNumber	 m_capacitor_eng_maximums;
	DB::BindableVarrayNumber	 m_capacitor_eng_recharge_rates;
	DB::BindableVarrayNumber	 m_engine_acc_rates;
	DB::BindableVarrayNumber	 m_engine_deceleration_rates;
	DB::BindableVarrayNumber	 m_engine_pitch_acc_rates;
	DB::BindableVarrayNumber	 m_engine_yaw_acc_rates;
	DB::BindableVarrayNumber	 m_engine_roll_acc_rates;
	DB::BindableVarrayNumber	 m_engine_pitch_rate_maximums;
	DB::BindableVarrayNumber	 m_engine_yaw_rate_maximums;
	DB::BindableVarrayNumber	 m_engine_roll_rate_maximums;
	DB::BindableVarrayNumber	 m_engine_speed_maximums;
	DB::BindableVarrayNumber	 m_reactor_eng_generation_rates;
	DB::BindableVarrayNumber	 m_booster_eng_maximums;
	DB::BindableVarrayNumber	 m_booster_eng_recharge_rates;
	DB::BindableVarrayNumber	 m_booster_eng_consumption_rates;
	DB::BindableVarrayNumber	 m_booster_accs;
	DB::BindableVarrayNumber	 m_booster_speed_maximums;
	DB::BindableVarrayNumber	 m_droid_if_cmd_speeds;
	DB::BindableVarrayString	 m_installed_dcds;
	DB::BindableVarrayNumber	 m_chassis_cmp_mass_maximums;
	DB::BindableVarrayString	 m_cmp_creatorss;
	DB::BindableVarrayNumber	 m_cargo_hold_contents_maximums;
	DB::BindableVarrayNumber	 m_cargo_hold_contents_currents;
	DB::BindableVarrayString	 m_cargo_hold_contentss;
	DB::BindableLong			 m_numItems;
};

class ShipObjectQuerySelect : public DB::Query
{
public:
	ShipObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::ShipObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::ShipObjectRow> m_data;
	const std::string m_schema;

	ShipObjectQuerySelect            (const ShipObjectQuerySelect&);
	ShipObjectQuerySelect& operator= (const ShipObjectQuerySelect&);
};

class StaticObjectQuery : public DatabaseProcessQuery
{
	StaticObjectQuery(const StaticObjectQuery&);
	StaticObjectQuery& operator= (const StaticObjectQuery&);
public:
	StaticObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableLong			 m_numItems;
};

class StaticObjectQuerySelect : public DB::Query
{
public:
	StaticObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::StaticObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::StaticObjectRow> m_data;
	const std::string m_schema;

	StaticObjectQuerySelect            (const StaticObjectQuerySelect&);
	StaticObjectQuerySelect& operator= (const StaticObjectQuerySelect&);
};

class TangibleObjectQuery : public DatabaseProcessQuery
{
	TangibleObjectQuery(const TangibleObjectQuery&);
	TangibleObjectQuery& operator= (const TangibleObjectQuery&);
public:
	TangibleObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayNumber	 m_max_hit_pointss;
	DB::BindableVarrayString	 m_owner_ids;
	DB::BindableVarrayString	 m_visibles;
	DB::BindableVarrayString	 m_appearance_datas;
	DB::BindableVarrayNumber	 m_interest_radiuss;
	DB::BindableVarrayNumber	 m_pvp_types;
	DB::BindableVarrayNumber	 m_pvp_factions;
	DB::BindableVarrayNumber	 m_damage_takens;
	DB::BindableVarrayString	 m_custom_appearances;
	DB::BindableVarrayNumber	 m_counts;
	DB::BindableVarrayNumber	 m_conditions;
	DB::BindableVarrayString	 m_creator_ids;
	DB::BindableVarrayNumber	 m_source_draft_schematics;
	DB::BindableLong			 m_numItems;
};

class TangibleObjectQuerySelect : public DB::Query
{
public:
	TangibleObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::TangibleObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::TangibleObjectRow> m_data;
	const std::string m_schema;

	TangibleObjectQuerySelect            (const TangibleObjectQuerySelect&);
	TangibleObjectQuerySelect& operator= (const TangibleObjectQuerySelect&);
};

class TokenObjectQuery : public DatabaseProcessQuery
{
	TokenObjectQuery(const TokenObjectQuery&);
	TokenObjectQuery& operator= (const TokenObjectQuery&);
public:
	TokenObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayString	 m_references;
	DB::BindableVarrayString	 m_target_server_template_names;
	DB::BindableVarrayString	 m_target_shared_template_names;
	DB::BindableVarrayString	 m_waypoints;
	DB::BindableLong			 m_numItems;
};

class TokenObjectQuerySelect : public DB::Query
{
public:
	TokenObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::TokenObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::TokenObjectRow> m_data;
	const std::string m_schema;

	TokenObjectQuerySelect            (const TokenObjectQuerySelect&);
	TokenObjectQuerySelect& operator= (const TokenObjectQuerySelect&);
};

class UniverseObjectQuery : public DatabaseProcessQuery
{
	UniverseObjectQuery(const UniverseObjectQuery&);
	UniverseObjectQuery& operator= (const UniverseObjectQuery&);
public:
	UniverseObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableLong			 m_numItems;
};

class UniverseObjectQuerySelect : public DB::Query
{
public:
	UniverseObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::UniverseObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::UniverseObjectRow> m_data;
	const std::string m_schema;

	UniverseObjectQuerySelect            (const UniverseObjectQuerySelect&);
	UniverseObjectQuerySelect& operator= (const UniverseObjectQuerySelect&);
};

class VehicleObjectQuery : public DatabaseProcessQuery
{
	VehicleObjectQuery(const VehicleObjectQuery&);
	VehicleObjectQuery& operator= (const VehicleObjectQuery&);
public:
	VehicleObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayNumber	 m_boguss;
	DB::BindableLong			 m_numItems;
};

class VehicleObjectQuerySelect : public DB::Query
{
public:
	VehicleObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::VehicleObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::VehicleObjectRow> m_data;
	const std::string m_schema;

	VehicleObjectQuerySelect            (const VehicleObjectQuerySelect&);
	VehicleObjectQuerySelect& operator= (const VehicleObjectQuerySelect&);
};

class WeaponObjectQuery : public DatabaseProcessQuery
{
	WeaponObjectQuery(const WeaponObjectQuery&);
	WeaponObjectQuery& operator= (const WeaponObjectQuery&);
public:
	WeaponObjectQuery();

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL(std::string &sql);

	bool setupData(DB::Session *session);
	bool addData(const DB::Row *_data);
	void clearData();
	void freeData();

	int getNumItems() const;

private:
	DB::BindableVarrayString	 m_object_ids;
	DB::BindableVarrayNumber	 m_min_damages;
	DB::BindableVarrayNumber	 m_max_damages;
	DB::BindableVarrayNumber	 m_damage_types;
	DB::BindableVarrayNumber	 m_elemental_types;
	DB::BindableVarrayNumber	 m_elemental_values;
	DB::BindableVarrayNumber	 m_attack_speeds;
	DB::BindableVarrayNumber	 m_wound_chances;
	DB::BindableVarrayNumber	 m_accuracys;
	DB::BindableVarrayNumber	 m_attack_costs;
	DB::BindableVarrayNumber	 m_damage_radiuss;
	DB::BindableVarrayNumber	 m_min_ranges;
	DB::BindableVarrayNumber	 m_max_ranges;
	DB::BindableLong			 m_numItems;
};

class WeaponObjectQuerySelect : public DB::Query
{
public:
	WeaponObjectQuerySelect(const std::string &schema);

	virtual bool bindParameters ();
	virtual bool bindColumns    ();
	virtual void getSQL         (std::string &sql);

	const std::vector<DBSchema::WeaponObjectRow> & getData() const;

protected:
	virtual QueryMode getExecutionMode() const;

private:

	std::vector<DBSchema::WeaponObjectRow> m_data;
	const std::string m_schema;

	WeaponObjectQuerySelect            (const WeaponObjectQuerySelect&);
	WeaponObjectQuerySelect& operator= (const WeaponObjectQuerySelect&);
};

//!!!END GENERATED QUERYDECLARATIONS

// ======================================================================

} // namespace

#endif
