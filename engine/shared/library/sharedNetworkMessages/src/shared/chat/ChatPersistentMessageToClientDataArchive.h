//======================================================================
//
// ChatPersistentMessageToClientDataArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ChatPersistentMessageToClientDataArchive_H
#define INCLUDED_ChatPersistentMessageToClientDataArchive_H

//======================================================================

#include "Archive/ByteStream.h"

struct ChatPersistentMessageToClientData;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, ChatPersistentMessageToClientData & target);
	void put (ByteStream & target, const ChatPersistentMessageToClientData & source);
}

//======================================================================

#endif
