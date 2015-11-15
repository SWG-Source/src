// ======================================================================
//
// PreloadListQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/PreloadListQuery.h"

#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

using namespace DBQuery;

// ----------------------------------------------------------------------

PreloadListQuery::PreloadListQuery(const std::string &sceneId)
{
	scene_id.setValue(sceneId);
}

// ----------------------------------------------------------------------

PreloadListQuery::~PreloadListQuery()
{
}

// ----------------------------------------------------------------------

void PreloadListQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ") + DatabaseProcess::getInstance().getSchemaQualifier() + "loader.get_preload_list(:planet); end;";
}

// ----------------------------------------------------------------------

bool PreloadListQuery::bindParameters()
{
	if (!bindParameter(scene_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool PreloadListQuery::bindColumns()
{
	if (!bindCol(object_id)) return false;
	if (!bindCol(chunk_x)) return false;
	if (!bindCol(chunk_z)) return false;
	if (!bindCol(preload_range)) return false;
	if (!bindCol(city_server_id)) return false;
	if (!bindCol(wilderness_server_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode PreloadListQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================
