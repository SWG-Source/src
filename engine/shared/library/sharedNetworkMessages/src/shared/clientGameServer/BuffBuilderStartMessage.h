// ======================================================================
//
// BuffBuilderStartMessage.h
//
// Copyright 2006 Sony Online Entertainment
//
// ======================================================================

#ifndef	_BuffBuilderStartMessage_H
#define	_BuffBuilderStartMessage_H

//-----------------------------------------------------------------------
#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

//-----------------------------------------------------------------------

class BuffBuilderStartMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	BuffBuilderStartMessage(NetworkId const & bufferId, NetworkId const & recipientId );

public:
	NetworkId const & getBufferId() const;
	NetworkId const & getRecipientId() const;

private:
//disabled
	BuffBuilderStartMessage(BuffBuilderStartMessage const & source);

private:
	NetworkId m_bufferId;
	NetworkId m_recipientId;
};

// ----------------------------------------------------------------------

inline NetworkId const & BuffBuilderStartMessage::getBufferId() const
{
	return m_bufferId;
}

// ----------------------------------------------------------------------

inline NetworkId const & BuffBuilderStartMessage::getRecipientId() const
{
	return m_recipientId;
}

// ----------------------------------------------------------------------

#endif 

