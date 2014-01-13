// ======================================================================
//
// TaskGetCharactersForDelete.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskGetCharactersForDelete_H
#define INCLUDED_TaskGetCharactersForDelete_H

// ======================================================================

#include "TaskGetAvatarList.h"

// ======================================================================

class TaskGetCharactersForDelete : public TaskGetAvatarList
{
  public:
	explicit TaskGetCharactersForDelete  (StationId stationId, int clusterGroupId);
	virtual ~TaskGetCharactersForDelete  ();
	
  public:
	virtual void onComplete  ();
};

// ======================================================================

#endif
