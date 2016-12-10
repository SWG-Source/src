// MessageQueueMissionListResponse.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_MessageQueueMissionListResponse_H
#define	_INCLUDED_MessageQueueMissionListResponse_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponseData.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"
#include <vector>

class MemoryBlockManager;

//-----------------------------------------------------------------------

class MessageQueueMissionListResponse : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	typedef MessageQueueMissionListResponseData DataElement;
	typedef std::vector<DataElement>         DataVector;

public:
	MessageQueueMissionListResponse();
	MessageQueueMissionListResponse(const DataVector & response, const uint8 sequenceId, const bool isBountyTerminal);
	~MessageQueueMissionListResponse();


	const bool           getBountyTerminal  () const;
	const DataVector &   getResponse        () const;
	const uint8          getSequenceId      () const;

	void                 set(const DataVector & response, const uint8 sequenceId, const bool isBountyTerminal);

private:
	MessageQueueMissionListResponse & operator = (const MessageQueueMissionListResponse & rhs);
	MessageQueueMissionListResponse(const MessageQueueMissionListResponse & source);

	DataVector   m_response;
	uint8        m_sequenceId;
	bool         m_bountyTerminal;
};

//-----------------------------------------------------------------------

inline const bool MessageQueueMissionListResponse::getBountyTerminal() const
{
	return m_bountyTerminal;
}

//-----------------------------------------------------------------------

inline const MessageQueueMissionListResponse::DataVector & MessageQueueMissionListResponse::getResponse() const
{
	return m_response;
}

//-----------------------------------------------------------------------

inline const uint8 MessageQueueMissionListResponse::getSequenceId() const
{
	return m_sequenceId;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MessageQueueMissionListResponse_H
