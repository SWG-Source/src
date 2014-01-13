//======================================================================
//
// MessageQueueSpatialChat.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueSpatialChatArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_WITH_ARCHIVE_NO_INSTALL(MessageQueueSpatialChat);

//===================================================================

void MessageQueueSpatialChat::install() 
{ 
	installMemoryBlockManager();
	ControllerMessageFactory::registerControllerMessageHandler(CM_spatialChatSend, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_spatialChatReceive, pack, unpack); 
	ExitChain::add(&remove, "MessageQueueSpatialChat::remove"); 
} 

//-----------------------------------------------------------------------

MessageQueueSpatialChat::MessageQueueSpatialChat (const NetworkId & sourceId,
						 const NetworkId & targetId,
						 const Unicode::String & text,
						 uint16 volume,
						 uint16 chatType,
						 uint16 moodType,
						 uint32 flags,
						 uint32 language,
						 const Unicode::String & oob,
						 const Unicode::String & sourceName) :
MessageQueue::Data (),
m_text         (text),
m_sourceId     (sourceId),
m_targetId     (targetId),
m_flags        (flags),
m_volume       (volume),
m_chatType     (chatType),
m_moodType     (moodType),
m_language     (static_cast<uint8>(language)),
m_outOfBand    (oob),
m_sourceName   (sourceName)
{
	DEBUG_FATAL (language > 255, ("MessageQueueSpatialChat language %d out of 8 bit range.", language));
}

//----------------------------------------------------------------------

MessageQueueSpatialChat::MessageQueueSpatialChat (const MessageQueueSpatialChat& rhs) :
MessageQueue::Data (),
m_text         (rhs.getText ()),
m_sourceId     (rhs.getSourceId ()),
m_targetId     (rhs.getTargetId ()),
m_flags        (rhs.getFlags ()),
m_volume       (rhs.getVolume ()),
m_chatType     (rhs.getChatType ()),
m_moodType     (rhs.getMoodType ()),
m_language     (rhs.m_language),
m_outOfBand    (rhs.m_outOfBand),
m_sourceName   (rhs.m_sourceName)
{
}

//----------------------------------------------------------------------

MessageQueueSpatialChat::~MessageQueueSpatialChat ()
{
}

//======================================================================
