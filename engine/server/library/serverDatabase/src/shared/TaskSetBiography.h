// ======================================================================
// 
// TaskSetBiography.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskSetBiography_H
#define INCLUDED_TaskSetBiography_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class TaskSetBiography : public DB::TaskRequest
{
  public:
	TaskSetBiography(const NetworkId &owner, const Unicode::String &bio);
	virtual ~TaskSetBiography();

	virtual bool process(DB::Session *session);
	virtual void onComplete();

  private:
	NetworkId m_owner;
	Unicode::String *m_bio;
};

// ======================================================================

#endif
