// MessageQueueMissionDetailsReqeust.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_MessageQueueMissionDetailsReqeust_H
#define	_INCLUDED_MessageQueueMissionDetailsReqeust_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

class MessageQueueMissionDetailsRequest : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueMissionDetailsRequest(const NetworkId & missionData, const NetworkId & terminalId, const unsigned char sequenceId);
	~MessageQueueMissionDetailsRequest();

	const NetworkId &    getMissionData  () const;
	const unsigned char  getSequenceId   () const;
	const NetworkId &    getTerminalId   () const;

private:
	MessageQueueMissionDetailsRequest & operator = (const MessageQueueMissionDetailsRequest & rhs);
	MessageQueueMissionDetailsRequest(const MessageQueueMissionDetailsRequest & source);

	NetworkId      missionData;
	NetworkId      terminalId;
	unsigned char  sequenceId;
};

//-----------------------------------------------------------------------

inline const NetworkId & MessageQueueMissionDetailsRequest::getMissionData() const
{
	return missionData;
}

//-----------------------------------------------------------------------

inline const unsigned char MessageQueueMissionDetailsRequest::getSequenceId() const
{
	return sequenceId;
}

//-----------------------------------------------------------------------

inline const NetworkId & MessageQueueMissionDetailsRequest::getTerminalId() const
{
	return terminalId;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MessageQueueMissionDetailsReqeust_H
