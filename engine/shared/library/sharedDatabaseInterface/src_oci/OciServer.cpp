// ======================================================================
//
// OCIServer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "OciServer.h"

#include "OciSession.h"
#include "sharedDatabaseInterface/DbException.h"
#include "sharedLog/Log.h"

#include <oci.h>

// ======================================================================

DB::OCIServer::OCIServer(const char *_dsn, const char *_uid, const char *_pwd, bool useMemoryManager) : 
		DB::Server(_dsn,_uid,_pwd, useMemoryManager)
{
	

}

// ----------------------------------------------------------------------

// Server destructor
DB::OCIServer::~OCIServer()
{
	disconnect();
}

// ----------------------------------------------------------------------

DB::Session *DB::OCIServer::createSession()
{
	return new DB::OCISession(this);
}

// ----------------------------------------------------------------------

bool DB::OCIServer::checkerr(OCISession const & session, int status)
{
	switch (status)
	{
		case OCI_SUCCESS:
			return true;
		case OCI_SUCCESS_WITH_INFO:
			REPORT_LOG(true,("Error - OCI_SUCCESS_WITH_INFO\n"));
			LOG("DatabaseError", ("Unhandled database error."));
			return false;
		case OCI_NEED_DATA:
			REPORT_LOG(true,("Error - OCI_NEED_DATA\n"));
			LOG("DatabaseError", ("Unhandled database error."));
			return false;
		case OCI_NO_DATA:
			REPORT_LOG(true,("Error - OCI_NODATA\n"));
			LOG("DatabaseError", ("Unhandled database error."));
			return false;
		case OCI_ERROR:
		{
			text errbuf[512];
			sb4 errcode = 0;
			
			OCIErrorGet((dvoid *)(session.errhp), (ub4) 1, (text *) nullptr, &errcode,
							   errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);

			WARNING(true,("Database error: %.*s",512,errbuf));
			LOG("DatabaseError",("Database error: %.*s",512,errbuf));
			FATAL(DB::Server::getFatalOnError() || session.getFatalOnError(),("Database error: %.*s",512,errbuf));
	
			switch ((int) errcode)
			{
				case 1013:
					FATAL(true,("Cancelled by user request (ctrl-c or kill signal).\n"));
					break;
				case 12541:
					REPORT_LOG(true,("Database Error - %.*s\n", 512, errbuf));
					return false;
				default:
					FATAL(true,("Unhandled Database Error - %.*s\n", 512, errbuf));
					break;
			}			

			return false;
		}
		case OCI_INVALID_HANDLE:
			REPORT_LOG(true,("Error - OCI_INVALID_HANDLE\n"));
			LOG("DatabaseError", ("Unhandled database error - OCI_INVALID_HANDLE"));
			return false;
		case OCI_STILL_EXECUTING:
			REPORT_LOG(true,("Error - OCI_STILL_EXECUTE\n"));
			LOG("DatabaseError", ("Unhandled database error."));
			return false;
		case OCI_CONTINUE:
			REPORT_LOG(true,("Error - OCI_CONTINUE\n"));
			LOG("DatabaseError", ("Unhandled database error."));
			return false;
		default:
			REPORT_LOG(true,("Error - unrecognized OCI error state\n"));
			LOG("DatabaseError", ("Unhandled database error."));
			return false;
	}
	return false;
}


// ======================================================================
