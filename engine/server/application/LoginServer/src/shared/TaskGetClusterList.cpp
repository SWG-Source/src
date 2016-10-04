// ======================================================================
//
// TaskGetClusterList.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskGetClusterList.h"

#include "serverNetworkMessages/AvatarList.h"
#include "ConfigLoginServer.h"
#include "DatabaseConnection.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

TaskGetClusterList::TaskGetClusterList (int const groupId) :
		TaskRequest(),
		m_clusterData(),
		m_groupId(groupId)
{
}

// ----------------------------------------------------------------------

bool TaskGetClusterList::process(DB::Session *session)
{
	GetClustersQuery qry;
	int rowsFetched;

	qry.group_id = m_groupId;
	
	if (! (session->exec(&qry)))
		return false;

	ClusterData temp;
	
	while ((rowsFetched = qry.fetch()) > 0)
	{
		qry.cluster_id.getValue(temp.m_clusterId);
		qry.cluster_name.getValue(temp.m_clusterName);
		qry.address.getValue(temp.m_address);
		if (qry.port.isNull())
			temp.m_port = ConfigLoginServer::getCentralServicePort();
		else
			temp.m_port = static_cast<uint16>(qry.port.getValue());
		qry.secret.getValue(temp.m_secret);
		qry.locked.getValue(temp.m_locked);
		qry.not_recommended.getValue(temp.m_notRecommended);
		qry.maxCharacterPerAccount.getValue(temp.m_maxCharactersPerAccount);
		qry.online_player_limit.getValue(temp.m_onlinePlayerLimit);
		qry.online_free_trial_limit.getValue(temp.m_onlineFreeTrialLimit);
		qry.free_trial_can_create_char.getValue(temp.m_freeTrialCanCreateChar);
		qry.online_tutorial_limit.getValue(temp.m_onlineTutorialLimit);

		DEBUG_WARNING(true, ("Customer facing cluster address for %s is %s", temp.m_clusterName.c_str(), temp.m_address.c_str()));

		m_clusterData.push_back(temp);
	}
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void TaskGetClusterList::onComplete()
{
	for (std::vector<ClusterData>::const_iterator i=m_clusterData.begin(); i!= m_clusterData.end(); ++i)
	{
		FATAL(i->m_clusterId<1,("Cluster id %i was specified in the database for cluster \"%s\".  1 is the minimum legal cluster id",i->m_clusterId, i->m_clusterName.c_str()));
		LoginServer::getInstance().updateClusterData((*i).m_clusterId, (*i).m_clusterName, (*i).m_address, (*i).m_port, (*i).m_secret, (*i).m_locked, (*i).m_notRecommended, (*i).m_maxCharactersPerAccount, (*i).m_onlinePlayerLimit, (*i).m_onlineFreeTrialLimit, (*i).m_freeTrialCanCreateChar, (*i).m_onlineTutorialLimit);
	}
}

// ======================================================================

void TaskGetClusterList::GetClustersQuery::getSQL(std::string &sql)
{
  sql=std::string("begin :result := ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.get_cluster_list(:group_id); end;";
  // DEBUG_REPORT_LOG(true, ("TaskGetClusterList SQL: %s\n", sql.c_str()));

}

// ----------------------------------------------------------------------

bool TaskGetClusterList::GetClustersQuery::bindParameters()
{
	if (!bindParameter(group_id)) return false;
	return true;	
}

// ----------------------------------------------------------------------

bool TaskGetClusterList::GetClustersQuery::bindColumns()
{
	if (!bindCol(cluster_id)) return false;
	if (!bindCol(cluster_name)) return false;
	if (!bindCol(address)) return false;
	if (!bindCol(port)) return false;
	if (!bindCol(secret)) return false;
	if (!bindCol(locked)) return false;
	if (!bindCol(not_recommended)) return false;
	if (!bindCol(maxCharacterPerAccount)) return false;
	if (!bindCol(online_player_limit)) return false;
	if (!bindCol(online_free_trial_limit)) return false;
	if (!bindCol(free_trial_can_create_char)) return false;
	if (!bindCol(online_tutorial_limit)) return false;
	
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskGetClusterList::GetClustersQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ----------------------------------------------------------------------

TaskGetClusterList::GetClustersQuery::GetClustersQuery() :
		Query(),
		group_id(),
		cluster_id(),
		cluster_name(),
		address(),
		port(),
		secret(),
		locked(),
		not_recommended(),
		maxCharacterPerAccount(),
		online_player_limit(),
		online_free_trial_limit(),
		free_trial_can_create_char(),
		online_tutorial_limit()
{
}

// ======================================================================
