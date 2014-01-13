//======================================================================
//
// MessageQueueMissionCreateRequestDataArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueMissionCreateRequestDataArchive_H
#define INCLUDED_MessageQueueMissionCreateRequestDataArchive_H

//======================================================================

#include "Archive/ByteStream.h"

class MessageQueueMissionCreateRequestData;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, MessageQueueMissionCreateRequestData & target);
	void put (ByteStream & target, const MessageQueueMissionCreateRequestData & source);
}

//======================================================================

#endif
