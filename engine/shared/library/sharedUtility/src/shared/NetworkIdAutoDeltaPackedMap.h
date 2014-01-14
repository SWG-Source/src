// ======================================================================
//
// NetworkIdAutoDeltaPackedMap.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NetworkIdAutoDeltaPackedMap_H
#define INCLUDED_NetworkIdAutoDeltaPackedMap_H

// ======================================================================

#include "Archive/AutoDeltaPackedMap.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

namespace Archive
{
	template <> void AutoDeltaPackedMap<int, NetworkId>::pack(ByteStream & target, const std::string & buffer);
	template <> void AutoDeltaPackedMap<int, NetworkId>::unpack(ReadIterator & source, std::string & buffer);
}

// ======================================================================

#endif
