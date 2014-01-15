#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbQuery.h"

#include "sharedDatabaseInterface/DbException.h"
#include "sharedDatabaseInterface/DbQueryImplementation.h"
#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/Clock.h"
#include "sharedLog/Log.h"

#include <string>

using namespace DB;

namespace DBQueryNamespace
{
	void logLargeText(std::string const & logSection, std::string const & text);
}

Query::Query() :
		impl(0),
		m_session(0),
		inUse(false),
		m_execTime(0),
		m_fetchTime(0),
		m_execCount(0),
		m_fetchCount(0),
		m_rowsFetched(0),
		m_debugName(0),
		m_queryMode(MODE_SQL)
{
}

Query::~Query()
{
	if (inUse)
		done();

	delete impl;
	delete m_debugName;
	impl = 0;
	m_debugName = 0;
} //lint !e1740 // dtor doesn't delete m_session (because we don't own it)

int Query::fetch()
{
	int startTime = 0;
	if (Server::isProfilingEnabled())
		startTime = Clock::timeMs();

	NOT_NULL(impl);
	m_rowsFetched = impl->fetch();

	if (Server::isProfilingEnabled())
	{
		m_fetchTime += Clock::timeMs() - startTime;
	}
	if (m_rowsFetched >= 0)
	{
		m_fetchCount += m_rowsFetched;
		return m_rowsFetched;
	}
	else
		return -1;
}

void Query::done()
{
	if (inUse) // so that it's safe to call Done() on a query that hasn't been used
	{
		NOT_NULL(impl);
		NOT_NULL(m_session);

		impl->done();
		m_session->queryDone();
		m_session->addQueryCount(m_execCount, m_fetchCount);
		if (Server::isProfilingEnabled())
		{
			NOT_NULL(m_debugName); // should have been set by setup()
			Server::addQueryProfileData(*m_debugName, m_execTime, m_fetchTime, m_execCount, m_fetchCount);
			
			// reset the profiler data in case we run this query again:
			m_execTime=0;
			m_fetchTime=0;
			m_execCount=0;
			m_fetchCount=0;
		}
		inUse=false;
		m_session=0;
	}
}

// ----------------------------------------------------------------------

/**
 * Returns the number of rows affected by the most recent query.  (Only applies
 * to DML queries.)
 */
int Query::rowCount()
{
	if (!inUse)
	{
		return 0;
	}
	else
	{
		NOT_NULL(impl);
		return impl->rowCount();
	}
}

// ----------------------------------------------------------------------

bool Query::setup(Session *session)
{
	if (impl == 0)
	{
		impl=session->createQueryImpl(this);
	}
	NOT_NULL(impl);
	NOT_NULL(session);
	m_session=session;

	if (Server::isProfilingEnabled() && !m_debugName)
	{
		m_debugName = new std::string;
		getDebugName(*m_debugName);
	}

	if (impl->setup(session))
	{
		inUse=true;
		return true;
	}
	else
		return false;
}

bool Query::prepare()
{
	NOT_NULL(impl);
	return impl->prepare();
}

bool Query::exec()
{
	int startTime = 0;
	if (Server::isProfilingEnabled())
		startTime = Clock::timeMs();

	m_queryMode=getExecutionMode();

	NOT_NULL(impl);
	if (Server::isVerboseModeEnabled())
	{
		std::string context;
		getDebugName(context);
		DEBUG_REPORT_LOG(true,("query:  %s\n",context.c_str()));
	}
	bool result = impl->exec();

	if (Server::isProfilingEnabled())
	{
		m_execTime += Clock::timeMs() - startTime;
	}
	++m_execCount;

	if (! result)
	{
		std::string data = outputDataValues();
		LOG("DatabaseError",("Data from previous error:"));
		DBQueryNamespace::logLargeText("DatabaseError", data);
		done();
	}
	return result;
}

void Query::setColArrayMode(size_t skipSize, size_t numElements)
{
	NOT_NULL(impl);
	impl->setColArrayMode(skipSize, numElements);
}

bool Query::bindCol(BindableLong &buffer)
{
	NOT_NULL(impl);
	return impl->bindCol(buffer);
}

bool Query::bindParameter(BindableLong &buffer)
{
	NOT_NULL(impl);
	return impl->bindParameter(buffer);
}

bool Query::bindCol(BindableDouble &buffer)
{
	NOT_NULL(impl);
	return impl->bindCol(buffer);
}

bool Query::bindParameter(BindableDouble &buffer)
{
	NOT_NULL(impl);
	return impl->bindParameter(buffer);
}

bool Query::bindCol(BindableStringBase &buffer)
{
	NOT_NULL(impl);
	return impl->bindCol(buffer);
}

bool Query::bindParameter(BindableStringBase &buffer)
{
	NOT_NULL(impl);
	return impl->bindParameter(buffer);
}

bool Query::bindCol(BindableUnicodeBase &buffer)
{
	NOT_NULL(impl);
	return impl->bindCol(buffer);
}

bool Query::bindParameter(BindableUnicodeBase &buffer)
{
	NOT_NULL(impl);
	return impl->bindParameter(buffer);
}

bool Query::bindCol(BindableBool &buffer)
{
	NOT_NULL(impl);
	return impl->bindCol(buffer);
}

bool Query::bindParameter(BindableBool &buffer)
{
	NOT_NULL(impl);
	return impl->bindParameter(buffer);
}

bool Query::bindParameter(BindableVarray &buffer)
{
	NOT_NULL(impl);
	return impl->bindParameter(buffer);
}

// ----------------------------------------------------------------------

/**
 * Get the protocol being used to communicate with the database.
 *
 * @returns The protocol being used, or PROTOCOL_DEFAULT if the query is not
 * associated with a database connection.
 */
Protocol Query::getProtocol() const
{
	if (inUse)
	{
		NOT_NULL(impl);
		return impl->getProtocol();
	}
	else
		return PROTOCOL_DEFAULT;
}

// ----------------------------------------------------------------------

/**
 * Override this function to change the execution mode for the query.
 *
 * This function gets invoked after the QueryImpl has been created, so
 * it can return different values depending on the protocol used.
 *
 * The default implementation returns MODE_SQL, which is suitable for
 * most purposes.
 */
Query::QueryMode Query::getExecutionMode() const
{
	return MODE_SQL;
}

// ----------------------------------------------------------------------

/** Override this function to return the debugging name for the query.
 *  The non-overriden version calls getSQL() and uses the result as the query name.
 *  Not const because getSQL() isn't const.
 */

void Query::getDebugName(std::string &debugName)
{
	getSQL(debugName);
}

// ----------------------------------------------------------------------

std::string Query::outputDataValues() const
{
	return impl->outputDataValues();	
}

// ----------------------------------------------------------------------

void DBQueryNamespace::logLargeText(std::string const & logSection, std::string const & text) 
{
	int const targetLineSize = 80;
	int const maxLineSize = 100;
	char buffer[maxLineSize+1];
	buffer[sizeof(buffer)-1]='\0';

	int linePos = 0;
	for (std::string::const_iterator sourcePos = text.begin(); sourcePos != text.end(); ++sourcePos)
	{
		if (*sourcePos < 27)
			buffer[linePos++]='*'; // Hide unprintable/garbage/control characters
		else
			buffer[linePos++]=*sourcePos;
		if ((linePos >= targetLineSize && *sourcePos == ' ') || (linePos >= maxLineSize))
		{
			buffer[linePos]='\0';
			LOG(logSection.c_str(),(buffer));
			linePos=0;
		}
	}
	if (linePos != 0)
	{
		buffer[linePos]='\0';
		LOG(logSection.c_str(),(buffer));
	}
}

// ======================================================================
