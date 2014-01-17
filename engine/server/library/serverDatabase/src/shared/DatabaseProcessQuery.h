// ======================================================================
//
// DatabaseProcessQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DatabaseProcessQuery_H
#define INCLUDED_DatabaseProcessQuery_H

// ======================================================================

#include "sharedDatabaseInterface/DbModeQuery.h"

// ======================================================================

/**
 * ModeQuery specialized for how DatabaseProcess uses them.
 *
 * The main difference is that this query uses Oracle refcursor mode
 * when appropriate.
 */

class DatabaseProcessQuery : public DB::ModeQuery
{
  public:
	explicit DatabaseProcessQuery(DB::Row *derivedRow);
	
  protected:
	virtual QueryMode getExecutionMode() const;

  private:
	DatabaseProcessQuery(const DatabaseProcessQuery &); // disable
	DatabaseProcessQuery &operator=(DatabaseProcessQuery &); //disable
};

// ======================================================================

#endif
