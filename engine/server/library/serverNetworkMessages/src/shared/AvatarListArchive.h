// ======================================================================
//
// AvatarList.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AvatarListArchive_H
#define INCLUDED_AvatarListArchive_H

// ======================================================================

#include "serverNetworkMessages/AvatarList.h"
#include "sharedFoundation/NetworkId.h"
namespace Archive
{
	class ByteStream;
	class ReadIterator;
}

// ======================================================================

namespace Archive
{
	void get (ReadIterator & source, AvatarRecord & target);
	void put (ByteStream & target, const AvatarRecord & source);
}

// ======================================================================

#endif
