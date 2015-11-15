// ======================================================================
//
// TaskDailyCleanup.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskDailyCleanup_H
#define INCLUDED_TaskDailyCleanup_H

// ======================================================================

#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

/**
 * Runs scripts to do regular daily cleanup in the database.
 */
class TaskDailyCleanup : public DB::TaskRequest
{
  public:
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  private:
	class DataCleanupQuery : public DB::Query
	{
	  public:
		DataCleanupQuery();
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private:
		DataCleanupQuery(const DataCleanupQuery&);
		DataCleanupQuery& operator=(const DataCleanupQuery&);
	};
};

// ======================================================================

#endif
