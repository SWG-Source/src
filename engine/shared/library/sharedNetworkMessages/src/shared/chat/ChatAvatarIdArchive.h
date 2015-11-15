//======================================================================
//
// ChatAvatarIdArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ChatAvatarIdArchive_H
#define INCLUDED_ChatAvatarIdArchive_H

//======================================================================

#include "Archive/ByteStream.h"

struct ChatAvatarId;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, ChatAvatarId & target);
	void put (ByteStream & target, const ChatAvatarId & source);
}

//======================================================================

#endif
