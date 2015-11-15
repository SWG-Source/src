// ======================================================================
//
// TaskFixBadCells.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskFixBadCells_H
#define INCLUDED_TaskFixBadCells_H

// ======================================================================

#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

/**
 * Fixes cells that have the wrong load_with.
 */
class TaskFixBadCells : public DB::TaskRequest
{
  public:
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  private:
	class FixBadCellsQuery : public DB::Query
	{
	  public:
		FixBadCellsQuery();
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private:
		FixBadCellsQuery(const FixBadCellsQuery&);
		FixBadCellsQuery& operator=(const FixBadCellsQuery&);
	};
};

// ======================================================================

#endif
