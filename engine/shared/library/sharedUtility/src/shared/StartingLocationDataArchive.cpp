//======================================================================
//
// StartingLocationDataArchive.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/StartingLocationDataArchive.h"

#include "Archive/Archive.h"
#include "sharedUtility/StartingLocationData.h"
#include "unicodeArchive/UnicodeArchive.h"

//======================================================================

namespace Archive
{
	//----------------------------------------------------------------------

	void get (Archive::ReadIterator & source, StartingLocationData & target)
	{
		get (source, target.name);
		get (source, target.planet);
		get (source, target.x);
		get (source, target.z);
		get (source, target.cellId);
		get (source, target.image);
		get (source, target.descriptionId);
	}

	//----------------------------------------------------------------------

	void put (ByteStream & target, const StartingLocationData & source)
	{
		put (target, source.name);
		put (target, source.planet);
		put (target, source.x);
		put (target, source.z);		
		put (target, source.cellId);
		put (target, source.image);
		put (target, source.descriptionId);
	}
}

//======================================================================
