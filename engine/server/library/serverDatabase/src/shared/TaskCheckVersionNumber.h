// ======================================================================
//
// TaskCheckVersionNumber.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskCheckVersionNumber_H
#define INCLUDED_TaskCheckVersionNumber_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"

// ======================================================================

class TaskCheckVersionNumber : public DB::TaskRequest
{
  public:
	TaskCheckVersionNumber(int codeVersion, bool fatalOnMismatch);
	virtual ~TaskCheckVersionNumber();

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
