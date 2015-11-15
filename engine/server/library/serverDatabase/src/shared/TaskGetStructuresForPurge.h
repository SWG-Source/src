// ======================================================================
// 
// TaskGetStructuresForPurge.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskGetStructuresForPurge_H
#define INCLUDED_TaskGetStructuresForPurge_H

// ======================================================================

#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/StationId.h"
#include "Unicode.h"
#include <vector>

// ======================================================================

class TaskGetStructuresForPurge : public DB::TaskRequest
{
  public:
	explicit TaskGetStructuresForPurge(StationId account, bool warnOnly);
	virtual ~TaskGetStructuresForPurge();

	virtual bool process(DB::Session *session);
	virtual void onComplete();

  private:
	StationId m_account;
	bool m_warnOnly;
	std::vector<std::pair<NetworkId, NetworkId> > m_structures;
	std::vector<std::pair<NetworkId, std::pair<NetworkId, Unicode::String> > > m_vendors;
};

// ======================================================================

#endif
