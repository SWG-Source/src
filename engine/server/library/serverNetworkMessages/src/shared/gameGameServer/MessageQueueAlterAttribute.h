//======================================================================
//
// MessageQueueAlterAttribute.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueAlterAttribute_H
#define INCLUDED_MessageQueueAlterAttribute_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

//-----------------------------------------------------------------------

class MessageQueueAlterAttribute : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueAlterAttribute(int attrib, int delta, bool checkIncapacitation, const NetworkId & source);
	virtual ~MessageQueueAlterAttribute();

	int               getAttrib() const;
	int               getDelta() const;
	bool              getCheckIncapacitation() const;
	const NetworkId & getSource() const;
	

private:

	MessageQueueAlterAttribute              (const MessageQueueAlterAttribute & source);
	MessageQueueAlterAttribute & operator = (const MessageQueueAlterAttribute & source);

private:
	int               m_attrib;
	int               m_delta;
	bool              m_checkIncapacitation;
	const NetworkId & m_source;
};


//-----------------------------------------------------------------------

inline int MessageQueueAlterAttribute::getAttrib() const
{
	return m_attrib;
}

inline int MessageQueueAlterAttribute::getDelta() const
{
	return m_delta;
}

inline bool MessageQueueAlterAttribute::getCheckIncapacitation() const
{
	return m_checkIncapacitation;
}

inline const NetworkId & MessageQueueAlterAttribute::getSource() const
{
	return m_source;
}


//-----------------------------------------------------------------------


#endif	// INCLUDED_MessageQueueAlterAttribute_H

