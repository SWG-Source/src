// MessageQueueMissionListRequest.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_MessageQueueMissionListRequest_H
#define	_INCLUDED_MessageQueueMissionListRequest_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

class MessageQueueMissionListRequest : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	enum Flags
	{
		F_mineOnly  = 0x0001,
	};

	MessageQueueMissionListRequest(const NetworkId & terminalId, const uint8 flags, const uint8 sequenceId);
	~MessageQueueMissionListRequest();

	const uint8        getFlags       () const;
	const uint8        getSequenceId  () const;
	const NetworkId &  getTerminalId  () const;

	bool               hasFlags       (Flags f) const;
	bool               hasAnyFlags    (Flags f) const;

private:
	MessageQueueMissionListRequest & operator = (const MessageQueueMissionListRequest & rhs);
	MessageQueueMissionListRequest(const MessageQueueMissionListRequest & source);

	uint8       m_flags;
	uint8       m_sequenceId;
	NetworkId   m_terminalId;
};

//-----------------------------------------------------------------------

inline const uint8 MessageQueueMissionListRequest::getFlags() const
{
	return m_flags;
}

//-----------------------------------------------------------------------

inline const uint8 MessageQueueMissionListRequest::getSequenceId() const
{
	return m_sequenceId;
}

//-----------------------------------------------------------------------

inline const NetworkId & MessageQueueMissionListRequest::getTerminalId() const
{
	return m_terminalId;
}

//----------------------------------------------------------------------

inline bool MessageQueueMissionListRequest::hasFlags(Flags f) const
{
	return (m_flags & f) == f;
}

//----------------------------------------------------------------------

inline bool MessageQueueMissionListRequest::hasAnyFlags(Flags f) const
{
	return (m_flags & f) != 0;
}


//-----------------------------------------------------------------------

#endif	// _INCLUDED_MessageQueueMissionListRequest_H
