//======================================================================
//
// MessageQueueTeleportAck.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueTeleportAck_H
#define INCLUDED_MessageQueueTeleportAck_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

/**
 * A teleport ack message is sent from a client to a game server to
 * acknowledge receipt of a teleport message.
 *
 */

class MessageQueueTeleportAck : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	
	MessageQueueTeleportAck(int sequenceId);
	MessageQueueTeleportAck(const MessageQueueTeleportAck&);

	int getSequenceId() const;

private:

	MessageQueueTeleportAck();
	MessageQueueTeleportAck &operator=(const MessageQueueTeleportAck &);

private:
	int m_sequenceId;
};

//----------------------------------------------------------------------


inline int MessageQueueTeleportAck::getSequenceId() const
{
	return m_sequenceId;
}

//======================================================================

#endif // INCLUDED_MessageQueueTeleportAck_H

