//======================================================================
//
// MessageQueueMissionGenericResponseArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueMissionGenericResponseArchive_H
#define INCLUDED_MessageQueueMissionGenericResponseArchive_H

//======================================================================

#include "Archive/ByteStream.h"
class MessageQueueMissionGenericResponse;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, MessageQueueMissionGenericResponse & target);
	void put (ByteStream & target, const MessageQueueMissionGenericResponse & source);
}

//======================================================================

#endif
