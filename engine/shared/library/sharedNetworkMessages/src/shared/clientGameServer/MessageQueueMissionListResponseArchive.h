//======================================================================
//
// MessageQueueMissionListResponseArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueMissionListResponseArchive_H
#define INCLUDED_MessageQueueMissionListResponseArchive_H

//======================================================================

class MessageQueueMissionListResponse;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, MessageQueueMissionListResponse & target);
	void put (ByteStream & target, const MessageQueueMissionListResponse & source);
}

//======================================================================

#endif
