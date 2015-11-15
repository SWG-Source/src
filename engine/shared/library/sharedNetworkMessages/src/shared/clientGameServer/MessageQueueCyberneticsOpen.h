//========================================================================
//
// MessageQueueCyberneticsOpen.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueCyberneticsOpen_H
#define INCLUDED_MessageQueueCyberneticsOpen_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

//===================================================================

class MessageQueueCyberneticsOpen : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	enum OpenType
	{
		OT_INSTALL,
		OT_UNINSTALL,
		OT_REPAIR,
		OT_VIEW
	};

public:

	         MessageQueueCyberneticsOpen(OpenType openType, NetworkId const & npc);
	virtual ~MessageQueueCyberneticsOpen();

	OpenType getOpenType() const;
	NetworkId const & getNPC() const;

private:
	OpenType m_openType;
	NetworkId m_npc;

private:
	MessageQueueCyberneticsOpen (MessageQueueCyberneticsOpen const &);
	MessageQueueCyberneticsOpen & operator= (MessageQueueCyberneticsOpen const &);
};

//===================================================================

inline MessageQueueCyberneticsOpen::MessageQueueCyberneticsOpen(MessageQueueCyberneticsOpen::OpenType const openType, NetworkId const & npc) :
m_openType(openType),
m_npc(npc)
{
}	// MessageQueueCyberneticsOpen::MessageQueueCyberneticsOpen

//----------------------------------------------------------------------

inline MessageQueueCyberneticsOpen::OpenType MessageQueueCyberneticsOpen::getOpenType() const
{
	return m_openType;
}

//----------------------------------------------------------------------

inline NetworkId const & MessageQueueCyberneticsOpen::getNPC() const
{
	return m_npc;
}

//===================================================================

#endif	// INCLUDED_MessageQueueCyberneticsOpen_H
