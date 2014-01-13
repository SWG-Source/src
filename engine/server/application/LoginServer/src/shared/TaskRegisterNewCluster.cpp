// ======================================================================
//
// TaskRegisterNewCluster.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskRegisterNewCluster.h"

#include "serverNetworkMessages/AvatarList.h"
#include "DatabaseConnection.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

TaskRegisterNewCluster::TaskRegisterNewCluster (const std::string &newClusterName, const std::string &address) :
		TaskRequest(),
		m_clusterId(0),
		m_clusterName(newClusterName),
		m_address(address)
{
}

// ----------------------------------------------------------------------

bool TaskRegisterNewCluster::process(DB::Session *session)
{
	RegisterClusterQuery qry;
	qry.cluster_name=m_clusterName;
	qry.address=m_address;
	
	bool rval = session->exec(&qry);

	qry.cluster_id.getValue(m_clusterId);
	return rval;
}

// ----------------------------------------------------------------------

void TaskRegisterNewCluster::onComplete()
{
	DEBUG_FATAL(m_clusterId == 0 || m_clusterId > 10000,("TaskRegisterNewCluster got invalid number %u for cluster id when trying to register cluster %s\n",m_clusterId,m_clusterName.c_str()));
	LoginServer::getInstance().onClusterRegistered(m_clusterId, m_clusterName);
}

// ======================================================================

void TaskRegisterNewCluster::RegisterClusterQuery::getSQL(std::string &sql)
{
	sql = std::string("begin ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.register_new_cluster(:cluster_name, :address, :cluster_id); end;";
 	// DEBUG_REPORT_LOG(true, ("TaskRegisterNewCluster SQL: %s\n", sql.c_str()));
}

// ----------------------------------------------------------------------

bool TaskRegisterNewCluster::RegisterClusterQuery::bindParameters()
{
	if (!bindParameter(cluster_name)) return false;
	if (!bindParameter(address)) return false;
	if (!bindParameter(cluster_id)) return false;

	return true;	
}

// ----------------------------------------------------------------------

bool TaskRegisterNewCluster::RegisterClusterQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskRegisterNewCluster::RegisterClusterQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

TaskRegisterNewCluster::RegisterClusterQuery::RegisterClusterQuery() :
		Query(),
		cluster_id(),
		cluster_name(),
		address()
{
}

// ======================================================================
