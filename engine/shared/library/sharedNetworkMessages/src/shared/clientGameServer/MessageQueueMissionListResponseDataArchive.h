//======================================================================
//
// MessageQueueMissionListResponseDataArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueMissionListResponseDataArchive_H
#define INCLUDED_MessageQueueMissionListResponseDataArchive_H

//======================================================================

class MessageQueueMissionListResponseData;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, MessageQueueMissionListResponseData & target);
	void put (ByteStream & target, const MessageQueueMissionListResponseData & source);
}

//======================================================================

#endif
