// ======================================================================
//
// TaskSaveSnapshot.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskSaveSnapshot_H
#define INCLUDED_TaskSaveSnapshot_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"

class Snapshot;

// ======================================================================

class TaskSaveSnapshot : public DB::TaskRequest
{
  public:
	TaskSaveSnapshot(Snapshot *snapshot);
	bool process(DB::Session *session);
	void onComplete();
	
  private:
	Snapshot *m_snapshot;
};

// ======================================================================

#endif
