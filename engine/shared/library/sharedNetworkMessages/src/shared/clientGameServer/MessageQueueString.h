//======================================================================
//
// MessageQueueString.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueString_H
#define INCLUDED_MessageQueueString_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

/**
* A generic message queue data that contains a simple payload of one String.
* The meaning of the String is context and message dependant.
*
*/

class MessageQueueString : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	explicit                MessageQueueString (const std::string & str);
	virtual                ~MessageQueueString();

	const std::string &     getString(void) const;

private:

	MessageQueueString&  operator= (const MessageQueueString & source);
	                     MessageQueueString(const MessageQueueString & source);

	std::string m_string;
};

//-----------------------------------------------------------------------

inline const std::string & MessageQueueString::getString(void) const
{
	return m_string;
}

//-----------------------------------------------------------------------

#endif	// _MessageQueueString_H

