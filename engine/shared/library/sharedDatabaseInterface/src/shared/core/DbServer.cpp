// ======================================================================
//
// DBServer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbServer.h"

#include <string>
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedDatabaseInterface/DbProfiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"

#ifdef DBLIBRARY_ODBC
#include "OdbcServer.h" //TODO:  is this the best way to do DBServer::create()
#endif
#ifdef DBLIBRARY_OCI
#include "OciServer.h"
#endif

using namespace DB;

// ======================================================================

Mutex Server::ms_profileMutex;
bool Server::ms_enableProfiling = false;
bool Server::ms_enableVerboseMode = false;
DB::Profiler *Server::ms_profiler = nullptr;
int Server::ms_reconnectTime=0;
int Server::ms_maxErrorCountBeforeDisconnect=5;
int Server::ms_maxErrorCountBeforeBailout=10;
int Server::ms_errorSleepTime=5000;
int Server::ms_disconnectSleepTime=45000;
int Server::ms_execCount=0;
int Server::ms_fetchCount=0;
int Server::ms_prefetchNumRows=0;
int Server::ms_prefetchMemory=0;
bool Server::ms_fatalOnError=false;
bool Server::ms_fatalOnDataError=false;

// ======================================================================

Server::SesListElem::SesListElem (Session *_ses, SesListElem *_next) : ses (_ses), next (_next)
{
}

// ----------------------------------------------------------------------

Server::Server(const char *_dsn, const char *_uid, const char *_pwd, bool useMemoryManager) :
		sessionList (nullptr),
		sessionListLock(),
		m_useMemoryManager(useMemoryManager)
{
	strncpy(dsn,_dsn,sizeof(dsn)-1);
	strncpy(uid,_uid,sizeof(uid)-1);
	strncpy(pwd,_pwd,sizeof(pwd)-1);

	//insure terminating \0:
	dsn[sizeof(dsn)-1]='\0';
	uid[sizeof(uid)-1]='\0';
	pwd[sizeof(pwd)-1]='\0';
}

// ----------------------------------------------------------------------

// Server destructor
// TODO:  what to do if deallocating handles failes?  (This could happen if connection isn't closed properly, etc.)
Server::~Server()
{
}

// ----------------------------------------------------------------------

/**
 * Establish connections to the database server.
 * Sessions will be pre-connected and put in the pool of sessions.  The
 * caller can later request up to that number of sessions by calling
 * GetSession(), with no blocking.  Requests for additional sessions may
 * block.
 *
 * @param numSessions The number of sessions to establish.  
 */

bool Server::connect(int numSessions)
{
	disconnect();

	for (int i=0;i<numSessions;i++)
	{
		Session *ses=createSession();
		if (!ses->connect())
		{
			ses->disconnect();
			LOG("DatabaseError", ("Failed to connect to database in getSession()"));
			FATAL(true, ("Failed to connect to database in getSession()"));
		}
		pushSession (ses);
	}

	return true;
}

// ----------------------------------------------------------------------

/**
 * Drop all the sessions in the pool of sessions.
 * After calling Disconnect(), any call to GetSession() will require
 * establishing a new connection to the database server.
 * Note that this call only affects Sessions in the pool.  Sessions that
 * are currently in use are not affected.
 */
bool Server::disconnect()
{
	Session *ses=popSession();
	while(ses)
	{
		delete ses;
		ses=popSession();
	}
	return true;
}

// ----------------------------------------------------------------------

/**
 * Get a Session object, representing a session on the database server.
 * Will return a Session object from the pool, if one is avaiable.
 * Otherwise, it will establish a new session.  Warning:  establishing
 * a new session may require a database roundtrip. (i.e. It may block.)
 * To avoid blocking, use connect() ahead of time to establish all the
 * needed Sessions.
 */
Session *Server::getSession()
{
	Session *ses=popSession();
	
	if (!ses) // pool was empty, so make a new one.  TODO:  set a limit on max # of sessions
	{
		ses=createSession();
		while (!ses->connect())
		{
			ses->disconnect();
			LOG("DatabaseError", ("Failed to connect to database in getSession().  Will reconnect in 30 seconds."));
			Os::sleep(30000);
		}
	}
	return ses;
}

// ----------------------------------------------------------------------

/**
 * Call when finished with a Session object.
 * Returns the Session object to the pool.
 * @todo for debugging, make sure the session released actually came from this server
 */
void Server::releaseSession(Session *session)
{
	session->reset();
	pushSession(session);
}

// ----------------------------------------------------------------------

void Server::pushSession(Session *session)
{
	sessionListLock.enter();
	
	SesListElem *e=new SesListElem(session, sessionList);
	sessionList=e;

	sessionListLock.leave();
}

// ----------------------------------------------------------------------

Session *Server::popSession()
{
	sessionListLock.enter();
	
	SesListElem *e=sessionList;
	if (e)
	{
		Session *ses=e->ses;
		sessionList=e->next;
		delete e;
		sessionListLock.leave();
		return ses;
	}
	else
	{
		sessionListLock.leave();
		return nullptr;
	}
}

// ----------------------------------------------------------------------

Server *Server::create(const char *_dsn, const char *_uid, const char *_pwd, Protocol protocol, bool useMemoryManager)
{
	UNREF(_dsn);
	UNREF(_uid);
	UNREF(_pwd);

	switch (protocol)
	{
		
#ifdef DBLIBRARY_ODBC
		case PROTOCOL_ODBC:
			return new ODBCServer(_dsn,_uid,_pwd);
#endif
			
#ifdef DBLIBRARY_OCI
		case PROTOCOL_OCI:
			return new OCIServer(_dsn,_uid,_pwd, useMemoryManager);
#endif
			
		case PROTOCOL_DEFAULT:
			// Create any kind of server we can, prefering ODBCServer
#ifdef DBLIBRARY_ODBC
		return new ODBCServer(_dsn,_uid,_pwd);
#endif
			
#ifdef DBLIBRARY_OCI
		return new OCIServer(_dsn,_uid,_pwd, useMemoryManager);
#endif

		default:
			FATAL(true,("Database protocol specified in Server::create() is not available in this build of the library.\n"));
			return 0;
	}
}

// ----------------------------------------------------------------------

Protocol Server::getProtocolByName(const std::string &name)
{
		
#ifdef DBLIBRARY_ODBC
	if (name=="ODBC")
		return PROTOCOL_ODBC;
#endif
			
#ifdef DBLIBRARY_OCI
	if (name=="OCI")
		return PROTOCOL_OCI;
#endif

	if (name=="DEFAULT")
		return PROTOCOL_DEFAULT;

	FATAL(true,("Specified protocol (%s) is not available in this version of the database library.",name.c_str()));
	return PROTOCOL_DEFAULT;
}

// ----------------------------------------------------------------------

void Server::addQueryProfileData(const std::string &queryName, int execTime, int fetchTime, int execCount, int fetchCount)
{
	ms_profileMutex.enter();
	
	DEBUG_FATAL(!isProfilingEnabled(),("Programmer bug:  you're supposed to check whether profiling is enabled before calling DB::Server::addProfileData (or there could be a race condition that is disabling profiling while queries are running).\n"));
	if (!ms_profiler)
	{
		ms_profiler = new DB::Profiler;
	}

	ms_profiler->addQueryProfileData(queryName,execTime,fetchTime,execCount,fetchCount);
	ms_profileMutex.leave();
}

// ----------------------------------------------------------------------

void Server::debugOutputProfile()
{
	ms_profileMutex.enter();
	if (ms_profiler)
		ms_profiler->debugOutput();
	else
		DEBUG_REPORT_LOG(true,("No database profiler.\n"));
	ms_profileMutex.leave();
}

// ----------------------------------------------------------------------

/**
 * Stop collecting profile data and delete the Profiler.
 */
void Server::endProfiling()
{
	ms_profileMutex.enter();
	delete ms_profiler;
	ms_profiler = nullptr;
	ms_enableProfiling = false;
	ms_profileMutex.leave();	
}

// ----------------------------------------------------------------------

/**
 * Set a timeout for server connections.  If a connection to the database
 * has been open for more than reconnectTime, it will drop the connection
 * and reestablish it the next time Session::reset() is called.
 */
void Server::setReconnectTime(int reconnectTime)
{
	ms_reconnectTime = reconnectTime;
}

// ----------------------------------------------------------------------

/**
 * Count the total number of queries executed and the total number of fetches
 * for profiling.  (This level of profiling is always enabled.  The
 * per-query profiling is optionally enabled.)
 */
void Server::addQueryCount(int execCount, int fetchCount)
{
	ms_profileMutex.enter();

	ms_execCount+=execCount;
	ms_fetchCount+=fetchCount;
	
	ms_profileMutex.leave();
}

// ----------------------------------------------------------------------

void Server::enablePrefetch(int numRows, int memory)
{
	ms_prefetchNumRows = numRows;
	ms_prefetchMemory = memory;
}

// ----------------------------------------------------------------------

void Server::disablePrefetch()
{
	ms_prefetchNumRows = 0;
}

// ----------------------------------------------------------------------

bool Server::isPrefetchEnabled()
{
	return (ms_prefetchNumRows > 0);
}

// ----------------------------------------------------------------------

int Server::getPrefetchRows()
{
	return ms_prefetchNumRows;
}

// ----------------------------------------------------------------------

int Server::getPrefetchMemory()
{
	return ms_prefetchMemory;
}

// ----------------------------------------------------------------------

void Server::getAndResetQueryCount(int &execCount, int &fetchCount)
{
	ms_profileMutex.enter();

	execCount=ms_execCount;
	fetchCount=ms_fetchCount;

	ms_execCount=0;
	ms_fetchCount=0;
	
	ms_profileMutex.leave();
}

// ----------------------------------------------------------------------

void Server::setFatalOnError(bool fatalOnError)
{
	ms_fatalOnError=fatalOnError;
}

// ----------------------------------------------------------------------

bool Server::getFatalOnError()
{
	return ms_fatalOnError;
}

// ----------------------------------------------------------------------

void Server::setFatalOnDataError(bool fatalOnDataError)
{
	ms_fatalOnDataError=fatalOnDataError;
}

// ----------------------------------------------------------------------

bool Server::getFatalOnDataError()
{
	return ms_fatalOnDataError;
}

// ======================================================================
