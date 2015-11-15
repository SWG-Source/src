//======================================================================
//
// EnvironmentalHitData.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_EnvironmentalHitData_H
#define INCLUDED_EnvironmentalHitData_H

//======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;
}

//----------------------------------------------------------------------

class EnvironmentalHitData
{
public:

	EnvironmentalHitData();
	EnvironmentalHitData(int _nebulaId, int8 _side, float _damage);

	int nebulaId;
	int8 side;
	float damage;

	void get(Archive::ReadIterator & source);
	void put(Archive::ByteStream & target) const;
};

//----------------------------------------------------------------------

namespace Archive
{
	inline void get(ReadIterator & source, EnvironmentalHitData & target)
	{
		target.get(source);
	}

	inline void put(ByteStream & target, EnvironmentalHitData const& source)
	{
		source.put(target);
	}
}

//======================================================================

#endif
