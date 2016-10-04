// ======================================================================
//
// TaskGetAccountForPurge.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskGetAccountForPurge.h"

#include "ConfigLoginServer.h"
#include "DatabaseConnection.h"
#include "PurgeManager.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

namespace TaskGetAccountForPurgeNamespace
{
	class GetAccountForPurgeQuery : public DB::Query
	{
	public:
		GetAccountForPurgeQuery(int purgePhase, int minAge);

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		StationId getAccount() const;

	private:
		DB::BindableLong m_account;
		DB::BindableLong m_min_age;
		DB::BindableLong m_purge_phase;

	private: //disable
		GetAccountForPurgeQuery(const GetAccountForPurgeQuery&);
		GetAccountForPurgeQuery& operator=(const GetAccountForPurgeQuery&);
	};
}
using namespace TaskGetAccountForPurgeNamespace;

// ======================================================================

TaskGetAccountForPurge::TaskGetAccountForPurge(int purgePhase) :
	DB::TaskRequest(),
	m_purgePhase(purgePhase),
	m_account(0)
{
}

// ----------------------------------------------------------------------

bool TaskGetAccountForPurge::process(DB::Session *session)
{
	GetAccountForPurgeQuery qry(m_purgePhase, ConfigLoginServer::getPurgePhaseAdvanceDays(m_purgePhase));
	if (!session->exec(&qry))
		return false;
	m_account = qry.getAccount();
	return true;
}

// ----------------------------------------------------------------------

void TaskGetAccountForPurge::onComplete()
{
	PurgeManager::onGetAccountForPurge(m_account, m_purgePhase);
}

// ======================================================================

GetAccountForPurgeQuery::GetAccountForPurgeQuery(int purgePhase, int minAge) :
	Query(),
	m_account(),
	m_min_age(minAge),
	m_purge_phase(purgePhase)
{
}

// ----------------------------------------------------------------------

void GetAccountForPurgeQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :account := ") + DatabaseConnection::getInstance().getSchemaQualifier() + "purge_process.get_account_for_purge(:purge_phase, :min_age); end;";
}

// ----------------------------------------------------------------------

bool GetAccountForPurgeQuery::bindParameters()
{
	if (!bindParameter(m_account)) return false;
	if (!bindParameter(m_purge_phase)) return false;
	if (!bindParameter(m_min_age)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool GetAccountForPurgeQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

GetAccountForPurgeQuery::QueryMode GetAccountForPurgeQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

StationId GetAccountForPurgeQuery::getAccount() const
{
	if (!m_account.isNull())
		return m_account.getValue();
	else
		return 0;
}

// ======================================================================