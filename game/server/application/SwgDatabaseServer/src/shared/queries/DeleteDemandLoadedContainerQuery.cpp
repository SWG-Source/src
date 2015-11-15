// ======================================================================
//
// DeleteDemandLoadedContainerQuery.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/DeleteDemandLoadedContainerQuery.h"

#include "serverDatabase/DatabaseProcess.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

using namespace DBQuery;

// ======================================================================

DeleteDemandLoadedContainerQuery::DeleteDemandLoadedContainerQuery() :
		object_ids(),
		reasons(),
		num_items(0)
{
}

// ----------------------------------------------------------------------

DeleteDemandLoadedContainerQuery::~DeleteDemandLoadedContainerQuery()
{
}

// ----------------------------------------------------------------------

bool DeleteDemandLoadedContainerQuery::bindParameters()
{
	if (!bindParameter(object_ids)) return false;
	if (!bindParameter(reasons)) return false;
	if (!bindParameter(num_items)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool DeleteDemandLoadedContainerQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

void DeleteDemandLoadedContainerQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.delete_demand_container (:object_ids, :reasons, :num_items); end;";
}

// ----------------------------------------------------------------------

DB::Query::QueryMode DeleteDemandLoadedContainerQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

bool DeleteDemandLoadedContainerQuery::addData(const NetworkId &networkId, int reason)
{
	if (!object_ids.push_back(networkId.getValueString())) return false;
	if (!reasons.push_back(reason)) return false;
	num_items=num_items.getValue() + 1;
	return true;
}

// ----------------------------------------------------------------------

bool DeleteDemandLoadedContainerQuery::setupData(DB::Session &session)
{
	if (!object_ids.create(&session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!reasons.create(&session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
	return true;
}

// ----------------------------------------------------------------------

void DeleteDemandLoadedContainerQuery::clearData()
{
	object_ids.clear();
	reasons.clear();
}

// ----------------------------------------------------------------------

void DeleteDemandLoadedContainerQuery::freeData()
{
	object_ids.free();
	reasons.free();
}

// ----------------------------------------------------------------------

int DeleteDemandLoadedContainerQuery::getCount() const
{
	return num_items.getValue();
}

// ======================================================================
