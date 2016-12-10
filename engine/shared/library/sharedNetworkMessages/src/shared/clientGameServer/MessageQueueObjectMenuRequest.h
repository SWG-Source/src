
// ======================================================================
//
// MessageQueueObjectMenuRequest.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageQueueObjectMenuRequest_H
#define INCLUDED_MessageQueueObjectMenuRequest_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/ObjectMenuRequestDataArchive.h"
#include "Unicode.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
//-----------------------------------------------------------------------

class GameInByteStream;
class GameOutByteStream;

/**
* This message is used in both directions for client-server communication regarding
* scripted radial menus for objects.
*
*/

class MessageQueueObjectMenuRequest : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	
	           MessageQueueObjectMenuRequest (const NetworkId & id, const NetworkId & requestorId, const std::vector<ObjectMenuRequestData> & data, uint8 sequence);
	virtual   ~MessageQueueObjectMenuRequest ();

	const std::vector<ObjectMenuRequestData> &  getData         () const;
	const NetworkId &                              getRequestorId  () const;
	const NetworkId &                              getTargetId     () const;

	uint8                                  m_sequence;

private:

	std::vector<ObjectMenuRequestData>  m_data;
	NetworkId                              m_requestorId;
	NetworkId                              m_targetId;

private:
	MessageQueueObjectMenuRequest(const MessageQueueObjectMenuRequest&);
	MessageQueueObjectMenuRequest& operator= (const MessageQueueObjectMenuRequest&);
};

//----------------------------------------------------------------------

inline const std::vector<ObjectMenuRequestData> &  MessageQueueObjectMenuRequest::getData () const
{
	return m_data;
}

//-----------------------------------------------------------------------

inline const NetworkId & MessageQueueObjectMenuRequest::getTargetId () const
{
	return m_targetId;
}

//-----------------------------------------------------------------------

inline const NetworkId & MessageQueueObjectMenuRequest::getRequestorId() const
{
	return m_requestorId;
}

//-----------------------------------------------------------------

#endif

