// ======================================================================
//
// LocationData.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LocationData_H
#define INCLUDED_LocationData_H

// ======================================================================

#include "Archive/AutoByteStream.h"
#include "sharedMath/Sphere.h"
#include "Unicode.h"
//#include "sharedFoundation/NetworkId.h"
//#include "sharedMath/Vector.h"

// ======================================================================

/* namespace DB */
/* { */
/* 	class BindableDouble; */
/* 	template <int S> */
/* 	class BindableString; */
/* } */

// ======================================================================

class LocationData
{
  public:
	Sphere       location;
	Unicode::String  name;
	std::string  scene;
	
	inline bool operator==(const LocationData & rhs) const
	{
		return this == &rhs || (scene == rhs.scene && location == rhs.location && name == rhs.name);
	}
};

namespace Archive
{
	void get(ReadIterator & source, LocationData & target);
	void put(ByteStream & target, const LocationData & source);
}

// ======================================================================

#endif
