//======================================================================
//
// NebulaLightningHitData.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_NebulaLightningHitData_H
#define INCLUDED_NebulaLightningHitData_H

//======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;
}

//----------------------------------------------------------------------

class NebulaLightningHitData
{
public:

	NebulaLightningHitData();
	NebulaLightningHitData(uint16 _lightningId, int8 _side, float _damage);

	uint16 lightningId;
	int8 side;
	float damage;

	void get(Archive::ReadIterator & source);
	void put(Archive::ByteStream & target) const;
};

//----------------------------------------------------------------------

namespace Archive
{
	inline void get(ReadIterator & source, NebulaLightningHitData & target)
	{
		target.get(source);
	}

	inline void put(ByteStream & target, NebulaLightningHitData const& source)
	{
		source.put(target);
	}
}

//======================================================================

#endif
