// ======================================================================
//
// LocationData.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/LocationData.h"

#include "sharedMathArchive/VectorArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, LocationData & target)
	{
		get(source, target.scene);
		Vector center;
		float radius;
		get(source, center);
		get(source, radius);
		target.location.setCenter(center);
		target.location.setRadius(radius);
		get(source, target.name);
	}

	void put(ByteStream & target, const LocationData & source)
	{
		put(target, source.scene);
		put(target, source.location.getCenter());
		put(target, source.location.getRadius());
		put(target, source.name);
	}
}

// ======================================================================
