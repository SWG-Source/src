//======================================================================
//
// MessageQueueSpatialChatArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueSpatialChatArchive.h"

#include "Archive/ByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/ProsePackageArchive.h"

//======================================================================

MessageQueue::Data * MessageQueueSpatialChatArchive::get (Archive::ReadIterator & source)
{
	NetworkId sourceId;
	NetworkId targetId;
	Unicode::String text;
	uint32          flags    = 0;
	uint16          volume   = 0;
	uint16          chatType = 0;
	uint16          moodType = 0;
	uint8           language = 0;
	Unicode::String oob;
	Unicode::String sourceName;

	Archive::get(source, sourceId);
	Archive::get(source, targetId);
	Archive::get(source, text);
	Archive::get(source, flags);
	Archive::get(source, volume);
	Archive::get(source, chatType);
	Archive::get(source, moodType);
	Archive::get(source, language);
	Archive::get(source, oob);
	Archive::get(source, sourceName);

	MessageQueueSpatialChat * const msg = new MessageQueueSpatialChat (sourceId, targetId, text, volume, chatType, moodType, flags, language, oob, sourceName);
	return msg;
}

//----------------------------------------------------------------------

void MessageQueueSpatialChatArchive::put (const MessageQueue::Data * source, Archive::ByteStream & target)
{
	const MessageQueueSpatialChat * const message = dynamic_cast<const MessageQueueSpatialChat*>(source);
	if (!message)
	{
		WARNING_STRICT_FATAL(true, ("attempted to pack SetTargetAndAttack message queue message with wrong data type"));
		return;
	}

	const NetworkId &       sourceId = message->getSourceId  ();
	const NetworkId &       targetId = message->getTargetId  ();
	const Unicode::String & text     = message->getText      ();
	const uint32            flags    = message->getFlags     ();
	const uint16            volume   = message->getVolume    ();
	const uint16            chatType = message->getChatType  ();
	const uint16            moodtype = message->getMoodType  ();
	const uint8             language = static_cast<uint8>(message->getLanguage ()); //language is only 8 bit on the network
	const Unicode::String & oob      = message->getOutOfBand ();
	const Unicode::String & sourceName = message->getSourceName ();

	Archive::put(target, sourceId);
	Archive::put(target, targetId);
	Archive::put(target, text);
	Archive::put(target, flags);
	Archive::put(target, volume);
	Archive::put(target, chatType);
	Archive::put(target, moodtype);
	Archive::put(target, language);
	Archive::put(target, oob);
	Archive::put(target, sourceName);
}

//======================================================================
