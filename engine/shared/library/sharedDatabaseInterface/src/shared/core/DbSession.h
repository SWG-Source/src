// ======================================================================
//
// DBSession.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DBSession_H
#define INCLUDED_DBSession_H

// ======================================================================

#include <string>

// ======================================================================

namespace DB {

class Server;
class Query;
class QueryImpl; 

/**
 * A DBSession is an object representing a connection on which a series 
 * of queries can be performed.  A database session is much like a session
 * at a command prompt in an operating system.  Within a session, one query
 * can be sent at a time and nothing else can be done in that session until
 * the results of the query are returned.  A session may be thought of as a
 * logical connection to the database.
 *
 * Do not create DBSession objects directly.  Use the GetSession method on
 * DBServer instead.  Be sure to call DBServer::ReleaseSession() when you
 * are done with the session, so that others may use it.
 *
 * Note that DBSession objects can't be copied.  Database libraries do not
 * allow a session to be cloned, so the semantics of copying a session object
 * are not clear.  Therefore, it is not allowed.
 *
 * TODO:  Consider making an autoptr-like object to hold sessions.  It would
 * automatically call ReleaseSession() when it goes out of scope.
 */
class Session
{
    bool inUse;

	// following are disallowed:
	Session(const Session& rhs);        
	Session &operator=(const Session& rhs);

protected:
	bool connected;

private:
	Query *lastQuery;
	std::string lastQueryStatement;
	int m_execCount;
	int m_fetchCount;
	bool m_fatalOnError;

public:
	Session(); // TODO:  should be private
	virtual ~Session();

	// functions used by other SQLClasses
	void queryDone();

/** [Function used by other classes in the database library.]  Establish connection to a database server.
 */
	virtual bool connect() =0;

/** [Function used by other classes in the database library.]  Drop connection to a database server.
 */
	virtual bool disconnect() =0;

/** [Function used by other classes in the database library.]  Reset the session to its default status
	(e.g. before returning it to the session pool).  Undo any special options the user may have set, 
	such as setting manual-commit mode.

	If this returns false, an error occurred trying to restore the state.  The best thing to do is 
	probably to drop the session and establish a new one.
 */
	virtual bool reset() =0;

	bool exec(Query *query);

/**	Set the session to auto-commit or manual-commit mode.

	If in auto-commit mode, every query set to Exec() is automatically
	commited immediately.  If in manual commit mode, the changes made
	by queries passed to Exec() are considered to be part of a transaction.
	They are remembered by the database, but they don't take effect
	immediately.  When CommitTransaction() is called, the changes all
	take effect	simultaneously.

	(Warning:  switching from manual-commit mode to auto-commit mode
	automatically commits any open transactions.)

	@param autocommit TRUE turns on auto-commit mode (the default), 
	FALSE turns on manual-commit mode
 */
	virtual bool setAutoCommitMode(bool autocommit) =0;

/** Set the session to FATAL on a database error, rather than returning
	"false" to the caller.
 */
	void setFatalOnError(bool fatalOnError);
	bool getFatalOnError() const;
	   
/** Commit a transaction.
	Everything done within the transaction will simultaneously take effect.
 */
	virtual bool commitTransaction() =0;

/** Rollback a transaction
	Evertyhing done within the transaction will be undone.
 */
	virtual bool rollbackTransaction() =0;

/** Add to the count of the number of fetches and executes.
	
	Each query adds to this count as it finishes.
 */
	void addQueryCount(int execCount, int fetchCount);

/** Update the total count of fetches and executes in the Server object
 */
	void updateServerQueryCount();

/** Set/Get the last query statement that was run; useful for debugging purposes
 */
	void setLastQueryStatement(std::string const & queryStatement);
	std::string getLastQueryStatement() const;
	
	virtual QueryImpl *createQueryImpl(Query *owner) const =0;
};

}

// ======================================================================

#endif
