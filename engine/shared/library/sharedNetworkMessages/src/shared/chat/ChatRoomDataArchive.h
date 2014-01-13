//======================================================================
//
// ChatRoomDataArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ChatRoomDataArchive_H
#define INCLUDED_ChatRoomDataArchive_H

//======================================================================

#include "Archive/ByteStream.h"

struct ChatRoomData;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, ChatRoomData & target);
	void put (ByteStream & target, const ChatRoomData & source);
}

//======================================================================

#endif
