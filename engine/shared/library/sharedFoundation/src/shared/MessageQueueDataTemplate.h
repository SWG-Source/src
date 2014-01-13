//======================================================================
//
// MessageQueueDataTemplate.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueDataTemplate_H
#define INCLUDED_MessageQueueDataTemplate_H

#include "sharedFoundation/MessageQueue.h"

//======================================================================

template <typename T> class MessageQueueDataTemplate : 
public MessageQueue::Data
{
public:

	MessageQueueDataTemplate () :
	MessageQueue::Data (),
	m_data ()
	{
	}

	explicit MessageQueueDataTemplate (const T & t) :
	MessageQueue::Data (),
	m_data (t)
	{
	}

	const T & getData () const
	{
		return m_data;
	}

	T & getData ()
	{
		return m_data;
	}

	void setData (const T & t) const
	{
		m_data = t;
	}

private:

	MessageQueueDataTemplate (const MessageQueueDataTemplate & rhs);
	MessageQueueDataTemplate & operator= (const MessageQueueDataTemplate & rhs);

	T m_data;
};

//======================================================================

#endif
