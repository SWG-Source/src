// ======================================================================
//
// GetChunkQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/GetChunkQuery.h"

#include "serverDatabase/DatabaseProcess.h"

using namespace DBQuery;

// ======================================================================

GetChunkQuery::GetChunkQuery(const std::string &schema) :
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

void GetChunkQuery::setChunk(const std::string &sceneId, int nodeX, int nodeZ)
{
	scene_id.setValue(sceneId);
	node_x.setValue(nodeX);
	node_z.setValue(nodeZ);
}

// ----------------------------------------------------------------------

DB::Query::QueryMode GetChunkQuery::getExecutionMode() const
{
	return (MODE_PROCEXEC);
}

// ----------------------------------------------------------------------

void GetChunkQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :object_count := ") + m_schema + "loader.load_chunk_object_list (:scene_id, :node_x, :node_z); end;";
}

// ----------------------------------------------------------------------

bool GetChunkQuery::bindParameters()
{
	if (!bindParameter(object_count)) return false;
	if (!bindParameter(scene_id)) return false;
	if (!bindParameter(node_x)) return false;
	if (!bindParameter(node_z)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool GetChunkQuery::bindColumns()
{
	return true;
}

// ======================================================================
