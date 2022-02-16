// ======================================================================
//
// TaskMarketAttributesCleanup.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskMarketAttributesCleanup_H
#define INCLUDED_TaskMarketAttributesCleanup_H

// ======================================================================

#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedDatabaseInterface/Bindable.h"
#include <time.h>

// ======================================================================

/**
 * Runs scripts to do regular daily cleanup in the database.
 */
class TaskMarketAttributesCleanup : public DB::TaskRequest
{
  public:
			    TaskMarketAttributesCleanup( time_t tSeconds ) { m_tSeconds = tSeconds; }

	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  private:
	time_t	            m_tSeconds;

	class DataCleanupQuery : public DB::Query
	{
	  public:
		DataCleanupQuery();
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		DB::BindableLong result;

	  private:
		DataCleanupQuery(const DataCleanupQuery&);
		DataCleanupQuery& operator=(const DataCleanupQuery&);

	};
};

// ======================================================================

#endif
