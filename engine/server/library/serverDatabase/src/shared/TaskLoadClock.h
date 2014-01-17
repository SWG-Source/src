// ======================================================================
//
// TaskLoadClock.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskLoadClock_H
#define INCLUDED_TaskLoadClock_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

/**
 * Sets the clock based on the last timestamp saved in the database.
 */
class TaskLoadClock : public DB::TaskRequest
{
  public:
	TaskLoadClock();
	
	virtual bool process(DB::Session *session);
	virtual void onComplete();

  private:
	int m_timestamp;
};

#endif
