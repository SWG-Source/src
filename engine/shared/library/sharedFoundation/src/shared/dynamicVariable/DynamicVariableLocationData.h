//========================================================================
//
// DynamicVariableLocationData.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_DynamicVariableLocationData_H
#define _INCLUDED_DynamicVariableLocationData_H

#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"


class DynamicVariableLocationData
{
public:
	Vector pos;
	std::string scene;
	NetworkId cell;

	DynamicVariableLocationData();
	DynamicVariableLocationData(const Vector & _pos, const std::string & _scene, const NetworkId & _cell);
};


inline DynamicVariableLocationData::DynamicVariableLocationData() :
	pos(),
	scene(),
	cell()
{
}

inline DynamicVariableLocationData::DynamicVariableLocationData(const Vector & _pos, 
	const std::string & _scene, const NetworkId & _cell) :
	pos(_pos),
	scene(_scene),
	cell(_cell)
{
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, DynamicVariableLocationData & target);
	void put(ByteStream & target, const DynamicVariableLocationData & source);
}

//========================================================================

#endif	// _INCLUDED_DynamicVariableLocationData_H
