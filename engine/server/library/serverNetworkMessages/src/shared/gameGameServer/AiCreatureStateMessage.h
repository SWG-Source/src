// ======================================================================
//
// AiCreatureStateMessage.h
// copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AiCreatureStateMessage_H
#define INCLUDED_AiCreatureStateMessage_H

#include "serverGame/AiMovementBase.h"
#include "serverNetworkMessages/AiMovementMessage.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedUtility/Location.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class AiCreatureStateMessage;

namespace Archive
{
	// Implemented in AiMovementArchive

	void get(ReadIterator & source, AiCreatureStateMessage & target);
	void put(ByteStream & target, const AiCreatureStateMessage & source);
}

// ----------------------------------------------------------------------
class AiCreatureStateMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

	friend void Archive::get(Archive::ReadIterator & source, AiCreatureStateMessage & target);

public:

	AiCreatureStateMessage();

	NetworkId m_networkId;
	AiMovementMessage m_movement;
};

// ======================================================================

#endif	// INCLUDED_AiCreatureStateMessage_H
