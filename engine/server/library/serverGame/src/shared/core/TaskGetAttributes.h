// ======================================================================
//
// TaskGetAttributes.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskGetAttributes_H
#define INCLUDED_TaskGetAttributes_H

// ======================================================================

#include "serverGame/NonCriticalTaskQueue.h"
#include "sharedFoundation/NetworkId.h"

// ----------------------------------------------------------------------

/**
 * Get the attributes for an object and send them to the specified client.
 */
class TaskGetAttributes : public NonCriticalTaskQueue::TaskRequest
{
  public:
	TaskGetAttributes(NetworkId const & playerId, NetworkId const & target, int clientRevision);
	TaskGetAttributes(NetworkId const & playerId, std::string const & staticItemName, int clientRevision);
	virtual bool run();
	
  private:
	NetworkId m_playerId;
	NetworkId m_targetId;
	std::string m_staticItemName;
	int m_clientRevision;
};

// ======================================================================

#endif
