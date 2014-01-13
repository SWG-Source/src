//======================================================================
//
// NebulaLightningData.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_NebulaLightningData_H
#define INCLUDED_NebulaLightningData_H

//======================================================================

#include "sharedMath/Vector.h"

namespace Archive
{
	class ReadIterator;
	class ByteStream;
}

//----------------------------------------------------------------------

class NebulaLightningData
{
public:

	NebulaLightningData();

	uint16 lightningId;
	int nebulaId;
	uint32 syncStampStart;
	uint32 syncStampEnd;
	Vector endpoint0;
	Vector endpoint1;

	void get(Archive::ReadIterator & source);
	void put(Archive::ByteStream & target) const;
};

//----------------------------------------------------------------------

namespace Archive
{
	inline void get(ReadIterator & source, NebulaLightningData & target)
	{
		target.get(source);
	}

	inline void put(ByteStream & target, NebulaLightningData const& source)
	{
		source.put(target);
	}
}

//======================================================================

#endif
