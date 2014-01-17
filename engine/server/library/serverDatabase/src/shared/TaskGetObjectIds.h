// ======================================================================
// 
// TaskGetObjectIds.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskGetObjectIds_H
#define INCLUDED_TaskGetObjectIds_H

// ======================================================================

#include <vector>

#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class TaskGetObjectIds : public DB::TaskRequest
{
  public:
	TaskGetObjectIds(int processId, int howMany, bool logRequest);
	virtual ~TaskGetObjectIds() {}

	virtual bool process(DB::Session *session);
	virtual void onComplete();

  private:
	int m_processId, m_howMany;
	bool m_logRequest;

	class ObjectIdBlock
	{
	  public:
		NetworkId m_startId;
		NetworkId m_endId;
		ObjectIdBlock(const NetworkId &startId,const NetworkId &endId);
	};

	typedef std::vector<ObjectIdBlock> BlockListType;
	BlockListType m_blockList;
};

// ----------------------------------------------------------------------

inline TaskGetObjectIds::ObjectIdBlock::ObjectIdBlock(const NetworkId &startId,const NetworkId &endId) :
	m_startId(startId),
	m_endId(endId)
{
}

// ======================================================================

#endif
