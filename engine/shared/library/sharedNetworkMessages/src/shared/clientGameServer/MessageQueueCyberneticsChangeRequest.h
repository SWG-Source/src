//========================================================================
//
// MessageQueueCyberneticsChangeRequest.h
//
// copyright 2005 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#ifndef INCLUDED_MessageQueueCyberneticsChangeRequest_H
#define INCLUDED_MessageQueueCyberneticsChangeRequest_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

//========================================================================

class MessageQueueCyberneticsChangeRequest : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	enum ChangeType
	{
		CT_INSTALL,
		CT_UNINSTALL,
		CT_REPAIR
	};

public:
	MessageQueueCyberneticsChangeRequest(ChangeType changeType, const NetworkId & cyberneticPiece, const NetworkId & npc);
	virtual ~MessageQueueCyberneticsChangeRequest();

	ChangeType getChangeType() const;
	NetworkId const & getCyberneticPiece() const;
	//this will be the npc id when coming from the client, then will be the player Id when forwarded to the auth server for the npc
	NetworkId const & getTarget() const;

private:
	ChangeType m_changeType;
	NetworkId  m_cyberneticPiece;
	NetworkId  m_target;

private:
	MessageQueueCyberneticsChangeRequest (const MessageQueueCyberneticsChangeRequest &);
	MessageQueueCyberneticsChangeRequest & operator= (const MessageQueueCyberneticsChangeRequest &);
};

//========================================================================

inline MessageQueueCyberneticsChangeRequest::MessageQueueCyberneticsChangeRequest(MessageQueueCyberneticsChangeRequest::ChangeType const changeType, NetworkId const & cyberneticPiece, const NetworkId & target) :
m_changeType(changeType),
m_cyberneticPiece(cyberneticPiece),
m_target(target)
{
}

//------------------------------------------------------------------------

inline MessageQueueCyberneticsChangeRequest::ChangeType  MessageQueueCyberneticsChangeRequest::getChangeType() const
{
	return m_changeType;
}

//------------------------------------------------------------------------

inline NetworkId const & MessageQueueCyberneticsChangeRequest::getCyberneticPiece() const
{
	return m_cyberneticPiece;
}

//------------------------------------------------------------------------

inline NetworkId const & MessageQueueCyberneticsChangeRequest::getTarget() const
{
	return m_target;
}

//========================================================================

#endif	// INCLUDED_MessageQueueCyberneticsChangeRequest_H
