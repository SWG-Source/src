// ======================================================================
//
// TaskGetAccountForPurge.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskGetAccountForPurge_H
#define INCLUDED_TaskGetAccountForPurge_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"

class TaskGetAccountForPurge : public DB::TaskRequest
{
  public:
	explicit TaskGetAccountForPurge(int purgePhase);
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:	
	int m_purgePhase;
	StationId m_account;
};

// ======================================================================

#endif
