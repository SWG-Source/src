#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "ObjectQueries.h"
#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/DatabaseProcess.h"

using namespace DBQuery;
using namespace DBSchema;

// ======================================================================

namespace ObjectQueriesNamespace
{
	const int ms_fetchBatchSize = 1000;
}
using namespace ObjectQueriesNamespace;

// ======================================================================

//!!!BEGIN GENERATED QUERYDEFINITIONS
BattlefieldMarkerObjectQuery::BattlefieldMarkerObjectQuery() :
	DatabaseProcessQuery(new BattlefieldMarkerObjectRow),
	m_numItems(0)
{
}

bool BattlefieldMarkerObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_region_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool BattlefieldMarkerObjectQuery::addData(const DB::Row *_data)
{
	const BattlefieldMarkerObjectBufferRow *myData=safe_cast<const BattlefieldMarkerObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_region_names.push_back(myData->region_name.getValueASCII())) return false;
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

int BattlefieldMarkerObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void BattlefieldMarkerObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_region_names.clear();
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

void BattlefieldMarkerObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_region_names.free();
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

bool BattlefieldMarkerObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_region_names)) return false;
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

bool BattlefieldMarkerObjectQuery::bindColumns()
{
	return true;
}

void BattlefieldMarkerObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_battlefield_marker_obj (:object_id, :region_name, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_battlefield_marker_obj (:object_id, :region_name, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_battlefield_marker_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

BattlefieldMarkerObjectQuerySelect::BattlefieldMarkerObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool BattlefieldMarkerObjectQuerySelect::bindParameters ()
{
	return true;
}

bool BattlefieldMarkerObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].region_name)) return false;
	return true;
}

const std::vector<BattlefieldMarkerObjectRow> & BattlefieldMarkerObjectQuerySelect::getData() const
{
	return m_data;
}

void BattlefieldMarkerObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_battlefield_marker_object; end;";
}

DB::Query::QueryMode BattlefieldMarkerObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

BuildingObjectQuery::BuildingObjectQuery() :
	DatabaseProcessQuery(new BuildingObjectRow),
	m_numItems(0)
{
}

bool BuildingObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_maintenance_costs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_time_last_checkeds.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_is_publics.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_city_ids.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
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

bool BuildingObjectQuery::addData(const DB::Row *_data)
{
	const BuildingObjectBufferRow *myData=safe_cast<const BuildingObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_maintenance_costs.push_back(myData->maintenance_cost.isNull(), myData->maintenance_cost.getValue())) return false;
			if (!m_time_last_checkeds.push_back(myData->time_last_checked.isNull(), myData->time_last_checked.getValue())) return false;
			if (!m_is_publics.push_back(myData->is_public.getValueASCII())) return false;
			if (!m_city_ids.push_back(myData->city_id.isNull(), myData->city_id.getValue())) return false;
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

int BuildingObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void BuildingObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_maintenance_costs.clear();
			m_time_last_checkeds.clear();
			m_is_publics.clear();
			m_city_ids.clear();
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

void BuildingObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_maintenance_costs.free();
			m_time_last_checkeds.free();
			m_is_publics.free();
			m_city_ids.free();
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

bool BuildingObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_maintenance_costs)) return false;
			if (!bindParameter(m_time_last_checkeds)) return false;
			if (!bindParameter(m_is_publics)) return false;
			if (!bindParameter(m_city_ids)) return false;
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

bool BuildingObjectQuery::bindColumns()
{
	return true;
}

void BuildingObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_building_obj (:object_id, :maintenance_cost, :time_last_checked, :is_public, :city_id, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_building_obj (:object_id, :maintenance_cost, :time_last_checked, :is_public, :city_id, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_building_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

BuildingObjectQuerySelect::BuildingObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool BuildingObjectQuerySelect::bindParameters ()
{
	return true;
}

bool BuildingObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].maintenance_cost)) return false;
	if (!bindCol(m_data[0].time_last_checked)) return false;
	if (!bindCol(m_data[0].is_public)) return false;
	if (!bindCol(m_data[0].city_id)) return false;
	return true;
}

const std::vector<BuildingObjectRow> & BuildingObjectQuerySelect::getData() const
{
	return m_data;
}

void BuildingObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_building_object; end;";
}

DB::Query::QueryMode BuildingObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

CellObjectQuery::CellObjectQuery() :
	DatabaseProcessQuery(new CellObjectRow),
	m_numItems(0)
{
}

bool CellObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_cell_numbers.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_is_publics.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool CellObjectQuery::addData(const DB::Row *_data)
{
	const CellObjectBufferRow *myData=safe_cast<const CellObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_cell_numbers.push_back(myData->cell_number.isNull(), myData->cell_number.getValue())) return false;
			if (!m_is_publics.push_back(myData->is_public.getValueASCII())) return false;
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

int CellObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void CellObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_cell_numbers.clear();
			m_is_publics.clear();
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

void CellObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_cell_numbers.free();
			m_is_publics.free();
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

bool CellObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_cell_numbers)) return false;
			if (!bindParameter(m_is_publics)) return false;
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

bool CellObjectQuery::bindColumns()
{
	return true;
}

void CellObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_cell_obj (:object_id, :cell_number, :is_public, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_cell_obj (:object_id, :cell_number, :is_public, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_cell_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

CellObjectQuerySelect::CellObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool CellObjectQuerySelect::bindParameters ()
{
	return true;
}

bool CellObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].cell_number)) return false;
	if (!bindCol(m_data[0].is_public)) return false;
	return true;
}

const std::vector<CellObjectRow> & CellObjectQuerySelect::getData() const
{
	return m_data;
}

void CellObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_cell_object; end;";
}

DB::Query::QueryMode CellObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

CityObjectQuery::CityObjectQuery() :
	DatabaseProcessQuery(new CityObjectRow),
	m_numItems(0)
{
}

bool CityObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool CityObjectQuery::addData(const DB::Row *_data)
{
	const CityObjectBufferRow *myData=safe_cast<const CityObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
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

int CityObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void CityObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
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

void CityObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
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

bool CityObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
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

bool CityObjectQuery::bindColumns()
{
	return true;
}

void CityObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_city_obj (:object_id, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_city_obj (:object_id, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_city_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

CityObjectQuerySelect::CityObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool CityObjectQuerySelect::bindParameters ()
{
	return true;
}

bool CityObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	return true;
}

const std::vector<CityObjectRow> & CityObjectQuerySelect::getData() const
{
	return m_data;
}

void CityObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_city_object; end;";
}

DB::Query::QueryMode CityObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

CreatureObjectQuery::CreatureObjectQuery() :
	DatabaseProcessQuery(new CreatureObjectRow),
	m_numItems(0)
{
}

bool CreatureObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_scale_factors.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_statess.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_postures.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_shock_woundss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_master_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_ranks.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_base_walk_speeds.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_base_run_speeds.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_0s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_1s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_2s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_3s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_4s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_5s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_6s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_7s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_8s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_9s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_10s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_11s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_12s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_13s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_14s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_15s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_16s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_17s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_18s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_19s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_20s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_21s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_22s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_23s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_24s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_25s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attribute_26s.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_persisted_buffss.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_ws_xs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_ws_ys.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_ws_zs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
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

bool CreatureObjectQuery::addData(const DB::Row *_data)
{
	const CreatureObjectBufferRow *myData=safe_cast<const CreatureObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_scale_factors.push_back(myData->scale_factor.isNull(), myData->scale_factor.getValue())) return false;
			if (!m_statess.push_back(myData->states.isNull(), myData->states.getValue())) return false;
			if (!m_postures.push_back(myData->posture.isNull(), myData->posture.getValue())) return false;
			if (!m_shock_woundss.push_back(myData->shock_wounds.isNull(), myData->shock_wounds.getValue())) return false;
			if (!m_master_ids.push_back(myData->master_id.getValueASCII())) return false;
			if (!m_ranks.push_back(myData->rank.isNull(), myData->rank.getValue())) return false;
			if (!m_base_walk_speeds.push_back(myData->base_walk_speed.isNull(), myData->base_walk_speed.getValue())) return false;
			if (!m_base_run_speeds.push_back(myData->base_run_speed.isNull(), myData->base_run_speed.getValue())) return false;
			if (!m_attribute_0s.push_back(myData->attribute_0.isNull(), myData->attribute_0.getValue())) return false;
			if (!m_attribute_1s.push_back(myData->attribute_1.isNull(), myData->attribute_1.getValue())) return false;
			if (!m_attribute_2s.push_back(myData->attribute_2.isNull(), myData->attribute_2.getValue())) return false;
			if (!m_attribute_3s.push_back(myData->attribute_3.isNull(), myData->attribute_3.getValue())) return false;
			if (!m_attribute_4s.push_back(myData->attribute_4.isNull(), myData->attribute_4.getValue())) return false;
			if (!m_attribute_5s.push_back(myData->attribute_5.isNull(), myData->attribute_5.getValue())) return false;
			if (!m_attribute_6s.push_back(myData->attribute_6.isNull(), myData->attribute_6.getValue())) return false;
			if (!m_attribute_7s.push_back(myData->attribute_7.isNull(), myData->attribute_7.getValue())) return false;
			if (!m_attribute_8s.push_back(myData->attribute_8.isNull(), myData->attribute_8.getValue())) return false;
			if (!m_attribute_9s.push_back(myData->attribute_9.isNull(), myData->attribute_9.getValue())) return false;
			if (!m_attribute_10s.push_back(myData->attribute_10.isNull(), myData->attribute_10.getValue())) return false;
			if (!m_attribute_11s.push_back(myData->attribute_11.isNull(), myData->attribute_11.getValue())) return false;
			if (!m_attribute_12s.push_back(myData->attribute_12.isNull(), myData->attribute_12.getValue())) return false;
			if (!m_attribute_13s.push_back(myData->attribute_13.isNull(), myData->attribute_13.getValue())) return false;
			if (!m_attribute_14s.push_back(myData->attribute_14.isNull(), myData->attribute_14.getValue())) return false;
			if (!m_attribute_15s.push_back(myData->attribute_15.isNull(), myData->attribute_15.getValue())) return false;
			if (!m_attribute_16s.push_back(myData->attribute_16.isNull(), myData->attribute_16.getValue())) return false;
			if (!m_attribute_17s.push_back(myData->attribute_17.isNull(), myData->attribute_17.getValue())) return false;
			if (!m_attribute_18s.push_back(myData->attribute_18.isNull(), myData->attribute_18.getValue())) return false;
			if (!m_attribute_19s.push_back(myData->attribute_19.isNull(), myData->attribute_19.getValue())) return false;
			if (!m_attribute_20s.push_back(myData->attribute_20.isNull(), myData->attribute_20.getValue())) return false;
			if (!m_attribute_21s.push_back(myData->attribute_21.isNull(), myData->attribute_21.getValue())) return false;
			if (!m_attribute_22s.push_back(myData->attribute_22.isNull(), myData->attribute_22.getValue())) return false;
			if (!m_attribute_23s.push_back(myData->attribute_23.isNull(), myData->attribute_23.getValue())) return false;
			if (!m_attribute_24s.push_back(myData->attribute_24.isNull(), myData->attribute_24.getValue())) return false;
			if (!m_attribute_25s.push_back(myData->attribute_25.isNull(), myData->attribute_25.getValue())) return false;
			if (!m_attribute_26s.push_back(myData->attribute_26.isNull(), myData->attribute_26.getValue())) return false;
			if (!m_persisted_buffss.push_back(myData->persisted_buffs.getValueASCII())) return false;
			if (!m_ws_xs.push_back(myData->ws_x.isNull(), myData->ws_x.getValue())) return false;
			if (!m_ws_ys.push_back(myData->ws_y.isNull(), myData->ws_y.getValue())) return false;
			if (!m_ws_zs.push_back(myData->ws_z.isNull(), myData->ws_z.getValue())) return false;
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

int CreatureObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void CreatureObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_scale_factors.clear();
			m_statess.clear();
			m_postures.clear();
			m_shock_woundss.clear();
			m_master_ids.clear();
			m_ranks.clear();
			m_base_walk_speeds.clear();
			m_base_run_speeds.clear();
			m_attribute_0s.clear();
			m_attribute_1s.clear();
			m_attribute_2s.clear();
			m_attribute_3s.clear();
			m_attribute_4s.clear();
			m_attribute_5s.clear();
			m_attribute_6s.clear();
			m_attribute_7s.clear();
			m_attribute_8s.clear();
			m_attribute_9s.clear();
			m_attribute_10s.clear();
			m_attribute_11s.clear();
			m_attribute_12s.clear();
			m_attribute_13s.clear();
			m_attribute_14s.clear();
			m_attribute_15s.clear();
			m_attribute_16s.clear();
			m_attribute_17s.clear();
			m_attribute_18s.clear();
			m_attribute_19s.clear();
			m_attribute_20s.clear();
			m_attribute_21s.clear();
			m_attribute_22s.clear();
			m_attribute_23s.clear();
			m_attribute_24s.clear();
			m_attribute_25s.clear();
			m_attribute_26s.clear();
			m_persisted_buffss.clear();
			m_ws_xs.clear();
			m_ws_ys.clear();
			m_ws_zs.clear();
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

void CreatureObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_scale_factors.free();
			m_statess.free();
			m_postures.free();
			m_shock_woundss.free();
			m_master_ids.free();
			m_ranks.free();
			m_base_walk_speeds.free();
			m_base_run_speeds.free();
			m_attribute_0s.free();
			m_attribute_1s.free();
			m_attribute_2s.free();
			m_attribute_3s.free();
			m_attribute_4s.free();
			m_attribute_5s.free();
			m_attribute_6s.free();
			m_attribute_7s.free();
			m_attribute_8s.free();
			m_attribute_9s.free();
			m_attribute_10s.free();
			m_attribute_11s.free();
			m_attribute_12s.free();
			m_attribute_13s.free();
			m_attribute_14s.free();
			m_attribute_15s.free();
			m_attribute_16s.free();
			m_attribute_17s.free();
			m_attribute_18s.free();
			m_attribute_19s.free();
			m_attribute_20s.free();
			m_attribute_21s.free();
			m_attribute_22s.free();
			m_attribute_23s.free();
			m_attribute_24s.free();
			m_attribute_25s.free();
			m_attribute_26s.free();
			m_persisted_buffss.free();
			m_ws_xs.free();
			m_ws_ys.free();
			m_ws_zs.free();
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

bool CreatureObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_scale_factors)) return false;
			if (!bindParameter(m_statess)) return false;
			if (!bindParameter(m_postures)) return false;
			if (!bindParameter(m_shock_woundss)) return false;
			if (!bindParameter(m_master_ids)) return false;
			if (!bindParameter(m_ranks)) return false;
			if (!bindParameter(m_base_walk_speeds)) return false;
			if (!bindParameter(m_base_run_speeds)) return false;
			if (!bindParameter(m_attribute_0s)) return false;
			if (!bindParameter(m_attribute_1s)) return false;
			if (!bindParameter(m_attribute_2s)) return false;
			if (!bindParameter(m_attribute_3s)) return false;
			if (!bindParameter(m_attribute_4s)) return false;
			if (!bindParameter(m_attribute_5s)) return false;
			if (!bindParameter(m_attribute_6s)) return false;
			if (!bindParameter(m_attribute_7s)) return false;
			if (!bindParameter(m_attribute_8s)) return false;
			if (!bindParameter(m_attribute_9s)) return false;
			if (!bindParameter(m_attribute_10s)) return false;
			if (!bindParameter(m_attribute_11s)) return false;
			if (!bindParameter(m_attribute_12s)) return false;
			if (!bindParameter(m_attribute_13s)) return false;
			if (!bindParameter(m_attribute_14s)) return false;
			if (!bindParameter(m_attribute_15s)) return false;
			if (!bindParameter(m_attribute_16s)) return false;
			if (!bindParameter(m_attribute_17s)) return false;
			if (!bindParameter(m_attribute_18s)) return false;
			if (!bindParameter(m_attribute_19s)) return false;
			if (!bindParameter(m_attribute_20s)) return false;
			if (!bindParameter(m_attribute_21s)) return false;
			if (!bindParameter(m_attribute_22s)) return false;
			if (!bindParameter(m_attribute_23s)) return false;
			if (!bindParameter(m_attribute_24s)) return false;
			if (!bindParameter(m_attribute_25s)) return false;
			if (!bindParameter(m_attribute_26s)) return false;
			if (!bindParameter(m_persisted_buffss)) return false;
			if (!bindParameter(m_ws_xs)) return false;
			if (!bindParameter(m_ws_ys)) return false;
			if (!bindParameter(m_ws_zs)) return false;
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

bool CreatureObjectQuery::bindColumns()
{
	return true;
}

void CreatureObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_creature_obj (:object_id, :scale_factor, :states, :posture, :shock_wounds, :master_id, :rank, :base_walk_speed, :base_run_speed, :attribute_0, :attribute_1, :attribute_2, :attribute_3, :attribute_4, :attribute_5, :attribute_6, :attribute_7, :attribute_8, :attribute_9, :attribute_10, :attribute_11, :attribute_12, :attribute_13, :attribute_14, :attribute_15, :attribute_16, :attribute_17, :attribute_18, :attribute_19, :attribute_20, :attribute_21, :attribute_22, :attribute_23, :attribute_24, :attribute_25, :attribute_26, :persisted_buffs, :ws_x, :ws_y, :ws_z, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_creature_obj (:object_id, :scale_factor, :states, :posture, :shock_wounds, :master_id, :rank, :base_walk_speed, :base_run_speed, :attribute_0, :attribute_1, :attribute_2, :attribute_3, :attribute_4, :attribute_5, :attribute_6, :attribute_7, :attribute_8, :attribute_9, :attribute_10, :attribute_11, :attribute_12, :attribute_13, :attribute_14, :attribute_15, :attribute_16, :attribute_17, :attribute_18, :attribute_19, :attribute_20, :attribute_21, :attribute_22, :attribute_23, :attribute_24, :attribute_25, :attribute_26, :persisted_buffs, :ws_x, :ws_y, :ws_z, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_creature_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

CreatureObjectQuerySelect::CreatureObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool CreatureObjectQuerySelect::bindParameters ()
{
	return true;
}

bool CreatureObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].scale_factor)) return false;
	if (!bindCol(m_data[0].states)) return false;
	if (!bindCol(m_data[0].posture)) return false;
	if (!bindCol(m_data[0].shock_wounds)) return false;
	if (!bindCol(m_data[0].master_id)) return false;
	if (!bindCol(m_data[0].rank)) return false;
	if (!bindCol(m_data[0].base_walk_speed)) return false;
	if (!bindCol(m_data[0].base_run_speed)) return false;
	if (!bindCol(m_data[0].attribute_0)) return false;
	if (!bindCol(m_data[0].attribute_1)) return false;
	if (!bindCol(m_data[0].attribute_2)) return false;
	if (!bindCol(m_data[0].attribute_3)) return false;
	if (!bindCol(m_data[0].attribute_4)) return false;
	if (!bindCol(m_data[0].attribute_5)) return false;
	if (!bindCol(m_data[0].attribute_6)) return false;
	if (!bindCol(m_data[0].attribute_7)) return false;
	if (!bindCol(m_data[0].attribute_8)) return false;
	if (!bindCol(m_data[0].attribute_9)) return false;
	if (!bindCol(m_data[0].attribute_10)) return false;
	if (!bindCol(m_data[0].attribute_11)) return false;
	if (!bindCol(m_data[0].attribute_12)) return false;
	if (!bindCol(m_data[0].attribute_13)) return false;
	if (!bindCol(m_data[0].attribute_14)) return false;
	if (!bindCol(m_data[0].attribute_15)) return false;
	if (!bindCol(m_data[0].attribute_16)) return false;
	if (!bindCol(m_data[0].attribute_17)) return false;
	if (!bindCol(m_data[0].attribute_18)) return false;
	if (!bindCol(m_data[0].attribute_19)) return false;
	if (!bindCol(m_data[0].attribute_20)) return false;
	if (!bindCol(m_data[0].attribute_21)) return false;
	if (!bindCol(m_data[0].attribute_22)) return false;
	if (!bindCol(m_data[0].attribute_23)) return false;
	if (!bindCol(m_data[0].attribute_24)) return false;
	if (!bindCol(m_data[0].attribute_25)) return false;
	if (!bindCol(m_data[0].attribute_26)) return false;
	if (!bindCol(m_data[0].persisted_buffs)) return false;
	if (!bindCol(m_data[0].ws_x)) return false;
	if (!bindCol(m_data[0].ws_y)) return false;
	if (!bindCol(m_data[0].ws_z)) return false;
	return true;
}

const std::vector<CreatureObjectRow> & CreatureObjectQuerySelect::getData() const
{
	return m_data;
}

void CreatureObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_creature_object; end;";
}

DB::Query::QueryMode CreatureObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

FactoryObjectQuery::FactoryObjectQuery() :
	DatabaseProcessQuery(new FactoryObjectRow),
	m_numItems(0)
{
}

bool FactoryObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool FactoryObjectQuery::addData(const DB::Row *_data)
{
	const FactoryObjectBufferRow *myData=safe_cast<const FactoryObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
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

int FactoryObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void FactoryObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
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

void FactoryObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
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

bool FactoryObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
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

bool FactoryObjectQuery::bindColumns()
{
	return true;
}

void FactoryObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_factory_obj (:object_id, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_factory_obj (:object_id, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_factory_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

FactoryObjectQuerySelect::FactoryObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool FactoryObjectQuerySelect::bindParameters ()
{
	return true;
}

bool FactoryObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	return true;
}

const std::vector<FactoryObjectRow> & FactoryObjectQuerySelect::getData() const
{
	return m_data;
}

void FactoryObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_factory_object; end;";
}

DB::Query::QueryMode FactoryObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

GuildObjectQuery::GuildObjectQuery() :
	DatabaseProcessQuery(new GuildObjectRow),
	m_numItems(0)
{
}

bool GuildObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool GuildObjectQuery::addData(const DB::Row *_data)
{
	const GuildObjectBufferRow *myData=safe_cast<const GuildObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
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

int GuildObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void GuildObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
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

void GuildObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
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

bool GuildObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
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

bool GuildObjectQuery::bindColumns()
{
	return true;
}

void GuildObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_guild_obj (:object_id, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_guild_obj (:object_id, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_guild_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

GuildObjectQuerySelect::GuildObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool GuildObjectQuerySelect::bindParameters ()
{
	return true;
}

bool GuildObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	return true;
}

const std::vector<GuildObjectRow> & GuildObjectQuerySelect::getData() const
{
	return m_data;
}

void GuildObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_guild_object; end;";
}

DB::Query::QueryMode GuildObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

HarvesterInstallationObjectQuery::HarvesterInstallationObjectQuery() :
	DatabaseProcessQuery(new HarvesterInstallationObjectRow),
	m_numItems(0)
{
}

bool HarvesterInstallationObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_installed_efficiencys.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_max_extraction_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_current_extraction_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_max_hopper_amounts.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_hopper_resources.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_hopper_amounts.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_resource_types.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool HarvesterInstallationObjectQuery::addData(const DB::Row *_data)
{
	const HarvesterInstallationObjectBufferRow *myData=safe_cast<const HarvesterInstallationObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_installed_efficiencys.push_back(myData->installed_efficiency.isNull(), myData->installed_efficiency.getValue())) return false;
			if (!m_max_extraction_rates.push_back(myData->max_extraction_rate.isNull(), myData->max_extraction_rate.getValue())) return false;
			if (!m_current_extraction_rates.push_back(myData->current_extraction_rate.isNull(), myData->current_extraction_rate.getValue())) return false;
			if (!m_max_hopper_amounts.push_back(myData->max_hopper_amount.isNull(), myData->max_hopper_amount.getValue())) return false;
			if (!m_hopper_resources.push_back(myData->hopper_resource.getValueASCII())) return false;
			if (!m_hopper_amounts.push_back(myData->hopper_amount.isNull(), myData->hopper_amount.getValue())) return false;
			if (!m_resource_types.push_back(myData->resource_type.getValueASCII())) return false;
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

int HarvesterInstallationObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void HarvesterInstallationObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_installed_efficiencys.clear();
			m_max_extraction_rates.clear();
			m_current_extraction_rates.clear();
			m_max_hopper_amounts.clear();
			m_hopper_resources.clear();
			m_hopper_amounts.clear();
			m_resource_types.clear();
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

void HarvesterInstallationObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_installed_efficiencys.free();
			m_max_extraction_rates.free();
			m_current_extraction_rates.free();
			m_max_hopper_amounts.free();
			m_hopper_resources.free();
			m_hopper_amounts.free();
			m_resource_types.free();
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

bool HarvesterInstallationObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_installed_efficiencys)) return false;
			if (!bindParameter(m_max_extraction_rates)) return false;
			if (!bindParameter(m_current_extraction_rates)) return false;
			if (!bindParameter(m_max_hopper_amounts)) return false;
			if (!bindParameter(m_hopper_resources)) return false;
			if (!bindParameter(m_hopper_amounts)) return false;
			if (!bindParameter(m_resource_types)) return false;
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

bool HarvesterInstallationObjectQuery::bindColumns()
{
	return true;
}

void HarvesterInstallationObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_harvester_inst_obj (:object_id, :installed_efficiency, :max_extraction_rate, :current_extraction_rate, :max_hopper_amount, :hopper_resource, :hopper_amount, :resource_type, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_harvester_inst_obj (:object_id, :installed_efficiency, :max_extraction_rate, :current_extraction_rate, :max_hopper_amount, :hopper_resource, :hopper_amount, :resource_type, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_harvester_inst_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

HarvesterInstallationObjectQuerySelect::HarvesterInstallationObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool HarvesterInstallationObjectQuerySelect::bindParameters ()
{
	return true;
}

bool HarvesterInstallationObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].installed_efficiency)) return false;
	if (!bindCol(m_data[0].max_extraction_rate)) return false;
	if (!bindCol(m_data[0].current_extraction_rate)) return false;
	if (!bindCol(m_data[0].max_hopper_amount)) return false;
	if (!bindCol(m_data[0].hopper_resource)) return false;
	if (!bindCol(m_data[0].hopper_amount)) return false;
	if (!bindCol(m_data[0].resource_type)) return false;
	return true;
}

const std::vector<HarvesterInstallationObjectRow> & HarvesterInstallationObjectQuerySelect::getData() const
{
	return m_data;
}

void HarvesterInstallationObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_harvester_inst_object; end;";
}

DB::Query::QueryMode HarvesterInstallationObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

InstallationObjectQuery::InstallationObjectQuery() :
	DatabaseProcessQuery(new InstallationObjectRow),
	m_numItems(0)
{
}

bool InstallationObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_installation_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_activateds.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_tick_counts.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_activate_start_times.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_powers.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_power_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
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

bool InstallationObjectQuery::addData(const DB::Row *_data)
{
	const InstallationObjectBufferRow *myData=safe_cast<const InstallationObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_installation_types.push_back(myData->installation_type.isNull(), myData->installation_type.getValue())) return false;
			if (!m_activateds.push_back(myData->activated.getValueASCII())) return false;
			if (!m_tick_counts.push_back(myData->tick_count.isNull(), myData->tick_count.getValue())) return false;
			if (!m_activate_start_times.push_back(myData->activate_start_time.isNull(), myData->activate_start_time.getValue())) return false;
			if (!m_powers.push_back(myData->power.isNull(), myData->power.getValue())) return false;
			if (!m_power_rates.push_back(myData->power_rate.isNull(), myData->power_rate.getValue())) return false;
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

int InstallationObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void InstallationObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_installation_types.clear();
			m_activateds.clear();
			m_tick_counts.clear();
			m_activate_start_times.clear();
			m_powers.clear();
			m_power_rates.clear();
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

void InstallationObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_installation_types.free();
			m_activateds.free();
			m_tick_counts.free();
			m_activate_start_times.free();
			m_powers.free();
			m_power_rates.free();
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

bool InstallationObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_installation_types)) return false;
			if (!bindParameter(m_activateds)) return false;
			if (!bindParameter(m_tick_counts)) return false;
			if (!bindParameter(m_activate_start_times)) return false;
			if (!bindParameter(m_powers)) return false;
			if (!bindParameter(m_power_rates)) return false;
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

bool InstallationObjectQuery::bindColumns()
{
	return true;
}

void InstallationObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_installation_obj (:object_id, :installation_type, :activated, :tick_count, :activate_start_time, :power, :power_rate, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_installation_obj (:object_id, :installation_type, :activated, :tick_count, :activate_start_time, :power, :power_rate, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_installation_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

InstallationObjectQuerySelect::InstallationObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool InstallationObjectQuerySelect::bindParameters ()
{
	return true;
}

bool InstallationObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].installation_type)) return false;
	if (!bindCol(m_data[0].activated)) return false;
	if (!bindCol(m_data[0].tick_count)) return false;
	if (!bindCol(m_data[0].activate_start_time)) return false;
	if (!bindCol(m_data[0].power)) return false;
	if (!bindCol(m_data[0].power_rate)) return false;
	return true;
}

const std::vector<InstallationObjectRow> & InstallationObjectQuerySelect::getData() const
{
	return m_data;
}

void InstallationObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_installation_object; end;";
}

DB::Query::QueryMode InstallationObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

IntangibleObjectQuery::IntangibleObjectQuery() :
	DatabaseProcessQuery(new IntangibleObjectRow),
	m_numItems(0)
{
}

bool IntangibleObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_counts.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
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

bool IntangibleObjectQuery::addData(const DB::Row *_data)
{
	const IntangibleObjectBufferRow *myData=safe_cast<const IntangibleObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_counts.push_back(myData->count.isNull(), myData->count.getValue())) return false;
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

int IntangibleObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void IntangibleObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_counts.clear();
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

void IntangibleObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_counts.free();
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

bool IntangibleObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_counts)) return false;
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

bool IntangibleObjectQuery::bindColumns()
{
	return true;
}

void IntangibleObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_intangible_obj (:object_id, :count, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_intangible_obj (:object_id, :count, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_intangible_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

IntangibleObjectQuerySelect::IntangibleObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool IntangibleObjectQuerySelect::bindParameters ()
{
	return true;
}

bool IntangibleObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].count)) return false;
	return true;
}

const std::vector<IntangibleObjectRow> & IntangibleObjectQuerySelect::getData() const
{
	return m_data;
}

void IntangibleObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_intangible_object; end;";
}

DB::Query::QueryMode IntangibleObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

ManufactureSchematicObjectQuery::ManufactureSchematicObjectQuery() :
	DatabaseProcessQuery(new ManufactureSchematicObjectRow),
	m_numItems(0)
{
}

bool ManufactureSchematicObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_creator_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_creator_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_items_per_containers.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_manufacture_times.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_draft_schematics.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
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

bool ManufactureSchematicObjectQuery::addData(const DB::Row *_data)
{
	const ManufactureSchematicObjectBufferRow *myData=safe_cast<const ManufactureSchematicObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_creator_ids.push_back(myData->creator_id.getValueASCII())) return false;
			if (!m_creator_names.push_back(myData->creator_name.getValueASCII())) return false;
			if (!m_items_per_containers.push_back(myData->items_per_container.isNull(), myData->items_per_container.getValue())) return false;
			if (!m_manufacture_times.push_back(myData->manufacture_time.isNull(), myData->manufacture_time.getValue())) return false;
			if (!m_draft_schematics.push_back(myData->draft_schematic.isNull(), myData->draft_schematic.getValue())) return false;
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

int ManufactureSchematicObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void ManufactureSchematicObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_creator_ids.clear();
			m_creator_names.clear();
			m_items_per_containers.clear();
			m_manufacture_times.clear();
			m_draft_schematics.clear();
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

void ManufactureSchematicObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_creator_ids.free();
			m_creator_names.free();
			m_items_per_containers.free();
			m_manufacture_times.free();
			m_draft_schematics.free();
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

bool ManufactureSchematicObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_creator_ids)) return false;
			if (!bindParameter(m_creator_names)) return false;
			if (!bindParameter(m_items_per_containers)) return false;
			if (!bindParameter(m_manufacture_times)) return false;
			if (!bindParameter(m_draft_schematics)) return false;
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

bool ManufactureSchematicObjectQuery::bindColumns()
{
	return true;
}

void ManufactureSchematicObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_manf_schematic_obj (:object_id, :creator_id, :creator_name, :items_per_container, :manufacture_time, :draft_schematic, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_manf_schematic_obj (:object_id, :creator_id, :creator_name, :items_per_container, :manufacture_time, :draft_schematic, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_manf_schematic_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

ManufactureSchematicObjectQuerySelect::ManufactureSchematicObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool ManufactureSchematicObjectQuerySelect::bindParameters ()
{
	return true;
}

bool ManufactureSchematicObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].creator_id)) return false;
	if (!bindCol(m_data[0].creator_name)) return false;
	if (!bindCol(m_data[0].items_per_container)) return false;
	if (!bindCol(m_data[0].manufacture_time)) return false;
	if (!bindCol(m_data[0].draft_schematic)) return false;
	return true;
}

const std::vector<ManufactureSchematicObjectRow> & ManufactureSchematicObjectQuerySelect::getData() const
{
	return m_data;
}

void ManufactureSchematicObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_manf_schematic_object; end;";
}

DB::Query::QueryMode ManufactureSchematicObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

ManufactureInstallationObjectQuery::ManufactureInstallationObjectQuery() :
	DatabaseProcessQuery(new ManufactureInstallationObjectRow),
	m_numItems(0)
{
}

bool ManufactureInstallationObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool ManufactureInstallationObjectQuery::addData(const DB::Row *_data)
{
	const ManufactureInstallationObjectBufferRow *myData=safe_cast<const ManufactureInstallationObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
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

int ManufactureInstallationObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void ManufactureInstallationObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
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

void ManufactureInstallationObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
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

bool ManufactureInstallationObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
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

bool ManufactureInstallationObjectQuery::bindColumns()
{
	return true;
}

void ManufactureInstallationObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_manufacture_inst_obj (:object_id, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_manufacture_inst_obj (:object_id, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_manufacture_inst_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

ManufactureInstallationObjectQuerySelect::ManufactureInstallationObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool ManufactureInstallationObjectQuerySelect::bindParameters ()
{
	return true;
}

bool ManufactureInstallationObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	return true;
}

const std::vector<ManufactureInstallationObjectRow> & ManufactureInstallationObjectQuerySelect::getData() const
{
	return m_data;
}

void ManufactureInstallationObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_manufacture_inst_object; end;";
}

DB::Query::QueryMode ManufactureInstallationObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

MissionObjectQuery::MissionObjectQuery() :
	DatabaseProcessQuery(new MissionObjectRow),
	m_numItems(0)
{
}

bool MissionObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_difficultys.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_end_xs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_end_ys.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_end_zs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_end_cells.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_end_scenes.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_mission_creators.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_rewards.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_root_script_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_start_xs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_start_ys.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_start_zs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_start_cells.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_start_scenes.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_description_tables.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_description_texts.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_title_tables.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_title_texts.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_mission_holder_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_statuss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_mission_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_target_appearances.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_target_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool MissionObjectQuery::addData(const DB::Row *_data)
{
	const MissionObjectBufferRow *myData=safe_cast<const MissionObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_difficultys.push_back(myData->difficulty.isNull(), myData->difficulty.getValue())) return false;
			if (!m_end_xs.push_back(myData->end_x.isNull(), myData->end_x.getValue())) return false;
			if (!m_end_ys.push_back(myData->end_y.isNull(), myData->end_y.getValue())) return false;
			if (!m_end_zs.push_back(myData->end_z.isNull(), myData->end_z.getValue())) return false;
			if (!m_end_cells.push_back(myData->end_cell.getValueASCII())) return false;
			if (!m_end_scenes.push_back(myData->end_scene.isNull(), myData->end_scene.getValue())) return false;
			if (!m_mission_creators.push_back(myData->mission_creator.getValueASCII())) return false;
			if (!m_rewards.push_back(myData->reward.isNull(), myData->reward.getValue())) return false;
			if (!m_root_script_names.push_back(myData->root_script_name.getValueASCII())) return false;
			if (!m_start_xs.push_back(myData->start_x.isNull(), myData->start_x.getValue())) return false;
			if (!m_start_ys.push_back(myData->start_y.isNull(), myData->start_y.getValue())) return false;
			if (!m_start_zs.push_back(myData->start_z.isNull(), myData->start_z.getValue())) return false;
			if (!m_start_cells.push_back(myData->start_cell.getValueASCII())) return false;
			if (!m_start_scenes.push_back(myData->start_scene.isNull(), myData->start_scene.getValue())) return false;
			if (!m_description_tables.push_back(myData->description_table.getValueASCII())) return false;
			if (!m_description_texts.push_back(myData->description_text.getValueASCII())) return false;
			if (!m_title_tables.push_back(myData->title_table.getValueASCII())) return false;
			if (!m_title_texts.push_back(myData->title_text.getValueASCII())) return false;
			if (!m_mission_holder_ids.push_back(myData->mission_holder_id.getValueASCII())) return false;
			if (!m_statuss.push_back(myData->status.isNull(), myData->status.getValue())) return false;
			if (!m_mission_types.push_back(myData->mission_type.isNull(), myData->mission_type.getValue())) return false;
			if (!m_target_appearances.push_back(myData->target_appearance.isNull(), myData->target_appearance.getValue())) return false;
			if (!m_target_names.push_back(myData->target_name.getValueASCII())) return false;
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

int MissionObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void MissionObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_difficultys.clear();
			m_end_xs.clear();
			m_end_ys.clear();
			m_end_zs.clear();
			m_end_cells.clear();
			m_end_scenes.clear();
			m_mission_creators.clear();
			m_rewards.clear();
			m_root_script_names.clear();
			m_start_xs.clear();
			m_start_ys.clear();
			m_start_zs.clear();
			m_start_cells.clear();
			m_start_scenes.clear();
			m_description_tables.clear();
			m_description_texts.clear();
			m_title_tables.clear();
			m_title_texts.clear();
			m_mission_holder_ids.clear();
			m_statuss.clear();
			m_mission_types.clear();
			m_target_appearances.clear();
			m_target_names.clear();
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

void MissionObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_difficultys.free();
			m_end_xs.free();
			m_end_ys.free();
			m_end_zs.free();
			m_end_cells.free();
			m_end_scenes.free();
			m_mission_creators.free();
			m_rewards.free();
			m_root_script_names.free();
			m_start_xs.free();
			m_start_ys.free();
			m_start_zs.free();
			m_start_cells.free();
			m_start_scenes.free();
			m_description_tables.free();
			m_description_texts.free();
			m_title_tables.free();
			m_title_texts.free();
			m_mission_holder_ids.free();
			m_statuss.free();
			m_mission_types.free();
			m_target_appearances.free();
			m_target_names.free();
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

bool MissionObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_difficultys)) return false;
			if (!bindParameter(m_end_xs)) return false;
			if (!bindParameter(m_end_ys)) return false;
			if (!bindParameter(m_end_zs)) return false;
			if (!bindParameter(m_end_cells)) return false;
			if (!bindParameter(m_end_scenes)) return false;
			if (!bindParameter(m_mission_creators)) return false;
			if (!bindParameter(m_rewards)) return false;
			if (!bindParameter(m_root_script_names)) return false;
			if (!bindParameter(m_start_xs)) return false;
			if (!bindParameter(m_start_ys)) return false;
			if (!bindParameter(m_start_zs)) return false;
			if (!bindParameter(m_start_cells)) return false;
			if (!bindParameter(m_start_scenes)) return false;
			if (!bindParameter(m_description_tables)) return false;
			if (!bindParameter(m_description_texts)) return false;
			if (!bindParameter(m_title_tables)) return false;
			if (!bindParameter(m_title_texts)) return false;
			if (!bindParameter(m_mission_holder_ids)) return false;
			if (!bindParameter(m_statuss)) return false;
			if (!bindParameter(m_mission_types)) return false;
			if (!bindParameter(m_target_appearances)) return false;
			if (!bindParameter(m_target_names)) return false;
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

bool MissionObjectQuery::bindColumns()
{
	return true;
}

void MissionObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_mission_obj (:object_id, :difficulty, :end_x, :end_y, :end_z, :end_cell, :end_scene, :mission_creator, :reward, :root_script_name, :start_x, :start_y, :start_z, :start_cell, :start_scene, :description_table, :description_text, :title_table, :title_text, :mission_holder_id, :status, :mission_type, :target_appearance, :target_name, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_mission_obj (:object_id, :difficulty, :end_x, :end_y, :end_z, :end_cell, :end_scene, :mission_creator, :reward, :root_script_name, :start_x, :start_y, :start_z, :start_cell, :start_scene, :description_table, :description_text, :title_table, :title_text, :mission_holder_id, :status, :mission_type, :target_appearance, :target_name, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_mission_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

MissionObjectQuerySelect::MissionObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool MissionObjectQuerySelect::bindParameters ()
{
	return true;
}

bool MissionObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].difficulty)) return false;
	if (!bindCol(m_data[0].end_x)) return false;
	if (!bindCol(m_data[0].end_y)) return false;
	if (!bindCol(m_data[0].end_z)) return false;
	if (!bindCol(m_data[0].end_cell)) return false;
	if (!bindCol(m_data[0].end_scene)) return false;
	if (!bindCol(m_data[0].mission_creator)) return false;
	if (!bindCol(m_data[0].reward)) return false;
	if (!bindCol(m_data[0].root_script_name)) return false;
	if (!bindCol(m_data[0].start_x)) return false;
	if (!bindCol(m_data[0].start_y)) return false;
	if (!bindCol(m_data[0].start_z)) return false;
	if (!bindCol(m_data[0].start_cell)) return false;
	if (!bindCol(m_data[0].start_scene)) return false;
	if (!bindCol(m_data[0].description_table)) return false;
	if (!bindCol(m_data[0].description_text)) return false;
	if (!bindCol(m_data[0].title_table)) return false;
	if (!bindCol(m_data[0].title_text)) return false;
	if (!bindCol(m_data[0].mission_holder_id)) return false;
	if (!bindCol(m_data[0].status)) return false;
	if (!bindCol(m_data[0].mission_type)) return false;
	if (!bindCol(m_data[0].target_appearance)) return false;
	if (!bindCol(m_data[0].target_name)) return false;
	return true;
}

const std::vector<MissionObjectRow> & MissionObjectQuerySelect::getData() const
{
	return m_data;
}

void MissionObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_mission_object; end;";
}

DB::Query::QueryMode MissionObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

PlanetObjectQuery::PlanetObjectQuery() :
	DatabaseProcessQuery(new PlanetObjectRow),
	m_numItems(0)
{
}

bool PlanetObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_planet_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool PlanetObjectQuery::addData(const DB::Row *_data)
{
	const PlanetObjectBufferRow *myData=safe_cast<const PlanetObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_planet_names.push_back(myData->planet_name.getValueASCII())) return false;
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

int PlanetObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void PlanetObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_planet_names.clear();
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

void PlanetObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_planet_names.free();
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

bool PlanetObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_planet_names)) return false;
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

bool PlanetObjectQuery::bindColumns()
{
	return true;
}

void PlanetObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_planet_obj (:object_id, :planet_name, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_planet_obj (:object_id, :planet_name, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_planet_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

PlanetObjectQuerySelect::PlanetObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool PlanetObjectQuerySelect::bindParameters ()
{
	return true;
}

bool PlanetObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].planet_name)) return false;
	return true;
}

const std::vector<PlanetObjectRow> & PlanetObjectQuerySelect::getData() const
{
	return m_data;
}

void PlanetObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_planet_object; end;";
}

DB::Query::QueryMode PlanetObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

PlayerObjectQuery::PlayerObjectQuery() :
	DatabaseProcessQuery(new PlayerObjectRow),
	m_numItems(0)
{
}

bool PlayerObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_station_ids.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_house_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_account_num_lotss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_account_is_outcasts.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_account_cheater_levels.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_account_max_lots_adjustments.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_personal_profile_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_character_profile_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_skill_titles.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_born_dates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_played_times.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_force_regen_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_force_powers.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_max_force_powers.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_active_questss.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_completed_questss.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_current_quests.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_questss.create(session, "VAOFLONGSTRING", DatabaseProcess::getInstance().getSchema(), 4000)) return false;
			if (!m_role_icon_choices.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_quests2s.create(session, "VAOFLONGSTRING", DatabaseProcess::getInstance().getSchema(), 4000)) return false;
			if (!m_quests3s.create(session, "VAOFLONGSTRING", DatabaseProcess::getInstance().getSchema(), 4000)) return false;
			if (!m_quests4s.create(session, "VAOFLONGSTRING", DatabaseProcess::getInstance().getSchema(), 4000)) return false;
			if (!m_skill_templates.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_working_skills.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_current_gcw_pointss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_current_gcw_ratings.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_current_pvp_killss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_lifetime_gcw_pointss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_max_gcw_imperial_ratings.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_max_gcw_rebel_ratings.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_lifetime_pvp_killss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_next_gcw_rating_calc_times.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_collectionss.create(session, "VAOFLONGSTRING", DatabaseProcess::getInstance().getSchema(), 4000)) return false;
			if (!m_show_backpacks.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_show_helmets.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_collections2s.create(session, "VAOFLONGSTRING", DatabaseProcess::getInstance().getSchema(), 4000)) return false;
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

bool PlayerObjectQuery::addData(const DB::Row *_data)
{
	const PlayerObjectBufferRow *myData=safe_cast<const PlayerObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_station_ids.push_back(myData->station_id.isNull(), myData->station_id.getValue())) return false;
			if (!m_house_ids.push_back(myData->house_id.getValueASCII())) return false;
			if (!m_account_num_lotss.push_back(myData->account_num_lots.isNull(), myData->account_num_lots.getValue())) return false;
			if (!m_account_is_outcasts.push_back(myData->account_is_outcast.getValueASCII())) return false;
			if (!m_account_cheater_levels.push_back(myData->account_cheater_level.isNull(), myData->account_cheater_level.getValue())) return false;
			if (!m_account_max_lots_adjustments.push_back(myData->account_max_lots_adjustment.isNull(), myData->account_max_lots_adjustment.getValue())) return false;
			if (!m_personal_profile_ids.push_back(myData->personal_profile_id.getValueASCII())) return false;
			if (!m_character_profile_ids.push_back(myData->character_profile_id.getValueASCII())) return false;
			if (!m_skill_titles.push_back(myData->skill_title.getValueASCII())) return false;
			if (!m_born_dates.push_back(myData->born_date.isNull(), myData->born_date.getValue())) return false;
			if (!m_played_times.push_back(myData->played_time.isNull(), myData->played_time.getValue())) return false;
			if (!m_force_regen_rates.push_back(myData->force_regen_rate.isNull(), myData->force_regen_rate.getValue())) return false;
			if (!m_force_powers.push_back(myData->force_power.isNull(), myData->force_power.getValue())) return false;
			if (!m_max_force_powers.push_back(myData->max_force_power.isNull(), myData->max_force_power.getValue())) return false;
			if (!m_active_questss.push_back(myData->active_quests.getValueASCII())) return false;
			if (!m_completed_questss.push_back(myData->completed_quests.getValueASCII())) return false;
			if (!m_current_quests.push_back(myData->current_quest.isNull(), myData->current_quest.getValue())) return false;
			if (!m_questss.push_back(myData->quests.getValueASCII())) return false;
			if (!m_role_icon_choices.push_back(myData->role_icon_choice.isNull(), myData->role_icon_choice.getValue())) return false;
			if (!m_quests2s.push_back(myData->quests2.getValueASCII())) return false;
			if (!m_quests3s.push_back(myData->quests3.getValueASCII())) return false;
			if (!m_quests4s.push_back(myData->quests4.getValueASCII())) return false;
			if (!m_skill_templates.push_back(myData->skill_template.getValueASCII())) return false;
			if (!m_working_skills.push_back(myData->working_skill.getValueASCII())) return false;
			if (!m_current_gcw_pointss.push_back(myData->current_gcw_points.isNull(), myData->current_gcw_points.getValue())) return false;
			if (!m_current_gcw_ratings.push_back(myData->current_gcw_rating.isNull(), myData->current_gcw_rating.getValue())) return false;
			if (!m_current_pvp_killss.push_back(myData->current_pvp_kills.isNull(), myData->current_pvp_kills.getValue())) return false;
			if (!m_lifetime_gcw_pointss.push_back(myData->lifetime_gcw_points.isNull(), myData->lifetime_gcw_points.getValue())) return false;
			if (!m_max_gcw_imperial_ratings.push_back(myData->max_gcw_imperial_rating.isNull(), myData->max_gcw_imperial_rating.getValue())) return false;
			if (!m_max_gcw_rebel_ratings.push_back(myData->max_gcw_rebel_rating.isNull(), myData->max_gcw_rebel_rating.getValue())) return false;
			if (!m_lifetime_pvp_killss.push_back(myData->lifetime_pvp_kills.isNull(), myData->lifetime_pvp_kills.getValue())) return false;
			if (!m_next_gcw_rating_calc_times.push_back(myData->next_gcw_rating_calc_time.isNull(), myData->next_gcw_rating_calc_time.getValue())) return false;
			if (!m_collectionss.push_back(myData->collections.getValueASCII())) return false;
			if (!m_show_backpacks.push_back(myData->show_backpack.getValueASCII())) return false;
			if (!m_show_helmets.push_back(myData->show_helmet.getValueASCII())) return false;
			if (!m_collections2s.push_back(myData->collections2.getValueASCII())) return false;
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

int PlayerObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void PlayerObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_station_ids.clear();
			m_house_ids.clear();
			m_account_num_lotss.clear();
			m_account_is_outcasts.clear();
			m_account_cheater_levels.clear();
			m_account_max_lots_adjustments.clear();
			m_personal_profile_ids.clear();
			m_character_profile_ids.clear();
			m_skill_titles.clear();
			m_born_dates.clear();
			m_played_times.clear();
			m_force_regen_rates.clear();
			m_force_powers.clear();
			m_max_force_powers.clear();
			m_active_questss.clear();
			m_completed_questss.clear();
			m_current_quests.clear();
			m_questss.clear();
			m_role_icon_choices.clear();
			m_quests2s.clear();
			m_quests3s.clear();
			m_quests4s.clear();
			m_skill_templates.clear();
			m_working_skills.clear();
			m_current_gcw_pointss.clear();
			m_current_gcw_ratings.clear();
			m_current_pvp_killss.clear();
			m_lifetime_gcw_pointss.clear();
			m_max_gcw_imperial_ratings.clear();
			m_max_gcw_rebel_ratings.clear();
			m_lifetime_pvp_killss.clear();
			m_next_gcw_rating_calc_times.clear();
			m_collectionss.clear();
			m_show_backpacks.clear();
			m_show_helmets.clear();
			m_collections2s.clear();
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

void PlayerObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_station_ids.free();
			m_house_ids.free();
			m_account_num_lotss.free();
			m_account_is_outcasts.free();
			m_account_cheater_levels.free();
			m_account_max_lots_adjustments.free();
			m_personal_profile_ids.free();
			m_character_profile_ids.free();
			m_skill_titles.free();
			m_born_dates.free();
			m_played_times.free();
			m_force_regen_rates.free();
			m_force_powers.free();
			m_max_force_powers.free();
			m_active_questss.free();
			m_completed_questss.free();
			m_current_quests.free();
			m_questss.free();
			m_role_icon_choices.free();
			m_quests2s.free();
			m_quests3s.free();
			m_quests4s.free();
			m_skill_templates.free();
			m_working_skills.free();
			m_current_gcw_pointss.free();
			m_current_gcw_ratings.free();
			m_current_pvp_killss.free();
			m_lifetime_gcw_pointss.free();
			m_max_gcw_imperial_ratings.free();
			m_max_gcw_rebel_ratings.free();
			m_lifetime_pvp_killss.free();
			m_next_gcw_rating_calc_times.free();
			m_collectionss.free();
			m_show_backpacks.free();
			m_show_helmets.free();
			m_collections2s.free();
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

bool PlayerObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_station_ids)) return false;
			if (!bindParameter(m_house_ids)) return false;
			if (!bindParameter(m_account_num_lotss)) return false;
			if (!bindParameter(m_account_is_outcasts)) return false;
			if (!bindParameter(m_account_cheater_levels)) return false;
			if (!bindParameter(m_account_max_lots_adjustments)) return false;
			if (!bindParameter(m_personal_profile_ids)) return false;
			if (!bindParameter(m_character_profile_ids)) return false;
			if (!bindParameter(m_skill_titles)) return false;
			if (!bindParameter(m_born_dates)) return false;
			if (!bindParameter(m_played_times)) return false;
			if (!bindParameter(m_force_regen_rates)) return false;
			if (!bindParameter(m_force_powers)) return false;
			if (!bindParameter(m_max_force_powers)) return false;
			if (!bindParameter(m_active_questss)) return false;
			if (!bindParameter(m_completed_questss)) return false;
			if (!bindParameter(m_current_quests)) return false;
			if (!bindParameter(m_questss)) return false;
			if (!bindParameter(m_role_icon_choices)) return false;
			if (!bindParameter(m_quests2s)) return false;
			if (!bindParameter(m_quests3s)) return false;
			if (!bindParameter(m_quests4s)) return false;
			if (!bindParameter(m_skill_templates)) return false;
			if (!bindParameter(m_working_skills)) return false;
			if (!bindParameter(m_current_gcw_pointss)) return false;
			if (!bindParameter(m_current_gcw_ratings)) return false;
			if (!bindParameter(m_current_pvp_killss)) return false;
			if (!bindParameter(m_lifetime_gcw_pointss)) return false;
			if (!bindParameter(m_max_gcw_imperial_ratings)) return false;
			if (!bindParameter(m_max_gcw_rebel_ratings)) return false;
			if (!bindParameter(m_lifetime_pvp_killss)) return false;
			if (!bindParameter(m_next_gcw_rating_calc_times)) return false;
			if (!bindParameter(m_collectionss)) return false;
			if (!bindParameter(m_show_backpacks)) return false;
			if (!bindParameter(m_show_helmets)) return false;
			if (!bindParameter(m_collections2s)) return false;
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

bool PlayerObjectQuery::bindColumns()
{
	return true;
}

void PlayerObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_player_obj (:object_id, :station_id, :house_id, :account_num_lots, :account_is_outcast, :account_cheater_level, :account_max_lots_adjustment, :personal_profile_id, :character_profile_id, :skill_title, :born_date, :played_time, :force_regen_rate, :force_power, :max_force_power, :active_quests, :completed_quests, :current_quest, :quests, :role_icon_choice, :quests2, :quests3, :quests4, :skill_template, :working_skill, :current_gcw_points, :current_gcw_rating, :current_pvp_kills, :lifetime_gcw_points, :max_gcw_imperial_rating, :max_gcw_rebel_rating, :lifetime_pvp_kills, :next_gcw_rating_calc_time, :collections, :show_backpack, :show_helmet, :collections2, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_player_obj (:object_id, :station_id, :house_id, :account_num_lots, :account_is_outcast, :account_cheater_level, :account_max_lots_adjustment, :personal_profile_id, :character_profile_id, :skill_title, :born_date, :played_time, :force_regen_rate, :force_power, :max_force_power, :active_quests, :completed_quests, :current_quest, :quests, :role_icon_choice, :quests2, :quests3, :quests4, :skill_template, :working_skill, :current_gcw_points, :current_gcw_rating, :current_pvp_kills, :lifetime_gcw_points, :max_gcw_imperial_rating, :max_gcw_rebel_rating, :lifetime_pvp_kills, :next_gcw_rating_calc_time, :collections, :show_backpack, :show_helmet, :collections2, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_player_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

PlayerObjectQuerySelect::PlayerObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool PlayerObjectQuerySelect::bindParameters ()
{
	return true;
}

bool PlayerObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].station_id)) return false;
	if (!bindCol(m_data[0].house_id)) return false;
	if (!bindCol(m_data[0].account_num_lots)) return false;
	if (!bindCol(m_data[0].account_is_outcast)) return false;
	if (!bindCol(m_data[0].account_cheater_level)) return false;
	if (!bindCol(m_data[0].account_max_lots_adjustment)) return false;
	if (!bindCol(m_data[0].personal_profile_id)) return false;
	if (!bindCol(m_data[0].character_profile_id)) return false;
	if (!bindCol(m_data[0].skill_title)) return false;
	if (!bindCol(m_data[0].born_date)) return false;
	if (!bindCol(m_data[0].played_time)) return false;
	if (!bindCol(m_data[0].force_regen_rate)) return false;
	if (!bindCol(m_data[0].force_power)) return false;
	if (!bindCol(m_data[0].max_force_power)) return false;
	if (!bindCol(m_data[0].active_quests)) return false;
	if (!bindCol(m_data[0].completed_quests)) return false;
	if (!bindCol(m_data[0].current_quest)) return false;
	if (!bindCol(m_data[0].quests)) return false;
	if (!bindCol(m_data[0].role_icon_choice)) return false;
	if (!bindCol(m_data[0].quests2)) return false;
	if (!bindCol(m_data[0].quests3)) return false;
	if (!bindCol(m_data[0].quests4)) return false;
	if (!bindCol(m_data[0].skill_template)) return false;
	if (!bindCol(m_data[0].working_skill)) return false;
	if (!bindCol(m_data[0].current_gcw_points)) return false;
	if (!bindCol(m_data[0].current_gcw_rating)) return false;
	if (!bindCol(m_data[0].current_pvp_kills)) return false;
	if (!bindCol(m_data[0].lifetime_gcw_points)) return false;
	if (!bindCol(m_data[0].max_gcw_imperial_rating)) return false;
	if (!bindCol(m_data[0].max_gcw_rebel_rating)) return false;
	if (!bindCol(m_data[0].lifetime_pvp_kills)) return false;
	if (!bindCol(m_data[0].next_gcw_rating_calc_time)) return false;
	if (!bindCol(m_data[0].collections)) return false;
	if (!bindCol(m_data[0].show_backpack)) return false;
	if (!bindCol(m_data[0].show_helmet)) return false;
	if (!bindCol(m_data[0].collections2)) return false;
	return true;
}

const std::vector<PlayerObjectRow> & PlayerObjectQuerySelect::getData() const
{
	return m_data;
}

void PlayerObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_player_object; end;";
}

DB::Query::QueryMode PlayerObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

PlayerQuestObjectQuery::PlayerQuestObjectQuery() :
	DatabaseProcessQuery(new PlayerQuestObjectRow),
	m_numItems(0)
{
}

bool PlayerQuestObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_titles.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_descriptions.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_creators.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_total_taskss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_difficultys.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_task_title1s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_description1s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_title2s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_description2s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_title3s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_description3s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_title4s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_description4s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_title5s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_description5s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_title6s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_description6s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_title7s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_description7s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_title8s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_description8s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_title9s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_description9s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_title10s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_description10s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_title11s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_description11s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_title12s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_task_description12s.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool PlayerQuestObjectQuery::addData(const DB::Row *_data)
{
	const PlayerQuestObjectBufferRow *myData=safe_cast<const PlayerQuestObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_titles.push_back(myData->title.getValueASCII())) return false;
			if (!m_descriptions.push_back(myData->description.getValueASCII())) return false;
			if (!m_creators.push_back(myData->creator.getValueASCII())) return false;
			if (!m_total_taskss.push_back(myData->total_tasks.isNull(), myData->total_tasks.getValue())) return false;
			if (!m_difficultys.push_back(myData->difficulty.isNull(), myData->difficulty.getValue())) return false;
			if (!m_task_title1s.push_back(myData->task_title1.getValueASCII())) return false;
			if (!m_task_description1s.push_back(myData->task_description1.getValueASCII())) return false;
			if (!m_task_title2s.push_back(myData->task_title2.getValueASCII())) return false;
			if (!m_task_description2s.push_back(myData->task_description2.getValueASCII())) return false;
			if (!m_task_title3s.push_back(myData->task_title3.getValueASCII())) return false;
			if (!m_task_description3s.push_back(myData->task_description3.getValueASCII())) return false;
			if (!m_task_title4s.push_back(myData->task_title4.getValueASCII())) return false;
			if (!m_task_description4s.push_back(myData->task_description4.getValueASCII())) return false;
			if (!m_task_title5s.push_back(myData->task_title5.getValueASCII())) return false;
			if (!m_task_description5s.push_back(myData->task_description5.getValueASCII())) return false;
			if (!m_task_title6s.push_back(myData->task_title6.getValueASCII())) return false;
			if (!m_task_description6s.push_back(myData->task_description6.getValueASCII())) return false;
			if (!m_task_title7s.push_back(myData->task_title7.getValueASCII())) return false;
			if (!m_task_description7s.push_back(myData->task_description7.getValueASCII())) return false;
			if (!m_task_title8s.push_back(myData->task_title8.getValueASCII())) return false;
			if (!m_task_description8s.push_back(myData->task_description8.getValueASCII())) return false;
			if (!m_task_title9s.push_back(myData->task_title9.getValueASCII())) return false;
			if (!m_task_description9s.push_back(myData->task_description9.getValueASCII())) return false;
			if (!m_task_title10s.push_back(myData->task_title10.getValueASCII())) return false;
			if (!m_task_description10s.push_back(myData->task_description10.getValueASCII())) return false;
			if (!m_task_title11s.push_back(myData->task_title11.getValueASCII())) return false;
			if (!m_task_description11s.push_back(myData->task_description11.getValueASCII())) return false;
			if (!m_task_title12s.push_back(myData->task_title12.getValueASCII())) return false;
			if (!m_task_description12s.push_back(myData->task_description12.getValueASCII())) return false;
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

int PlayerQuestObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void PlayerQuestObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_titles.clear();
			m_descriptions.clear();
			m_creators.clear();
			m_total_taskss.clear();
			m_difficultys.clear();
			m_task_title1s.clear();
			m_task_description1s.clear();
			m_task_title2s.clear();
			m_task_description2s.clear();
			m_task_title3s.clear();
			m_task_description3s.clear();
			m_task_title4s.clear();
			m_task_description4s.clear();
			m_task_title5s.clear();
			m_task_description5s.clear();
			m_task_title6s.clear();
			m_task_description6s.clear();
			m_task_title7s.clear();
			m_task_description7s.clear();
			m_task_title8s.clear();
			m_task_description8s.clear();
			m_task_title9s.clear();
			m_task_description9s.clear();
			m_task_title10s.clear();
			m_task_description10s.clear();
			m_task_title11s.clear();
			m_task_description11s.clear();
			m_task_title12s.clear();
			m_task_description12s.clear();
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

void PlayerQuestObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_titles.free();
			m_descriptions.free();
			m_creators.free();
			m_total_taskss.free();
			m_difficultys.free();
			m_task_title1s.free();
			m_task_description1s.free();
			m_task_title2s.free();
			m_task_description2s.free();
			m_task_title3s.free();
			m_task_description3s.free();
			m_task_title4s.free();
			m_task_description4s.free();
			m_task_title5s.free();
			m_task_description5s.free();
			m_task_title6s.free();
			m_task_description6s.free();
			m_task_title7s.free();
			m_task_description7s.free();
			m_task_title8s.free();
			m_task_description8s.free();
			m_task_title9s.free();
			m_task_description9s.free();
			m_task_title10s.free();
			m_task_description10s.free();
			m_task_title11s.free();
			m_task_description11s.free();
			m_task_title12s.free();
			m_task_description12s.free();
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

bool PlayerQuestObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_titles)) return false;
			if (!bindParameter(m_descriptions)) return false;
			if (!bindParameter(m_creators)) return false;
			if (!bindParameter(m_total_taskss)) return false;
			if (!bindParameter(m_difficultys)) return false;
			if (!bindParameter(m_task_title1s)) return false;
			if (!bindParameter(m_task_description1s)) return false;
			if (!bindParameter(m_task_title2s)) return false;
			if (!bindParameter(m_task_description2s)) return false;
			if (!bindParameter(m_task_title3s)) return false;
			if (!bindParameter(m_task_description3s)) return false;
			if (!bindParameter(m_task_title4s)) return false;
			if (!bindParameter(m_task_description4s)) return false;
			if (!bindParameter(m_task_title5s)) return false;
			if (!bindParameter(m_task_description5s)) return false;
			if (!bindParameter(m_task_title6s)) return false;
			if (!bindParameter(m_task_description6s)) return false;
			if (!bindParameter(m_task_title7s)) return false;
			if (!bindParameter(m_task_description7s)) return false;
			if (!bindParameter(m_task_title8s)) return false;
			if (!bindParameter(m_task_description8s)) return false;
			if (!bindParameter(m_task_title9s)) return false;
			if (!bindParameter(m_task_description9s)) return false;
			if (!bindParameter(m_task_title10s)) return false;
			if (!bindParameter(m_task_description10s)) return false;
			if (!bindParameter(m_task_title11s)) return false;
			if (!bindParameter(m_task_description11s)) return false;
			if (!bindParameter(m_task_title12s)) return false;
			if (!bindParameter(m_task_description12s)) return false;
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

bool PlayerQuestObjectQuery::bindColumns()
{
	return true;
}

void PlayerQuestObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_player_quest_obj (:object_id, :title, :description, :creator, :total_tasks, :difficulty, :task_title1, :task_description1, :task_title2, :task_description2, :task_title3, :task_description3, :task_title4, :task_description4, :task_title5, :task_description5, :task_title6, :task_description6, :task_title7, :task_description7, :task_title8, :task_description8, :task_title9, :task_description9, :task_title10, :task_description10, :task_title11, :task_description11, :task_title12, :task_description12, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_player_quest_obj (:object_id, :title, :description, :creator, :total_tasks, :difficulty, :task_title1, :task_description1, :task_title2, :task_description2, :task_title3, :task_description3, :task_title4, :task_description4, :task_title5, :task_description5, :task_title6, :task_description6, :task_title7, :task_description7, :task_title8, :task_description8, :task_title9, :task_description9, :task_title10, :task_description10, :task_title11, :task_description11, :task_title12, :task_description12, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_player_quest_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

PlayerQuestObjectQuerySelect::PlayerQuestObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool PlayerQuestObjectQuerySelect::bindParameters ()
{
	return true;
}

bool PlayerQuestObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].title)) return false;
	if (!bindCol(m_data[0].description)) return false;
	if (!bindCol(m_data[0].creator)) return false;
	if (!bindCol(m_data[0].total_tasks)) return false;
	if (!bindCol(m_data[0].difficulty)) return false;
	if (!bindCol(m_data[0].task_title1)) return false;
	if (!bindCol(m_data[0].task_description1)) return false;
	if (!bindCol(m_data[0].task_title2)) return false;
	if (!bindCol(m_data[0].task_description2)) return false;
	if (!bindCol(m_data[0].task_title3)) return false;
	if (!bindCol(m_data[0].task_description3)) return false;
	if (!bindCol(m_data[0].task_title4)) return false;
	if (!bindCol(m_data[0].task_description4)) return false;
	if (!bindCol(m_data[0].task_title5)) return false;
	if (!bindCol(m_data[0].task_description5)) return false;
	if (!bindCol(m_data[0].task_title6)) return false;
	if (!bindCol(m_data[0].task_description6)) return false;
	if (!bindCol(m_data[0].task_title7)) return false;
	if (!bindCol(m_data[0].task_description7)) return false;
	if (!bindCol(m_data[0].task_title8)) return false;
	if (!bindCol(m_data[0].task_description8)) return false;
	if (!bindCol(m_data[0].task_title9)) return false;
	if (!bindCol(m_data[0].task_description9)) return false;
	if (!bindCol(m_data[0].task_title10)) return false;
	if (!bindCol(m_data[0].task_description10)) return false;
	if (!bindCol(m_data[0].task_title11)) return false;
	if (!bindCol(m_data[0].task_description11)) return false;
	if (!bindCol(m_data[0].task_title12)) return false;
	if (!bindCol(m_data[0].task_description12)) return false;
	return true;
}

const std::vector<PlayerQuestObjectRow> & PlayerQuestObjectQuerySelect::getData() const
{
	return m_data;
}

void PlayerQuestObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_player_quest_object; end;";
}

DB::Query::QueryMode PlayerQuestObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

ResourceContainerObjectQuery::ResourceContainerObjectQuery() :
	DatabaseProcessQuery(new ResourceContainerObjectRow),
	m_numItems(0)
{
}

bool ResourceContainerObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_resource_types.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_quantitys.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_sources.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool ResourceContainerObjectQuery::addData(const DB::Row *_data)
{
	const ResourceContainerObjectBufferRow *myData=safe_cast<const ResourceContainerObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_resource_types.push_back(myData->resource_type.getValueASCII())) return false;
			if (!m_quantitys.push_back(myData->quantity.isNull(), myData->quantity.getValue())) return false;
			if (!m_sources.push_back(myData->source.getValueASCII())) return false;
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

int ResourceContainerObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void ResourceContainerObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_resource_types.clear();
			m_quantitys.clear();
			m_sources.clear();
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

void ResourceContainerObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_resource_types.free();
			m_quantitys.free();
			m_sources.free();
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

bool ResourceContainerObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_resource_types)) return false;
			if (!bindParameter(m_quantitys)) return false;
			if (!bindParameter(m_sources)) return false;
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

bool ResourceContainerObjectQuery::bindColumns()
{
	return true;
}

void ResourceContainerObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_resource_container_obj (:object_id, :resource_type, :quantity, :source, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_resource_container_obj (:object_id, :resource_type, :quantity, :source, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_resource_container_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

ResourceContainerObjectQuerySelect::ResourceContainerObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool ResourceContainerObjectQuerySelect::bindParameters ()
{
	return true;
}

bool ResourceContainerObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].resource_type)) return false;
	if (!bindCol(m_data[0].quantity)) return false;
	if (!bindCol(m_data[0].source)) return false;
	return true;
}

const std::vector<ResourceContainerObjectRow> & ResourceContainerObjectQuerySelect::getData() const
{
	return m_data;
}

void ResourceContainerObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_resource_container_object; end;";
}

DB::Query::QueryMode ResourceContainerObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

ShipObjectQuery::ShipObjectQuery() :
	DatabaseProcessQuery(new ShipObjectRow),
	m_numItems(0)
{
}

bool ShipObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_slide_dampeners.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_current_chassis_hit_pointss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_maximum_chassis_hit_pointss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_chassis_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_cmp_armor_hp_maximums.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_cmp_armor_hp_currents.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_cmp_efficiency_generals.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_cmp_efficiency_engs.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_cmp_eng_maintenances.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_cmp_masss.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_cmp_crcs.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_cmp_hp_currents.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_cmp_hp_maximums.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_cmp_flagss.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_cmp_namess.create(session, "VAOFLONGSTRING", DatabaseProcess::getInstance().getSchema(), 4000)) return false;
			if (!m_weapon_damage_maximums.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_weapon_damage_minimums.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_weapon_effectiveness_shieldss.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_weapon_effectiveness_armors.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_weapon_eng_per_shots.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_weapon_refire_rates.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_weapon_ammo_currents.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_weapon_ammo_maximums.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_weapon_ammo_types.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_shield_hp_front_maximums.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_shield_hp_back_maximums.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_shield_recharge_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_capacitor_eng_maximums.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_capacitor_eng_recharge_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_engine_acc_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_engine_deceleration_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_engine_pitch_acc_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_engine_yaw_acc_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_engine_roll_acc_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_engine_pitch_rate_maximums.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_engine_yaw_rate_maximums.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_engine_roll_rate_maximums.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_engine_speed_maximums.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_reactor_eng_generation_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_booster_eng_maximums.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_booster_eng_recharge_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_booster_eng_consumption_rates.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_booster_accs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_booster_speed_maximums.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_droid_if_cmd_speeds.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_installed_dcds.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_chassis_cmp_mass_maximums.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_cmp_creatorss.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_cargo_hold_contents_maximums.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_cargo_hold_contents_currents.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_cargo_hold_contentss.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool ShipObjectQuery::addData(const DB::Row *_data)
{
	const ShipObjectBufferRow *myData=safe_cast<const ShipObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_slide_dampeners.push_back(myData->slide_dampener.isNull(), myData->slide_dampener.getValue())) return false;
			if (!m_current_chassis_hit_pointss.push_back(myData->current_chassis_hit_points.isNull(), myData->current_chassis_hit_points.getValue())) return false;
			if (!m_maximum_chassis_hit_pointss.push_back(myData->maximum_chassis_hit_points.isNull(), myData->maximum_chassis_hit_points.getValue())) return false;
			if (!m_chassis_types.push_back(myData->chassis_type.isNull(), myData->chassis_type.getValue())) return false;
			if (!m_cmp_armor_hp_maximums.push_back(myData->cmp_armor_hp_maximum.getValueASCII())) return false;
			if (!m_cmp_armor_hp_currents.push_back(myData->cmp_armor_hp_current.getValueASCII())) return false;
			if (!m_cmp_efficiency_generals.push_back(myData->cmp_efficiency_general.getValueASCII())) return false;
			if (!m_cmp_efficiency_engs.push_back(myData->cmp_efficiency_eng.getValueASCII())) return false;
			if (!m_cmp_eng_maintenances.push_back(myData->cmp_eng_maintenance.getValueASCII())) return false;
			if (!m_cmp_masss.push_back(myData->cmp_mass.getValueASCII())) return false;
			if (!m_cmp_crcs.push_back(myData->cmp_crc.getValueASCII())) return false;
			if (!m_cmp_hp_currents.push_back(myData->cmp_hp_current.getValueASCII())) return false;
			if (!m_cmp_hp_maximums.push_back(myData->cmp_hp_maximum.getValueASCII())) return false;
			if (!m_cmp_flagss.push_back(myData->cmp_flags.getValueASCII())) return false;
			if (!m_cmp_namess.push_back(myData->cmp_names.getValueASCII())) return false;
			if (!m_weapon_damage_maximums.push_back(myData->weapon_damage_maximum.getValueASCII())) return false;
			if (!m_weapon_damage_minimums.push_back(myData->weapon_damage_minimum.getValueASCII())) return false;
			if (!m_weapon_effectiveness_shieldss.push_back(myData->weapon_effectiveness_shields.getValueASCII())) return false;
			if (!m_weapon_effectiveness_armors.push_back(myData->weapon_effectiveness_armor.getValueASCII())) return false;
			if (!m_weapon_eng_per_shots.push_back(myData->weapon_eng_per_shot.getValueASCII())) return false;
			if (!m_weapon_refire_rates.push_back(myData->weapon_refire_rate.getValueASCII())) return false;
			if (!m_weapon_ammo_currents.push_back(myData->weapon_ammo_current.getValueASCII())) return false;
			if (!m_weapon_ammo_maximums.push_back(myData->weapon_ammo_maximum.getValueASCII())) return false;
			if (!m_weapon_ammo_types.push_back(myData->weapon_ammo_type.getValueASCII())) return false;
			if (!m_shield_hp_front_maximums.push_back(myData->shield_hp_front_maximum.isNull(), myData->shield_hp_front_maximum.getValue())) return false;
			if (!m_shield_hp_back_maximums.push_back(myData->shield_hp_back_maximum.isNull(), myData->shield_hp_back_maximum.getValue())) return false;
			if (!m_shield_recharge_rates.push_back(myData->shield_recharge_rate.isNull(), myData->shield_recharge_rate.getValue())) return false;
			if (!m_capacitor_eng_maximums.push_back(myData->capacitor_eng_maximum.isNull(), myData->capacitor_eng_maximum.getValue())) return false;
			if (!m_capacitor_eng_recharge_rates.push_back(myData->capacitor_eng_recharge_rate.isNull(), myData->capacitor_eng_recharge_rate.getValue())) return false;
			if (!m_engine_acc_rates.push_back(myData->engine_acc_rate.isNull(), myData->engine_acc_rate.getValue())) return false;
			if (!m_engine_deceleration_rates.push_back(myData->engine_deceleration_rate.isNull(), myData->engine_deceleration_rate.getValue())) return false;
			if (!m_engine_pitch_acc_rates.push_back(myData->engine_pitch_acc_rate.isNull(), myData->engine_pitch_acc_rate.getValue())) return false;
			if (!m_engine_yaw_acc_rates.push_back(myData->engine_yaw_acc_rate.isNull(), myData->engine_yaw_acc_rate.getValue())) return false;
			if (!m_engine_roll_acc_rates.push_back(myData->engine_roll_acc_rate.isNull(), myData->engine_roll_acc_rate.getValue())) return false;
			if (!m_engine_pitch_rate_maximums.push_back(myData->engine_pitch_rate_maximum.isNull(), myData->engine_pitch_rate_maximum.getValue())) return false;
			if (!m_engine_yaw_rate_maximums.push_back(myData->engine_yaw_rate_maximum.isNull(), myData->engine_yaw_rate_maximum.getValue())) return false;
			if (!m_engine_roll_rate_maximums.push_back(myData->engine_roll_rate_maximum.isNull(), myData->engine_roll_rate_maximum.getValue())) return false;
			if (!m_engine_speed_maximums.push_back(myData->engine_speed_maximum.isNull(), myData->engine_speed_maximum.getValue())) return false;
			if (!m_reactor_eng_generation_rates.push_back(myData->reactor_eng_generation_rate.isNull(), myData->reactor_eng_generation_rate.getValue())) return false;
			if (!m_booster_eng_maximums.push_back(myData->booster_eng_maximum.isNull(), myData->booster_eng_maximum.getValue())) return false;
			if (!m_booster_eng_recharge_rates.push_back(myData->booster_eng_recharge_rate.isNull(), myData->booster_eng_recharge_rate.getValue())) return false;
			if (!m_booster_eng_consumption_rates.push_back(myData->booster_eng_consumption_rate.isNull(), myData->booster_eng_consumption_rate.getValue())) return false;
			if (!m_booster_accs.push_back(myData->booster_acc.isNull(), myData->booster_acc.getValue())) return false;
			if (!m_booster_speed_maximums.push_back(myData->booster_speed_maximum.isNull(), myData->booster_speed_maximum.getValue())) return false;
			if (!m_droid_if_cmd_speeds.push_back(myData->droid_if_cmd_speed.isNull(), myData->droid_if_cmd_speed.getValue())) return false;
			if (!m_installed_dcds.push_back(myData->installed_dcd.getValueASCII())) return false;
			if (!m_chassis_cmp_mass_maximums.push_back(myData->chassis_cmp_mass_maximum.isNull(), myData->chassis_cmp_mass_maximum.getValue())) return false;
			if (!m_cmp_creatorss.push_back(myData->cmp_creators.getValueASCII())) return false;
			if (!m_cargo_hold_contents_maximums.push_back(myData->cargo_hold_contents_maximum.isNull(), myData->cargo_hold_contents_maximum.getValue())) return false;
			if (!m_cargo_hold_contents_currents.push_back(myData->cargo_hold_contents_current.isNull(), myData->cargo_hold_contents_current.getValue())) return false;
			if (!m_cargo_hold_contentss.push_back(myData->cargo_hold_contents.getValueASCII())) return false;
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

int ShipObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void ShipObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_slide_dampeners.clear();
			m_current_chassis_hit_pointss.clear();
			m_maximum_chassis_hit_pointss.clear();
			m_chassis_types.clear();
			m_cmp_armor_hp_maximums.clear();
			m_cmp_armor_hp_currents.clear();
			m_cmp_efficiency_generals.clear();
			m_cmp_efficiency_engs.clear();
			m_cmp_eng_maintenances.clear();
			m_cmp_masss.clear();
			m_cmp_crcs.clear();
			m_cmp_hp_currents.clear();
			m_cmp_hp_maximums.clear();
			m_cmp_flagss.clear();
			m_cmp_namess.clear();
			m_weapon_damage_maximums.clear();
			m_weapon_damage_minimums.clear();
			m_weapon_effectiveness_shieldss.clear();
			m_weapon_effectiveness_armors.clear();
			m_weapon_eng_per_shots.clear();
			m_weapon_refire_rates.clear();
			m_weapon_ammo_currents.clear();
			m_weapon_ammo_maximums.clear();
			m_weapon_ammo_types.clear();
			m_shield_hp_front_maximums.clear();
			m_shield_hp_back_maximums.clear();
			m_shield_recharge_rates.clear();
			m_capacitor_eng_maximums.clear();
			m_capacitor_eng_recharge_rates.clear();
			m_engine_acc_rates.clear();
			m_engine_deceleration_rates.clear();
			m_engine_pitch_acc_rates.clear();
			m_engine_yaw_acc_rates.clear();
			m_engine_roll_acc_rates.clear();
			m_engine_pitch_rate_maximums.clear();
			m_engine_yaw_rate_maximums.clear();
			m_engine_roll_rate_maximums.clear();
			m_engine_speed_maximums.clear();
			m_reactor_eng_generation_rates.clear();
			m_booster_eng_maximums.clear();
			m_booster_eng_recharge_rates.clear();
			m_booster_eng_consumption_rates.clear();
			m_booster_accs.clear();
			m_booster_speed_maximums.clear();
			m_droid_if_cmd_speeds.clear();
			m_installed_dcds.clear();
			m_chassis_cmp_mass_maximums.clear();
			m_cmp_creatorss.clear();
			m_cargo_hold_contents_maximums.clear();
			m_cargo_hold_contents_currents.clear();
			m_cargo_hold_contentss.clear();
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

void ShipObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_slide_dampeners.free();
			m_current_chassis_hit_pointss.free();
			m_maximum_chassis_hit_pointss.free();
			m_chassis_types.free();
			m_cmp_armor_hp_maximums.free();
			m_cmp_armor_hp_currents.free();
			m_cmp_efficiency_generals.free();
			m_cmp_efficiency_engs.free();
			m_cmp_eng_maintenances.free();
			m_cmp_masss.free();
			m_cmp_crcs.free();
			m_cmp_hp_currents.free();
			m_cmp_hp_maximums.free();
			m_cmp_flagss.free();
			m_cmp_namess.free();
			m_weapon_damage_maximums.free();
			m_weapon_damage_minimums.free();
			m_weapon_effectiveness_shieldss.free();
			m_weapon_effectiveness_armors.free();
			m_weapon_eng_per_shots.free();
			m_weapon_refire_rates.free();
			m_weapon_ammo_currents.free();
			m_weapon_ammo_maximums.free();
			m_weapon_ammo_types.free();
			m_shield_hp_front_maximums.free();
			m_shield_hp_back_maximums.free();
			m_shield_recharge_rates.free();
			m_capacitor_eng_maximums.free();
			m_capacitor_eng_recharge_rates.free();
			m_engine_acc_rates.free();
			m_engine_deceleration_rates.free();
			m_engine_pitch_acc_rates.free();
			m_engine_yaw_acc_rates.free();
			m_engine_roll_acc_rates.free();
			m_engine_pitch_rate_maximums.free();
			m_engine_yaw_rate_maximums.free();
			m_engine_roll_rate_maximums.free();
			m_engine_speed_maximums.free();
			m_reactor_eng_generation_rates.free();
			m_booster_eng_maximums.free();
			m_booster_eng_recharge_rates.free();
			m_booster_eng_consumption_rates.free();
			m_booster_accs.free();
			m_booster_speed_maximums.free();
			m_droid_if_cmd_speeds.free();
			m_installed_dcds.free();
			m_chassis_cmp_mass_maximums.free();
			m_cmp_creatorss.free();
			m_cargo_hold_contents_maximums.free();
			m_cargo_hold_contents_currents.free();
			m_cargo_hold_contentss.free();
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

bool ShipObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_slide_dampeners)) return false;
			if (!bindParameter(m_current_chassis_hit_pointss)) return false;
			if (!bindParameter(m_maximum_chassis_hit_pointss)) return false;
			if (!bindParameter(m_chassis_types)) return false;
			if (!bindParameter(m_cmp_armor_hp_maximums)) return false;
			if (!bindParameter(m_cmp_armor_hp_currents)) return false;
			if (!bindParameter(m_cmp_efficiency_generals)) return false;
			if (!bindParameter(m_cmp_efficiency_engs)) return false;
			if (!bindParameter(m_cmp_eng_maintenances)) return false;
			if (!bindParameter(m_cmp_masss)) return false;
			if (!bindParameter(m_cmp_crcs)) return false;
			if (!bindParameter(m_cmp_hp_currents)) return false;
			if (!bindParameter(m_cmp_hp_maximums)) return false;
			if (!bindParameter(m_cmp_flagss)) return false;
			if (!bindParameter(m_cmp_namess)) return false;
			if (!bindParameter(m_weapon_damage_maximums)) return false;
			if (!bindParameter(m_weapon_damage_minimums)) return false;
			if (!bindParameter(m_weapon_effectiveness_shieldss)) return false;
			if (!bindParameter(m_weapon_effectiveness_armors)) return false;
			if (!bindParameter(m_weapon_eng_per_shots)) return false;
			if (!bindParameter(m_weapon_refire_rates)) return false;
			if (!bindParameter(m_weapon_ammo_currents)) return false;
			if (!bindParameter(m_weapon_ammo_maximums)) return false;
			if (!bindParameter(m_weapon_ammo_types)) return false;
			if (!bindParameter(m_shield_hp_front_maximums)) return false;
			if (!bindParameter(m_shield_hp_back_maximums)) return false;
			if (!bindParameter(m_shield_recharge_rates)) return false;
			if (!bindParameter(m_capacitor_eng_maximums)) return false;
			if (!bindParameter(m_capacitor_eng_recharge_rates)) return false;
			if (!bindParameter(m_engine_acc_rates)) return false;
			if (!bindParameter(m_engine_deceleration_rates)) return false;
			if (!bindParameter(m_engine_pitch_acc_rates)) return false;
			if (!bindParameter(m_engine_yaw_acc_rates)) return false;
			if (!bindParameter(m_engine_roll_acc_rates)) return false;
			if (!bindParameter(m_engine_pitch_rate_maximums)) return false;
			if (!bindParameter(m_engine_yaw_rate_maximums)) return false;
			if (!bindParameter(m_engine_roll_rate_maximums)) return false;
			if (!bindParameter(m_engine_speed_maximums)) return false;
			if (!bindParameter(m_reactor_eng_generation_rates)) return false;
			if (!bindParameter(m_booster_eng_maximums)) return false;
			if (!bindParameter(m_booster_eng_recharge_rates)) return false;
			if (!bindParameter(m_booster_eng_consumption_rates)) return false;
			if (!bindParameter(m_booster_accs)) return false;
			if (!bindParameter(m_booster_speed_maximums)) return false;
			if (!bindParameter(m_droid_if_cmd_speeds)) return false;
			if (!bindParameter(m_installed_dcds)) return false;
			if (!bindParameter(m_chassis_cmp_mass_maximums)) return false;
			if (!bindParameter(m_cmp_creatorss)) return false;
			if (!bindParameter(m_cargo_hold_contents_maximums)) return false;
			if (!bindParameter(m_cargo_hold_contents_currents)) return false;
			if (!bindParameter(m_cargo_hold_contentss)) return false;
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

bool ShipObjectQuery::bindColumns()
{
	return true;
}

void ShipObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_ship_obj (:object_id, :slide_dampener, :current_chassis_hit_points, :maximum_chassis_hit_points, :chassis_type, :cmp_armor_hp_maximum, :cmp_armor_hp_current, :cmp_efficiency_general, :cmp_efficiency_eng, :cmp_eng_maintenance, :cmp_mass, :cmp_crc, :cmp_hp_current, :cmp_hp_maximum, :cmp_flags, :cmp_names, :weapon_damage_maximum, :weapon_damage_minimum, :weapon_effectiveness_shields, :weapon_effectiveness_armor, :weapon_eng_per_shot, :weapon_refire_rate, :weapon_ammo_current, :weapon_ammo_maximum, :weapon_ammo_type, :shield_hp_front_maximum, :shield_hp_back_maximum, :shield_recharge_rate, :capacitor_eng_maximum, :capacitor_eng_recharge_rate, :engine_acc_rate, :engine_deceleration_rate, :engine_pitch_acc_rate, :engine_yaw_acc_rate, :engine_roll_acc_rate, :engine_pitch_rate_maximum, :engine_yaw_rate_maximum, :engine_roll_rate_maximum, :engine_speed_maximum, :reactor_eng_generation_rate, :booster_eng_maximum, :booster_eng_recharge_rate, :booster_eng_consumption_rate, :booster_acc, :booster_speed_maximum, :droid_if_cmd_speed, :installed_dcd, :chassis_cmp_mass_maximum, :cmp_creators, :cargo_hold_contents_maximum, :cargo_hold_contents_current, :cargo_hold_contents, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_ship_obj (:object_id, :slide_dampener, :current_chassis_hit_points, :maximum_chassis_hit_points, :chassis_type, :cmp_armor_hp_maximum, :cmp_armor_hp_current, :cmp_efficiency_general, :cmp_efficiency_eng, :cmp_eng_maintenance, :cmp_mass, :cmp_crc, :cmp_hp_current, :cmp_hp_maximum, :cmp_flags, :cmp_names, :weapon_damage_maximum, :weapon_damage_minimum, :weapon_effectiveness_shields, :weapon_effectiveness_armor, :weapon_eng_per_shot, :weapon_refire_rate, :weapon_ammo_current, :weapon_ammo_maximum, :weapon_ammo_type, :shield_hp_front_maximum, :shield_hp_back_maximum, :shield_recharge_rate, :capacitor_eng_maximum, :capacitor_eng_recharge_rate, :engine_acc_rate, :engine_deceleration_rate, :engine_pitch_acc_rate, :engine_yaw_acc_rate, :engine_roll_acc_rate, :engine_pitch_rate_maximum, :engine_yaw_rate_maximum, :engine_roll_rate_maximum, :engine_speed_maximum, :reactor_eng_generation_rate, :booster_eng_maximum, :booster_eng_recharge_rate, :booster_eng_consumption_rate, :booster_acc, :booster_speed_maximum, :droid_if_cmd_speed, :installed_dcd, :chassis_cmp_mass_maximum, :cmp_creators, :cargo_hold_contents_maximum, :cargo_hold_contents_current, :cargo_hold_contents, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_ship_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

ShipObjectQuerySelect::ShipObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool ShipObjectQuerySelect::bindParameters ()
{
	return true;
}

bool ShipObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].slide_dampener)) return false;
	if (!bindCol(m_data[0].current_chassis_hit_points)) return false;
	if (!bindCol(m_data[0].maximum_chassis_hit_points)) return false;
	if (!bindCol(m_data[0].chassis_type)) return false;
	if (!bindCol(m_data[0].cmp_armor_hp_maximum)) return false;
	if (!bindCol(m_data[0].cmp_armor_hp_current)) return false;
	if (!bindCol(m_data[0].cmp_efficiency_general)) return false;
	if (!bindCol(m_data[0].cmp_efficiency_eng)) return false;
	if (!bindCol(m_data[0].cmp_eng_maintenance)) return false;
	if (!bindCol(m_data[0].cmp_mass)) return false;
	if (!bindCol(m_data[0].cmp_crc)) return false;
	if (!bindCol(m_data[0].cmp_hp_current)) return false;
	if (!bindCol(m_data[0].cmp_hp_maximum)) return false;
	if (!bindCol(m_data[0].cmp_flags)) return false;
	if (!bindCol(m_data[0].cmp_names)) return false;
	if (!bindCol(m_data[0].weapon_damage_maximum)) return false;
	if (!bindCol(m_data[0].weapon_damage_minimum)) return false;
	if (!bindCol(m_data[0].weapon_effectiveness_shields)) return false;
	if (!bindCol(m_data[0].weapon_effectiveness_armor)) return false;
	if (!bindCol(m_data[0].weapon_eng_per_shot)) return false;
	if (!bindCol(m_data[0].weapon_refire_rate)) return false;
	if (!bindCol(m_data[0].weapon_ammo_current)) return false;
	if (!bindCol(m_data[0].weapon_ammo_maximum)) return false;
	if (!bindCol(m_data[0].weapon_ammo_type)) return false;
	if (!bindCol(m_data[0].shield_hp_front_maximum)) return false;
	if (!bindCol(m_data[0].shield_hp_back_maximum)) return false;
	if (!bindCol(m_data[0].shield_recharge_rate)) return false;
	if (!bindCol(m_data[0].capacitor_eng_maximum)) return false;
	if (!bindCol(m_data[0].capacitor_eng_recharge_rate)) return false;
	if (!bindCol(m_data[0].engine_acc_rate)) return false;
	if (!bindCol(m_data[0].engine_deceleration_rate)) return false;
	if (!bindCol(m_data[0].engine_pitch_acc_rate)) return false;
	if (!bindCol(m_data[0].engine_yaw_acc_rate)) return false;
	if (!bindCol(m_data[0].engine_roll_acc_rate)) return false;
	if (!bindCol(m_data[0].engine_pitch_rate_maximum)) return false;
	if (!bindCol(m_data[0].engine_yaw_rate_maximum)) return false;
	if (!bindCol(m_data[0].engine_roll_rate_maximum)) return false;
	if (!bindCol(m_data[0].engine_speed_maximum)) return false;
	if (!bindCol(m_data[0].reactor_eng_generation_rate)) return false;
	if (!bindCol(m_data[0].booster_eng_maximum)) return false;
	if (!bindCol(m_data[0].booster_eng_recharge_rate)) return false;
	if (!bindCol(m_data[0].booster_eng_consumption_rate)) return false;
	if (!bindCol(m_data[0].booster_acc)) return false;
	if (!bindCol(m_data[0].booster_speed_maximum)) return false;
	if (!bindCol(m_data[0].droid_if_cmd_speed)) return false;
	if (!bindCol(m_data[0].installed_dcd)) return false;
	if (!bindCol(m_data[0].chassis_cmp_mass_maximum)) return false;
	if (!bindCol(m_data[0].cmp_creators)) return false;
	if (!bindCol(m_data[0].cargo_hold_contents_maximum)) return false;
	if (!bindCol(m_data[0].cargo_hold_contents_current)) return false;
	if (!bindCol(m_data[0].cargo_hold_contents)) return false;
	return true;
}

const std::vector<ShipObjectRow> & ShipObjectQuerySelect::getData() const
{
	return m_data;
}

void ShipObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_ship_object; end;";
}

DB::Query::QueryMode ShipObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

StaticObjectQuery::StaticObjectQuery() :
	DatabaseProcessQuery(new StaticObjectRow),
	m_numItems(0)
{
}

bool StaticObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool StaticObjectQuery::addData(const DB::Row *_data)
{
	const StaticObjectBufferRow *myData=safe_cast<const StaticObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
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

int StaticObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void StaticObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
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

void StaticObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
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

bool StaticObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
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

bool StaticObjectQuery::bindColumns()
{
	return true;
}

void StaticObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_static_obj (:object_id, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_static_obj (:object_id, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_static_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

StaticObjectQuerySelect::StaticObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool StaticObjectQuerySelect::bindParameters ()
{
	return true;
}

bool StaticObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	return true;
}

const std::vector<StaticObjectRow> & StaticObjectQuerySelect::getData() const
{
	return m_data;
}

void StaticObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_static_object; end;";
}

DB::Query::QueryMode StaticObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

TangibleObjectQuery::TangibleObjectQuery() :
	DatabaseProcessQuery(new TangibleObjectRow),
	m_numItems(0)
{
}

bool TangibleObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_max_hit_pointss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_owner_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_visibles.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_appearance_datas.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_interest_radiuss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_pvp_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_pvp_factions.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_damage_takens.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_custom_appearances.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_counts.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_conditions.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_creator_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_source_draft_schematics.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
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

bool TangibleObjectQuery::addData(const DB::Row *_data)
{
	const TangibleObjectBufferRow *myData=safe_cast<const TangibleObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_max_hit_pointss.push_back(myData->max_hit_points.isNull(), myData->max_hit_points.getValue())) return false;
			if (!m_owner_ids.push_back(myData->owner_id.getValueASCII())) return false;
			if (!m_visibles.push_back(myData->visible.getValueASCII())) return false;
			if (!m_appearance_datas.push_back(myData->appearance_data.getValueASCII())) return false;
			if (!m_interest_radiuss.push_back(myData->interest_radius.isNull(), myData->interest_radius.getValue())) return false;
			if (!m_pvp_types.push_back(myData->pvp_type.isNull(), myData->pvp_type.getValue())) return false;
			if (!m_pvp_factions.push_back(myData->pvp_faction.isNull(), myData->pvp_faction.getValue())) return false;
			if (!m_damage_takens.push_back(myData->damage_taken.isNull(), myData->damage_taken.getValue())) return false;
			if (!m_custom_appearances.push_back(myData->custom_appearance.getValueASCII())) return false;
			if (!m_counts.push_back(myData->count.isNull(), myData->count.getValue())) return false;
			if (!m_conditions.push_back(myData->condition.isNull(), myData->condition.getValue())) return false;
			if (!m_creator_ids.push_back(myData->creator_id.getValueASCII())) return false;
			if (!m_source_draft_schematics.push_back(myData->source_draft_schematic.isNull(), myData->source_draft_schematic.getValue())) return false;
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

int TangibleObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void TangibleObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_max_hit_pointss.clear();
			m_owner_ids.clear();
			m_visibles.clear();
			m_appearance_datas.clear();
			m_interest_radiuss.clear();
			m_pvp_types.clear();
			m_pvp_factions.clear();
			m_damage_takens.clear();
			m_custom_appearances.clear();
			m_counts.clear();
			m_conditions.clear();
			m_creator_ids.clear();
			m_source_draft_schematics.clear();
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

void TangibleObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_max_hit_pointss.free();
			m_owner_ids.free();
			m_visibles.free();
			m_appearance_datas.free();
			m_interest_radiuss.free();
			m_pvp_types.free();
			m_pvp_factions.free();
			m_damage_takens.free();
			m_custom_appearances.free();
			m_counts.free();
			m_conditions.free();
			m_creator_ids.free();
			m_source_draft_schematics.free();
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

bool TangibleObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_max_hit_pointss)) return false;
			if (!bindParameter(m_owner_ids)) return false;
			if (!bindParameter(m_visibles)) return false;
			if (!bindParameter(m_appearance_datas)) return false;
			if (!bindParameter(m_interest_radiuss)) return false;
			if (!bindParameter(m_pvp_types)) return false;
			if (!bindParameter(m_pvp_factions)) return false;
			if (!bindParameter(m_damage_takens)) return false;
			if (!bindParameter(m_custom_appearances)) return false;
			if (!bindParameter(m_counts)) return false;
			if (!bindParameter(m_conditions)) return false;
			if (!bindParameter(m_creator_ids)) return false;
			if (!bindParameter(m_source_draft_schematics)) return false;
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

bool TangibleObjectQuery::bindColumns()
{
	return true;
}

void TangibleObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_tangible_obj (:object_id, :max_hit_points, :owner_id, :visible, :appearance_data, :interest_radius, :pvp_type, :pvp_faction, :damage_taken, :custom_appearance, :count, :condition, :creator_id, :source_draft_schematic, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_tangible_obj (:object_id, :max_hit_points, :owner_id, :visible, :appearance_data, :interest_radius, :pvp_type, :pvp_faction, :damage_taken, :custom_appearance, :count, :condition, :creator_id, :source_draft_schematic, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_tangible_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

TangibleObjectQuerySelect::TangibleObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool TangibleObjectQuerySelect::bindParameters ()
{
	return true;
}

bool TangibleObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].max_hit_points)) return false;
	if (!bindCol(m_data[0].owner_id)) return false;
	if (!bindCol(m_data[0].visible)) return false;
	if (!bindCol(m_data[0].appearance_data)) return false;
	if (!bindCol(m_data[0].interest_radius)) return false;
	if (!bindCol(m_data[0].pvp_type)) return false;
	if (!bindCol(m_data[0].pvp_faction)) return false;
	if (!bindCol(m_data[0].damage_taken)) return false;
	if (!bindCol(m_data[0].custom_appearance)) return false;
	if (!bindCol(m_data[0].count)) return false;
	if (!bindCol(m_data[0].condition)) return false;
	if (!bindCol(m_data[0].creator_id)) return false;
	if (!bindCol(m_data[0].source_draft_schematic)) return false;
	return true;
}

const std::vector<TangibleObjectRow> & TangibleObjectQuerySelect::getData() const
{
	return m_data;
}

void TangibleObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_tangible_object; end;";
}

DB::Query::QueryMode TangibleObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

TokenObjectQuery::TokenObjectQuery() :
	DatabaseProcessQuery(new TokenObjectRow),
	m_numItems(0)
{
}

bool TokenObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_references.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_target_server_template_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_target_shared_template_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_waypoints.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool TokenObjectQuery::addData(const DB::Row *_data)
{
	const TokenObjectBufferRow *myData=safe_cast<const TokenObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_references.push_back(myData->reference.getValueASCII())) return false;
			if (!m_target_server_template_names.push_back(myData->target_server_template_name.getValueASCII())) return false;
			if (!m_target_shared_template_names.push_back(myData->target_shared_template_name.getValueASCII())) return false;
			if (!m_waypoints.push_back(myData->waypoint.getValueASCII())) return false;
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

int TokenObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void TokenObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_references.clear();
			m_target_server_template_names.clear();
			m_target_shared_template_names.clear();
			m_waypoints.clear();
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

void TokenObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_references.free();
			m_target_server_template_names.free();
			m_target_shared_template_names.free();
			m_waypoints.free();
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

bool TokenObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_references)) return false;
			if (!bindParameter(m_target_server_template_names)) return false;
			if (!bindParameter(m_target_shared_template_names)) return false;
			if (!bindParameter(m_waypoints)) return false;
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

bool TokenObjectQuery::bindColumns()
{
	return true;
}

void TokenObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_token_obj (:object_id, :reference, :target_server_template_name, :target_shared_template_name, :waypoint, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_token_obj (:object_id, :reference, :target_server_template_name, :target_shared_template_name, :waypoint, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_token_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

TokenObjectQuerySelect::TokenObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool TokenObjectQuerySelect::bindParameters ()
{
	return true;
}

bool TokenObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].reference)) return false;
	if (!bindCol(m_data[0].target_server_template_name)) return false;
	if (!bindCol(m_data[0].target_shared_template_name)) return false;
	if (!bindCol(m_data[0].waypoint)) return false;
	return true;
}

const std::vector<TokenObjectRow> & TokenObjectQuerySelect::getData() const
{
	return m_data;
}

void TokenObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_token_object; end;";
}

DB::Query::QueryMode TokenObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

UniverseObjectQuery::UniverseObjectQuery() :
	DatabaseProcessQuery(new UniverseObjectRow),
	m_numItems(0)
{
}

bool UniverseObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
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

bool UniverseObjectQuery::addData(const DB::Row *_data)
{
	const UniverseObjectBufferRow *myData=safe_cast<const UniverseObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
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

int UniverseObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void UniverseObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
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

void UniverseObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
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

bool UniverseObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
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

bool UniverseObjectQuery::bindColumns()
{
	return true;
}

void UniverseObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_universe_obj (:object_id, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_universe_obj (:object_id, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_universe_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

UniverseObjectQuerySelect::UniverseObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool UniverseObjectQuerySelect::bindParameters ()
{
	return true;
}

bool UniverseObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	return true;
}

const std::vector<UniverseObjectRow> & UniverseObjectQuerySelect::getData() const
{
	return m_data;
}

void UniverseObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_universe_object; end;";
}

DB::Query::QueryMode UniverseObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

VehicleObjectQuery::VehicleObjectQuery() :
	DatabaseProcessQuery(new VehicleObjectRow),
	m_numItems(0)
{
}

bool VehicleObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_boguss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
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

bool VehicleObjectQuery::addData(const DB::Row *_data)
{
	const VehicleObjectBufferRow *myData=safe_cast<const VehicleObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_boguss.push_back(myData->bogus.isNull(), myData->bogus.getValue())) return false;
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

int VehicleObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void VehicleObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_boguss.clear();
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

void VehicleObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_boguss.free();
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

bool VehicleObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_boguss)) return false;
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

bool VehicleObjectQuery::bindColumns()
{
	return true;
}

void VehicleObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_vehicle_obj (:object_id, :bogus, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_vehicle_obj (:object_id, :bogus, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_vehicle_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

VehicleObjectQuerySelect::VehicleObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool VehicleObjectQuerySelect::bindParameters ()
{
	return true;
}

bool VehicleObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].bogus)) return false;
	return true;
}

const std::vector<VehicleObjectRow> & VehicleObjectQuerySelect::getData() const
{
	return m_data;
}

void VehicleObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_vehicle_object; end;";
}

DB::Query::QueryMode VehicleObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

WeaponObjectQuery::WeaponObjectQuery() :
	DatabaseProcessQuery(new WeaponObjectRow),
	m_numItems(0)
{
}

bool WeaponObjectQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(), 1000)) return false;
			if (!m_min_damages.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_max_damages.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_damage_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_elemental_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_elemental_values.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attack_speeds.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_wound_chances.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_accuracys.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_attack_costs.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_damage_radiuss.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_min_ranges.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_max_ranges.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
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

bool WeaponObjectQuery::addData(const DB::Row *_data)
{
	const WeaponObjectBufferRow *myData=safe_cast<const WeaponObjectBufferRow*>(_data);
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_object_ids.push_back(myData->object_id.getValueASCII())) return false;
			if (!m_min_damages.push_back(myData->min_damage.isNull(), myData->min_damage.getValue())) return false;
			if (!m_max_damages.push_back(myData->max_damage.isNull(), myData->max_damage.getValue())) return false;
			if (!m_damage_types.push_back(myData->damage_type.isNull(), myData->damage_type.getValue())) return false;
			if (!m_elemental_types.push_back(myData->elemental_type.isNull(), myData->elemental_type.getValue())) return false;
			if (!m_elemental_values.push_back(myData->elemental_value.isNull(), myData->elemental_value.getValue())) return false;
			if (!m_attack_speeds.push_back(myData->attack_speed.isNull(), myData->attack_speed.getValue())) return false;
			if (!m_wound_chances.push_back(myData->wound_chance.isNull(), myData->wound_chance.getValue())) return false;
			if (!m_accuracys.push_back(myData->accuracy.isNull(), myData->accuracy.getValue())) return false;
			if (!m_attack_costs.push_back(myData->attack_cost.isNull(), myData->attack_cost.getValue())) return false;
			if (!m_damage_radiuss.push_back(myData->damage_radius.isNull(), myData->damage_radius.getValue())) return false;
			if (!m_min_ranges.push_back(myData->min_range.isNull(), myData->min_range.getValue())) return false;
			if (!m_max_ranges.push_back(myData->max_range.isNull(), myData->max_range.getValue())) return false;
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

int WeaponObjectQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void WeaponObjectQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.clear();
			m_min_damages.clear();
			m_max_damages.clear();
			m_damage_types.clear();
			m_elemental_types.clear();
			m_elemental_values.clear();
			m_attack_speeds.clear();
			m_wound_chances.clear();
			m_accuracys.clear();
			m_attack_costs.clear();
			m_damage_radiuss.clear();
			m_min_ranges.clear();
			m_max_ranges.clear();
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

void WeaponObjectQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_object_ids.free();
			m_min_damages.free();
			m_max_damages.free();
			m_damage_types.free();
			m_elemental_types.free();
			m_elemental_values.free();
			m_attack_speeds.free();
			m_wound_chances.free();
			m_accuracys.free();
			m_attack_costs.free();
			m_damage_radiuss.free();
			m_min_ranges.free();
			m_max_ranges.free();
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

bool WeaponObjectQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_object_ids)) return false;
			if (!bindParameter(m_min_damages)) return false;
			if (!bindParameter(m_max_damages)) return false;
			if (!bindParameter(m_damage_types)) return false;
			if (!bindParameter(m_elemental_types)) return false;
			if (!bindParameter(m_elemental_values)) return false;
			if (!bindParameter(m_attack_speeds)) return false;
			if (!bindParameter(m_wound_chances)) return false;
			if (!bindParameter(m_accuracys)) return false;
			if (!bindParameter(m_attack_costs)) return false;
			if (!bindParameter(m_damage_radiuss)) return false;
			if (!bindParameter(m_min_ranges)) return false;
			if (!bindParameter(m_max_ranges)) return false;
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

bool WeaponObjectQuery::bindColumns()
{
	return true;
}

void WeaponObjectQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_weapon_obj (:object_id, :min_damage, :max_damage, :damage_type, :elemental_type, :elemental_value, :attack_speed, :wound_chance, :accuracy, :attack_cost, :damage_radius, :min_range, :max_range, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_weapon_obj (:object_id, :min_damage, :max_damage, :damage_type, :elemental_type, :elemental_value, :attack_speed, :wound_chance, :accuracy, :attack_cost, :damage_radius, :min_range, :max_range, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.remove_weapon_obj (:object_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

WeaponObjectQuerySelect::WeaponObjectQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool WeaponObjectQuerySelect::bindParameters ()
{
	return true;
}

bool WeaponObjectQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].min_damage)) return false;
	if (!bindCol(m_data[0].max_damage)) return false;
	if (!bindCol(m_data[0].damage_type)) return false;
	if (!bindCol(m_data[0].elemental_type)) return false;
	if (!bindCol(m_data[0].elemental_value)) return false;
	if (!bindCol(m_data[0].attack_speed)) return false;
	if (!bindCol(m_data[0].wound_chance)) return false;
	if (!bindCol(m_data[0].accuracy)) return false;
	if (!bindCol(m_data[0].attack_cost)) return false;
	if (!bindCol(m_data[0].damage_radius)) return false;
	if (!bindCol(m_data[0].min_range)) return false;
	if (!bindCol(m_data[0].max_range)) return false;
	return true;
}

const std::vector<WeaponObjectRow> & WeaponObjectQuerySelect::getData() const
{
	return m_data;
}

void WeaponObjectQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"loader.load_weapon_object; end;";
}

DB::Query::QueryMode WeaponObjectQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

//!!!END GENERATED QUERYDEFINITIONS

// ======================================================================
