// ======================================================================
//
// TaskCheckGoldVersionNumber.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskCheckGoldVersionNumber_H
#define INCLUDED_TaskCheckGoldVersionNumber_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskCheckGoldVersionNumber : public DB::TaskRequest
{
  public:
	TaskCheckGoldVersionNumber(int codeVersion, bool fatalOnMismatch);
	virtual ~TaskCheckGoldVersionNumber();

	virtual bool process(DB::Session *session);
	virtual void onComplete();

  private:
	int m_codeVersion;
	int m_databaseVersion;
	int m_minVersion;
	bool m_fatalOnMismatch;
};

// ======================================================================

#endif
