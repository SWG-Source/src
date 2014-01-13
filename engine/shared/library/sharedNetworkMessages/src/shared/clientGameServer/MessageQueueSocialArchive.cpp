//======================================================================
//
// MessageQueueSocialArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueSocialArchive.h"

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/MessageQueueSocial.h"


//======================================================================

MessageQueue::Data * MessageQueueSocialArchive::get (Archive::ReadIterator & source)
{
	NetworkId sender;
	NetworkId target;
	uint32    socialType = 0;
	uint8     flags = 0;

	Archive::get(source, sender);
	Archive::get(source, target);
	Archive::get(source, socialType);
	Archive::get(source, flags);

	return new MessageQueueSocial (sender, target, socialType, (flags & MessageQueueSocial::F_animationOk) != 0, (flags & MessageQueueSocial::F_textOk) != 0);
}

//----------------------------------------------------------------------

void MessageQueueSocialArchive::put (const MessageQueue::Data * source, Archive::ByteStream & target)
{
	const MessageQueueSocial * const msg = dynamic_cast<const MessageQueueSocial  *>(source);
	NOT_NULL (msg);

	if (msg)
	{
		Archive::put (target, msg->getSourceId());
		Archive::put (target, msg->getTargetId());
		Archive::put (target, msg->getSocialType());
		Archive::put (target, msg->m_flags);
	}
}

//======================================================================
