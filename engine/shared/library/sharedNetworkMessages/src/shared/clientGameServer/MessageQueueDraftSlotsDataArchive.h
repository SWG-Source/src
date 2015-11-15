//======================================================================
//
// MessageQueueDraftSlotsDataArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueDraftSlotsDataArchive_H
#define INCLUDED_MessageQueueDraftSlotsDataArchive_H

//======================================================================

class MessageQueueDraftSlotsData;

namespace Archive
{
	class ByteStream;
	class ReadIterator;

	void get (ReadIterator & source, MessageQueueDraftSlotsData & target);
	void put (ByteStream & target, const MessageQueueDraftSlotsData & source);
}

//======================================================================

#endif
