// ======================================================================
//
// DBSession.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"

#include <cstring>
#include <string>
#include <cassert>

#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbServer.h"

using namespace DB;

// ======================================================================

/**
 * Do not create session objects directly.
 * Instead, use DB::Server::GetSession().
 */
Session::Session() : inUse(false), connected(false), lastQuery(0), lastQueryStatement(), m_execCount(0), m_fetchCount(0), m_fatalOnError(false)
{
}

// ----------------------------------------------------------------------

Session::~Session()
{
	DEBUG_FATAL(inUse,("Deleted a session that was still in use.\n"));
	DEBUG_FATAL(connected,("Derived session did not disconnect."));
} //lint !e1540 // didn't delete lastQuery

// ----------------------------------------------------------------------

/**
 * Execute a database query, represented by a Query object.
 */
bool Session::exec(Query *query)
{
	NOT_NULL(query);

	if (!inUse)	// if the query was just run, we don't need to do the setup again
	{
		if (!query->setup(this)) return false;
	}
	else
	{
		// debugging:  make sure this is the _same_ query
		DEBUG_FATAL(lastQuery!=query,("Called Session::exec() on a query without calling done() on the previous query."));
	}

	if (!query->exec())
	{
		std::string context;
		query->getDebugName(context);
		WARNING(true, ("Context from previous database error:  %s", context.c_str()));
		LOG("DatabaseError", ("Context from previous database error:  %s", context.c_str()));
		return false;
	}
	
	lastQuery=query;
	inUse=true;
	return true;
}

// ----------------------------------------------------------------------

/** [Function used by other classes in the database library.]  Called when a Query object finishes its work, so that
	the Session is now free to run another Query.
 */
void Session::queryDone()
{
	inUse=false;
	lastQuery=0;
}

// ----------------------------------------------------------------------

void Session::addQueryCount(int execCount, int fetchCount)
{
	m_execCount += execCount;
	m_fetchCount += fetchCount;
}

// ----------------------------------------------------------------------

void Session::updateServerQueryCount()
{
	Server::addQueryCount(m_execCount, m_fetchCount);
	m_execCount=0;
	m_fetchCount=0;
}

// ----------------------------------------------------------------------

void Session::setLastQueryStatement(std::string const & queryStatement)
{
	lastQueryStatement = queryStatement;
}

// ----------------------------------------------------------------------

std::string Session::getLastQueryStatement() const
{
	return lastQueryStatement;
}

// ----------------------------------------------------------------------

void Session::setFatalOnError(bool fatalOnError)
{
	m_fatalOnError=fatalOnError;
}

// ----------------------------------------------------------------------

bool Session::getFatalOnError() const
{
	return m_fatalOnError;
}

// ======================================================================
