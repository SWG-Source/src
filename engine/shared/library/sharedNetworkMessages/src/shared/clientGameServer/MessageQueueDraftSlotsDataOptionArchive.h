//======================================================================
//
// MessageQueueDraftSlotsDataOptionArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueDraftSlotsDataOptionArchive_H
#define INCLUDED_MessageQueueDraftSlotsDataOptionArchive_H

//======================================================================

class MessageQueueDraftSlotsDataOption;

namespace Archive
{
	class ByteStream;
	class ReadIterator;

	void get (ReadIterator & source, MessageQueueDraftSlotsDataOption & target);
	void put (ByteStream & target, const MessageQueueDraftSlotsDataOption & source);
}

//======================================================================

#endif
