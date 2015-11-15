// CachedNetworkIdArchive.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	INCLUDED_CachedNetworkIdArchive_H
#define	INCLUDED_CachedNetworkIdArchive_H

//-----------------------------------------------------------------------

#include "Archive/ByteStream.h"
#include "sharedObject/CachedNetworkId.h"

//-----------------------------------------------------------------------

namespace Archive
{
	inline void get(ReadIterator & source, CachedNetworkId & target)
	{
		uint64 id;
		source.get(&id,8);
		target = CachedNetworkId(id);
	}

	inline void put(ByteStream & target, const CachedNetworkId & source)
	{
		uint64 tmp = source.getValue();
		target.put(&tmp,8);
	}
}

//-----------------------------------------------------------------------

#endif
