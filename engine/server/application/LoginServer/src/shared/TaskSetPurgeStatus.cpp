// ======================================================================
//
// TaskSetPurgeStatus.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskSetPurgeStatus.h"

#include "DatabaseConnection.h"
#include "PurgeManager.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

namespace TaskSetPurgeStatusNamespace
{
	class SetPurgeStatusQuery : public DB::Query
	{
	public:
		SetPurgeStatusQuery(StationId account, int purgePhase);

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	private:
		DB::BindableLong m_account;
		DB::BindableLong m_purge_phase;

	private: //disable
		SetPurgeStatusQuery(const SetPurgeStatusQuery&);
		SetPurgeStatusQuery& operator=(const SetPurgeStatusQuery&);
	};
}
using namespace TaskSetPurgeStatusNamespace;

// ======================================================================

TaskSetPurgeStatus::TaskSetPurgeStatus(StationId account, int purgePhase) :
		DB::TaskRequest(),
		m_account(account),
		m_purgePhase(purgePhase)
{
}

// ----------------------------------------------------------------------

bool TaskSetPurgeStatus::process(DB::Session *session)
{
	SetPurgeStatusQuery qry(m_account, m_purgePhase);
	if (!session->exec(&qry))
		return false;
	return true;
}

// ----------------------------------------------------------------------

void TaskSetPurgeStatus::onComplete()
{
}

// ======================================================================


SetPurgeStatusQuery::SetPurgeStatusQuery(StationId account, int purgePhase) :
		Query(),
		m_account(account),
		m_purge_phase(purgePhase)
{
}

// ----------------------------------------------------------------------

void SetPurgeStatusQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ")+DatabaseConnection::getInstance().getSchemaQualifier()+"purge_process.set_purge_status(:account, :purge_phase); end;";
}

// ----------------------------------------------------------------------

bool SetPurgeStatusQuery::bindParameters()
{
	if (!bindParameter(m_account)) return false;
	if (!bindParameter(m_purge_phase)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool SetPurgeStatusQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

SetPurgeStatusQuery::QueryMode SetPurgeStatusQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
