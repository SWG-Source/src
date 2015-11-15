// ======================================================================
// 
// TaskGetBiography.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskGetBiography_H
#define INCLUDED_TaskGetBiography_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class TaskGetBiography : public DB::TaskRequest
{
  public:
	TaskGetBiography(const NetworkId &owner, uint32 requestingProcess);
	virtual ~TaskGetBiography();

	virtual bool process(DB::Session *session);
	virtual void onComplete();

  private:
	NetworkId m_owner;
	uint32 m_requestingProcess;
	Unicode::String *m_bio;
};

// ======================================================================

#endif
