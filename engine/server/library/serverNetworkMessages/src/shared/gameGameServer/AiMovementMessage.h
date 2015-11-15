// ======================================================================
//
// AiMovementMessage.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AiMovementMessage_H
#define INCLUDED_AiMovementMessage_H

//-----------------------------------------------------------------------

#include "serverGame/AiMovementBase.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class AiMovementMessage;

namespace Archive
{
	// implemented in serverGame
	void get(ReadIterator & source, AiMovementMessage & target);
	void put(ByteStream & target, const AiMovementMessage & source);
}

// ======================================================================

/**
 * Used to tell proxies an authoritative object's current behavior.
 * We don't use auto-syncronization since the data is stored on the 
 * controller.
 */
class AiMovementMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

	friend void Archive::get(Archive::ReadIterator & source, AiMovementMessage & target);

public:
	         AiMovementMessage();
	         AiMovementMessage(const NetworkId & objectId, AiMovementBasePtr behavior);
	         AiMovementMessage(const NetworkId & objectId);
	virtual ~AiMovementMessage();

	const NetworkId & getObjectId() const;
	AiMovementType getMovementType() const;
	AiMovementBasePtr getMovement() const;

private:

	NetworkId         m_objectId;
	AiMovementBasePtr m_movement;
};

//-----------------------------------------------------------------------

inline const NetworkId & AiMovementMessage::getObjectId() const
{
	return m_objectId;
}

//-----------------------------------------------------------------------

inline AiMovementBasePtr AiMovementMessage::getMovement() const
{
	return m_movement;
}

// ======================================================================

#endif	// INCLUDED_AiMovementMessage_H
