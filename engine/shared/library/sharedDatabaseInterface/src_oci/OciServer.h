// ======================================================================
//
// OCIServer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_OCIServer_H
#define INCLUDED_OCIServer_H

#include "sharedDatabaseInterface/DbServer.h"

// ======================================================================

struct OCIEnv;
struct OCIError;
struct OCIServer;
struct OCISvcCtx;

namespace DB {
	class OCISession;
	
	class OCIServer : public Server
	{
		public:

		OCIServer(const char *_dsn, const char *_uid, const char *_pwd, bool useMemoryManager);
		virtual ~OCIServer();

		virtual Session *createSession();

		/**	Check the results from an ODBC call to identify whether the call succeeded.  Returns TRUE if everything is OK, i.e.
			the call returned SQL_SUCCESS, or it returned SQL_SUCCESS_WITH_INFO and the additional information does not indicate 
			a problem.
		*/
		//	static bool checkError(int rc, SQLSMALLINT handleType, SQLHANDLE handle);

		static bool checkerr(OCISession const & session, int status);
		
	  private:
				
		// OCI requires sharing so many handles at different levels that it's easiest to just make everybody friends.
		friend class OCISession;
		friend class OCIQueryImpl;
		OCIServer(const OCIServer &);
		OCIServer & operator = (const OCIServer &);
	};

}

// ======================================================================

#endif
