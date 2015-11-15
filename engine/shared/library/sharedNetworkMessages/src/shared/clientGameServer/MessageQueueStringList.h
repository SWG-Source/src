//========================================================================
//
// MessageQueueStringList.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueStringList_H
#define INCLUDED_MessageQueueStringList_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//========================================================================

class MessageQueueStringList : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueStringList(void);
	virtual ~MessageQueueStringList();

	void                    addString(const Unicode::String & response);
	int                     getStringCount(void) const;
	const Unicode::String & getString(int index) const;

private:
	std::vector<Unicode::String> m_strings;
};


inline MessageQueueStringList::MessageQueueStringList()
{
}	// MessageQueueStringList::MessageQueueStringList()

inline void MessageQueueStringList::addString(const Unicode::String & string)
{
	m_strings.push_back(string);
}	// MessageQueueStringList::addString

inline int MessageQueueStringList::getStringCount(void) const
{
	return m_strings.size();
}	// MessageQueueStringList::getStringCount

inline const Unicode::String & MessageQueueStringList::getString(int index) const
{
	DEBUG_FATAL(index < 0 || index >= static_cast<int>(m_strings.size()), 
		("MessageQueueStringList::getString index out of range"));
	return m_strings.at(index);
}	// MessageQueueStringList::getString


#endif	// INCLUDED_MessageQueueStringList_H
