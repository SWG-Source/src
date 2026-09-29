// ======================================================================
//
// TaskGetStationIdScheme.cpp
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskGetStationIdScheme.h"

#include "DatabaseConnection.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

TaskGetStationIdScheme::TaskGetStationIdScheme() :
		TaskRequest(),
		m_queried(false),
		m_rows(0),
		m_scheme(),
		m_basis()
{
}

// ----------------------------------------------------------------------

bool TaskGetStationIdScheme::process(DB::Session *session)
{
	GetStationIdSchemeQuery qry;

	// A failed query (for example, no station_id_scheme table because
	// update 272 has not been applied) is reported by onComplete().
	if (session->exec(&qry))
	{
		m_queried = true;
		while (qry.fetch() > 0)
		{
			++m_rows;
			qry.scheme.getValue(m_scheme);
			qry.basis.getValue(m_basis);
		}
	}
	qry.done();
	return true;
}

// ----------------------------------------------------------------------

void TaskGetStationIdScheme::onComplete()
{
	DatabaseConnection::getInstance().onStationIdSchemeRetrieved(m_queried, m_rows, m_scheme, m_basis);
}

// ======================================================================

TaskGetStationIdScheme::GetStationIdSchemeQuery::GetStationIdSchemeQuery() :
		Query(),
		scheme(),
		basis()
{
}

// ----------------------------------------------------------------------

void TaskGetStationIdScheme::GetStationIdSchemeQuery::getSQL(std::string &sql)
{
	sql = std::string("select scheme, basis from ") + DatabaseConnection::getInstance().getSchemaQualifier() + "station_id_scheme";
}

// ----------------------------------------------------------------------

bool TaskGetStationIdScheme::GetStationIdSchemeQuery::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool TaskGetStationIdScheme::GetStationIdSchemeQuery::bindColumns()
{
	if (!bindCol(scheme)) return false;
	if (!bindCol(basis)) return false;
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskGetStationIdScheme::GetStationIdSchemeQuery::getExecutionMode() const
{
	return MODE_SQL;
}

// ======================================================================
