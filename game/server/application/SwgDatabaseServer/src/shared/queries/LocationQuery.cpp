// ======================================================================
//
// LocationQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/LocationQuery.h"

#include "serverUtility/LocationData.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbCheckedConversion.h"

using namespace DBQuery;

// ======================================================================

SaveLocationQuery::SaveLocationQuery()
{
}

// ----------------------------------------------------------------------

bool SaveLocationQuery::bindParameters()
{
	if (!bindParameter(object_id)) return false;
	if (!bindParameter(list_id)) return false;
	if (!bindParameter(index)) return false;
	if (!bindParameter(action)) return false;
	if (!bindParameter(name)) return false;
	if (!bindParameter(scene)) return false;
	if (!bindParameter(x)) return false;
	if (!bindParameter(y)) return false;
	if (!bindParameter(z)) return false;
	if (!bindParameter(radius)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool SaveLocationQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

void SaveLocationQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_location(:object_id,:list_id,:sequence_number,:operation,:name,:scene,:x,:y,:z,:radius); end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode SaveLocationQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

bool SaveLocationQuery::setData(const NetworkId &newObjectId, size_t newListId, int newAction, size_t newIndex, const LocationData &newLocation)
{
	// The list id and index are unsigned 32-bit columns (a 32-bit server's
	// size_t, stored in signed form); a larger value cannot be saved.
	uint32_t listId = 0;
	uint32_t listIndex = 0;
	auto const where = [&newObjectId]() { return "SaveLocationQuery::setData, object " + newObjectId.getValueString(); };
	if (!DB::checkedNarrow(newListId, listId, "location_lists.list_id (save)", where)
		|| !DB::checkedNarrow(newIndex, listIndex, "location_lists.sequence_number (save)", where))
		return false;

	object_id = newObjectId;
	list_id = listId;
	index = listIndex;
	action = newAction;
	name = newLocation.name;
	scene = newLocation.scene;
	const Vector &vect = newLocation.location.getCenter();
	x = vect.x;
	y = vect.y;
	z = vect.z;
	radius = newLocation.location.getRadius();
	return true;
}

// ======================================================================

LoadLocationQuery::LoadLocationQuery(const std::string &schema) :
		m_data(ConfigServerDatabase::getDefaultFetchBatchSize()),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

bool LoadLocationQuery::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool LoadLocationQuery::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].list_id)) return false;
	if (!bindCol(m_data[0].index)) return false;
	if (!bindCol(m_data[0].name)) return false;
	if (!bindCol(m_data[0].scene)) return false;
	if (!bindCol(m_data[0].x)) return false;
	if (!bindCol(m_data[0].y)) return false;
	if (!bindCol(m_data[0].z)) return false;
	if (!bindCol(m_data[0].radius)) return false;

	return true;
}

// ----------------------------------------------------------------------

void LoadLocationQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+m_schema+"loader.load_location (); end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode LoadLocationQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================

