//======================================================================
//
// MessageQueueGenericString.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueGenericString_H
#define INCLUDED_MessageQueueGenericString_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

/**
* A generic message queue data that contains a simple payload of one String.
* The meaning of the String is context and message dependant.
*
*/

class MessageQueueGenericString : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	explicit                MessageQueueGenericString (const Unicode::String & string, uint8 sequenceId);
	virtual                ~MessageQueueGenericString();

	uint8                       getSequenceId ()    const;
	const Unicode::String &     getString(void) const;

private:

	MessageQueueGenericString&  operator= (const MessageQueueGenericString & source);
	                     MessageQueueGenericString(const MessageQueueGenericString & source);

	Unicode::String m_string;
	uint8           m_sequenceId;
};


//-----------------------------------------------------------------------

inline MessageQueueGenericString::MessageQueueGenericString (const Unicode::String & string, uint8 sequenceId) :
	m_string(string),
	m_sequenceId (sequenceId)
{
}

//-----------------------------------------------------------------------

inline const Unicode::String & MessageQueueGenericString::getString(void) const
{
	return m_string;
}

//-----------------------------------------------------------------------

inline uint8 MessageQueueGenericString::getSequenceId ()    const
{
	return m_sequenceId;
}


//-----------------------------------------------------------------------

#endif	// _MessageQueueGenericString_H

