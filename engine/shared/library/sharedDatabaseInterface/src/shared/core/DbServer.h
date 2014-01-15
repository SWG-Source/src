// ======================================================================
//
// DBServer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DBServer_H
#define INCLUDED_DBServer_H

// ======================================================================

#include "sharedDatabaseInterface/DbProtocol.def"
#include "sharedSynchronization/Mutex.h"

// ======================================================================

namespace DB {

	class Session;
	class Profiler;

/**
 * The Server class handles connecting to an external database server.  It knows how to establish connections,
 * do error handling, etc.  Create a Server object for each database you with to connect to.  Then, call getSession()
 * to get a Session object, which represents an individual session on the database server.  Queries can be passed
 * to the Session object.
 *
 * The Server class does session pooling.  Call the Connect() method to pre-allocate some Sessions in a pool.
 * getSession() will return one of the Sessions from the pool, without blocking.  If the pool is empty, getSession()
 * will establish a new Session, which may require some blocking while the underlying network connection is set up.
 * 
 * The Server class is threadsafe.  Servers can be shared between threads.  Sessions and Queries should not be shared.
 * @todo Actually, it isn't threadsafe.  Need to fix that.
 */
	class Server
	{
/** A sub-class to hold the elements in the list of Sessions making up the Session pool
 */
	struct SesListElem {
		Session *ses;
		SesListElem *next;

		SesListElem (Session *_ses, SesListElem *_next);
	};

	  protected:
	
	char dsn[80],uid[80],pwd[80];

	  private:
	
/** Pool of available Sessions.
 */
	SesListElem *sessionList;

	Mutex sessionListLock;
	bool m_useMemoryManager;
	
	static Mutex ms_profileMutex;
	static bool ms_enableProfiling;
	static bool ms_enableVerboseMode;
	static Profiler *ms_profiler;

	static int ms_prefetchNumRows;
	static int ms_prefetchMemory;
	
	static int ms_reconnectTime;
	static int ms_maxErrorCountBeforeDisconnect;
	static int ms_maxErrorCountBeforeBailout;
	static int ms_errorSleepTime;
	static int ms_disconnectSleepTime;

	static int ms_execCount;
	static int ms_fetchCount;

	static bool ms_fatalOnError;
	static bool ms_fatalOnDataError;
			
	  public:

	Server(const char *_dsn, const char *_uid, const char *_pwd, bool useMemoryManager); // TODO:  should be protected?
	virtual ~Server();

	static Server *create(const char *_dsn, const char *_uid, const char *_pwd, Protocol protocol, bool useMemoryManager);
	static Protocol getProtocolByName(const std::string &name);
	
	Session *getSession();
	void releaseSession(Session *session);

	bool connect(int numSessions=1);
	bool disconnect();

	const char *getDSN() const;
	bool getUseMemoryManager() const;

	static void enableProfiling();
	static void disableProfiling();
	static void endProfiling();
	static bool isProfilingEnabled();
	static void enableVerboseMode();
	static void disableVerboseMode();
	static bool isVerboseModeEnabled();
	static void enablePrefetch(int numRows, int memory);
	static void disablePrefetch();
	static bool isPrefetchEnabled();
	static int  getPrefetchRows();
	static int  getPrefetchMemory();
	static void addQueryProfileData(const std::string &queryName, int execTime, int fetchTime, int execCount, int fetchCount);
	static void debugOutputProfile();
	
	static void addQueryCount(int execCount, int fetchCount);
	static void getAndResetQueryCount(int &execCount, int &fetchCount);

	static void setReconnectTime(int reconnectTime);
	static int  getReconnectTime();	

	static void setMaxErrorCountBeforeDisconnect(int maxErrorCountBeforeDisconnect);
	static int  getMaxErrorCountBeforeDisconnect();

	static void setMaxErrorCountBeforeBailout(int maxErrorCountBeforeBailout);
	static int  getMaxErrorCountBeforeBailout();

	static void setErrorSleepTime(int errorSleepTime);
	static int  getErrorSleepTime();
		
	static void setDisconnectSleepTime(int disconnectSleepTime);
	static int  getDisconnectSleepTime();

	static void setFatalOnError(bool fatalOnError);
	static bool getFatalOnError();

	static void setFatalOnDataError(bool fatalOnDataError);
	static bool getFatalOnDataError();
	
	  protected:
	
	virtual Session *createSession() =0;

	  private:
	
	void pushSession(Session *session);
	Session *popSession();

	  private:
	Server(); //disable
	Server(const Server &); //disable
	Server & operator=(const Server &); //disable
	}; //lint !e1712 // no default constructor for SesListElem

}

// ======================================================================

inline const char *DB::Server::getDSN() const
{
	return dsn;
}

// ----------------------------------------------------------------------

inline bool DB::Server::isProfilingEnabled()
{
	return ms_enableProfiling;
}

// ----------------------------------------------------------------------

inline bool DB::Server::isVerboseModeEnabled()
{
	return ms_enableVerboseMode;
}

// ----------------------------------------------------------------------

inline void DB::Server::enableProfiling()
{
	ms_enableProfiling = true;
}

// ----------------------------------------------------------------------

inline void DB::Server::disableProfiling()
{
	ms_enableProfiling = false;
}

// ----------------------------------------------------------------------

inline void DB::Server::enableVerboseMode()
{
	ms_enableVerboseMode = true;
}

// ----------------------------------------------------------------------

inline void DB::Server::disableVerboseMode()
{
	ms_enableVerboseMode = false;
}

// ----------------------------------------------------------------------

inline int DB::Server::getReconnectTime()
{
	return ms_reconnectTime;
}

// ----------------------------------------------------------------------

inline bool DB::Server::getUseMemoryManager() const
{
	return m_useMemoryManager;
}

// ----------------------------------------------------------------------

inline void DB::Server::setMaxErrorCountBeforeDisconnect(int maxErrorCountBeforeDisconnect)
{
	ms_maxErrorCountBeforeDisconnect = maxErrorCountBeforeDisconnect;
}

// ----------------------------------------------------------------------

inline int DB::Server::getMaxErrorCountBeforeDisconnect()
{
	return ms_maxErrorCountBeforeDisconnect;
}

// ----------------------------------------------------------------------

inline void DB::Server::setMaxErrorCountBeforeBailout(int maxErrorCountBeforeBailout)
{
	ms_maxErrorCountBeforeBailout = maxErrorCountBeforeBailout;
}

// ----------------------------------------------------------------------

inline int DB::Server::getMaxErrorCountBeforeBailout()
{
	return ms_maxErrorCountBeforeBailout;
}

// ----------------------------------------------------------------------

inline void DB::Server::setErrorSleepTime(int errorSleepTime)
{
	ms_errorSleepTime=errorSleepTime;
}

// ----------------------------------------------------------------------

inline int DB::Server::getErrorSleepTime()
{
	return ms_errorSleepTime;
}

// ----------------------------------------------------------------------

inline void DB::Server::setDisconnectSleepTime(int disconnectSleepTime)
{
	ms_disconnectSleepTime=disconnectSleepTime;
}

// ----------------------------------------------------------------------

inline int DB::Server::getDisconnectSleepTime()
{
	return ms_disconnectSleepTime;
}

// ======================================================================

#endif
