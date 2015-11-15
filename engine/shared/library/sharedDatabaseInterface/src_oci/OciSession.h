// ======================================================================
//
// OCISession.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_OCISession_H
#define INCLUDED_OCISession_H

// ======================================================================

#include "sharedDatabaseInterface/DbSession.h"

struct OCIEnv;
struct OCIError;
struct OCIServer;
struct OCISession;
struct OCISvcCtx;

namespace DB {
	class OCIServer; // not to be confused with ::OCIServer
	
	class OCISession : public Session
	{
		OCIServer *m_server;

		::OCIEnv *envhp;
		::OCIError *errhp;
		::OCIServer *srvhp;
		::OCISession *sesp;
		::OCISvcCtx *svchp;
		
		bool autoCommitMode;
		time_t  m_resetTime;
		bool m_okToFetch;
	
			// following are disallowed:
		OCISession(const OCISession& rhs);        
		OCISession &operator=(const OCISession& rhs);

	  public:
		OCISession(OCIServer *server);
		virtual ~OCISession();

		virtual bool connect();
		virtual bool disconnect();
		virtual bool reset();
		virtual bool setAutoCommitMode(bool autocommit);
		virtual bool commitTransaction();
		virtual bool rollbackTransaction();
		virtual QueryImpl *createQueryImpl(Query *owner) const;
		void setOkToFetch();
		bool isOkToFetch() const;
		
        // OCI requires sharing so many handles at different levels that it's easiest to just make everybody friends.
		friend class OCIQueryImpl;
		friend class OCIServer;
		friend class BindableVarray;
		friend class BindableVarrayNumber;
		friend class BindableVarrayString;
	};

}

// ======================================================================

inline bool DB::OCISession::isOkToFetch() const
{
	return m_okToFetch;
}

// ======================================================================

#endif

