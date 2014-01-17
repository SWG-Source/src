// ======================================================================
//
// ObjectsTableQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/ObjectsTableQuery.h"

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

using namespace DBQuery;




using namespace DBSchema;

// ======================================================================

ObjectsTableQuery::ObjectsTableQuery() : 
		DatabaseProcessQuery(new ObjectBufferRow),
		m_numItems(0)
{
}

bool ObjectsTableQuery::setupData(DB::Session *session)
{
	
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
		  	if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_xs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_ys.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_zs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_quaternion_ws.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_quaternion_xs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_quaternion_ys.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_quaternion_zs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_node_xs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_node_ys.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_node_zs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_type_ids.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_scene_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_controller_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_deleteds.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_object_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_volumes.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_contained_bys.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_slot_arrangements.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_player_controlleds.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_cache_versions.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_load_contentss.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_cash_balances.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_bank_balances.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_complexitys.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_name_string_tables.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_name_string_texts.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_static_item_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_static_item_versions.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_conversion_ids.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_object_template_ids.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_load_withs.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_0_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_0_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_0_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_1_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_1_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_1_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_2_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_2_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_2_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_3_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_3_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_3_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_4_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_4_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_4_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_5_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_5_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_5_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_6_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_6_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_6_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_7_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_7_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_7_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_8_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_8_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_8_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_9_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_9_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_9_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_10_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_10_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_10_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_11_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_11_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_11_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_12_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_12_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_12_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_13_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_13_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_13_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_14_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_14_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_14_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_15_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_15_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_15_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_16_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_16_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_16_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_17_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_17_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_17_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_18_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_18_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_18_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_19_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_objvar_19_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_objvar_19_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_script_lists.create(session, "VAOFLONGSTRING", DatabaseProcess::getInstance().getSchema(),4000)) return false;

			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

	return true;
}

bool ObjectsTableQuery::addData(const DB::Row *_data)
{
	const ObjectBufferRow *myData=safe_cast<const ObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_xs.push_back(myData->x.isNull(), myData->x.getValue())) return false;
			if (!m_ys.push_back(myData->y.isNull(), myData->y.getValue())) return false;
			if (!m_zs.push_back(myData->z.isNull(), myData->z.getValue())) return false;
			if (!m_quaternion_ws.push_back(myData->quaternion_w.isNull(), myData->quaternion_w.getValue())) return false;
			if (!m_quaternion_xs.push_back(myData->quaternion_x.isNull(), myData->quaternion_x.getValue())) return false;
			if (!m_quaternion_ys.push_back(myData->quaternion_y.isNull(), myData->quaternion_y.getValue())) return false;
			if (!m_quaternion_zs.push_back(myData->quaternion_z.isNull(), myData->quaternion_z.getValue())) return false;
			if (!m_node_xs.push_back(myData->node_x.isNull(), myData->node_x.getValue())) return false;
			if (!m_node_ys.push_back(myData->node_y.isNull(), myData->node_y.getValue())) return false;
			if (!m_node_zs.push_back(myData->node_z.isNull(), myData->node_z.getValue())) return false;
			if (!m_type_ids.push_back(myData->type_id.isNull(), myData->type_id.getValue())) return false;
			if (!m_scene_ids.push_back(myData->scene_id.getValueASCII())) return false;
			if (!m_controller_types.push_back(myData->controller_type.isNull(), myData->controller_type.getValue())) return false;
			if (!m_deleteds.push_back(myData->deleted.isNull(), myData->deleted.getValue())) return false;
			if (!m_object_names.push_back(myData->object_name.getValueASCII())) return false;
			if (!m_volumes.push_back(myData->volume.isNull(), myData->volume.getValue())) return false;
			if (!m_contained_bys.push_back(myData->contained_by.getValueASCII())) return false;
			if (!m_slot_arrangements.push_back(myData->slot_arrangement.isNull(), myData->slot_arrangement.getValue())) return false;
			if (!m_player_controlleds.push_back(myData->player_controlled.getValueASCII())) return false;
			if (!m_cache_versions.push_back(myData->cache_version.isNull(), myData->cache_version.getValue())) return false;
			if (!m_load_contentss.push_back(myData->load_contents.getValueASCII())) return false;
			if (!m_cash_balances.push_back(myData->cash_balance.isNull(), myData->cash_balance.getValue())) return false;
			if (!m_bank_balances.push_back(myData->bank_balance.isNull(), myData->bank_balance.getValue())) return false;
			if (!m_complexitys.push_back(myData->complexity.isNull(), myData->complexity.getValue())) return false;
			if (!m_name_string_tables.push_back(myData->name_string_table.getValueASCII())) return false;
			if (!m_name_string_texts.push_back(myData->name_string_text.getValueASCII())) return false;
			if (!m_static_item_names.push_back(myData->static_item_name.getValueASCII())) return false;
			if (!m_static_item_versions.push_back(myData->static_item_version.isNull(), myData->static_item_version.getValue())) return false;
			if (!m_conversion_ids.push_back(myData->conversion_id.isNull(), myData->conversion_id.getValue())) return false;
			if (!m_object_template_ids.push_back(myData->object_template_id.isNull(), myData->object_template_id.getValue())) return false;
			if (!m_load_withs.push_back(myData->load_with.getValueASCII())) return false;
			if (!m_objvar_0_names.push_back(myData->objvar_0_name.getValueASCII())) return false;
			if (!m_objvar_0_types.push_back(myData->objvar_0_type.isNull(), myData->objvar_0_type.getValue())) return false;
			if (!m_objvar_0_values.push_back(myData->objvar_0_value.getValueASCII())) return false;
			if (!m_objvar_1_names.push_back(myData->objvar_1_name.getValueASCII())) return false;
			if (!m_objvar_1_types.push_back(myData->objvar_1_type.isNull(), myData->objvar_1_type.getValue())) return false;
			if (!m_objvar_1_values.push_back(myData->objvar_1_value.getValueASCII())) return false;
			if (!m_objvar_2_names.push_back(myData->objvar_2_name.getValueASCII())) return false;
			if (!m_objvar_2_types.push_back(myData->objvar_2_type.isNull(), myData->objvar_2_type.getValue())) return false;
			if (!m_objvar_2_values.push_back(myData->objvar_2_value.getValueASCII())) return false;
			if (!m_objvar_3_names.push_back(myData->objvar_3_name.getValueASCII())) return false;
			if (!m_objvar_3_types.push_back(myData->objvar_3_type.isNull(), myData->objvar_3_type.getValue())) return false;
			if (!m_objvar_3_values.push_back(myData->objvar_3_value.getValueASCII())) return false;
			if (!m_objvar_4_names.push_back(myData->objvar_4_name.getValueASCII())) return false;
			if (!m_objvar_4_types.push_back(myData->objvar_4_type.isNull(), myData->objvar_4_type.getValue())) return false;
			if (!m_objvar_4_values.push_back(myData->objvar_4_value.getValueASCII())) return false;
			if (!m_objvar_5_names.push_back(myData->objvar_5_name.getValueASCII())) return false;
			if (!m_objvar_5_types.push_back(myData->objvar_5_type.isNull(), myData->objvar_5_type.getValue())) return false;
			if (!m_objvar_5_values.push_back(myData->objvar_5_value.getValueASCII())) return false;
			if (!m_objvar_6_names.push_back(myData->objvar_6_name.getValueASCII())) return false;
			if (!m_objvar_6_types.push_back(myData->objvar_6_type.isNull(), myData->objvar_6_type.getValue())) return false;
			if (!m_objvar_6_values.push_back(myData->objvar_6_value.getValueASCII())) return false;
			if (!m_objvar_7_names.push_back(myData->objvar_7_name.getValueASCII())) return false;
			if (!m_objvar_7_types.push_back(myData->objvar_7_type.isNull(), myData->objvar_7_type.getValue())) return false;
			if (!m_objvar_7_values.push_back(myData->objvar_7_value.getValueASCII())) return false;
			if (!m_objvar_8_names.push_back(myData->objvar_8_name.getValueASCII())) return false;
			if (!m_objvar_8_types.push_back(myData->objvar_8_type.isNull(), myData->objvar_8_type.getValue())) return false;
			if (!m_objvar_8_values.push_back(myData->objvar_8_value.getValueASCII())) return false;
			if (!m_objvar_9_names.push_back(myData->objvar_9_name.getValueASCII())) return false;
			if (!m_objvar_9_types.push_back(myData->objvar_9_type.isNull(), myData->objvar_9_type.getValue())) return false;
			if (!m_objvar_9_values.push_back(myData->objvar_9_value.getValueASCII())) return false;
			if (!m_objvar_10_names.push_back(myData->objvar_10_name.getValueASCII())) return false;
			if (!m_objvar_10_types.push_back(myData->objvar_10_type.isNull(), myData->objvar_10_type.getValue())) return false;
			if (!m_objvar_10_values.push_back(myData->objvar_10_value.getValueASCII())) return false;
			if (!m_objvar_11_names.push_back(myData->objvar_11_name.getValueASCII())) return false;
			if (!m_objvar_11_types.push_back(myData->objvar_11_type.isNull(), myData->objvar_11_type.getValue())) return false;
			if (!m_objvar_11_values.push_back(myData->objvar_11_value.getValueASCII())) return false;
			if (!m_objvar_12_names.push_back(myData->objvar_12_name.getValueASCII())) return false;
			if (!m_objvar_12_types.push_back(myData->objvar_12_type.isNull(), myData->objvar_12_type.getValue())) return false;
			if (!m_objvar_12_values.push_back(myData->objvar_12_value.getValueASCII())) return false;
			if (!m_objvar_13_names.push_back(myData->objvar_13_name.getValueASCII())) return false;
			if (!m_objvar_13_types.push_back(myData->objvar_13_type.isNull(), myData->objvar_13_type.getValue())) return false;
			if (!m_objvar_13_values.push_back(myData->objvar_13_value.getValueASCII())) return false;
			if (!m_objvar_14_names.push_back(myData->objvar_14_name.getValueASCII())) return false;
			if (!m_objvar_14_types.push_back(myData->objvar_14_type.isNull(), myData->objvar_14_type.getValue())) return false;
			if (!m_objvar_14_values.push_back(myData->objvar_14_value.getValueASCII())) return false;
			if (!m_objvar_15_names.push_back(myData->objvar_15_name.getValueASCII())) return false;
			if (!m_objvar_15_types.push_back(myData->objvar_15_type.isNull(), myData->objvar_15_type.getValue())) return false;
			if (!m_objvar_15_values.push_back(myData->objvar_15_value.getValueASCII())) return false;
			if (!m_objvar_16_names.push_back(myData->objvar_16_name.getValueASCII())) return false;
			if (!m_objvar_16_types.push_back(myData->objvar_16_type.isNull(), myData->objvar_16_type.getValue())) return false;
			if (!m_objvar_16_values.push_back(myData->objvar_16_value.getValueASCII())) return false;
			if (!m_objvar_17_names.push_back(myData->objvar_17_name.getValueASCII())) return false;
			if (!m_objvar_17_types.push_back(myData->objvar_17_type.isNull(), myData->objvar_17_type.getValue())) return false;
			if (!m_objvar_17_values.push_back(myData->objvar_17_value.getValueASCII())) return false;
			if (!m_objvar_18_names.push_back(myData->objvar_18_name.getValueASCII())) return false;
			if (!m_objvar_18_types.push_back(myData->objvar_18_type.isNull(), myData->objvar_18_type.getValue())) return false;
			if (!m_objvar_18_values.push_back(myData->objvar_18_value.getValueASCII())) return false;
			if (!m_objvar_19_names.push_back(myData->objvar_19_name.getValueASCII())) return false;
			if (!m_objvar_19_types.push_back(myData->objvar_19_type.isNull(), myData->objvar_19_type.getValue())) return false;
			if (!m_objvar_19_values.push_back(myData->objvar_19_value.getValueASCII())) return false;
			if (!m_script_lists.push_back(myData->script_list.getValueASCII())) return false;
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
	
	m_numItems=m_numItems.getValue() + 1;
	return true;
}

int ObjectsTableQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void ObjectsTableQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_xs.clear();
			m_ys.clear();
			m_zs.clear();
			m_quaternion_ws.clear();
			m_quaternion_xs.clear();
			m_quaternion_ys.clear();
			m_quaternion_zs.clear();
			m_node_xs.clear();
			m_node_ys.clear();
			m_node_zs.clear();
			m_type_ids.clear();
			m_scene_ids.clear();
			m_controller_types.clear();
			m_deleteds.clear();
			m_object_names.clear();
			m_volumes.clear();
			m_contained_bys.clear();
			m_slot_arrangements.clear();
			m_player_controlleds.clear();
			m_cache_versions.clear();
			m_load_contentss.clear();
			m_cash_balances.clear();
			m_bank_balances.clear();
			m_complexitys.clear();
			m_name_string_tables.clear();
			m_name_string_texts.clear();
			m_static_item_names.clear();
			m_static_item_versions.clear();
			m_conversion_ids.clear();
			m_object_template_ids.clear();
			m_load_withs.clear();
			m_objvar_0_names.clear();
			m_objvar_0_types.clear();
			m_objvar_0_values.clear();
			m_objvar_1_names.clear();
			m_objvar_1_types.clear();
			m_objvar_1_values.clear();
			m_objvar_2_names.clear();
			m_objvar_2_types.clear();
			m_objvar_2_values.clear();
			m_objvar_3_names.clear();
			m_objvar_3_types.clear();
			m_objvar_3_values.clear();
			m_objvar_4_names.clear();
			m_objvar_4_types.clear();
			m_objvar_4_values.clear();
			m_objvar_5_names.clear();
			m_objvar_5_types.clear();
			m_objvar_5_values.clear();
			m_objvar_6_names.clear();
			m_objvar_6_types.clear();
			m_objvar_6_values.clear();
			m_objvar_7_names.clear();
			m_objvar_7_types.clear();
			m_objvar_7_values.clear();
			m_objvar_8_names.clear();
			m_objvar_8_types.clear();
			m_objvar_8_values.clear();
			m_objvar_9_names.clear();
			m_objvar_9_types.clear();
			m_objvar_9_values.clear();
			m_objvar_10_names.clear();
			m_objvar_10_types.clear();
			m_objvar_10_values.clear();
			m_objvar_11_names.clear();
			m_objvar_11_types.clear();
			m_objvar_11_values.clear();
			m_objvar_12_names.clear();
			m_objvar_12_types.clear();
			m_objvar_12_values.clear();
			m_objvar_13_names.clear();
			m_objvar_13_types.clear();
			m_objvar_13_values.clear();
			m_objvar_14_names.clear();
			m_objvar_14_types.clear();
			m_objvar_14_values.clear();
			m_objvar_15_names.clear();
			m_objvar_15_types.clear();
			m_objvar_15_values.clear();
			m_objvar_16_names.clear();
			m_objvar_16_types.clear();
			m_objvar_16_values.clear();
			m_objvar_17_names.clear();
			m_objvar_17_types.clear();
			m_objvar_17_values.clear();
			m_objvar_18_names.clear();
			m_objvar_18_types.clear();
			m_objvar_18_values.clear();
			m_objvar_19_names.clear();
			m_objvar_19_types.clear();
			m_objvar_19_values.clear();
			m_script_lists.clear();
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			m_object_ids.clear();
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

	m_numItems=0;
}

void ObjectsTableQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_xs.free();
			m_ys.free();
			m_zs.free();
			m_quaternion_ws.free();
			m_quaternion_xs.free();
			m_quaternion_ys.free();
			m_quaternion_zs.free();
			m_node_xs.free();
			m_node_ys.free();
			m_node_zs.free();
			m_type_ids.free();
			m_scene_ids.free();
			m_controller_types.free();
			m_deleteds.free();
			m_object_names.free();
			m_volumes.free();
			m_contained_bys.free();
			m_slot_arrangements.free();
			m_player_controlleds.free();
			m_cache_versions.free();
			m_load_contentss.free();
			m_cash_balances.free();
			m_bank_balances.free();
			m_complexitys.free();
			m_name_string_tables.free();
			m_name_string_texts.free();
			m_static_item_names.free();
			m_static_item_versions.free();
			m_conversion_ids.free();
			m_object_template_ids.free();
			m_load_withs.free();
			m_objvar_0_names.free();
			m_objvar_0_types.free();
			m_objvar_0_values.free();
			m_objvar_1_names.free();
			m_objvar_1_types.free();
			m_objvar_1_values.free();
			m_objvar_2_names.free();
			m_objvar_2_types.free();
			m_objvar_2_values.free();
			m_objvar_3_names.free();
			m_objvar_3_types.free();
			m_objvar_3_values.free();
			m_objvar_4_names.free();
			m_objvar_4_types.free();
			m_objvar_4_values.free();
			m_objvar_5_names.free();
			m_objvar_5_types.free();
			m_objvar_5_values.free();
			m_objvar_6_names.free();
			m_objvar_6_types.free();
			m_objvar_6_values.free();
			m_objvar_7_names.free();
			m_objvar_7_types.free();
			m_objvar_7_values.free();
			m_objvar_8_names.free();
			m_objvar_8_types.free();
			m_objvar_8_values.free();
			m_objvar_9_names.free();
			m_objvar_9_types.free();
			m_objvar_9_values.free();
			m_objvar_10_names.free();
			m_objvar_10_types.free();
			m_objvar_10_values.free();
			m_objvar_11_names.free();
			m_objvar_11_types.free();
			m_objvar_11_values.free();
			m_objvar_12_names.free();
			m_objvar_12_types.free();
			m_objvar_12_values.free();
			m_objvar_13_names.free();
			m_objvar_13_types.free();
			m_objvar_13_values.free();
			m_objvar_14_names.free();
			m_objvar_14_types.free();
			m_objvar_14_values.free();
			m_objvar_15_names.free();
			m_objvar_15_types.free();
			m_objvar_15_values.free();
			m_objvar_16_names.free();
			m_objvar_16_types.free();
			m_objvar_16_values.free();
			m_objvar_17_names.free();
			m_objvar_17_types.free();
			m_objvar_17_values.free();
			m_objvar_18_names.free();
			m_objvar_18_types.free();
			m_objvar_18_values.free();
			m_objvar_19_names.free();
			m_objvar_19_types.free();
			m_objvar_19_values.free();
			m_script_lists.free();
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			m_object_ids.free();
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

bool ObjectsTableQuery::bindParameters()
{
	switch (mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_xs)) return false;
			if (!bindParameter(m_ys)) return false;
			if (!bindParameter(m_zs)) return false;
			if (!bindParameter(m_quaternion_ws)) return false;
			if (!bindParameter(m_quaternion_xs)) return false;
			if (!bindParameter(m_quaternion_ys)) return false;
			if (!bindParameter(m_quaternion_zs)) return false;
			if (!bindParameter(m_node_xs)) return false;
			if (!bindParameter(m_node_ys)) return false;
			if (!bindParameter(m_node_zs)) return false;
			if (!bindParameter(m_object_template_ids)) return false;
			if (!bindParameter(m_type_ids)) return false;
			if (!bindParameter(m_scene_ids)) return false;
			if (!bindParameter(m_controller_types)) return false;
			if (!bindParameter(m_deleteds)) return false;
			if (!bindParameter(m_object_names)) return false;
			if (!bindParameter(m_volumes)) return false;
			if (!bindParameter(m_contained_bys)) return false;
			if (!bindParameter(m_slot_arrangements)) return false;
			if (!bindParameter(m_player_controlleds)) return false;
			if (!bindParameter(m_cache_versions)) return false;
			if (!bindParameter(m_load_contentss)) return false;
			if (!bindParameter(m_cash_balances)) return false;
			if (!bindParameter(m_bank_balances)) return false;
			if (!bindParameter(m_complexitys)) return false;
			if (!bindParameter(m_name_string_tables)) return false;
			if (!bindParameter(m_name_string_texts)) return false;
			if (!bindParameter(m_static_item_names)) return false;
			if (!bindParameter(m_static_item_versions)) return false;
			if (!bindParameter(m_conversion_ids)) return false;
			if (!bindParameter(m_load_withs)) return false;	
			if (!bindParameter(m_objvar_0_names)) return false;
			if (!bindParameter(m_objvar_0_types)) return false;
			if (!bindParameter(m_objvar_0_values)) return false;
			if (!bindParameter(m_objvar_1_names)) return false;
			if (!bindParameter(m_objvar_1_types)) return false;
			if (!bindParameter(m_objvar_1_values)) return false;
			if (!bindParameter(m_objvar_2_names)) return false;
			if (!bindParameter(m_objvar_2_types)) return false;
			if (!bindParameter(m_objvar_2_values)) return false;
			if (!bindParameter(m_objvar_3_names)) return false;
			if (!bindParameter(m_objvar_3_types)) return false;
			if (!bindParameter(m_objvar_3_values)) return false;
			if (!bindParameter(m_objvar_4_names)) return false;
			if (!bindParameter(m_objvar_4_types)) return false;
			if (!bindParameter(m_objvar_4_values)) return false;
			if (!bindParameter(m_objvar_5_names)) return false;
			if (!bindParameter(m_objvar_5_types)) return false;
			if (!bindParameter(m_objvar_5_values)) return false;
			if (!bindParameter(m_objvar_6_names)) return false;
			if (!bindParameter(m_objvar_6_types)) return false;
			if (!bindParameter(m_objvar_6_values)) return false;
			if (!bindParameter(m_objvar_7_names)) return false;
			if (!bindParameter(m_objvar_7_types)) return false;
			if (!bindParameter(m_objvar_7_values)) return false;
			if (!bindParameter(m_objvar_8_names)) return false;
			if (!bindParameter(m_objvar_8_types)) return false;
			if (!bindParameter(m_objvar_8_values)) return false;
			if (!bindParameter(m_objvar_9_names)) return false;
			if (!bindParameter(m_objvar_9_types)) return false;
			if (!bindParameter(m_objvar_9_values)) return false;
			if (!bindParameter(m_objvar_10_names)) return false;
			if (!bindParameter(m_objvar_10_types)) return false;
			if (!bindParameter(m_objvar_10_values)) return false;
			if (!bindParameter(m_objvar_11_names)) return false;
			if (!bindParameter(m_objvar_11_types)) return false;
			if (!bindParameter(m_objvar_11_values)) return false;
			if (!bindParameter(m_objvar_12_names)) return false;
			if (!bindParameter(m_objvar_12_types)) return false;
			if (!bindParameter(m_objvar_12_values)) return false;
			if (!bindParameter(m_objvar_13_names)) return false;
			if (!bindParameter(m_objvar_13_types)) return false;
			if (!bindParameter(m_objvar_13_values)) return false;
			if (!bindParameter(m_objvar_14_names)) return false;
			if (!bindParameter(m_objvar_14_types)) return false;
			if (!bindParameter(m_objvar_14_values)) return false;
			if (!bindParameter(m_objvar_15_names)) return false;
			if (!bindParameter(m_objvar_15_types)) return false;
			if (!bindParameter(m_objvar_15_values)) return false;
			if (!bindParameter(m_objvar_16_names)) return false;
			if (!bindParameter(m_objvar_16_types)) return false;
			if (!bindParameter(m_objvar_16_values)) return false;
			if (!bindParameter(m_objvar_17_names)) return false;
			if (!bindParameter(m_objvar_17_types)) return false;
			if (!bindParameter(m_objvar_17_values)) return false;
			if (!bindParameter(m_objvar_18_names)) return false;
			if (!bindParameter(m_objvar_18_types)) return false;
			if (!bindParameter(m_objvar_18_values)) return false;
			if (!bindParameter(m_objvar_19_names)) return false;
			if (!bindParameter(m_objvar_19_types)) return false;
			if (!bindParameter(m_objvar_19_values)) return false;
			if (!bindParameter(m_script_lists)) return false;
 			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_numItems)) return false;
			break;

		case mode_SELECT:
			break;
			
		case mode_DELETE:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_numItems)) return false;
			break;
			
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
	return true;
}

bool ObjectsTableQuery::bindColumns()
{
	return true;
}

void ObjectsTableQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_object (:x, :y, :z, :quaternion_w, :quaternion_x, :quaternion_y, :quaternion_z, :node_x, :node_y, :node_z, :object_template, :type_id, :scene_id, :controller_type, :deleted, :object_name, :volume, :contained_by, :slot_arrangement, :player_controlled, :cache_version, :load_contents, :cash_balance, :bank_balance, :complexity, :name_string_table, :name_string_text, :static_item_name, :static_item_version, :conversion_id, :load_with, :objvar_0_name, :objvar_0_type, :objvar_0_value, :objvar_1_name, :objvar_1_type, :objvar_1_value, :objvar_2_name, :objvar_2_type, :objvar_2_value, :objvar_3_name, :objvar_3_type, :objvar_3_value, :objvar_4_name, :objvar_4_type, :objvar_4_value, :objvar_5_name, :objvar_5_type, :objvar_5_value, :objvar_6_name, :objvar_6_type, :objvar_6_value, :objvar_7_name, :objvar_7_type, :objvar_7_value, :objvar_8_name, :objvar_8_type, :objvar_8_value, :objvar_9_name, :objvar_9_type, :objvar_9_value, :objvar_10_name, :objvar_10_type, :objvar_10_value, :objvar_11_name, :objvar_11_type, :objvar_11_value, :objvar_12_name, :objvar_12_type, :objvar_12_value, :objvar_13_name, :objvar_13_type, :objvar_13_value, :objvar_14_name, :objvar_14_type, :objvar_14_value, :objvar_15_name, :objvar_15_type, :objvar_15_value, :objvar_16_name, :objvar_16_type, :objvar_16_value, :objvar_17_name, :objvar_17_type, :objvar_17_value, :objvar_18_name, :objvar_18_type, :objvar_18_value, :objvar_19_name, :objvar_19_type, :objvar_19_value, :script_list, :object_id, :chunk_size); end;";
			break;
		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_object (:x,:y,:z,:quaternion_w,:quaternion_x,:quaternion_y,:quaternion_z,:node_x,:node_y,:node_z,:object_template,:type_id,:scene_id,:controller_type,:deleted,:object_name,:volume,:contained_by,:slot_arrangement,:player_controlled,:cache_version, :load_contents, :cash_balance, :bank_balance, :complexity, :name_string_table, :name_string_text, :static_item_name, :static_item_version, :conversion_id, :load_with, :objvar_0_name, :objvar_0_type, :objvar_0_value, :objvar_1_name, :objvar_1_type, :objvar_1_value, :objvar_2_name, :objvar_2_type, :objvar_2_value, :objvar_3_name, :objvar_3_type, :objvar_3_value, :objvar_4_name, :objvar_4_type, :objvar_4_value, :objvar_5_name, :objvar_5_type, :objvar_5_value, :objvar_6_name, :objvar_6_type, :objvar_6_value, :objvar_7_name, :objvar_7_type, :objvar_7_value, :objvar_8_name, :objvar_8_type, :objvar_8_value, :objvar_9_name, :objvar_9_type, :objvar_9_value, :objvar_10_name, :objvar_10_type, :objvar_10_value, :objvar_11_name, :objvar_11_type, :objvar_11_value, :objvar_12_name, :objvar_12_type, :objvar_12_value, :objvar_13_name, :objvar_13_type, :objvar_13_value, :objvar_14_name, :objvar_14_type, :objvar_14_value, :objvar_15_name, :objvar_15_type, :objvar_15_value, :objvar_16_name, :objvar_16_type, :objvar_16_value, :objvar_17_name, :objvar_17_type, :objvar_17_value, :objvar_18_name, :objvar_18_type, :objvar_18_value, :objvar_19_name, :objvar_19_type, :objvar_19_value, :script_list, :object_id, :chunk_size); end;";

			break;
		case mode_SELECT:
			break;
		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_object (:object_id, :chunk_size); end;";
			break;
		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
}

void ObjectsTableQuery::setObjectId(const NetworkId &objectId)
{
	ObjectRow *myData=safe_cast<ObjectRow*>(data);
	myData->object_id.setValue(objectId);
}

// ======================================================================

ObjectsTableQuerySelect::ObjectsTableQuerySelect(const std::string &schema) :
		m_data(1000),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

ObjectsTableQuerySelect::~ObjectsTableQuerySelect()
{
}

// ----------------------------------------------------------------------

const std::vector<DBSchema::ObjectRow> & ObjectsTableQuerySelect::getData()
{
	return m_data;
}

// ----------------------------------------------------------------------

bool ObjectsTableQuerySelect::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool ObjectsTableQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, 1000);
	
	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].x)) return false;
	if (!bindCol(m_data[0].y)) return false;
	if (!bindCol(m_data[0].z)) return false;
	if (!bindCol(m_data[0].quaternion_w)) return false;
	if (!bindCol(m_data[0].quaternion_x)) return false;
	if (!bindCol(m_data[0].quaternion_y)) return false;
	if (!bindCol(m_data[0].quaternion_z)) return false;
	if (!bindCol(m_data[0].node_x)) return false;
	if (!bindCol(m_data[0].node_y)) return false;
	if (!bindCol(m_data[0].node_z)) return false;
	if (!bindCol(m_data[0].object_template_id)) return false;
	if (!bindCol(m_data[0].type_id)) return false;
	if (!bindCol(m_data[0].scene_id)) return false;
	if (!bindCol(m_data[0].controller_type)) return false;
	if (!bindCol(m_data[0].deleted)) return false;
	if (!bindCol(m_data[0].object_name)) return false;
	if (!bindCol(m_data[0].volume)) return false;
	if (!bindCol(m_data[0].contained_by)) return false;
	if (!bindCol(m_data[0].slot_arrangement)) return false;
	if (!bindCol(m_data[0].player_controlled)) return false;
	if (!bindCol(m_data[0].cache_version)) return false;
	if (!bindCol(m_data[0].load_contents)) return false;
	if (!bindCol(m_data[0].cash_balance)) return false;
	if (!bindCol(m_data[0].bank_balance)) return false;
	if (!bindCol(m_data[0].complexity)) return false;
	if (!bindCol(m_data[0].name_string_table)) return false;
	if (!bindCol(m_data[0].name_string_text)) return false;
	if (!bindCol(m_data[0].static_item_name)) return false;
	if (!bindCol(m_data[0].static_item_version)) return false;
	if (!bindCol(m_data[0].conversion_id)) return false;
	if (!bindCol(m_data[0].load_with)) return false;

	if (!bindCol(m_data[0].objvar_0_name)) return false;
	if (!bindCol(m_data[0].objvar_0_type)) return false;
	if (!bindCol(m_data[0].objvar_0_value)) return false;
	if (!bindCol(m_data[0].objvar_1_name)) return false;
	if (!bindCol(m_data[0].objvar_1_type)) return false;
	if (!bindCol(m_data[0].objvar_1_value)) return false;
	if (!bindCol(m_data[0].objvar_2_name)) return false;
	if (!bindCol(m_data[0].objvar_2_type)) return false;
	if (!bindCol(m_data[0].objvar_2_value)) return false;
	if (!bindCol(m_data[0].objvar_3_name)) return false;
	if (!bindCol(m_data[0].objvar_3_type)) return false;
	if (!bindCol(m_data[0].objvar_3_value)) return false;
	if (!bindCol(m_data[0].objvar_4_name)) return false;
	if (!bindCol(m_data[0].objvar_4_type)) return false;
	if (!bindCol(m_data[0].objvar_4_value)) return false;
	if (!bindCol(m_data[0].objvar_5_name)) return false;
	if (!bindCol(m_data[0].objvar_5_type)) return false;
	if (!bindCol(m_data[0].objvar_5_value)) return false;
	if (!bindCol(m_data[0].objvar_6_name)) return false;
	if (!bindCol(m_data[0].objvar_6_type)) return false;
	if (!bindCol(m_data[0].objvar_6_value)) return false;
	if (!bindCol(m_data[0].objvar_7_name)) return false;
	if (!bindCol(m_data[0].objvar_7_type)) return false;
	if (!bindCol(m_data[0].objvar_7_value)) return false;
	if (!bindCol(m_data[0].objvar_8_name)) return false;
	if (!bindCol(m_data[0].objvar_8_type)) return false;
	if (!bindCol(m_data[0].objvar_8_value)) return false;
	if (!bindCol(m_data[0].objvar_9_name)) return false;
	if (!bindCol(m_data[0].objvar_9_type)) return false;
	if (!bindCol(m_data[0].objvar_9_value)) return false; 

	if (!bindCol(m_data[0].objvar_10_name)) return false;
	if (!bindCol(m_data[0].objvar_10_type)) return false;
	if (!bindCol(m_data[0].objvar_10_value)) return false;
	if (!bindCol(m_data[0].objvar_11_name)) return false;
	if (!bindCol(m_data[0].objvar_11_type)) return false;
	if (!bindCol(m_data[0].objvar_11_value)) return false;
	if (!bindCol(m_data[0].objvar_12_name)) return false;
	if (!bindCol(m_data[0].objvar_12_type)) return false;
	if (!bindCol(m_data[0].objvar_12_value)) return false;
	if (!bindCol(m_data[0].objvar_13_name)) return false;
	if (!bindCol(m_data[0].objvar_13_type)) return false;
	if (!bindCol(m_data[0].objvar_13_value)) return false;
	if (!bindCol(m_data[0].objvar_14_name)) return false;
	if (!bindCol(m_data[0].objvar_14_type)) return false;
	if (!bindCol(m_data[0].objvar_14_value)) return false;
	if (!bindCol(m_data[0].objvar_15_name)) return false;
	if (!bindCol(m_data[0].objvar_15_type)) return false;
	if (!bindCol(m_data[0].objvar_15_value)) return false;
	if (!bindCol(m_data[0].objvar_16_name)) return false;
	if (!bindCol(m_data[0].objvar_16_type)) return false;
	if (!bindCol(m_data[0].objvar_16_value)) return false;
	if (!bindCol(m_data[0].objvar_17_name)) return false;
	if (!bindCol(m_data[0].objvar_17_type)) return false;
	if (!bindCol(m_data[0].objvar_17_value)) return false;
	if (!bindCol(m_data[0].objvar_18_name)) return false;
	if (!bindCol(m_data[0].objvar_18_type)) return false;
	if (!bindCol(m_data[0].objvar_18_value)) return false;
	if (!bindCol(m_data[0].objvar_19_name)) return false;
	if (!bindCol(m_data[0].objvar_19_type)) return false;
	if (!bindCol(m_data[0].objvar_19_value)) return false;
	
	if (!bindCol(m_data[0].script_list)) return false; 
 	
	if (!bindCol(m_data[0].container_level)) return false;

	return true;
}

// ----------------------------------------------------------------------

void ObjectsTableQuerySelect::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+m_schema+"loader.load_object; end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode ObjectsTableQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================
