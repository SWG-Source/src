//======================================================================
//
// MessageQueueShowFlyTextArchive.cpp
// Copyright (c) 2002 Sony Online Entertainment, Inc.
// All rights reserved.
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueShowFlyTextArchive.h"

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/MessageQueueShowFlyText.h"

//======================================================================

MessageQueue::Data *MessageQueueShowFlyTextArchive::get(Archive::ReadIterator & source)
{
	NetworkId emitterId;
	StringId  outputTextId;
	Unicode::String outputTextOOB;
	float     scale;
	uint8     red;
	uint8     green;
	uint8     blue;
	uint32     flags;

	Archive::get(source, emitterId);
	Archive::get(source, outputTextId);
	Archive::get(source, outputTextOOB);
	Archive::get(source, scale);
	Archive::get(source, red);
	Archive::get(source, green);
	Archive::get(source, blue);
	Archive::get(source, flags);

	if(outputTextId.isValid())
		return new MessageQueueShowFlyText(emitterId, outputTextId, scale, static_cast<int>(red), static_cast<int>(green), static_cast<int>(blue), static_cast<int>(flags));
	else
		return new MessageQueueShowFlyText(emitterId, outputTextOOB, scale, static_cast<int>(red), static_cast<int>(green), static_cast<int>(blue), static_cast<int>(flags));
}

//----------------------------------------------------------------------

void MessageQueueShowFlyTextArchive::put(const MessageQueue::Data *source, Archive::ByteStream &target)
{
	const MessageQueueShowFlyText * const message = dynamic_cast<const MessageQueueShowFlyText*>(source);
	if (!message)
	{
		WARNING_STRICT_FATAL(true, ("attempted to pack ShowFlyText message queue message with wrong data type."));
		return;
	}

	Archive::put(target, message->getEmitterId());
	Archive::put(target, message->getOutputTextId());
	Archive::put(target, message->getOutputTextOOB());
	Archive::put(target, message->getScale());
	Archive::put(target, static_cast<uint8>(message->getRed   ()));
	Archive::put(target, static_cast<uint8>(message->getGreen ()));
	Archive::put(target, static_cast<uint8>(message->getBlue  ()));
	Archive::put(target, static_cast<uint32>(message->getFlags ()));
}

//======================================================================
