// ======================================================================
//
// TaskUpdatePurgeAccountList.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskUpdatePurgeAccountList.h"

#include "ConfigLoginServer.h"
#include "DatabaseConnection.h"
#include "PurgeManager.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

namespace TaskUpdatePurgeAccountListNamespace
{
	class UpdateAccoutListQuery : public DB::Query
	{
	public:
		UpdateAccoutListQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		DB::BindableString<100> m_source_table;

	private: //disable
		UpdateAccoutListQuery(const UpdateAccoutListQuery&);
		UpdateAccoutListQuery& operator=(const UpdateAccoutListQuery&);
	};
}
using namespace TaskUpdatePurgeAccountListNamespace;

// ======================================================================

TaskUpdatePurgeAccountList::TaskUpdatePurgeAccountList() :
		DB::TaskRequest()
{
}

// ----------------------------------------------------------------------

bool TaskUpdatePurgeAccountList::process(DB::Session *session)
{
	UpdateAccoutListQuery qry;
	if (!session->exec(&qry))
		return false;
	return true;
}

// ----------------------------------------------------------------------

void TaskUpdatePurgeAccountList::onComplete()
{
}

// ======================================================================


UpdateAccoutListQuery::UpdateAccoutListQuery() :
		Query(),
		m_source_table(ConfigLoginServer::getPurgeAccountSourceTable())
{
}

// ----------------------------------------------------------------------

void UpdateAccoutListQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ")+DatabaseConnection::getInstance().getSchemaQualifier()+"purge_process.update_account_list(:source_table); end;";
}

// ----------------------------------------------------------------------

bool UpdateAccoutListQuery::bindParameters()
{
	if (!bindParameter(m_source_table)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool UpdateAccoutListQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

UpdateAccoutListQuery::QueryMode UpdateAccoutListQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
