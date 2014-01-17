// ======================================================================
//
// TaskLoadSnapshots.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskLoadSnapshots_H
#define INCLUDED_TaskLoadSnapshots_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"

class LoaderSnapshotGroup;

// ======================================================================

class TaskLoadSnapshots : public DB::TaskRequest
{
  public:
	TaskLoadSnapshots(LoaderSnapshotGroup *snap);
	virtual ~TaskLoadSnapshots();
	
	virtual bool process(DB::Session *session);
	virtual void onComplete();

  private:
	LoaderSnapshotGroup *m_snapshotGroup;
};


// ======================================================================
#endif
