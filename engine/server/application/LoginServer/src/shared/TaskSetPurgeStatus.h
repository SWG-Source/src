// ======================================================================
//
// TaskSetPurgeStatus.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskSetPurgeStatus_H
#define INCLUDED_TaskSetPurgeStatus_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"

class TaskSetPurgeStatus : public DB::TaskRequest
{
  public:
	explicit TaskSetPurgeStatus(StationId account, int purgePhase);
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:	
	StationId m_account;
	int m_purgePhase;
};

// ======================================================================

#endif
