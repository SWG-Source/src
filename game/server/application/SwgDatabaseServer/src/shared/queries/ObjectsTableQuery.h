// ======================================================================
//
// ObjectsTableQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectsTableQuery_H
#define INCLUDED_ObjectsTableQuery_H

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
	
	class ObjectsTableQuery : public DatabaseProcessQuery
	{
		ObjectsTableQuery(const ObjectsTableQuery&); // disable
		ObjectsTableQuery& operator=(const ObjectsTableQuery&); // disable
	  public:
		ObjectsTableQuery();
			
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);

		bool setupData(DB::Session *session);
		bool addData(const DB::Row *_data);
		void clearData();
		void freeData();

		int getNumItems() const;
			
		void setObjectId(const NetworkId &objectId);
	  private:
		  	DB::BindableVarrayString		m_object_ids;
			DB::BindableVarrayNumber		m_xs;
			DB::BindableVarrayNumber		m_ys;
			DB::BindableVarrayNumber		m_zs;
			DB::BindableVarrayNumber		m_quaternion_ws;
			DB::BindableVarrayNumber		m_quaternion_xs;
			DB::BindableVarrayNumber		m_quaternion_ys;
			DB::BindableVarrayNumber		m_quaternion_zs;
			DB::BindableVarrayNumber		m_node_xs;
			DB::BindableVarrayNumber		m_node_ys;
			DB::BindableVarrayNumber		m_node_zs;
			DB::BindableVarrayNumber		m_type_ids;
			DB::BindableVarrayString		m_scene_ids;
			DB::BindableVarrayNumber		m_controller_types;
			DB::BindableVarrayNumber		m_deleteds;
			DB::BindableVarrayString		m_object_names;
			DB::BindableVarrayNumber		m_volumes;
			DB::BindableVarrayString		m_contained_bys;
			DB::BindableVarrayNumber		m_slot_arrangements;
			DB::BindableVarrayString		m_player_controlleds;
			DB::BindableVarrayNumber		m_cache_versions;
			DB::BindableVarrayString		m_load_contentss;
			DB::BindableVarrayNumber		m_cash_balances;
			DB::BindableVarrayNumber		m_bank_balances;
			DB::BindableVarrayNumber		m_complexitys;
			DB::BindableVarrayString		m_name_string_tables;
			DB::BindableVarrayString		m_name_string_texts;
			DB::BindableVarrayString		m_static_item_names;
			DB::BindableVarrayNumber		m_static_item_versions;
			DB::BindableVarrayNumber		m_conversion_ids;
			DB::BindableVarrayNumber		m_object_template_ids;
			DB::BindableVarrayString		m_load_withs;
			DB::BindableVarrayString		m_objvar_0_names;
			DB::BindableVarrayNumber		m_objvar_0_types;
			DB::BindableVarrayString		m_objvar_0_values;
			DB::BindableVarrayString		m_objvar_1_names;
			DB::BindableVarrayNumber		m_objvar_1_types;
			DB::BindableVarrayString		m_objvar_1_values;
			DB::BindableVarrayString		m_objvar_2_names;
			DB::BindableVarrayNumber		m_objvar_2_types;
			DB::BindableVarrayString		m_objvar_2_values;
			DB::BindableVarrayString		m_objvar_3_names;
			DB::BindableVarrayNumber		m_objvar_3_types;
			DB::BindableVarrayString		m_objvar_3_values;
			DB::BindableVarrayString		m_objvar_4_names;
			DB::BindableVarrayNumber		m_objvar_4_types;
			DB::BindableVarrayString		m_objvar_4_values;
			DB::BindableVarrayString		m_objvar_5_names;
			DB::BindableVarrayNumber		m_objvar_5_types;
			DB::BindableVarrayString		m_objvar_5_values;
			DB::BindableVarrayString		m_objvar_6_names;
			DB::BindableVarrayNumber		m_objvar_6_types;
			DB::BindableVarrayString		m_objvar_6_values;
			DB::BindableVarrayString		m_objvar_7_names;
			DB::BindableVarrayNumber		m_objvar_7_types;
			DB::BindableVarrayString		m_objvar_7_values;
			DB::BindableVarrayString		m_objvar_8_names;
			DB::BindableVarrayNumber		m_objvar_8_types;
			DB::BindableVarrayString		m_objvar_8_values;
			DB::BindableVarrayString		m_objvar_9_names;
			DB::BindableVarrayNumber		m_objvar_9_types;
			DB::BindableVarrayString		m_objvar_9_values;
			DB::BindableVarrayString		m_objvar_10_names;
			DB::BindableVarrayNumber		m_objvar_10_types;
			DB::BindableVarrayString		m_objvar_10_values;
			DB::BindableVarrayString		m_objvar_11_names;
			DB::BindableVarrayNumber		m_objvar_11_types;
			DB::BindableVarrayString		m_objvar_11_values;
			DB::BindableVarrayString		m_objvar_12_names;
			DB::BindableVarrayNumber		m_objvar_12_types;
			DB::BindableVarrayString		m_objvar_12_values;
			DB::BindableVarrayString		m_objvar_13_names;
			DB::BindableVarrayNumber		m_objvar_13_types;
			DB::BindableVarrayString		m_objvar_13_values;
			DB::BindableVarrayString		m_objvar_14_names;
			DB::BindableVarrayNumber		m_objvar_14_types;
			DB::BindableVarrayString		m_objvar_14_values;
			DB::BindableVarrayString		m_objvar_15_names;
			DB::BindableVarrayNumber		m_objvar_15_types;
			DB::BindableVarrayString		m_objvar_15_values;
			DB::BindableVarrayString		m_objvar_16_names;
			DB::BindableVarrayNumber		m_objvar_16_types;
			DB::BindableVarrayString		m_objvar_16_values;
			DB::BindableVarrayString		m_objvar_17_names;
			DB::BindableVarrayNumber		m_objvar_17_types;
			DB::BindableVarrayString		m_objvar_17_values;
			DB::BindableVarrayString		m_objvar_18_names;
			DB::BindableVarrayNumber		m_objvar_18_types;
			DB::BindableVarrayString		m_objvar_18_values;
			DB::BindableVarrayString		m_objvar_19_names;
			DB::BindableVarrayNumber		m_objvar_19_types;
			DB::BindableVarrayString		m_objvar_19_values;
			DB::BindableVarrayString		m_script_lists;

			DB::BindableLong				m_numItems;
	};

	class ObjectsTableQuerySelect : public DB::Query
	{
	  public:
		ObjectsTableQuerySelect(const std::string &schema);
		~ObjectsTableQuerySelect();
			
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);

		const std::vector<DBSchema::ObjectRow> & getData();

	  protected:
		virtual QueryMode getExecutionMode() const;

	  private:
		std::vector<DBSchema::ObjectRow> m_data;
		std::string m_schema;

	  private:
		ObjectsTableQuerySelect(const ObjectsTableQuerySelect&); // disable
		ObjectsTableQuerySelect& operator=(const ObjectsTableQuerySelect&); // disable
	};

}

// ======================================================================

#endif
