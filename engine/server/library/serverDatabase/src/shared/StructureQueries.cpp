// ======================================================================
//
// StructureQueries.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/StructureQueries.h"

#include "serverDatabase/DatabaseProcess.h"

using namespace DBQuery;

// ======================================================================

void GetStructures::getSQL(std::string &sql)
{
	sql="begin :result := "+DatabaseProcess::getInstance().getSchemaQualifier()+"custserv_procs.get_structures(:character_id); end;";
}

// ----------------------------------------------------------------------

bool GetStructures::bindParameters()
{
	if (!bindParameter(data.character_id)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool GetStructures::bindColumns()
{
	if (!bindCol(data.object_id)) return false;
	if (!bindCol(data.x)) return false;
	if (!bindCol(data.y)) return false;
	if (!bindCol(data.z)) return false;
	if (!bindCol(data.scene_id)) return false;
	if (!bindCol(data.object_template)) return false;
	if (!bindCol(data.deleted)) return false;
	
	return true;
}

// ----------------------------------------------------------------------

const GetStructures::GetStructuresRow &GetStructures::getData() const
{
	return data;
}

// ----------------------------------------------------------------------

void GetStructures::setCharacterId(NetworkId & characterId)
{
	data.character_id.setValue(characterId);
  //	data.station_id.setValue(static_cast<int32>(station_id));
}
		
// ----------------------------------------------------------------------

DB::Query::QueryMode GetStructures::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

