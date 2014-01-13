//======================================================================
//
// MessageQueueShowCombatTextArchive.cpp
// Copyright (c) 2005 Sony Online Entertainment, Inc.
// All rights reserved.
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueShowCombatTextArchive.h"

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/MessageQueueShowCombatText.h"

//======================================================================

MessageQueue::Data *MessageQueueShowCombatTextArchive::get(Archive::ReadIterator & source)
{
	NetworkId defenderId;
	NetworkId attackerId;
	StringId  outputTextId;
	Unicode::String outputTextOOB;
	float     scale;
	uint8     red;
	uint8     green;
	uint8     blue;
	uint8     flags;

	Archive::get(source, defenderId);
	Archive::get(source, attackerId);
	Archive::get(source, outputTextId);
	Archive::get(source, outputTextOOB);
	Archive::get(source, scale);
	Archive::get(source, red);
	Archive::get(source, green);
	Archive::get(source, blue);
	Archive::get(source, flags);

	if(outputTextId.isValid())
		return new MessageQueueShowCombatText(defenderId, attackerId, outputTextId, scale, static_cast<int>(red), static_cast<int>(green), static_cast<int>(blue), static_cast<int>(flags));
	else
		return new MessageQueueShowCombatText(defenderId, attackerId, outputTextOOB, scale, static_cast<int>(red), static_cast<int>(green), static_cast<int>(blue), static_cast<int>(flags));
}

//----------------------------------------------------------------------

void MessageQueueShowCombatTextArchive::put(const MessageQueue::Data *source, Archive::ByteStream &target)
{
	const MessageQueueShowCombatText * const message = dynamic_cast<const MessageQueueShowCombatText*>(source);
	if (!message)
	{
		WARNING_STRICT_FATAL(true, ("attempted to pack ShowCombatText message queue message with wrong data type."));
		return;
	}

	Archive::put(target, message->getDefenderId());
	Archive::put(target, message->getAttackerId());
	Archive::put(target, message->getOutputTextId());
	Archive::put(target, message->getOutputTextOOB());
	Archive::put(target, message->getScale());
	Archive::put(target, static_cast<uint8>(message->getRed   ()));
	Archive::put(target, static_cast<uint8>(message->getGreen ()));
	Archive::put(target, static_cast<uint8>(message->getBlue  ()));
	Archive::put(target, static_cast<uint8>(message->getFlags ()));
}

//======================================================================
