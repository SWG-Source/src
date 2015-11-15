// ======================================================================
//
// MissionLocation.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/MissionLocation.h"

#include "localizationArchive/StringIdArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

namespace Archive
{

	void get(ReadIterator & source, MissionLocation & target)
	{
		get(source, target.coordinates);
		get(source, target.planetName);
		get(source, target.regionName);
		get(source, target.cell);
	}

	void put(ByteStream & target, const MissionLocation & source)
	{
		put(target, source.coordinates);
		put(target, source.planetName);
		put(target, source.regionName);
		put(target, source.cell);
	}
}

// ======================================================================
