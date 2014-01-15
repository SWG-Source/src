// ======================================================================
//
// MessageQueueSetState.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/MessageQueueSetState.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueSetState, CM_setState);

//===================================================================



MessageQueueSetState::MessageQueueSetState(int state, bool value) :
	MessageQueue::Data(),
	m_state(state),
	m_value(value)
{
}

// ----------------------------------------------------------------------

MessageQueueSetState::MessageQueueSetState(
	MessageQueueSetState const &source) :
	MessageQueue::Data(),
	m_state(source.m_state),
	m_value(source.m_value)
{
}	

// ----------------------------------------------------------------------

int MessageQueueSetState::getState() const
{
	return m_state;
}

// ----------------------------------------------------------------------

bool MessageQueueSetState::getValue() const
{
	return m_value;
}

//-----------------------------------------------------------------------

void MessageQueueSetState::pack(const MessageQueue::Data* data, Archive::ByteStream & target)
{
	const MessageQueueSetState * const msg = safe_cast<const MessageQueueSetState *>(data);
	if (msg)
	{
		Archive::put(target, msg->getState());
		Archive::put(target, msg->getValue());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data * MessageQueueSetState::unpack(Archive::ReadIterator & source)
{
	int state;
	bool value;
	Archive::get(source, state);
	Archive::get(source, value);
	return new MessageQueueSetState(state, value);
}

