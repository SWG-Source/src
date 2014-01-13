//======================================================================
//
// MessageQueueMissionDetailsResponseDataArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueMissionDetailsResponseDataArchive_H
#define INCLUDED_MessageQueueMissionDetailsResponseDataArchive_H

//======================================================================

#include "Archive/ByteStream.h"

struct MessageQueueMissionDetailsResponseData;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, MessageQueueMissionDetailsResponseData & target);
	void put (ByteStream & target, const MessageQueueMissionDetailsResponseData & source);
}

//======================================================================

#endif
