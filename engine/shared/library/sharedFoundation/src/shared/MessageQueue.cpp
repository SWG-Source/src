// ======================================================================
//
// MessageQueue.cpp
// Portions copyright 1998 Bootprint Entertainment.
// Portions copyright 2000-2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/MessageQueue.h"

#include <algorithm>
#include <vector>

// ======================================================================
// PUBLIC MessageQueue::Data
// ======================================================================

MessageQueue::Data::Data()
{
}

// ----------------------------------------------------------------------

MessageQueue::Data::~Data()
{
}

// ======================================================================
// PUBLIC MessageQueue::Notification
// ======================================================================

MessageQueue::Notification::Notification()
{
}

// ----------------------------------------------------------------------

MessageQueue::Notification::~Notification()
{
}

// ======================================================================
// PRIVATE MessageQueue::Message
// ======================================================================

MessageQueue::Message::Message() :
	m_message(0),
	m_value(0),
	m_data(0),
	m_flags(0)
{
}

//----------------------------------------------------------------------

MessageQueue::Message::~Message()
{
	m_data = 0;
}

// ======================================================================
// PUBLIC MessageQueue
// ======================================================================

MessageQueue::MessageQueue(const int initialSize) :	
	m_messageQueue1(new MessageList),
	m_messageQueue2(new MessageList),
	m_messageQueueRead(m_messageQueue1),
	m_messageQueueWrite(m_messageQueue2),
	m_notification(0)
{
	DEBUG_WARNING(initialSize < 0, ("initialSize < 0"));
	m_messageQueue1->reserve(static_cast<uint>(std::max(initialSize, 0)));
	m_messageQueue2->reserve(static_cast<uint>(std::max(initialSize, 0)));
}

// ----------------------------------------------------------------------
/**
 * Destroy the MessageQueue.
 */

MessageQueue::~MessageQueue()
{
	clearDataFromMessageList(*m_messageQueue1, true);
	clearDataFromMessageList(*m_messageQueue2, true);
	delete m_messageQueue1;
	delete m_messageQueue2;
	m_messageQueueRead = 0;
	m_messageQueueWrite = 0;
	m_notification = 0;
}

// ----------------------------------------------------------------------

void MessageQueue::clearDataFromMessageList(MessageList &messageList, bool destructor)
{
	UNREF(destructor);
	MessageList::iterator iEnd = messageList.end();
	for (MessageList::iterator i = messageList.begin(); i != iEnd; ++i)
	{
		Message & message = *i;
		if (message.m_data)
		{
			DEBUG_WARNING(!destructor, ("clearing message data from beginFrame"));
			delete message.m_data;
			message.m_data = nullptr;
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Get a message from the MessageQueue.
 * 
 * The message index ranges from 0 to getNumberOfMessages()-1.
 * 
 * @param index  [In ] The index of the message to retrieve
 * @param message  [Out] The message received
 * @param value  [Out] Argument for the message
 * @param flags  [Out] User flags for this message
 * @see MessageQueue::getNumberOfMessages()
 */

void MessageQueue::getMessage(const int index, int *const message, float *const value, uint32 *const flags) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfMessages());
	NOT_NULL(message);
	NOT_NULL(value);
	NOT_NULL(m_messageQueueRead);

	const Message &currentMessage = (*m_messageQueueRead)[static_cast<uint>(index)];
	*message = currentMessage.m_message;
	*value   = currentMessage.m_value;
	if (flags)
		*flags = currentMessage.m_flags;
}

// ----------------------------------------------------------------------
/**
 * Get a message from the MessageQueue.
 * 
 * The message index ranges from 0 to getNumberOfMessages()-1.
 * 
 * @param index  [In ] The index of the message to retrieve
 * @param message  [Out] The message received
 * @param value  [Out] Argument for the message
 * @param data  [Out] Data for the message
 * @param flags  [Out] User flags for this message
 * @see MessageQueue::getNumberOfMessages()
 */

void MessageQueue::getMessage(const int index, int *const message, float *const value, Data **const data, uint32 *const flags) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfMessages());
	NOT_NULL(message);
	NOT_NULL(value);
	NOT_NULL(data);
	NOT_NULL(m_messageQueueRead);

	const Message & currentMessage = (*m_messageQueueRead)[static_cast<uint>(index)];
	*message = currentMessage.m_message;
	*value   = currentMessage.m_value;
	*data    = currentMessage.m_data;

	if (flags)
		*flags = currentMessage.m_flags;
}

// ----------------------------------------------------------------------
/**
 * Sets message flags in the MessageQueue.
 * 
 * The message index ranges from 0 to getNumberOfMessages()-1.
 * 
 * @see MessageQueue::getNumberOfMessages()
 */

void MessageQueue::setMessageFlags(const int index, const uint32 flags)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfMessages());

	(*m_messageQueueRead)[static_cast<uint>(index)].m_flags = flags;
}

// ----------------------------------------------------------------------
/**
 * Clear a message in the MessageQueue.
 * 
 * The message index ranges from 0 to getNumberOfMessages()-1.
 * 
 * @param index  [In] The index of the message to clear
 * @see MessageQueue::getNumberOfMessages()
 */

void MessageQueue::clearMessage(const int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfMessages());

	Message& message = (*m_messageQueueRead)[static_cast<uint>(index)];
	DEBUG_WARNING(message.m_data, ("MessageQueue::clearMessage - clearing message with non-nullptr data"));
	message.m_message = 0;
}

// ----------------------------------------------------------------------
/**
 * Clear a message in the MessageQueue.
 * 
 * The message index ranges from 0 to getNumberOfMessages()-1.
 * 
 * @param index  [In] The index of the message data to clear
 * @see MessageQueue::getNumberOfMessages()
 */

void MessageQueue::clearMessageData(const int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfMessages());

	Message& message = (*m_messageQueueRead)[static_cast<uint>(index)];
	message.m_data = 0;
}

// ----------------------------------------------------------------------
/**
 * Append a new message to the MessageQueue.
 * 
 * The message will not be retrievable until the next call to endFrame().
 * 
 * @param message  [In] Message to add to the MessageQueue
 * @param value  [In] Value to be associated with the message
 * @param data  [In] Data to be associated with the message
 * @param flags  [In] User flags for this message
 */

void MessageQueue::appendMessage(const int message, const float value, Data *const data, const uint32 flags)
{
	if (message == 0)
	{
		WARNING_STRICT_FATAL(true, ("bad message"));
		return;
	}

#ifdef _DEBUG
	//-- check to see if any message with data is already in the list
	if (data)
	{
		for (MessageList::iterator iter = m_messageQueueWrite->begin(); iter != m_messageQueueWrite->end(); ++iter)
			WARNING_STRICT_FATAL(iter->m_data == data, ("message queue data already in message queue"));
	}
#endif

	Message newMessage;
	newMessage.m_message = message;
	newMessage.m_value   = value;
	newMessage.m_data    = data;
	newMessage.m_flags   = flags;
	m_messageQueueWrite->push_back(newMessage);

	if (m_notification)
		m_notification->onChanged();
}

// ----------------------------------------------------------------------
/**
 * Append a new message to the MessageQueue.
 * 
 * The message will not be retrievable until the next call to endFrame().
 * 
 * @param message  [In] Message to add to the MessageQueue
 * @param value  [In] Value to be associated with the message
 * @param flags  [In] User flags for this message
 */

void MessageQueue::appendMessage(const int message, const float value, const uint32 flags)
{
	appendMessage(message, value, 0, flags);
}

// ----------------------------------------------------------------------
// Get the number of messages in the current frame
//
// Return value:
//
//   The number of messages in the current frame for the MessageQueue.
//
// Remarks:
//
//   getMessage() will take as indices the values from 0 the value returned
//   by this routine inclusive.
//
// See also:
//
//   MessageQueue::getMessage()

int MessageQueue::getNumberOfMessages() const
{
	return static_cast<int>(m_messageQueueRead->size());
}

// ----------------------------------------------------------------------

void MessageQueue::beginFrame()
{
	//-- swap read and write
	std::swap (m_messageQueueRead, m_messageQueueWrite);

	//-- clear write
	clearDataFromMessageList(*m_messageQueueWrite, false);
	m_messageQueueWrite->clear ();
}

// ----------------------------------------------------------------------

void MessageQueue::setNotification(Notification* notification)
{
	m_notification = notification;
}

// ======================================================================
