// ======================================================================
//
// OCISession.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "OciSession.h"

#include "OciQueryImplementation.h"
#include "OciServer.h"
#include "sharedDatabaseInterface/DbException.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedSynchronization/RecursiveMutex.h"

#include <oci.h>
#include <time.h>

// ======================================================================

namespace DB
{
	namespace OCISessionNamespace
	{
		RecursiveMutex connectLock;
	}
}

using namespace DB::OCISessionNamespace;

// ======================================================================

static dvoid *mallocHook(dvoid *, size_t size)
{
	return reinterpret_cast<dvoid *>(new char[size]);
}

// ----------------------------------------------------------------------

static dvoid *reallocHook(dvoid *, dvoid *memptr, size_t newsize)
{
	return reinterpret_cast<dvoid *>(realloc(memptr, newsize));
}

// ----------------------------------------------------------------------

static void freeHook(dvoid *, dvoid *memptr)
{
	if (memptr)
		delete [] reinterpret_cast<char *>(memptr);
}

// ======================================================================

DB::OCISession::OCISession(DB::OCIServer *server) :
		m_server(server),
		envhp(nullptr),
		errhp(nullptr),
		srvhp(nullptr),
		sesp(nullptr),
		svchp(nullptr),
		autoCommitMode(true),
		m_resetTime(server->getReconnectTime()==0 ?	0 : time(0) + server->getReconnectTime()),
		m_okToFetch(true)
{
}

DB::OCISession::~OCISession()
{
	if (connected)
		disconnect();
}

bool DB::OCISession::connect()
{
	connectLock.enter();
	
	// Create OCI environment.  We're creating one environment per session,
	// so that each session can be used by a different thread concurrently.

	sword result=OCI_ERROR;
		result = OCIEnvCreate(&envhp, // OCIEnv        **envhpp,
							  OCI_THREADED | OCI_OBJECT, //ub4           mode, //TODO:  do we have to use threaded mode?
							  0, // CONST dvoid   *ctxp,
							  0, // CONST dvoid   *(*malocfp)
							  //           (dvoid *ctxp,
							  //               size_t size),
							  0, // CONST dvoid   *(*ralocfp)
							  //            (dvoid *ctxp,
							  //             dvoid *memptr,
							  //             size_t newsize),
							  0, // CONST void    (*mfreefp)
							  //            (dvoid *ctxp,
							  //             dvoid *memptr))
							  0, //size_t    xtramemsz,
							  0 ); //dvoid     **usrmempp );
		
	FATAL(result != OCI_SUCCESS,("OciEnvCreate failed with error code %hd",result));
	
	// Create error handle.
	OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &errhp, OCI_HTYPE_ERROR,
						   (size_t) 0, (dvoid **) 0);
	
	// Create handle to the server and establish connection
	OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &srvhp, OCI_HTYPE_SERVER,
						   (size_t) 0, (dvoid **) 0);

	int retryCount = 0;
	connected = false;
	while (!connected && retryCount < 5)
	{
		LOG("DatabaseConnect", ("calling OCIServerAttach() for OCISession=[%p] with dsn=[%s], uid=[%s], pwd=[%s]", this, m_server->getDSN(), m_server->uid, m_server->pwd));

		connected = DB::OCIServer::checkerr(*this,
									OCIServerAttach( srvhp, errhp, reinterpret_cast<OraText*>(const_cast<char*>(m_server->getDSN())), strlen(m_server->getDSN()), 0));
			
		if (! connected)
		{
			++retryCount;
			DEBUG_REPORT_LOG(retryCount<5,("Retrying database connection\n"));
			Os::sleep(1000);
		}	
	}
	if (!connected)
	{
		LOG("DatabaseError", ("Database error, Failed to connect to database after 5 tries"));
		disconnect(); // cleanup
		connectLock.leave();
		return false;
	}

	// Create a session, set userid & password
	OCIHandleAlloc(envhp, (void**) &sesp,
				   (ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0);

	OCIAttrSet((dvoid *) sesp, (ub4) OCI_HTYPE_SESSION,
			   (dvoid *) m_server->uid, strlen(m_server->uid),
			   (ub4) OCI_ATTR_USERNAME, errhp);

	OCIAttrSet((dvoid *) sesp, (ub4) OCI_HTYPE_SESSION,
			   (dvoid *) m_server->pwd, strlen(m_server->pwd),
			   (ub4) OCI_ATTR_PASSWORD, errhp);

	// create service context (Wtf is a service context?  Don't know, but you gotta have one.)
	OCIHandleAlloc( envhp, (dvoid **) &svchp, OCI_HTYPE_SVCCTX,
						   (size_t) 0, (dvoid **) 0);

	// Service context has pointers to Server and Session
	OCIAttrSet( (dvoid *) svchp, OCI_HTYPE_SVCCTX, srvhp,
					   (ub4) 0, OCI_ATTR_SERVER, errhp);

	OCIAttrSet((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX,
			   (dvoid *) sesp, (ub4) 0,
			   (ub4) OCI_ATTR_SESSION, errhp);

	// Establish session (i.e. check password)
	if (DB::OCIServer::checkerr(*this, OCISessionBegin ( svchp, errhp, sesp, OCI_CRED_RDBMS, (ub4) OCI_DEFAULT)))
	{
		connected=true;
	}
	else
	{
		disconnect(); // cleanup
		connected=false;
	}

	connectLock.leave();
	return connected;
}

bool DB::OCISession::disconnect()
{
	connectLock.enter();

	bool success = true;

	connected=false;

	if (srvhp && errhp && !DB::OCIServer::checkerr(*this,OCIServerDetach( srvhp, errhp, OCI_DEFAULT)))
	{
		LOG("DatabaseError", ("OCIDetach returned an error."));
		success = false;
	}

	if (svchp && (OCIHandleFree(svchp,OCI_HTYPE_SVCCTX) != OCI_SUCCESS))
	{
		LOG("DatabaseError", ("OCIHandleFree OCI_HTYPE_SVCCTX returned an error."));
		success = false;
	}

	if (sesp && (OCIHandleFree(sesp,OCI_HTYPE_SESSION) != OCI_SUCCESS))
	{
		LOG("DatabaseError", ("OCIHandleFree OCI_HTYPE_SESSION returned an error."));
		success = false;
	}

	if (srvhp && (OCIHandleFree(srvhp,OCI_HTYPE_SERVER) != OCI_SUCCESS))
	{
		LOG("DatabaseError", ("OCIHandleFree OCI_HTYPE_SERVER returned an error."));
		success = false;
	}

	if (errhp && (OCIHandleFree(errhp,OCI_HTYPE_ERROR) != OCI_SUCCESS))
	{
		LOG("DatabaseError", ("OCIHandleFree OCI_HTYPE_ERROR returned an error."));
		success = false;
	}

	if (envhp && (OCIHandleFree(envhp,OCI_HTYPE_ENV) != OCI_SUCCESS))
	{
		LOG("DatabaseError", ("OCIHandleFree OCI_HTYPE_ENV returned an error."));
		success = false;
	}

	svchp = nullptr;
	sesp = nullptr;
	srvhp = nullptr;
	errhp = nullptr;
	envhp = nullptr;

	connectLock.leave();
	return success;
}

bool DB::OCISession::setAutoCommitMode(bool autocommit)
{
	autoCommitMode=autocommit;
	return true;
}

bool DB::OCISession::commitTransaction()
{
	m_okToFetch = false;
	return m_server->checkerr(*this, OCITransCommit(svchp, errhp, 0));
}

bool DB::OCISession::rollbackTransaction()
{
	return m_server->checkerr(*this, OCITransRollback(svchp, errhp, 0));
}

bool DB::OCISession::reset()
{
	autoCommitMode = true;
	if (m_resetTime !=0 && time(0) > m_resetTime)
	{
		LOG("Reconnections",("Reconnecting"));
		commitTransaction(); // just to be safe -- there shouldn't be an oustanding transaction now anyway
		disconnect();
		connect();
		m_resetTime = time(0) + m_server->getReconnectTime();
	}
	return true;
}

DB::QueryImpl *DB::OCISession::createQueryImpl(Query *owner) const
{
	return new DB::OCIQueryImpl(owner, autoCommitMode);
}

void DB::OCISession::setOkToFetch()
{
	m_okToFetch = true;
}

// ======================================================================
