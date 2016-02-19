// ======================================================================
//
// TaskUpdatePurgeAccountList.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskUpdatePurgeAccountList_H
#define INCLUDED_TaskUpdatePurgeAccountList_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"

class TaskUpdatePurgeAccountList : public DB::TaskRequest
{
  public:
	TaskUpdatePurgeAccountList();
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();
};

// ======================================================================

#endif
