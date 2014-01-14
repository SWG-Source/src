// SlotIdArchive.h
// copyright 2001 Verant Interactive

#ifndef	_INCLUDED_SlotIdArchive_H
#define	_INCLUDED_SlotIdArchive_H

//-----------------------------------------------------------------------

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "sharedObject/SlotId.h"

//-----------------------------------------------------------------------

namespace Archive
{

//-----------------------------------------------------------------------

	inline void get(ReadIterator & source, SlotId & target)
	{
		int id;
		get(source, id);
		target = SlotId(id);
	}

//-----------------------------------------------------------------------

	inline void put(ByteStream & target, const SlotId & source)
	{
		int tmp = source.getSlotId();
		put(target, tmp);
	}

//-----------------------------------------------------------------------


}// namespace Archive
//-----------------------------------------------------------------------

#endif	
