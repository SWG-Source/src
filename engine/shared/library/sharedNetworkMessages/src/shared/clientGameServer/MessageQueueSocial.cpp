//======================================================================
//
// MessageQueueSocial.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueSocial.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueSocialArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_WITH_ARCHIVE(MessageQueueSocial, CM_socialReceive);

//======================================================================

MessageQueueSocial::MessageQueueSocial (const NetworkId & sourceId, const NetworkId & targetId, uint32 socialType, bool animationOk, bool textOk) :
MessageQueue::Data (),
m_sourceId    (sourceId),
m_targetId    (targetId),
m_socialType  (socialType),
m_flags       (static_cast<uint8>((animationOk ? F_animationOk : 0) | (textOk ? F_textOk : 0)))
{

}

//----------------------------------------------------------------------

MessageQueueSocial::MessageQueueSocial (const MessageQueueSocial& rhs) :
MessageQueue::Data (),
m_sourceId    (rhs.getSourceId   ()),
m_targetId    (rhs.getTargetId   ()),
m_socialType  (rhs.getSocialType ()),
m_flags       (rhs.m_flags)
{
}

//======================================================================
