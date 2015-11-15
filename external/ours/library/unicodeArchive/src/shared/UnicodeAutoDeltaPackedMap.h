// ======================================================================
//
// UnicodeAutoDeltaPackedMap.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UnicodeAutoDeltaPackedMap_H
#define INCLUDED_UnicodeAutoDeltaPackedMap_H

// ======================================================================

#include "Archive/AutoDeltaPackedMap.h"
#include "Unicode.h"

// ======================================================================

namespace Archive
{
	template <> void AutoDeltaPackedMap<int, Unicode::String>::pack(ByteStream & target, const std::string & buffer);
	template <> void AutoDeltaPackedMap<int, Unicode::String>::unpack(ReadIterator & source, std::string & buffer);
}

// ======================================================================

#endif
