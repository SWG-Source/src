// ======================================================================
//
// ImageDesignStartMessage.cpp
//
// Copyright 2002-2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ImageDesignStartMessage.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

// ======================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(ImageDesignStartMessage, CM_imageDesignerStart);

//----------------------------------------------------------------------

ImageDesignStartMessage::ImageDesignStartMessage(NetworkId const & designerId, NetworkId const & recipientId, NetworkId const & terminalId, std::string const & currentHoloEmote) :
MessageQueue::Data(),
m_designerId(designerId),
m_recipientId(recipientId),
m_terminalId(terminalId),
m_currentHoloEmote(currentHoloEmote)
{
}

//----------------------------------------------------------------------

void ImageDesignStartMessage::pack(const MessageQueue::Data * const data, Archive::ByteStream & target)
{
	ImageDesignStartMessage const * const message = safe_cast<ImageDesignStartMessage const *> (data);
	if(message)
	{
		Archive::put(target, message->getDesignerId());
		Archive::put(target, message->getRecipientId());
		Archive::put(target, message->getTerminalId());
		Archive::put(target, message->getCurrentHoloEmote());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data * ImageDesignStartMessage::unpack(Archive::ReadIterator & source)
{
	NetworkId designerId;
	NetworkId recipientId;
	NetworkId terminalId;
	std::string currentHoloEmote;
	
	Archive::get(source, designerId);
	Archive::get(source, recipientId);
	Archive::get(source, terminalId);
	Archive::get(source, currentHoloEmote);
	
	return new ImageDesignStartMessage(designerId, recipientId, terminalId, currentHoloEmote);
}

//----------------------------------------------------------------------
