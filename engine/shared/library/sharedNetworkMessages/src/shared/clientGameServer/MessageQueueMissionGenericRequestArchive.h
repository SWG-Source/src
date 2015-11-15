//======================================================================
//
// MessageQueueMissionGenericRequestArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueMissionGenericRequestArchive_H
#define INCLUDED_MessageQueueMissionGenericRequestArchive_H

//======================================================================

#include "Archive/ByteStream.h"
class MessageQueueMissionGenericRequest;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, MessageQueueMissionGenericRequest & target);
	void put (ByteStream & target, const MessageQueueMissionGenericRequest & source);
}

//======================================================================

#endif
