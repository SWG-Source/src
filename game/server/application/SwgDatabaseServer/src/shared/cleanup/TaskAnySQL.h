// ======================================================================
//
// TaskAnySQL.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskAnySQL_H
#define INCLUDED_TaskAnySQL_H

// ======================================================================

#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include <string>

// ======================================================================

/**
 * Run arbitrary cleanup SQL (doesn't support any input or output) 
 */
class TaskAnySQL : public DB::TaskRequest
{
  public:
			TaskAnySQL( const char* pSQL ) : m_sSQL(pSQL) {}
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  private:
	class AnyQuery : public DB::Query
	{
	  public:
		AnyQuery( const char* pSQL );
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private:
		AnyQuery(const AnyQuery&);
		AnyQuery& operator=(const AnyQuery&);


		std::string m_sSQL;
	};
	std::string m_sSQL;
};

// ======================================================================

#endif
